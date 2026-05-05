#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "Theme.h"

namespace lxr {

/**
 * LxrLookAndFeel - drawing for our rotary knobs.
 *
 * Knob has:
 *  - a recessed dark circular track on the back (knobTrack)
 *  - an amber fill arc tracking the value (knobFill)
 *  - a slightly raised body disc (knobBody)
 *  - a pointer line from center toward the edge (knobPointer)
 *
 * Bipolar (-1..+1) parameters draw the fill arc starting from 12
 * o'clock and extending toward whichever direction the value points.
 */
class LxrLookAndFeel : public juce::LookAndFeel_V4
{
public:
    LxrLookAndFeel()
    {
        setColour (juce::Slider::textBoxTextColourId, theme::textPrimary);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::Label::textColourId, theme::textMuted);
        setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    }

    void drawRotarySlider (juce::Graphics& g,
                           int x, int y, int w, int h,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider& s) override
    {
        const auto bounds = juce::Rectangle<float> ((float)x, (float)y, (float)w, (float)h).reduced (3.0f);
        const auto centre = bounds.getCentre();
        const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        // Background track arc (recessed look).
        const float trackThickness = juce::jmax (3.0f, radius * 0.18f);
        const float arcRadius = radius - trackThickness * 0.5f - 1.0f;

        juce::Path trackArc;
        trackArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius,
                                0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (theme::knobTrack);
        g.strokePath (trackArc, juce::PathStrokeType (trackThickness,
                                                      juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));

        // Value arc - bipolar parameters fill from centre outward.
        const float midPos = 0.5f;
        const bool  isBipolar = isBipolarParam (s);
        const float valueAngle = juce::jmap (sliderPosProportional, rotaryStartAngle, rotaryEndAngle);

        juce::Path valueArc;
        if (isBipolar)
        {
            const float midAngle = juce::jmap (midPos, rotaryStartAngle, rotaryEndAngle);
            const float a = juce::jmin (midAngle, valueAngle);
            const float b = juce::jmax (midAngle, valueAngle);
            valueArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius,
                                    0.0f, a, b, true);
        }
        else
        {
            valueArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius,
                                    0.0f, rotaryStartAngle, valueAngle, true);
        }
        g.setColour (theme::knobFill);
        g.strokePath (valueArc, juce::PathStrokeType (trackThickness,
                                                      juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));

        // Solid body disc.
        const float bodyRadius = arcRadius - trackThickness * 0.5f - 2.0f;
        g.setColour (theme::knobBody);
        g.fillEllipse (juce::Rectangle<float> (bodyRadius * 2.0f, bodyRadius * 2.0f).withCentre (centre));

        // Subtle highlight at the top of the disc for depth.
        juce::ColourGradient grad (theme::knobBody.brighter (0.08f),
                                   centre.x, centre.y - bodyRadius,
                                   theme::knobBody.darker (0.15f),
                                   centre.x, centre.y + bodyRadius,
                                   false);
        g.setGradientFill (grad);
        g.fillEllipse (juce::Rectangle<float> (bodyRadius * 2.0f - 2.0f,
                                               bodyRadius * 2.0f - 2.0f).withCentre (centre));

        // Pointer line.
        juce::Path pointer;
        const float pointerLen   = bodyRadius * 0.85f;
        const float pointerThick = juce::jmax (1.5f, radius * 0.06f);
        pointer.addRoundedRectangle (-pointerThick * 0.5f,
                                     -pointerLen,
                                     pointerThick,
                                     pointerLen * 0.55f,
                                     pointerThick * 0.5f);
        g.setColour (theme::knobPointer);
        g.fillPath (pointer, juce::AffineTransform::rotation (valueAngle).translated (centre));
    }

private:
    static bool isBipolarParam (const juce::Slider& s) noexcept
    {
        const auto range = s.getRange();
        return range.getStart() < 0.0 && range.getEnd() > 0.0;
    }
};

} // namespace lxr
