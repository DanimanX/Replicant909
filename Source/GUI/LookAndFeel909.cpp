#include "LookAndFeel909.h"
#include <cmath>

namespace Replicant909
{
    LookAndFeel909::LookAndFeel909()
    {
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xfff36b21));
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff333333));
        setColour(juce::TextButton::textColourOffId, juce::Colours::black);
        setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    }

    void LookAndFeel909::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float rotaryStartAngle,
                                          float rotaryEndAngle, juce::Slider& /*slider*/)
    {
        const auto& theme = ThemeManager::getInstance().getCurrentTheme();
        const float radius = juce::jmin(width, height) * 0.44f;
        const float centreX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
        const float centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
        const float currentAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        juce::Graphics::ScopedSaveState state(g);

        if (theme.id == SKIN_RETRO_CLASSIC)
        {
            // =================================================================
            // Vintage TR-909 Rotary Knob
            // =================================================================
            // 1. Drop shadow
            g.setColour(juce::Colours::black.withAlpha(0.35f));
            g.fillEllipse(centreX - radius + 1.0f, centreY - radius + 2.0f, radius * 2.0f, radius * 2.0f);

            // 2. Fluted outer skirt (dark grooved plastic)
            g.setColour(theme.knobSkirt);
            g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

            // Fluted notch ring
            g.setColour(theme.knobSkirt.darker(0.3f));
            g.drawEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f, 1.5f);

            // 3. Spun aluminum cap
            const float capRadius = radius * 0.72f;
            juce::ColourGradient capGrad(theme.knobCapTop, centreX - capRadius * 0.5f, centreY - capRadius * 0.5f,
                                        theme.knobCapBottom, centreX + capRadius * 0.5f, centreY + capRadius * 0.5f, false);
            g.setGradientFill(capGrad);
            g.fillEllipse(centreX - capRadius, centreY - capRadius, capRadius * 2.0f, capRadius * 2.0f);

            // Cap rim highlight
            g.setColour(theme.chassisBevel.withAlpha(0.6f));
            g.drawEllipse(centreX - capRadius, centreY - capRadius, capRadius * 2.0f, capRadius * 2.0f, 1.0f);

            // 4. Pointer line
            juce::Path p;
            p.startNewSubPath(centreX, centreY);
            p.lineTo(centreX + (capRadius - 1.0f) * std::sin(currentAngle),
                     centreY - (capRadius - 1.0f) * std::cos(currentAngle));
            g.setColour(theme.knobPointer);
            g.strokePath(p, juce::PathStrokeType(2.5f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
        }
        else
        {
            // =================================================================
            // Modern Obsidian Cyberpunk Rotary Knob
            // =================================================================
            // 1. Glow Arc background
            juce::Path arcBg;
            arcBg.addCentredArc(centreX, centreY, radius + 2.0f, radius + 2.0f, 0.0f,
                                rotaryStartAngle, rotaryEndAngle, true);
            g.setColour(theme.displayUnlit.withAlpha(0.4f));
            g.strokePath(arcBg, juce::PathStrokeType(2.0f));

            // 2. Active illuminated value arc
            if (sliderPos > 0.001f)
            {
                juce::Path activeArc;
                activeArc.addCentredArc(centreX, centreY, radius + 2.0f, radius + 2.0f, 0.0f,
                                       rotaryStartAngle, currentAngle, true);
                g.setColour(theme.knobArc.withAlpha(0.35f));
                g.strokePath(activeArc, juce::PathStrokeType(4.5f));
                g.setColour(theme.knobArc);
                g.strokePath(activeArc, juce::PathStrokeType(2.0f));
            }

            // 3. Anodized matte dial body
            const float dialRadius = radius * 0.78f;
            juce::ColourGradient dialGrad(theme.knobCapTop, centreX, centreY - dialRadius,
                                         theme.knobCapBottom, centreX, centreY + dialRadius, false);
            g.setGradientFill(dialGrad);
            g.fillEllipse(centreX - dialRadius, centreY - dialRadius, dialRadius * 2.0f, dialRadius * 2.0f);

            g.setColour(theme.chassisBevel);
            g.drawEllipse(centreX - dialRadius, centreY - dialRadius, dialRadius * 2.0f, dialRadius * 2.0f, 1.0f);

            // 4. Illuminated neon pointer
            juce::Path p;
            p.startNewSubPath(centreX + dialRadius * 0.35f * std::sin(currentAngle),
                             centreY - dialRadius * 0.35f * std::cos(currentAngle));
            p.lineTo(centreX + (dialRadius - 2.0f) * std::sin(currentAngle),
                     centreY - (dialRadius - 2.0f) * std::cos(currentAngle));

            g.setColour(theme.knobPointer.withAlpha(0.5f));
            g.strokePath(p, juce::PathStrokeType(4.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
            g.setColour(theme.knobPointer);
            g.strokePath(p, juce::PathStrokeType(2.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
        }
    }

    void LookAndFeel909::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                              const juce::Colour& /*bg*/,
                                              bool isHighlighted,
                                              bool isDown)
    {
        const auto& theme = ThemeManager::getInstance().getCurrentTheme();
        auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);

        // Tactile key shading
        juce::Colour topCol = isDown ? theme.buttonPressedTop : (isHighlighted ? theme.buttonTop.brighter(0.08f) : theme.buttonTop);
        juce::Colour botCol = isDown ? theme.buttonPressedBottom : theme.buttonBottom;

        juce::ColourGradient grad(topCol, bounds.getX(), bounds.getY(),
                                  botCol, bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(bounds, 3.0f);

        // Border / bevel
        g.setColour(isDown ? theme.buttonBorder.darker(0.3f) : theme.buttonBorder);
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

        if (!isDown)
        {
            // Subtle top highlight
            g.setColour(theme.buttonBevel.withAlpha(0.7f));
            g.drawHorizontalLine(static_cast<int>(bounds.getY() + 1.0f), bounds.getX() + 2.0f, bounds.getRight() - 2.0f);
        }
    }

    void LookAndFeel909::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                          bool /*isHighlighted*/,
                                          bool /*isDown*/)
    {
        const auto& theme = ThemeManager::getInstance().getCurrentTheme();
        auto bounds = button.getLocalBounds().toFloat();
        float boxSize = 14.0f;
        auto boxRect = juce::Rectangle<float>(bounds.getX(), bounds.getCentreY() - boxSize * 0.5f, boxSize, boxSize);

        g.setColour(theme.chassisBottom.darker(0.4f));
        g.fillRoundedRectangle(boxRect, 2.0f);
        g.setColour(theme.silkscreenLines);
        g.drawRoundedRectangle(boxRect, 2.0f, 1.0f);

        if (button.getToggleState())
        {
            g.setColour(theme.accentPrimary);
            g.fillRoundedRectangle(boxRect.reduced(3.0f), 1.0f);
        }

        g.setColour(theme.silkscreenText);
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.drawText(button.getButtonText(),
                   bounds.removeFromRight(bounds.getWidth() - boxSize - 6.0f),
                   juce::Justification::centredLeft, true);
    }

    void LookAndFeel909::drawLed(juce::Graphics& g, float cx, float cy, float radius,
                                 bool lit, const juce::Colour& litColour, const juce::Colour& offColour)
    {
        // Dark bezel recess
        g.setColour(juce::Colour(0xff18191c));
        g.fillEllipse(cx - radius - 1.0f, cy - radius - 1.0f, (radius + 1.0f) * 2.0f, (radius + 1.0f) * 2.0f);

        if (lit)
        {
            // Diffuse glow
            g.setColour(litColour.withAlpha(0.4f));
            g.fillEllipse(cx - radius * 2.0f, cy - radius * 2.0f, radius * 4.0f, radius * 4.0f);

            // Bright core
            g.setColour(litColour);
            g.fillEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

            // Specular highlight
            g.setColour(juce::Colours::white.withAlpha(0.6f));
            g.fillEllipse(cx - radius * 0.5f, cy - radius * 0.6f, radius * 0.8f, radius * 0.5f);
        }
        else
        {
            g.setColour(offColour);
            g.fillEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
        }
    }

    void LookAndFeel909::drawStepLed(juce::Graphics& g, const juce::Rectangle<float>& bounds,
                                     uint8_t stepState, bool isCurrentPlayhead, const Theme909& theme)
    {
        // LED window area inside the step button
        auto ledRect = juce::Rectangle<float>(bounds.getX() + 4.0f, bounds.getY() + 4.0f,
                                              bounds.getWidth() - 8.0f, 7.0f);

        // Dark bezel
        g.setColour(theme.ledOff.darker(0.3f));
        g.fillRoundedRectangle(ledRect, 2.0f);
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.drawRoundedRectangle(ledRect, 2.0f, 0.8f);

        juce::Colour activeColor = theme.ledOff;
        bool isLit = false;

        if (stepState == STEP_NORMAL)
        {
            activeColor = theme.ledNormalLit;
            isLit = true;
        }
        else if (stepState == STEP_ACCENT)
        {
            activeColor = theme.ledAccentLit;
            isLit = true;
        }
        else if (stepState == STEP_FLAM)
        {
            activeColor = theme.ledFlamLit;
            isLit = true;
        }

        if (isLit)
        {
            // Lens glow
            g.setColour(activeColor.withAlpha(0.45f));
            g.fillRoundedRectangle(ledRect.expanded(2.0f), 3.0f);

            // Main lens
            g.setColour(activeColor);
            g.fillRoundedRectangle(ledRect.reduced(1.0f), 1.5f);

            // Specular reflection
            g.setColour(juce::Colours::white.withAlpha(0.7f));
            g.fillRoundedRectangle(ledRect.getX() + 2.0f, ledRect.getY() + 1.0f, ledRect.getWidth() - 4.0f, 2.0f, 1.0f);
        }
        else
        {
            g.setColour(theme.ledOff);
            g.fillRoundedRectangle(ledRect.reduced(1.0f), 1.5f);
        }

        // Playhead indicator: lower small pip
        if (isCurrentPlayhead)
        {
            float pipRadius = 2.5f;
            float pipX = bounds.getCentreX();
            float pipY = bounds.getBottom() - 6.0f;
            drawLed(g, pipX, pipY, pipRadius, true, theme.accentSecondary, theme.ledOff);
        }
    }

    void LookAndFeel909::drawSevenSegmentDisplay(juce::Graphics& g, const juce::Rectangle<float>& bounds,
                                                 int number, const Theme909& theme)
    {
        // 1. Smoked Glass Housing
        g.setColour(theme.displayHousing);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(theme.cheekShadow);
        g.drawRoundedRectangle(bounds, 4.0f, 1.5f);

        // 3-digit segmentation: e.g. 128 BPM
        int clamped = juce::jlimit(0, 999, number);
        int d0 = (clamped / 100) % 10;
        int d1 = (clamped / 10) % 10;
        int d2 = clamped % 10;

        int digits[3] = { d0, d1, d2 };

        // 7-segment bitmask (a, b, c, d, e, f, g)
        // bit 0: a (top)
        // bit 1: b (top-right)
        // bit 2: c (bottom-right)
        // bit 3: d (bottom)
        // bit 4: e (bottom-left)
        // bit 5: f (top-left)
        // bit 6: g (middle)
        static constexpr uint8_t segPatterns[10] = {
            0b00111111, // 0
            0b00000110, // 1
            0b01011011, // 2
            0b01001111, // 3
            0b01100110, // 4
            0b01101101, // 5
            0b01111101, // 6
            0b00000111, // 7
            0b01111111, // 8
            0b01101111  // 9
        };

        const float pad = 4.0f;
        const float digitWidth = (bounds.getWidth() - pad * 4.0f) / 3.0f;
        const float digitHeight = bounds.getHeight() - pad * 2.0f;

        for (int i = 0; i < 3; ++i)
        {
            auto dRect = juce::Rectangle<float>(bounds.getX() + pad + static_cast<float>(i) * (digitWidth + pad),
                                                bounds.getY() + pad,
                                                digitWidth,
                                                digitHeight);

            uint8_t mask = segPatterns[digits[i]];
            // If leading zero on first digit, blank it
            if (i == 0 && digits[0] == 0)
                mask = 0;

            const float w = dRect.getWidth();
            const float h = dRect.getHeight();
            const float th = 2.4f; // segment thickness
            const float slant = 1.5f;

            // Coordinates for 7 segments
            // a: top
            // b: top-right
            // c: bottom-right
            // d: bottom
            // e: bottom-left
            // f: top-left
            // g: middle
            auto drawSegment = [&](int segIndex, const juce::Rectangle<float>& r)
            {
                bool lit = (mask & (1 << segIndex)) != 0;
                if (lit)
                {
                    // Bloom
                    g.setColour(theme.displayGlow.withAlpha(0.35f));
                    g.fillRoundedRectangle(r.expanded(1.2f), 1.0f);
                    // Filament
                    g.setColour(theme.displayLit);
                    g.fillRoundedRectangle(r, 0.8f);
                }
                else
                {
                    // Ghost unlit segment
                    g.setColour(theme.displayUnlit);
                    g.fillRoundedRectangle(r, 0.8f);
                }
            };

            float x0 = dRect.getX() + slant;
            float y0 = dRect.getY();
            float halfH = h * 0.5f;

            // a (top)
            drawSegment(0, { x0 + th + 1.0f, y0 + 1.0f, w - (th * 2.0f + 2.0f), th });
            // b (top-right)
            drawSegment(1, { x0 + w - th - 1.0f, y0 + th + 1.5f, th, halfH - th - 2.0f });
            // c (bottom-right)
            drawSegment(2, { x0 + w - th - 1.0f, y0 + halfH + 1.5f, th, halfH - th - 2.0f });
            // d (bottom)
            drawSegment(3, { x0 + th + 1.0f, y0 + h - th - 1.0f, w - (th * 2.0f + 2.0f), th });
            // e (bottom-left)
            drawSegment(4, { x0 + 1.0f, y0 + halfH + 1.5f, th, halfH - th - 2.0f });
            // f (top-left)
            drawSegment(5, { x0 + 1.0f, y0 + th + 1.5f, th, halfH - th - 2.0f });
            // g (middle)
            drawSegment(6, { x0 + th + 1.0f, y0 + halfH - th * 0.5f, w - (th * 2.0f + 2.0f), th });
        }
    }
}
