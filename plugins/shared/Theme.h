#pragma once

#include <juce_graphics/juce_graphics.h>

namespace lxr::theme
{
    // -- Colors ---------------------------------------------------------
    // Dark panel inspired by the LXR/LXR-02 chassis: muted, slightly
    // warm to keep it from feeling clinical.
    inline const juce::Colour panelBg       { 0xff1f2225 };
    inline const juce::Colour sectionBg     { 0xff262a2e };
    inline const juce::Colour sectionBorder { 0xff141618 };
    inline const juce::Colour textPrimary   { 0xffe6e8eb };
    inline const juce::Colour textMuted     { 0xff7e858d };
    inline const juce::Colour textDim       { 0xff5a6168 };

    // Knob colors. Track is the unfilled arc, fill is the value arc,
    // pointer is the radial indicator line. Accent matches Erica
    // Synths' red display aesthetic.
    inline const juce::Colour knobTrack   { 0xff15171a };
    inline const juce::Colour knobFill    { 0xffe0463a };   // Erica red
    inline const juce::Colour knobPointer { 0xffffd8cc };
    inline const juce::Colour knobBody    { 0xff2a2e33 };

    // Segmented-choice button states.
    inline const juce::Colour segBgOff   { 0xff1a1d20 };
    inline const juce::Colour segBgOn    { 0xffe0463a };
    inline const juce::Colour segText    { 0xffc8ccd0 };
    inline const juce::Colour segTextOn  { 0xff1a1d20 };
    inline const juce::Colour segBorder  { 0xff15171a };

    // -- Sizes ----------------------------------------------------------
    constexpr int knobDiameter   = 62;
    constexpr int knobLabelGap   = 5;
    constexpr int knobLabelHeight = 16;
    constexpr int knobCellWidth  = 94;
    constexpr int knobCellHeight = knobDiameter + knobLabelGap + knobLabelHeight;  // 83

    constexpr int sectionHeaderHeight = 24;
    constexpr int sectionGap          = 10;
    constexpr int sectionPadding      = 12;

    constexpr int segHeight = 28;

    constexpr int windowPadding = 14;
    constexpr int columnGap     = 12;

    // -- Fonts ----------------------------------------------------------
    inline juce::Font headerFont()
    {
        // Small caps section label.
        return juce::Font (juce::FontOptions (13.0f, juce::Font::bold));
    }
    inline juce::Font knobLabelFont()
    {
        return juce::Font (juce::FontOptions (12.0f));
    }
    inline juce::Font segFont()
    {
        return juce::Font (juce::FontOptions (12.5f, juce::Font::bold));
    }
}
