#include "PluginEditor.h"

namespace t = lxr::theme;

namespace {
    // Parameter IDs - keep in sync with PluginProcessor.cpp
    const char* idPitch         = "pitch";
    const char* idWaveform      = "waveform";
    const char* idFmAmount      = "fm_amount";
    const char* idModPitch      = "mod_pitch";
    const char* idPitchEgAmt    = "pitch_eg_amount";
    const char* idPitchDecay    = "pitch_decay";
    const char* idVolAttack     = "vol_attack";
    const char* idVolDecay      = "vol_decay";
    const char* idVolSlope      = "vol_slope";
    const char* idTransientAmt  = "transient_amount";
    const char* idTransientWave = "transient_wave";
    const char* idSnap          = "snap";
    const char* idCutoff        = "cutoff";
    const char* idReso          = "reso";
    const char* idFilterType    = "filter_type";
    const char* idDistortion    = "distortion";
    const char* idVolume        = "volume";
    const char* idPan           = "pan";
}

LxrDrumEditor::LxrDrumEditor (LxrDrumProcessor& p)
    : juce::AudioProcessorEditor (p), proc (p)
{
    // IMPORTANT: only set the LAF on this editor instance. Don't touch
    // setDefaultLookAndFeel - that's process-global, points at our
    // member, and will dangle when this editor is destroyed, taking
    // down anything else in the host that uses the default LAF.
    setLookAndFeel (&laf);

    auto add = [this] (auto& uptr, auto* component) {
        uptr.reset (component);
        addAndMakeVisible (uptr.get());
    };

    add (kPitch,       new lxr::LabeledKnob (proc.apvts, idPitch,        "Pitch"));
    add (kModPitch,    new lxr::LabeledKnob (proc.apvts, idModPitch,     "Mod Pitch"));
    add (kFmAmount,    new lxr::LabeledKnob (proc.apvts, idFmAmount,     "FM Amount"));
    add (kPitchEgAmt,  new lxr::LabeledKnob (proc.apvts, idPitchEgAmt,   "Pitch EG"));
    add (kPitchDecay,  new lxr::LabeledKnob (proc.apvts, idPitchDecay,   "Pitch Dcy"));

    add (kAttack,      new lxr::LabeledKnob (proc.apvts, idVolAttack,    "Attack"));
    add (kDecay,       new lxr::LabeledKnob (proc.apvts, idVolDecay,     "Decay"));
    add (kSlope,       new lxr::LabeledKnob (proc.apvts, idVolSlope,     "Slope"));

    add (kTransAmt,    new lxr::LabeledKnob (proc.apvts, idTransientAmt, "Amount"));
    add (kTransWave,   new lxr::LabeledKnob (proc.apvts, idTransientWave,"Wave"));
    add (kSnap,        new lxr::LabeledKnob (proc.apvts, idSnap,         "Snap"));

    add (kCutoff,      new lxr::LabeledKnob (proc.apvts, idCutoff,       "Cutoff"));
    add (kReso,        new lxr::LabeledKnob (proc.apvts, idReso,         "Reso"));

    add (kDist,        new lxr::LabeledKnob (proc.apvts, idDistortion,   "Dist"));
    add (kVol,         new lxr::LabeledKnob (proc.apvts, idVolume,       "Volume"));
    add (kPan,         new lxr::LabeledKnob (proc.apvts, idPan,          "Pan"));

    waveformSeg.reset (new lxr::SegmentedChoice (proc.apvts, idWaveform,
        juce::StringArray { "Sin", "Saw", "Tri", "Rect", "Noise" }));
    addAndMakeVisible (waveformSeg.get());

    filterTypeSeg.reset (new lxr::SegmentedChoice (proc.apvts, idFilterType,
        juce::StringArray { "Off", "LP", "HP", "BP" }));
    addAndMakeVisible (filterTypeSeg.get());

    addAndMakeVisible (oscHeader);
    addAndMakeVisible (envHeader);
    addAndMakeVisible (transHeader);
    addAndMakeVisible (filterHeader);
    addAndMakeVisible (outputHeader);

    setSize (800, 560);
}

LxrDrumEditor::~LxrDrumEditor()
{
    setLookAndFeel (nullptr);
}

void LxrDrumEditor::paint (juce::Graphics& g)
{
    g.fillAll (t::panelBg);

    auto fillSection = [&] (juce::Rectangle<int> r)
    {
        g.setColour (t::sectionBg);
        g.fillRoundedRectangle (r.toFloat(), 6.0f);
        g.setColour (t::sectionBorder);
        g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 6.0f, 1.0f);
    };

    fillSection (oscRect);
    fillSection (envRect);
    fillSection (transRect);
    fillSection (filterRect);
    fillSection (outputRect);
}

