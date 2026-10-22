#include "PanelHeaderComponent.h"
#include "LookAndFeel909.h"

namespace Replicant909
{
    // =========================================================================
    // EditableTempoDisplay Implementation
    // =========================================================================
    EditableTempoDisplay::EditableTempoDisplay(GrooveSequencer909& seq, juce::Slider& slider, juce::AudioProcessorValueTreeState& state)
        : sequencer(seq), tempoSlider(slider), apvts(state)
    {
        tempoLabel.setEditable(true, true, false);
        tempoLabel.addListener(this);
        tempoLabel.setJustificationType(juce::Justification::centred);
        tempoLabel.setFont(juce::FontOptions(15.0f, juce::Font::bold));
        tempoLabel.setColour(juce::Label::textColourId, juce::Colours::transparentBlack);
        tempoLabel.setColour(juce::Label::textWhenEditingColourId, juce::Colours::white);
        tempoLabel.setColour(juce::Label::backgroundWhenEditingColourId, juce::Colour(0xff0d0f12));
        tempoLabel.setColour(juce::TextEditor::textColourId, juce::Colours::white);
        tempoLabel.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff0d0f12));
        tempoLabel.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xfff36b21));
        tempoLabel.setTooltip("Click to type BPM directly, or drag up/down to nudge");
        addAndMakeVisible(tempoLabel);
        updateDisplay();
    }

    void EditableTempoDisplay::updateDisplay()
    {
        if (!tempoLabel.isBeingEdited())
        {
            tempoLabel.setText(juce::String(sequencer.getBpm(), 1), juce::dontSendNotification);
            repaint();
        }
    }

    void EditableTempoDisplay::paint(juce::Graphics& g)
    {
        const auto& theme = ThemeManager::getInstance().getCurrentTheme();
        auto bounds = getLocalBounds().toFloat();
        int currentBpmInt = static_cast<int>(std::round(sequencer.getBpm()));
        LookAndFeel909::drawSevenSegmentDisplay(g, bounds, currentBpmInt, theme);
    }

    void EditableTempoDisplay::resized()
    {
        tempoLabel.setBounds(getLocalBounds());
    }

    void EditableTempoDisplay::mouseDown(const juce::MouseEvent&)
    {
        dragStartBpm = sequencer.getBpm();
    }

    void EditableTempoDisplay::mouseDrag(const juce::MouseEvent& e)
    {
        if (e.mouseWasDraggedSinceMouseDown())
        {
            double delta = -e.getDistanceFromDragStartY() * 0.25;
            double newBpm = juce::jlimit(30.0, 300.0, dragStartBpm + delta);
            sequencer.setBpm(newBpm);
            if (auto* param = apvts.getParameter("TEMPO"))
                param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(newBpm)));
            tempoSlider.setValue(newBpm, juce::dontSendNotification);
            updateDisplay();
        }
    }

    void EditableTempoDisplay::labelTextChanged(juce::Label*)
    {
        double val = tempoLabel.getText().getDoubleValue();
        val = juce::jlimit(30.0, 300.0, val);
        sequencer.setBpm(val);
        if (auto* param = apvts.getParameter("TEMPO"))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(val)));
        tempoSlider.setValue(val, juce::dontSendNotification);
        updateDisplay();
    }

    void EditableTempoDisplay::editorShown(juce::Label*, juce::TextEditor& ed)
    {
        ed.setSelectAllWhenFocused(true);
    }

    // =========================================================================
    // PanelHeaderComponent Implementation
    // =========================================================================
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
            if (tempoDisplay != nullptr)
                tempoDisplay->updateDisplay();
            repaint();
        };
        addAndMakeVisible(tempoSlider);
        tempoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, "TEMPO", tempoSlider);

        // 2. Editable 7-Segment Tempo Display
        tempoDisplay = std::make_unique<EditableTempoDisplay>(sequencer, tempoSlider, apvts);
        addAndMakeVisible(tempoDisplay.get());

        // 3. Master Volume
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

        // 4. Total Accent
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

        // 5. Play / Stop Button
        playStopButton.setButtonText("RUN / STOP");
        playStopButton.onClick = [this]() {
            if (sequencer.isPlaying())
                sequencer.stop();
            else
                sequencer.start();
            repaint();
        };
        addAndMakeVisible(playStopButton);

        // 6. Tap Tempo Button
        tapTempoButton.setButtonText("TAP");
        tapTempoButton.onClick = [this]() {
            sequencer.tapTempo();
            tempoSlider.setValue(sequencer.getBpm(), juce::dontSendNotification);
            if (tempoDisplay != nullptr)
                tempoDisplay->updateDisplay();
            repaint();
        };
        addAndMakeVisible(tapTempoButton);

        // 7. Sync Host Toggle
        syncHostToggle.setButtonText("SYNC");
        syncHostToggle.setToggleState(sequencer.isSyncToHost(), juce::dontSendNotification);
        syncHostToggle.onClick = [this]() {
            sequencer.setSyncToHost(syncHostToggle.getToggleState());
        };
        addAndMakeVisible(syncHostToggle);
        syncHostAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            apvts, "SYNC_HOST", syncHostToggle);

        // 8. Jack Cut Mode Toggle
        jackCutToggle.setButtonText("JACK CUT");
        jackCutToggle.setToggleState(engine.getJackCutMode(), juce::dontSendNotification);
        jackCutToggle.onClick = [this]() {
            engine.setJackCutMode(jackCutToggle.getToggleState());
        };
        addAndMakeVisible(jackCutToggle);

        // 9. Skin Selector ComboBox (Attached to APVTS!)
        skinSelector.addItem("Classic 909", SKIN_RETRO_CLASSIC + 1);
        skinSelector.addItem("Modern Obsidian", SKIN_MODERN_OBSIDIAN + 1);
        skinSelector.setSelectedId(ThemeManager::getInstance().getActiveSkin() + 1, juce::dontSendNotification);
        skinAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            apvts, "SKIN", skinSelector);

        skinSelector.onChange = [this]() {
            int id = skinSelector.getSelectedId() - 1;
            if (id >= 0 && id < NUM_SKINS)
            {
                ThemeManager::getInstance().setSkin(static_cast<SkinId>(id));
                if (auto* top = getTopLevelComponent())
                {
                    top->sendLookAndFeelChange();
                    top->repaint();
                }
            }
        };
        addAndMakeVisible(skinSelector);

        // 10. Bank & Pattern Selectors
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

        // 11. Pattern Drawer Toggle Button
        patternDrawerButton.setButtonText("☵ PATTERN DRAWER");
        patternDrawerButton.onClick = [this]() {
            if (onToggleDrawer)
                onToggleDrawer();
        };
        addAndMakeVisible(patternDrawerButton);

        ThemeManager::getInstance().addListener([this]() {
            int skinIdx = ThemeManager::getInstance().getActiveSkin();
            skinSelector.setSelectedId(skinIdx + 1, juce::dontSendNotification);
            repaint();
        });
    }

    void PanelHeaderComponent::setDrawerState(bool isOpen)
    {
        isDrawerOpen = isOpen;
        patternDrawerButton.setButtonText(isOpen ? "✕ CLOSE DRAWER" : "☵ PATTERN DRAWER");
        repaint();
    }

    void PanelHeaderComponent::updateTempoDisplay()
    {
        if (tempoDisplay != nullptr)
            tempoDisplay->updateDisplay();
    }

    void PanelHeaderComponent::paint(juce::Graphics& g)
    {
        const auto& theme = ThemeManager::getInstance().getCurrentTheme();
        auto bounds = getLocalBounds().toFloat();

        // Top decorative accent bar
        g.setColour(theme.accentPrimary);
        g.fillRect(bounds.getX(), bounds.getY(), bounds.getWidth(), 3.0f);

        // Secondary stripe
        g.setColour(theme.accentSecondary);
        g.fillRect(bounds.getX(), bounds.getY() + 3.0f, bounds.getWidth(), 1.5f);

        // Brand Typography
        g.setColour(theme.silkscreenHeader);
        g.setFont(juce::FontOptions(20.0f, juce::Font::bold));
        g.drawText("REPLICANT 909", 18, 8, 180, 22, juce::Justification::left, true);

        g.setColour(theme.silkscreenText);
        g.setFont(juce::FontOptions(9.0f, juce::Font::bold));
        g.drawText("CIRCUIT CLONE RHYTHM COMPOSER", 18, 28, 180, 14, juce::Justification::left, true);

        // Silkscreen section labels
        g.setFont(juce::FontOptions(8.5f, juce::Font::bold));
        g.setColour(theme.silkscreenText.withAlpha(0.85f));
        g.drawText("TEMPO (CLICK TO TYPE)", 215, 6, 120, 12, juce::Justification::centred);
        g.drawText("TOTAL ACCENT", 380, 6, 75, 12, juce::Justification::centred);
        g.drawText("VOLUME", 460, 6, 50, 12, juce::Justification::centred);
        g.drawText("PATTERN VAULT", 525, 6, 155, 12, juce::Justification::centred);
        g.drawText("SKIN", 690, 6, 95, 12, juce::Justification::centred);

        // Bottom divider line
        g.setColour(theme.silkscreenLines);
        g.drawHorizontalLine(getHeight() - 1, 0.0f, static_cast<float>(getWidth()));
    }

    void PanelHeaderComponent::resized()
    {
        // 7-segment tempo display (clickable & editable)
        if (tempoDisplay != nullptr)
            tempoDisplay->setBounds(215, 18, 70, 26);

        tapTempoButton.setBounds(215, 46, 32, 16);
        syncHostToggle.setBounds(250, 46, 40, 16);

        // Tempo rotary knob
        tempoSlider.setBounds(290, 14, 46, 46);

        // Accent slider
        totalAccentSlider.setBounds(395, 14, 46, 46);

        // Master Volume slider
        masterVolumeSlider.setBounds(462, 14, 46, 46);

        // Bank & Pattern Selectors
        bankSelector.setBounds(525, 18, 72, 20);
        patternSelector.setBounds(602, 18, 78, 20);
        playStopButton.setBounds(525, 42, 72, 18);
        jackCutToggle.setBounds(602, 42, 78, 18);

        // Skin Selector
        skinSelector.setBounds(690, 18, 95, 20);

        // Pattern Drawer button
        patternDrawerButton.setBounds(getWidth() - 175, 16, 155, 28);
    }
}
