#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include <functional>

namespace Replicant909
{
    enum SkinId : int
    {
        SKIN_RETRO_CLASSIC = 0,
        SKIN_MODERN_OBSIDIAN = 1,
        NUM_SKINS
    };

    struct Theme909
    {
        SkinId id = SKIN_RETRO_CLASSIC;
        juce::String name;
        juce::String description;

        // Chassis Colors
        juce::Colour chassisTop;
        juce::Colour chassisBottom;
        juce::Colour chassisBevel;
        juce::Colour chassisShadow;

        // Side Cheeks (left and right end-caps)
        juce::Colour cheekColor;
        juce::Colour cheekShadow;

        // Striping and Livery Accents
        juce::Colour accentPrimary;   // Classic 909 Orange / Modern Neon Cyan
        juce::Colour accentSecondary; // Classic Yellow / Modern Amber
        juce::Colour accentTertiary;  // Dark border / Silkscreen border

        // Silkscreen Typography
        juce::Colour silkscreenText;
        juce::Colour silkscreenHeader;
        juce::Colour silkscreenLines;

        // 7-Segment Tempo Display
        juce::Colour displayHousing;
        juce::Colour displayLit;
        juce::Colour displayGlow;
        juce::Colour displayUnlit;

        // Knobs
        juce::Colour knobCapTop;
        juce::Colour knobCapBottom;
        juce::Colour knobSkirt;
        juce::Colour knobPointer;
        juce::Colour knobArc;

        // Step Buttons & Keys
        juce::Colour buttonBevel;
        juce::Colour buttonTop;
        juce::Colour buttonBottom;
        juce::Colour buttonPressedTop;
        juce::Colour buttonPressedBottom;
        juce::Colour buttonBorder;

        // LEDs (Dual-Color: Step Normal vs Accent)
        juce::Colour ledOff;
        juce::Colour ledNormalLit;
        juce::Colour ledAccentLit;
        juce::Colour ledFlamLit;

        // Level Peak Meters
        juce::Colour meterGreen;
        juce::Colour meterYellow;
        juce::Colour meterRed;
    };

    class ThemeManager
    {
    public:
        static ThemeManager& getInstance();

        SkinId getActiveSkin() const noexcept { return currentSkin; }
        const Theme909& getCurrentTheme() const noexcept { return themes[currentSkin]; }

        void setSkin(SkinId skin);

        void addListener(std::function<void()> listener);

    private:
        ThemeManager();
        SkinId currentSkin = SKIN_RETRO_CLASSIC;
        std::vector<Theme909> themes;
        std::vector<std::function<void()>> listeners;

        void initThemes();
    };
}
