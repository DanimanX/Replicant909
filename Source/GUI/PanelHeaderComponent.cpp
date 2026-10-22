#include "PanelHeaderComponent.h"
#include "LookAndFeel909.h"

namespace Replicant909
{
    PanelHeaderComponent::PanelHeaderComponent(juce::AudioProcessorValueTreeState& state,
                                               DrumEngine909& eng,
                                               GrooveSequencer909& seq)
        : apvts(state), engine(eng), sequencer(seq)
    {
        // 1. Tempo Knob
        tempoSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        tempoSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        tempoSlider.setRange(30.0, 300.0, 0.1);
        tempoSlider.setValue(sequencer.getBpm());
        tempoSlider.onValueChange = [this]() {
            sequencer.setBpm(tempoSlider.getValue());
            repaint();
        };
        addAndMakeVisible(tempoSlider);
        tempoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, "TEMPO", tempoSlider);

        // 2. Master Volume
        masterVolumeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        masterVolumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        masterVolumeSlider.setRange(0.0, 1.0, 0.01);
        masterVolumeSlider.setValue(engine.getMasterVolume());
        masterVolumeSlider.onValueChange = [this]() {
            engine.setMasterVolume(static_cast<float>(masterVolumeSlider.getValue()));
        };
        addAndMakeVisible(masterVolumeSlider);
        masterVolAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, "MASTER_VOL", masterVolumeSlider);

        // 3. Total Accent
        totalAccentSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        totalAccentSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        totalAccentSlider.setRange(0.0, 1.0, 0.01);
        totalAccentSlider.setValue(engine.getAccentCircuit().getAccentLevel());
        totalAccentSlider.onValueChange = [this]() {
            engine.getAccentCircuit().setAccentLevel(static_cast<float>(totalAccentSlider.getValue()));
        };
        addAndMakeVisible(totalAccentSlider);
        totalAccentAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, "TOTAL_ACCENT", totalAccentSlider);

        // 4. Play / Stop Button
        playStopButton.setButtonText("RUN / STOP");
        playStopButton.onClick = [this]() {
            if (sequencer.isPlaying())
                sequencer.stop();
            else
                sequencer.start();
            repaint();
        };
        addAndMakeVisible(playStopButton);

        // 5. Tap Tempo Button
        tapTempoButton.setButtonText("TAP");
        tapTempoButton.onClick = [this]() {
            sequencer.tapTempo();
            tempoSlider.setValue(sequencer.getBpm(), juce::dontSendNotification);
            repaint();
        };
        addAndMakeVisible(tapTempoButton);

        // 6. Sync Host Toggle
        syncHostToggle.setButtonText("SYNC");
        syncHostToggle.setToggleState(sequencer.isSyncToHost(), juce::dontSendNotification);
        syncHostToggle.onClick = [this]() {
            sequencer.setSyncToHost(syncHostToggle.getToggleState());
        };
        addAndMakeVisible(syncHostToggle);
        syncHostAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            apvts, "SYNC_HOST", syncHostToggle);

        // 7. Jack Cut Mode Toggle
        jackCutToggle.setButtonText("JACK CUT");
        jackCutToggle.setToggleState(engine.getJackCutMode(), juce::dontSendNotification);
        jackCutToggle.onClick = [this]() {
            engine.setJackCutMode(jackCutToggle.getToggleState());
        };
        addAndMakeVisible(jackCutToggle);

        // 8. Skin Selector ComboBox
        skinSelector.addItem("Classic 909", SKIN_RETRO_CLASSIC + 1);
        skinSelector.addItem("Modern Obsidian", SKIN_MODERN_OBSIDIAN + 1);
        skinSelector.setSelectedId(ThemeManager::getInstance().getActiveSkin() + 1, juce::dontSendNotification);
        skinSelector.onChange = [this]() {
            int id = skinSelector.getSelectedId() - 1;
            ThemeManager::getInstance().setSkin(static_cast<SkinId>(id));
            if (auto* top = getTopLevelComponent())
                top->repaint();
        };
        addAndMakeVisible(skinSelector);

        // 9. Bank & Pattern Selectors
        bankSelector.addItem("BANK A", 1);
        bankSelector.addItem("BANK B", 2);
        bankSelector.setSelectedId(sequencer.getSelectedBank() + 1, juce::dontSendNotification);
        bankSelector.onChange = [this]() {
            sequencer.selectBank(bankSelector.getSelectedId() - 1);
            if (auto* p = getParentComponent())
                p->repaint();
        };
        addAndMakeVisible(bankSelector);

        for (int p = 1; p <= 16; ++p)
            patternSelector.addItem("PATTERN " + juce::String(p), p);
        patternSelector.setSelectedId(sequencer.getSelectedPattern() + 1, juce::dontSendNotification);
        patternSelector.onChange = [this]() {
            sequencer.selectPattern(patternSelector.getSelectedId() - 1);
            if (auto* p = getParentComponent())
                p->repaint();
        };
        addAndMakeVisible(patternSelector);

        ThemeManager::getInstance().addListener([this]() {
            repaint();
        });
    }

    void PanelHeaderComponent::paint(juce::Graphics& g)
    {
        const auto& theme = ThemeManager::getInstance().getCurrentTheme();
        auto bounds = getLocalBounds().toFloat();

        // Top decorative accent bar
        g.setColour(theme.accentPrimary);
        g.fillRect(bounds.getX(), bounds.getY(), bounds.getWidth(), 3.5f);

        // Secondary stripe
        g.setColour(theme.accentSecondary);
        g.fillRect(bounds.getX(), bounds.getY() + 3.5f, bounds.getWidth(), 2.0f);

        // Brand Typography
        g.setColour(theme.silkscreenHeader);
        g.setFont(juce::FontOptions(22.0f, juce::Font::bold));
        g.drawText("REPLICANT 909", 20, 10, 220, 24, juce::Justification::left, true);

        g.setColour(theme.silkscreenText);
        g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
        g.drawText("CIRCUIT CLONE RHYTHM COMPOSER", 20, 32, 220, 14, juce::Justification::left, true);

        // Draw 7-Segment Tempo Display
        auto displayRect = juce::Rectangle<float>(260.0f, 12.0f, 75.0f, 32.0f);
        int currentBpmInt = static_cast<int>(std::round(sequencer.getBpm()));
        LookAndFeel909::drawSevenSegmentDisplay(g, displayRect, currentBpmInt, theme);

        // Silkscreen section labels
        g.setFont(juce::FontOptions(9.0f, juce::Font::bold));
        g.setColour(theme.silkscreenText);
        g.drawText("TEMPO", 345, 6, 60, 12, juce::Justification::centred);
        g.drawText("TOTAL ACCENT", 415, 6, 80, 12, juce::Justification::centred);
        g.drawText("VOLUME", 505, 6, 60, 12, juce::Justification::centred);
        g.drawText("PATTERN VAULT", 575, 6, 175, 12, juce::Justification::centred);
        g.drawText("THEME SKIN", 760, 6, 120, 12, juce::Justification::centred);

        // Bottom divider line
        g.setColour(theme.silkscreenLines);
        g.drawHorizontalLine(getHeight() - 1, 0.0f, static_cast<float>(getWidth()));
    }

    void PanelHeaderComponent::resized()
    {
        // Layout:
        // Left: Logo & 7-Segment Display
        // Tempo slider, Tap button, Sync toggle
        tempoSlider.setBounds(350, 18, 48, 48);
        tapTempoButton.setBounds(260, 48, 35, 18);
        syncHostToggle.setBounds(300, 48, 45, 18);

        // Accent slider
        totalAccentSlider.setBounds(430, 18, 48, 48);

        // Master Volume slider
        masterVolumeSlider.setBounds(510, 18, 48, 48);

        // Bank & Pattern Selectors
        bankSelector.setBounds(575, 20, 80, 22);
        patternSelector.setBounds(660, 20, 90, 22);
        playStopButton.setBounds(575, 46, 80, 20);
        jackCutToggle.setBounds(660, 46, 90, 20);

        // Skin Selector
        skinSelector.setBounds(765, 20, 110, 22);
    }
}
