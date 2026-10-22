#include "Theme909.h"

namespace Replicant909
{
    ThemeManager& ThemeManager::getInstance()
    {
        static ThemeManager instance;
        return instance;
    }

    ThemeManager::ThemeManager()
    {
        initThemes();
    }

    void ThemeManager::initThemes()
    {
        themes.resize(NUM_SKINS);

        // =========================================================================
        // SKIN 1: Retro-Futuristic Classic (Authentic Roland TR-909)
        // Vintage cream/off-white metal housing, dark charcoal cheek panels,
        // iconic Roland orange & yellow racing stripes, ruby red 7-segment display,
        // spun aluminum knob inlays, and classic beige dual-LED step buttons.
        // =========================================================================
        {
            Theme909 t;
            t.id = SKIN_RETRO_CLASSIC;
            t.name = "Retro-Futuristic Classic";
            t.description = "Authentic Roland TR-909 ivory metal chassis with classic orange/yellow livery";

            t.chassisTop    = juce::Colour(0xffe6e4dc); // Vintage 909 warm ivory
            t.chassisBottom = juce::Colour(0xffd5d3ca);
            t.chassisBevel  = juce::Colour(0xfff5f3eb);
            t.chassisShadow = juce::Colour(0xff9e9c94);

            t.cheekColor  = juce::Colour(0xff2d2f34);  // Charcoal side cheeks
            t.cheekShadow = juce::Colour(0xff18191c);

            t.accentPrimary   = juce::Colour(0xfff36b21); // Classic TR-909 Orange
            t.accentSecondary = juce::Colour(0xfff4b025); // Classic TR-909 Yellow
            t.accentTertiary  = juce::Colour(0xff36383e);

            t.silkscreenText   = juce::Colour(0xff222428); // Sharp dark slate print
            t.silkscreenHeader = juce::Colour(0xfff36b21);
            t.silkscreenLines  = juce::Colour(0xff8a8c88);

            t.displayHousing = juce::Colour(0xff121316); // Smoked acrylic window
            t.displayLit     = juce::Colour(0xffff2a32); // Ruby red LED
            t.displayGlow    = juce::Colour(0xffff0a18);
            t.displayUnlit   = juce::Colour(0xff280608);

            t.knobCapTop    = juce::Colour(0xffd8dcda); // Spun metal aluminum cap
            t.knobCapBottom = juce::Colour(0xff8c928e);
            t.knobSkirt     = juce::Colour(0xff202422); // Fluted dark skirt
            t.knobPointer   = juce::Colour(0xff141615);
            t.knobArc       = juce::Colour(0xfff36b21);

            t.buttonBevel         = juce::Colour(0xffebece8);
            t.buttonTop           = juce::Colour(0xffe2e4de); // Warm beige key
            t.buttonBottom        = juce::Colour(0xffc2c5be);
            t.buttonPressedTop    = juce::Colour(0xffb5b8b0);
            t.buttonPressedBottom = juce::Colour(0xff989a93);
            t.buttonBorder        = juce::Colour(0xff6e716c);

            t.ledOff       = juce::Colour(0xff2e1416); // Unlit LED recess
            t.ledNormalLit = juce::Colour(0xffff7e1a); // Warm Amber LED (Normal step)
            t.ledAccentLit = juce::Colour(0xffff1e28); // Vivid Red LED (Accent step)
            t.ledFlamLit   = juce::Colour(0xffffd030); // Bright Yellow LED (Flam)

            t.meterGreen  = juce::Colour(0xff33cc44);
            t.meterYellow = juce::Colour(0xffffbb22);
            t.meterRed    = juce::Colour(0xffff2233);

            themes[SKIN_RETRO_CLASSIC] = t;
        }

        // =========================================================================
        // SKIN 2: Modern Obsidian Cyberpunk
        // Deep matte carbon chassis, electric neon cyan and amber accents,
        // high-contrast OLED digital display, knurled anodized rotary dials
        // with illuminated radial halo rings, and backlit silicone keys.
        // =========================================================================
        {
            Theme909 t;
            t.id = SKIN_MODERN_OBSIDIAN;
            t.name = "Modern Obsidian";
            t.description = "Stealth matte carbon chassis with electric neon cyan and amber illumination";

            t.chassisTop    = juce::Colour(0xff181a20);
            t.chassisBottom = juce::Colour(0xff0e1014);
            t.chassisBevel  = juce::Colour(0xff282c36);
            t.chassisShadow = juce::Colour(0xff060709);

            t.cheekColor  = juce::Colour(0xff101115);
            t.cheekShadow = juce::Colour(0xff050507);

            t.accentPrimary   = juce::Colour(0xff00e5ff); // Electric Neon Cyan
            t.accentSecondary = juce::Colour(0xffff7700); // Neon Amber
            t.accentTertiary  = juce::Colour(0xff0099b8);

            t.silkscreenText   = juce::Colour(0xffd5dbe6); // Crisp high-contrast futuristic text
            t.silkscreenHeader = juce::Colour(0xff00e5ff);
            t.silkscreenLines  = juce::Colour(0xff283344);

            t.displayHousing = juce::Colour(0xff060a0f); // OLED black
            t.displayLit     = juce::Colour(0xff00f0ff); // Electric Cyan OLED
            t.displayGlow    = juce::Colour(0xff0099cc);
            t.displayUnlit   = juce::Colour(0xff041520);

            t.knobCapTop    = juce::Colour(0xff2c303c); // Anodized matte black
            t.knobCapBottom = juce::Colour(0xff14161c);
            t.knobSkirt     = juce::Colour(0xff0f1116);
            t.knobPointer   = juce::Colour(0xff00e5ff);
            t.knobArc       = juce::Colour(0xff00e5ff);

            t.buttonBevel         = juce::Colour(0xff2a2e3a);
            t.buttonTop           = juce::Colour(0xff20242e);
            t.buttonBottom        = juce::Colour(0xff12151b);
            t.buttonPressedTop    = juce::Colour(0xff101318);
            t.buttonPressedBottom = juce::Colour(0xff080a0e);
            t.buttonBorder        = juce::Colour(0xff00e5ff);

            t.ledOff       = juce::Colour(0xff081a22);
            t.ledNormalLit = juce::Colour(0xff00e5ff); // Neon Cyan (Normal)
            t.ledAccentLit = juce::Colour(0xffff7700); // Neon Amber (Accent)
            t.ledFlamLit   = juce::Colour(0xffffffff); // Bright White (Flam)

            t.meterGreen  = juce::Colour(0xff00e5ff);
            t.meterYellow = juce::Colour(0xffff9900);
            t.meterRed    = juce::Colour(0xffff2255);

            themes[SKIN_MODERN_OBSIDIAN] = t;
        }
    }

    void ThemeManager::setSkin(SkinId skin)
    {
        if (skin >= 0 && skin < NUM_SKINS && currentSkin != skin)
        {
            currentSkin = skin;
            for (auto& l : listeners)
            {
                if (l)
                    l();
            }
        }
    }

    void ThemeManager::addListener(std::function<void()> listener)
    {
        listeners.push_back(listener);
    }
}
