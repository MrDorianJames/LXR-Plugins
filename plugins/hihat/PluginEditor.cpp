#include "PluginEditor.h"

namespace t = lxr::theme;

namespace {
    const char* idMode          = "mode";
    const char* idSplitNote     = "split_note";
    const char* idPitch         = "pitch";
    const char* idWaveform      = "waveform";
    const char* idMod1Pitch     = "mod1_pitch";
    const char* idMod1Amount    = "mod1_amount";
    const char* idMod1Waveform  = "mod1_waveform";
    const char* idMod2Pitch     = "mod2_pitch";
    const char* idMod2Amount    = "mod2_amount";
    const char* idMod2Waveform  = "mod2_waveform";
    const char* idDecayClosed   = "decay_closed";
    const char* idDecayOpen     = "decay_open";
    const char* idVolAttack     = "vol_attack";
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

    const juce::StringArray waveLabels { "Sin", "Saw", "Tri", "Rect", "Noise" };
}

LxrHiHatEditor::LxrHiHatEditor (LxrHiHatProcessor& p)
    : juce::AudioProcessorEditor (p), proc (p)
{
    setLookAndFeel (&laf);

    auto add = [this] (auto& uptr, auto* component) {
        uptr.reset (component);
        addAndMakeVisible (uptr.get());
    };

    add (kSplitNote,   new lxr::LabeledKnob (proc.apvts, idSplitNote,    "Split Note", /*showValue*/ true));
    add (kPitch,       new lxr::LabeledKnob (proc.apvts, idPitch,        "Pitch"));
    add (kFm1,         new lxr::LabeledKnob (proc.apvts, idMod1Pitch,    "FM1"));
    add (kGain1,       new lxr::LabeledKnob (proc.apvts, idMod1Amount,   "Gain1"));
    add (kFm2,         new lxr::LabeledKnob (proc.apvts, idMod2Pitch,    "FM2"));
    add (kGain2,       new lxr::LabeledKnob (proc.apvts, idMod2Amount,   "Gain2"));
    add (kAttack,      new lxr::LabeledKnob (proc.apvts, idVolAttack,    "Attack"));
    add (kDecayClosed, new lxr::LabeledKnob (proc.apvts, idDecayClosed,  "Dcy Closed"));
    add (kDecayOpen,   new lxr::LabeledKnob (proc.apvts, idDecayOpen,    "Dcy Open"));
    add (kSlope,       new lxr::LabeledKnob (proc.apvts, idVolSlope,     "Slope"));
    add (kTransAmt,    new lxr::LabeledKnob (proc.apvts, idTransientAmt, "Amount"));
    add (kTransWave,   new lxr::LabeledKnob (proc.apvts, idTransientWave,"Wave"));
    add (kSnap,        new lxr::LabeledKnob (proc.apvts, idSnap,         "Snap"));
    add (kCutoff,      new lxr::LabeledKnob (proc.apvts, idCutoff,       "Cutoff"));
    add (kReso,        new lxr::LabeledKnob (proc.apvts, idReso,         "Reso"));
    add (kDist,        new lxr::LabeledKnob (proc.apvts, idDistortion,   "Dist"));
    add (kVol,         new lxr::LabeledKnob (proc.apvts, idVolume,       "Volume"));
    add (kPan,         new lxr::LabeledKnob (proc.apvts, idPan,          "Pan"));

    modeSeg.reset       (new lxr::SegmentedChoice (proc.apvts, idMode,
        juce::StringArray { "Split", "Closed", "Open" }));
    waveformSeg.reset   (new lxr::SegmentedChoice (proc.apvts, idWaveform,     waveLabels));
    wave1Seg.reset      (new lxr::SegmentedChoice (proc.apvts, idMod1Waveform, waveLabels));
    wave2Seg.reset      (new lxr::SegmentedChoice (proc.apvts, idMod2Waveform, waveLabels));
    filterTypeSeg.reset (new lxr::SegmentedChoice (proc.apvts, idFilterType,
        juce::StringArray { "Off", "LP", "HP", "BP" }));

    addAndMakeVisible (modeSeg.get());
    addAndMakeVisible (waveformSeg.get());
    addAndMakeVisible (wave1Seg.get());
    addAndMakeVisible (wave2Seg.get());
    addAndMakeVisible (filterTypeSeg.get());

    addAndMakeVisible (triggerHeader);
    addAndMakeVisible (carrierHeader);
    addAndMakeVisible (fm1Header);
    addAndMakeVisible (fm2Header);
    addAndMakeVisible (envHeader);
    addAndMakeVisible (transHeader);
    addAndMakeVisible (filterHeader);
    addAndMakeVisible (outputHeader);

    setSize (860, 620);
}

LxrHiHatEditor::~LxrHiHatEditor()
{
    setLookAndFeel (nullptr);
}

void LxrHiHatEditor::paint (juce::Graphics& g)
{
    g.fillAll (t::panelBg);
    auto fillSection = [&] (juce::Rectangle<int> r) {
        g.setColour (t::sectionBg);
        g.fillRoundedRectangle (r.toFloat(), 6.0f);
        g.setColour (t::sectionBorder);
        g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 6.0f, 1.0f);
    };
    fillSection (triggerRect);
    fillSection (carrierRect);
    fillSection (fm1Rect);
    fillSection (fm2Rect);
    fillSection (envRect);
    fillSection (transRect);
    fillSection (filterRect);
    fillSection (outputRect);
}

