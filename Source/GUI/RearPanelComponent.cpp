#include "RearPanelComponent.h"
#include <cmath>

namespace Replicant909
{
    RearPanelComponent::RearPanelComponent(DrumEngine909& eng)
        : engine(eng)
    {
    }

    void RearPanelComponent::paint(juce::Graphics& g)
    {
        const auto& theme = ThemeManager::getInstance().getCurrentTheme();
        auto bounds = getLocalBounds().toFloat();

        // Dark anodized rear metal panel
        g.setColour(theme.chassisBottom.darker(0.35f));
        g.fillRect(bounds);

        // Top edge shadow
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.drawHorizontalLine(0, 0.0f, bounds.getWidth());

        // Header silkscreen
        g.setColour(theme.silkscreenText);
        g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
        g.drawText("REAR PANEL - MULTI-BUS INDIVIDUAL AUDIO OUTPUTS & MIDI INTERFACE",
                   15, 4, bounds.getWidth() - 30, 14, juce::Justification::left);

        // Draw Master Stereo Outs (Left/Mono, Right)
        float masterY = 32.0f;
        drawJack(g, 40.0f, masterY, "MASTER L (MONO)", true, theme);
        drawJack(g, 90.0f, masterY, "MASTER R", true, theme);

        // Draw 10 Individual Outs
        static constexpr const char* indLabels[10] = {
            "BD", "SD", "LT", "MT", "HT", "RS", "CP", "HI-HAT", "CRASH", "RIDE"
        };

        float startX = 160.0f;
        float spacing = 52.0f;

        for (int i = 0; i < 10; ++i)
        {
            float jackX = startX + static_cast<float>(i) * spacing;
            // Bus index = i + 1 (1 to 10)
            bool isConnected = (activeMask & (1u << (i + 1))) != 0;
            drawJack(g, jackX, masterY, indLabels[i], isConnected, theme);
        }

        // Draw MIDI DIN-5 ports (IN, OUT, THRU)
        float midiStartX = startX + 10.0f * spacing + 30.0f;
        drawDinPort(g, midiStartX, masterY, "MIDI IN", theme);
        drawDinPort(g, midiStartX + 45.0f, masterY, "MIDI OUT", theme);
        drawDinPort(g, midiStartX + 90.0f, masterY, "MIDI THRU", theme);

        // Manufacturer badge
        g.setColour(theme.silkscreenText.withAlpha(0.6f));
        g.setFont(juce::FontOptions(8.5f, juce::Font::plain));
        g.drawText("ROLAND CORP. TR-909 / REPLICANT-909 EMULATION ENGINE (DANIMANX)",
                   bounds.getWidth() - 380, bounds.getHeight() - 14, 365, 12, juce::Justification::right);
    }

    void RearPanelComponent::drawJack(juce::Graphics& g, float cx, float cy,
                                      const juce::String& label, bool plugged,
                                      const Theme909& theme)
    {
        float r = 11.0f;

        // Outer threaded chrome/metal nut
        g.setColour(juce::Colour(0xff757a82));
        g.fillEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f);
        g.setColour(juce::Colour(0xffa5acb5));
        g.drawEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f, 1.2f);

        // Inner black jack hole or plugged metal barrel
        if (plugged)
        {
            // Inserted 1/4" plug tip
            g.setColour(theme.accentPrimary);
            g.fillEllipse(cx - r * 0.6f, cy - r * 0.6f, r * 1.2f, r * 1.2f);
            g.setColour(juce::Colours::white.withAlpha(0.7f));
            g.fillEllipse(cx - r * 0.25f, cy - r * 0.25f, r * 0.5f, r * 0.5f);
        }
        else
        {
            // Empty jack hole
            g.setColour(juce::Colour(0xff0e1013));
            g.fillEllipse(cx - r * 0.65f, cy - r * 0.65f, r * 1.3f, r * 1.3f);
        }

        // Label below jack
        g.setColour(theme.silkscreenText);
        g.setFont(juce::FontOptions(8.0f, juce::Font::bold));
        g.drawText(label, cx - 25.0f, cy + r + 2.0f, 50.0f, 10.0f, juce::Justification::centred);
    }

    void RearPanelComponent::drawDinPort(juce::Graphics& g, float cx, float cy,
                                         const juce::String& label,
                                         const Theme909& theme)
    {
        float r = 12.0f;
        // Metal DIN socket ring
        g.setColour(juce::Colour(0xff22252a));
        g.fillEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f);
        g.setColour(juce::Colour(0xff889098));
        g.drawEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f, 1.0f);

        // 5 contact pins
        g.setColour(juce::Colour(0xffd0b840));
        for (int p = 0; p < 5; ++p)
        {
            float ang = -2.3f + static_cast<float>(p) * 1.15f;
            float px = cx + (r * 0.55f) * std::cos(ang);
            float py = cy + (r * 0.55f) * std::sin(ang);
            g.fillEllipse(px - 1.2f, py - 1.2f, 2.4f, 2.4f);
        }

        // Label below
        g.setColour(theme.silkscreenText);
        g.setFont(juce::FontOptions(8.0f, juce::Font::bold));
        g.drawText(label, cx - 22.0f, cy + r + 2.0f, 44.0f, 10.0f, juce::Justification::centred);
    }

    void RearPanelComponent::resized()
    {
    }
}
