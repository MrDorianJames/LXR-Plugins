#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "Theme.h"

namespace lxr {

/**
 * LabeledKnob
 *
 * A rotary slider with a small text label below it. Hooks into an
 * AudioProcessorValueTreeState parameter via the standard slider
 * attachment; the label text is the parameter's display name truncated
 * to fit. Tooltip shows the full name + current value.
 */
class LabeledKnob : public juce::Component
{
public:
    LabeledKnob (juce::AudioProcessorValueTreeState& apvts,
                 const juce::String& parameterID,
                 const juce::String& displayLabel,
                 bool showValueText = false)
        : label (displayLabel), showValue (showValueText)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f,
                                    juce::MathConstants<float>::pi * 2.8f,
                                    true);
        addAndMakeVisible (slider);

        attachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (
                              apvts, parameterID, slider));

        if (showValue)
        {
            // Repaint when value changes so the readout stays in sync.
            slider.onValueChange = [this] { repaint(); };
        }

        slider.setTooltip (displayLabel);
    }

    void paint (juce::Graphics& g) override
    {
        const auto labelArea = juce::Rectangle<int> (0,
                                                     theme::knobDiameter + theme::knobLabelGap,
                                                     getWidth(),
                                                     theme::knobLabelHeight);
        g.setColour (theme::textMuted);
        g.setFont (theme::knobLabelFont());
        g.drawFittedText (label, labelArea, juce::Justification::centred, 1);

        if (showValue)
        {
            // Small bright readout right below the label. Uses the
            // parameter's own valueToText so units / formatting come
            // for free (MIDI note numbers print as plain integers).
            const auto valArea = juce::Rectangle<int> (0,
                                                       theme::knobDiameter + theme::knobLabelGap
                                                         + theme::knobLabelHeight,
                                                       getWidth(),
                                                       theme::knobLabelHeight);
            g.setColour (theme::textPrimary);
            g.setFont (theme::knobLabelFont().withHeight (10.5f).boldened());
            g.drawFittedText (slider.getTextFromValue (slider.getValue()),
                              valArea, juce::Justification::centred, 1);
        }
    }

    void resized() override
    {
        const int kx = (getWidth() - theme::knobDiameter) / 2;
        slider.setBounds (kx, 0, theme::knobDiameter, theme::knobDiameter);
    }

private:
    juce::String label;
    bool         showValue;
    juce::Slider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

// ---------------------------------------------------------------------

/**
 * SegmentedChoice
 *
 * Horizontal strip of buttons for a juce::AudioParameterChoice (or any
 * RangedAudioParameter with discrete int values). Click a segment to
 * set that value.
 */
class SegmentedChoice : public juce::Component
{
public:
    SegmentedChoice (juce::AudioProcessorValueTreeState& apvts,
                     const juce::String& parameterID,
                     const juce::StringArray& options)
        : labels (options)
    {
        param = apvts.getParameter (parameterID);
        jassert (param != nullptr);

        attachment.reset (new juce::ParameterAttachment (
            *param,
            [this] (float newValue)
            {
                // newValue is denormalised by JUCE for choice params (0..N-1)
                currentIndex = static_cast<int> (std::round (newValue));
                repaint();
            }));
        attachment->sendInitialUpdate();
    }

    void paint (juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat();
        const float w = bounds.getWidth() / (float) labels.size();

        g.setFont (theme::segFont());
        for (int i = 0; i < labels.size(); ++i)
        {
            const auto cell = juce::Rectangle<float> (bounds.getX() + i * w,
                                                      bounds.getY(),
                                                      w,
                                                      bounds.getHeight()).reduced (1.0f);
            const bool active = (i == currentIndex);

            g.setColour (active ? theme::segBgOn : theme::segBgOff);
            g.fillRoundedRectangle (cell, 3.0f);

            g.setColour (theme::segBorder);
            g.drawRoundedRectangle (cell, 3.0f, 1.0f);

            g.setColour (active ? theme::segTextOn : theme::segText);
            g.drawText (labels[i], cell.toNearestInt(),
                        juce::Justification::centred, false);
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        const float w = (float) getWidth() / (float) labels.size();
        const int idx = juce::jlimit (0, labels.size() - 1,
                                      static_cast<int> ((float) e.x / w));
        if (idx != currentIndex)
        {
            attachment->setValueAsCompleteGesture (static_cast<float> (idx));
        }
    }

private:
    juce::StringArray labels;
    juce::RangedAudioParameter* param { nullptr };
    std::unique_ptr<juce::ParameterAttachment> attachment;
    int currentIndex { 0 };
};

// ---------------------------------------------------------------------

/**
 * SectionHeader
 *
 * Small caps label with a thin underline. Marks a group of parameters.
 */
class SectionHeader : public juce::Component
{
public:
    explicit SectionHeader (const juce::String& text) : title (text.toUpperCase()) {}

    void paint (juce::Graphics& g) override
    {
        g.setFont (theme::headerFont());
        g.setColour (theme::textPrimary);
        const auto textArea = getLocalBounds().reduced (2, 0)
                                .withTrimmedBottom (2);
        g.drawText (title, textArea, juce::Justification::centredLeft, false);

        g.setColour (theme::sectionBorder);
        g.drawHorizontalLine (getHeight() - 1, 0.0f, (float) getWidth());
    }

private:
    juce::String title;
};

} // namespace lxr