void LxrHiHatEditor::resized()
{
    auto bounds = getLocalBounds().reduced (t::windowPadding);
    // Three columns
    const int totalW   = bounds.getWidth();
    const int colWidth = (totalW - t::columnGap * 2) / 3;
    auto col1 = bounds.removeFromLeft (colWidth);
    bounds.removeFromLeft (t::columnGap);
    auto col2 = bounds.removeFromLeft (colWidth);
    bounds.removeFromLeft (t::columnGap);
    auto col3 = bounds;

    auto layoutKnobRow = [] (juce::Rectangle<int> rowArea,
                             std::initializer_list<lxr::LabeledKnob*> knobs) {
        const int n = (int) knobs.size();
        if (n == 0) return;
        const int cellW = rowArea.getWidth() / n;
        int x = rowArea.getX();
        for (auto* k : knobs) {
            if (k != nullptr)
                k->setBounds (x, rowArea.getY(), cellW, t::knobCellHeight);
            x += cellW;
        }
    };

    // Contract: caller passes the height needed for the section BODY
    // only (knobs + segments + their inter-row gaps). The helper adds
    // the header height + a small gap on top of that. Earlier I had
    // the caller do this and missed it in several places, which made
    // segments spill past the bottom of the section.
    auto layoutSection = [&] (juce::Rectangle<int>& outArea,
                              lxr::SectionHeader& header,
                              juce::Rectangle<int>& col,
                              int bodyH,
                              std::function<void (juce::Rectangle<int>&)> bodyLayout)
    {
        const int totalContentH = t::sectionHeaderHeight + 4 + bodyH;
        outArea = col.removeFromTop (totalContentH + t::sectionPadding * 2);
        col.removeFromTop (t::sectionGap);
        auto inner = outArea.reduced (t::sectionPadding);
        header.setBounds (inner.removeFromTop (t::sectionHeaderHeight));
        inner.removeFromTop (4);
        bodyLayout (inner);
    };

    // ============== COL 1: TRIGGER + CARRIER + FM1 ==============
    // TRIGGER: mode segment + split note knob (taller to fit value readout)
    const int splitKnobCellH = t::knobCellHeight + t::knobLabelHeight;  // extra row for value text
    layoutSection (triggerRect, triggerHeader, col1,
        t::segHeight + 6 + splitKnobCellH + 4,
        [&] (juce::Rectangle<int>& inner) {
            modeSeg->setBounds (inner.removeFromTop (t::segHeight));
            inner.removeFromTop (6);
            auto row = inner.removeFromTop (splitKnobCellH);
            kSplitNote->setBounds (row.getX(), row.getY(),
                                   row.getWidth(), splitKnobCellH);
        });

    // CARRIER: pitch knob + waveform segment
    layoutSection (carrierRect, carrierHeader, col1,
        t::knobCellHeight + 6 + t::segHeight + 4,
        [&] (juce::Rectangle<int>& inner) {
            layoutKnobRow (inner.removeFromTop (t::knobCellHeight),
                           { kPitch.get(), nullptr });
            inner.removeFromTop (6);
            waveformSeg->setBounds (inner.removeFromTop (t::segHeight));
        });

    // FM1: 2 knobs + segment
    layoutSection (fm1Rect, fm1Header, col1,
        t::knobCellHeight + 6 + t::segHeight + 4,
        [&] (juce::Rectangle<int>& inner) {
            layoutKnobRow (inner.removeFromTop (t::knobCellHeight),
                           { kFm1.get(), kGain1.get() });
            inner.removeFromTop (6);
            wave1Seg->setBounds (inner.removeFromTop (t::segHeight));
        });

    // ============== COL 2: FM2 + ENV ==============
    layoutSection (fm2Rect, fm2Header, col2,
        t::knobCellHeight + 6 + t::segHeight + 4,
        [&] (juce::Rectangle<int>& inner) {
            layoutKnobRow (inner.removeFromTop (t::knobCellHeight),
                           { kFm2.get(), kGain2.get() });
            inner.removeFromTop (6);
            wave2Seg->setBounds (inner.removeFromTop (t::segHeight));
        });

    // ENV: 4 knobs as 2x2 grid
    layoutSection (envRect, envHeader, col2,
        t::knobCellHeight + 6 + t::knobCellHeight + 4,
        [&] (juce::Rectangle<int>& inner) {
            layoutKnobRow (inner.removeFromTop (t::knobCellHeight),
                           { kAttack.get(), kDecayClosed.get() });
            inner.removeFromTop (6);
            layoutKnobRow (inner.removeFromTop (t::knobCellHeight),
                           { kDecayOpen.get(), kSlope.get() });
        });

    // ============== COL 3: TRANSIENT + FILTER + OUTPUT ==============
    layoutSection (transRect, transHeader, col3,
        t::knobCellHeight + 4,
        [&] (juce::Rectangle<int>& inner) {
            layoutKnobRow (inner.removeFromTop (t::knobCellHeight),
                           { kTransAmt.get(), kTransWave.get(), kSnap.get() });
        });

    layoutSection (filterRect, filterHeader, col3,
        t::knobCellHeight + 6 + t::segHeight + 4,
        [&] (juce::Rectangle<int>& inner) {
            layoutKnobRow (inner.removeFromTop (t::knobCellHeight),
                           { kCutoff.get(), kReso.get(), nullptr });
            inner.removeFromTop (6);
            filterTypeSeg->setBounds (inner.removeFromTop (t::segHeight));
        });

    layoutSection (outputRect, outputHeader, col3,
        t::knobCellHeight + 4,
        [&] (juce::Rectangle<int>& inner) {
            layoutKnobRow (inner.removeFromTop (t::knobCellHeight),
                           { kDist.get(), kVol.get(), kPan.get() });
        });
}
