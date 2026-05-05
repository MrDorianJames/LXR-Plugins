#include "PluginEditor.h"

namespace t = lxr::theme;

namespace {
    const char* idPitch         = "pitch";
    const char* idWaveform      = "waveform";
    const char* idMod1Pitch     = "mod1_pitch";
    const char* idMod1Amount    = "mod1_amount";
    const char* idMod1Waveform  = "mod1_waveform";
    const char* idMod2Pitch     = "mod2_pitch";
    const char* idMod2Amount    = "mod2_amount";
    const char* idMod2Waveform  = "mod2_waveform";
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

    const juce::StringArray waveLabels { "Sin", "Saw", "Tri", "Rect", "Noise" };
}

LxrCymbalEditor::LxrCymbalEditor (LxrCymbalProcessor& p)
    : juce::AudioProcessorEditor (p), proc (p)
{
    setLookAndFeel (&laf);

    auto add = [this] (auto& uptr, auto* component) {
        uptr.reset (component);
        addAndMakeVisible (uptr.get());
    };

    add (kPitch,    new lxr::LabeledKnob (proc.apvts, idPitch,     "Pitch"));
    add (kFm1,      new lxr::LabeledKnob (proc.apvts, idMod1Pitch, "FM1"));
    add (kGain1,    new lxr::LabeledKnob (proc.apvts, idMod1Amount,"Gain1"));
    add (kFm2,      new lxr::LabeledKnob (proc.apvts, idMod2Pitch, "FM2"));
    add (kGain2,    new lxr::LabeledKnob (proc.apvts, idMod2Amount,"Gain2"));

    add (kAttack,   new lxr::LabeledKnob (proc.apvts, idVolAttack, "Attack"));
    add (kDecay,    new lxr::LabeledKnob (proc.apvts, idVolDecay,  "Decay"));
    add (kSlope,    new lxr::LabeledKnob (proc.apvts, idVolSlope,  "Slope"));

    add (kTransAmt, new lxr::LabeledKnob (proc.apvts, idTransientAmt, "Amount"));
    add (kTransWave,new lxr::LabeledKnob (proc.apvts, idTransientWave,"Wave"));
    add (kSnap,     new lxr::LabeledKnob (proc.apvts, idSnap,         "Snap"));

    add (kCutoff,   new lxr::LabeledKnob (proc.apvts, idCutoff,    "Cutoff"));
    add (kReso,     new lxr::LabeledKnob (proc.apvts, idReso,      "Reso"));

    add (kDist,     new lxr::LabeledKnob (proc.apvts, idDistortion,"Dist"));
    add (kVol,      new lxr::LabeledKnob (proc.apvts, idVolume,    "Volume"));
    add (kPan,      new lxr::LabeledKnob (proc.apvts, idPan,       "Pan"));

    waveformSeg.reset   (new lxr::SegmentedChoice (proc.apvts, idWaveform,     waveLabels));
    wave1Seg.reset      (new lxr::SegmentedChoice (proc.apvts, idMod1Waveform, waveLabels));
    wave2Seg.reset      (new lxr::SegmentedChoice (proc.apvts, idMod2Waveform, waveLabels));
    filterTypeSeg.reset (new lxr::SegmentedChoice (proc.apvts, idFilterType,
        juce::StringArray { "Off", "LP", "HP", "BP" }));

    addAndMakeVisible (waveformSeg.get());
    addAndMakeVisible (wave1Seg.get());
    addAndMakeVisible (wave2Seg.get());
    addAndMakeVisible (filterTypeSeg.get());

    addAndMakeVisible (carrierHeader);
    addAndMakeVisible (fm1Header);
    addAndMakeVisible (fm2Header);
    addAndMakeVisible (envHeader);
    addAndMakeVisible (transHeader);
    addAndMakeVisible (filterHeader);
    addAndMakeVisible (outputHeader);

    setSize (800, 660);
}

LxrCymbalEditor::~LxrCymbalEditor()
{
    setLookAndFeel (nullptr);
}

void LxrCymbalEditor::paint (juce::Graphics& g)
{
    g.fillAll (t::panelBg);
    auto fillSection = [&] (juce::Rectangle<int> r) {
        g.setColour (t::sectionBg);
        g.fillRoundedRectangle (r.toFloat(), 6.0f);
        g.setColour (t::sectionBorder);
        g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 6.0f, 1.0f);
    };
    fillSection (carrierRect);
    fillSection (fm1Rect);
    fillSection (fm2Rect);
    fillSection (envRect);
    fillSection (transRect);
    fillSection (filterRect);
    fillSection (outputRect);
}

void LxrCymbalEditor::resized()
{
    auto bounds = getLocalBounds().reduced (t::windowPadding);
    const int colWidth = (bounds.getWidth() - t::columnGap) / 2;
    auto leftCol  = bounds.removeFromLeft (colWidth);
    bounds.removeFromLeft (t::columnGap);
    auto rightCol = bounds;

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

    // Knob+segment section: header + 1 knob row + segment
    auto layoutSection_KnobSeg = [&] (juce::Rectangle<int>& outArea,
                                      lxr::SectionHeader& header,
                                      std::initializer_list<lxr::LabeledKnob*> knobs,
                                      lxr::SegmentedChoice* seg,
                                      juce::Rectangle<int>& col)
    {
        const int contentH = t::sectionHeaderHeight
                           + 4 + t::knobCellHeight
                           + (seg ? 6 + t::segHeight : 0)
                           + 4;
        outArea = col.removeFromTop (contentH + t::sectionPadding * 2);
        col.removeFromTop (t::sectionGap);
        auto inner = outArea.reduced (t::sectionPadding);
        header.setBounds (inner.removeFromTop (t::sectionHeaderHeight));
        inner.removeFromTop (4);
        layoutKnobRow (inner.removeFromTop (t::knobCellHeight), knobs);
        if (seg) {
            inner.removeFromTop (6);
            seg->setBounds (inner.removeFromTop (t::segHeight));
        }
    };

    // ============== LEFT: CARRIER + FM1 + FM2 ==============
    layoutSection_KnobSeg (carrierRect, carrierHeader,
                           { kPitch.get(), nullptr, nullptr },
                           waveformSeg.get(), leftCol);
    layoutSection_KnobSeg (fm1Rect, fm1Header,
                           { kFm1.get(), kGain1.get(), nullptr },
                           wave1Seg.get(), leftCol);
    layoutSection_KnobSeg (fm2Rect, fm2Header,
                           { kFm2.get(), kGain2.get(), nullptr },
                           wave2Seg.get(), leftCol);

    // ============== RIGHT: ENV + TRANSIENT + FILTER + OUTPUT ==============
    layoutSection_KnobSeg (envRect,   envHeader,
                           { kAttack.get(), kDecay.get(), kSlope.get() },
                           nullptr, rightCol);
    layoutSection_KnobSeg (transRect, transHeader,
                           { kTransAmt.get(), kTransWave.get(), kSnap.get() },
                           nullptr, rightCol);
    layoutSection_KnobSeg (filterRect, filterHeader,
                           { kCutoff.get(), kReso.get(), nullptr },
                           filterTypeSeg.get(), rightCol);
    layoutSection_KnobSeg (outputRect, outputHeader,
                           { kDist.get(), kVol.get(), kPan.get() },
                           nullptr, rightCol);
}