void LxrDrumEditor::resized()
{
    // 2-column layout. Each column is independently flow-laid: section
    // background -> header -> rows of widgets.

    auto bounds = getLocalBounds().reduced (t::windowPadding);

    const int colWidth = (bounds.getWidth() - t::columnGap) / 2;
    auto leftCol  = bounds.removeFromLeft (colWidth);
    bounds.removeFromLeft (t::columnGap);
    auto rightCol = bounds;

    // Helper that lays out a knob row inside a section.
    // `numKnobs` knobs spread evenly across the section width.
    auto layoutKnobRow = [] (juce::Rectangle<int> rowArea,
                              std::initializer_list<lxr::LabeledKnob*> knobs)
    {
        const int n = (int) knobs.size();
        if (n == 0) return;
        const int cellW = rowArea.getWidth() / n;
        int x = rowArea.getX();
        for (auto* k : knobs)
        {
            if (k != nullptr)
                k->setBounds (x, rowArea.getY(), cellW, t::knobCellHeight);
            x += cellW;
        }
    };

    // ============== LEFT COLUMN ==============
    {
        // OSCILLATOR section
        // Two knob rows + waveform strip
        const int oscContentHeight = t::sectionHeaderHeight
                                   + t::knobCellHeight + 6
                                   + t::knobCellHeight + 6
                                   + t::segHeight + 4;
        oscRect = leftCol.removeFromTop (oscContentHeight + t::sectionPadding * 2);
        leftCol.removeFromTop (t::sectionGap);

        auto inner = oscRect.reduced (t::sectionPadding);
        oscHeader.setBounds (inner.removeFromTop (t::sectionHeaderHeight));
        inner.removeFromTop (4);
        layoutKnobRow (inner.removeFromTop (t::knobCellHeight),
                       { kPitch.get(), kModPitch.get(), kFmAmount.get() });
        inner.removeFromTop (6);
        layoutKnobRow (inner.removeFromTop (t::knobCellHeight),
                       { kPitchEgAmt.get(), kPitchDecay.get(), nullptr });
        inner.removeFromTop (6);
        waveformSeg->setBounds (inner.removeFromTop (t::segHeight));

        // VOL ENV section
        const int envContentHeight = t::sectionHeaderHeight + t::knobCellHeight + 4;
        envRect = leftCol.removeFromTop (envContentHeight + t::sectionPadding * 2);

        inner = envRect.reduced (t::sectionPadding);
        envHeader.setBounds (inner.removeFromTop (t::sectionHeaderHeight));
        inner.removeFromTop (4);
        layoutKnobRow (inner.removeFromTop (t::knobCellHeight),
                       { kAttack.get(), kDecay.get(), kSlope.get() });
    }

    // ============== RIGHT COLUMN ==============
    {
        // TRANSIENT section
        const int transContentHeight = t::sectionHeaderHeight + t::knobCellHeight + 4;
        transRect = rightCol.removeFromTop (transContentHeight + t::sectionPadding * 2);
        rightCol.removeFromTop (t::sectionGap);

        auto inner = transRect.reduced (t::sectionPadding);
        transHeader.setBounds (inner.removeFromTop (t::sectionHeaderHeight));
        inner.removeFromTop (4);
        layoutKnobRow (inner.removeFromTop (t::knobCellHeight),
                       { kTransAmt.get(), kTransWave.get(), kSnap.get() });

        // FILTER section
        const int filterContentHeight = t::sectionHeaderHeight
                                      + t::knobCellHeight + 6
                                      + t::segHeight + 4;
        filterRect = rightCol.removeFromTop (filterContentHeight + t::sectionPadding * 2);
        rightCol.removeFromTop (t::sectionGap);

        inner = filterRect.reduced (t::sectionPadding);
        filterHeader.setBounds (inner.removeFromTop (t::sectionHeaderHeight));
        inner.removeFromTop (4);
        layoutKnobRow (inner.removeFromTop (t::knobCellHeight),
                       { kCutoff.get(), kReso.get(), nullptr });
        inner.removeFromTop (6);
        filterTypeSeg->setBounds (inner.removeFromTop (t::segHeight));

        // OUTPUT section
        const int outContentHeight = t::sectionHeaderHeight + t::knobCellHeight + 4;
        outputRect = rightCol.removeFromTop (outContentHeight + t::sectionPadding * 2);

        inner = outputRect.reduced (t::sectionPadding);
        outputHeader.setBounds (inner.removeFromTop (t::sectionHeaderHeight));
        inner.removeFromTop (4);
        layoutKnobRow (inner.removeFromTop (t::knobCellHeight),
                       { kDist.get(), kVol.get(), kPan.get() });
    }
}
