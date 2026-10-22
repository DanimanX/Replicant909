#include "StepSequencerComponent.h"
#include "LookAndFeel909.h"

namespace Replicant909
{
    // =========================================================================
    // StepButtonComponent
    // =========================================================================
    StepButtonComponent::StepButtonComponent(int idx, GrooveSequencer909& seq)
        : step(idx), sequencer(seq)
    {
    }

    void StepButtonComponent::paint(juce::Graphics& g)
    {
        const auto& theme = ThemeManager::getInstance().getCurrentTheme();
        auto bounds = getLocalBounds().toFloat().reduced(2.0f);

        // Tactile rectangular key body
        // Every 4th step group has subtle shade differentiation like original 909 (1-4 beige, 5-8 grey/orange, etc.)
        bool isAltGroup = ((step / 4) % 2) != 0;
        juce::Colour keyTop = isAltGroup ? theme.buttonTop.darker(0.06f) : theme.buttonTop;
        juce::Colour keyBot = isAltGroup ? theme.buttonBottom.darker(0.06f) : theme.buttonBottom;

        juce::ColourGradient grad(keyTop, bounds.getX(), bounds.getY(),
                                  keyBot, bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(bounds, 3.5f);

        g.setColour(theme.buttonBorder);
        g.drawRoundedRectangle(bounds, 3.5f, 1.0f);

        // Top edge bevel highlight
        g.setColour(theme.buttonBevel.withAlpha(0.6f));
        g.drawHorizontalLine(static_cast<int>(bounds.getY() + 1.0f), bounds.getX() + 2.0f, bounds.getRight() - 2.0f);

        // Step number label on bottom of key
        g.setColour(theme.silkscreenText);
        g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
        g.drawText(juce::String(step + 1), bounds.removeFromBottom(16.0f), juce::Justification::centred);

        // Dual-state LED and Playhead indicator
        uint8_t stepState = sequencer.getActivePattern().getStep(currentTrack, step);
        bool isPlayhead = sequencer.isPlaying() && (sequencer.getCurrentStep() == step);
        LookAndFeel909::drawStepLed(g, bounds, stepState, isPlayhead, theme);
    }

    void StepButtonComponent::mouseDown(const juce::MouseEvent& e)
    {
        if (e.mods.isRightButtonDown())
        {
            // Right-click sets/clears Flam
            uint8_t cur = sequencer.getActivePattern().getStep(currentTrack, step);
            sequencer.getActivePattern().setStep(currentTrack, step, (cur == STEP_FLAM ? STEP_OFF : STEP_FLAM));
        }
        else
        {
            // Left-click cycles: Off -> Normal -> Accent -> Off
            sequencer.getActivePattern().toggleStep(currentTrack, step);
        }
        repaint();
    }

    // =========================================================================
    // StepSequencerComponent
    // =========================================================================
    StepSequencerComponent::StepSequencerComponent(juce::AudioProcessorValueTreeState& state,
                                                   GrooveSequencer909& seq)
        : apvts(state), sequencer(seq)
    {
        // 1. Track Selector Buttons
        for (int i = 0; i < NUM_TRACKS; ++i)
        {
            trackButtons[i] = std::make_unique<juce::TextButton>();
            juce::String name = (i < NUM_VOICES) ? VOICE_SHORT_NAMES[i] : "ACC";
            trackButtons[i]->setButtonText(name);
            trackButtons[i]->onClick = [this, i]() {
                activeTrack = i;
                updateTrackSelection();
            };
            addAndMakeVisible(trackButtons[i].get());
        }

        // 2. 16 Step Buttons
        for (int s = 0; s < NUM_STEPS; ++s)
        {
            stepButtons[s] = std::make_unique<StepButtonComponent>(s, sequencer);
            addAndMakeVisible(stepButtons[s].get());
        }

        // 3. Shuffle Slider (Settings 1..6)
        shuffleSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        shuffleSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        shuffleSlider.setRange(1.0, 6.0, 1.0);
        shuffleSlider.setValue(sequencer.getShuffle());
        shuffleSlider.onValueChange = [this]() {
            sequencer.setShuffle(static_cast<int>(shuffleSlider.getValue()));
            repaint();
        };
        addAndMakeVisible(shuffleSlider);
        shuffleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, "SHUFFLE", shuffleSlider);

        // 4. Flam Interval Slider (Settings 1..8)
        flamSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        flamSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        flamSlider.setRange(1.0, 8.0, 1.0);
        flamSlider.setValue(sequencer.getFlamInterval());
        flamSlider.onValueChange = [this]() {
            sequencer.setFlamInterval(static_cast<int>(flamSlider.getValue()));
            repaint();
        };
        addAndMakeVisible(flamSlider);
        flamAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, "FLAM", flamSlider);

        // 5. Clear Pattern Button
        clearPatternButton.setButtonText("CLEAR");
        clearPatternButton.onClick = [this]() {
            sequencer.getActivePattern().clear();
            for (auto& sb : stepButtons)
                sb->repaint();
        };
        addAndMakeVisible(clearPatternButton);

        updateTrackSelection();
    }

    void StepSequencerComponent::updateTrackSelection()
    {
        for (int i = 0; i < NUM_TRACKS; ++i)
        {
            trackButtons[i]->setColour(juce::TextButton::buttonColourId,
                                      (i == activeTrack) ? ThemeManager::getInstance().getCurrentTheme().accentPrimary
                                                         : ThemeManager::getInstance().getCurrentTheme().buttonTop);
        }
        for (auto& sb : stepButtons)
        {
            sb->setSelectedTrack(activeTrack);
            sb->repaint();
        }
        repaint();
    }

    void StepSequencerComponent::paint(juce::Graphics& g)
    {
        const auto& theme = ThemeManager::getInstance().getCurrentTheme();
        auto bounds = getLocalBounds().toFloat();

        // Background chassis bar
        g.setColour(theme.chassisBottom.darker(0.12f));
        g.fillRect(bounds);

        // Top divider line
        g.setColour(theme.silkscreenLines);
        g.drawHorizontalLine(0, 0.0f, bounds.getWidth());

        // Silkscreen section labels
        g.setColour(theme.silkscreenText);
        g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
        g.drawText("INSTRUMENT TRACK SELECT", 10, 6, 200, 14, juce::Justification::left);
        g.drawText("STEP SEQUENCER & GROOVE", 10, 42, 200, 14, juce::Justification::left);

        // Shuffle & Flam labels
        int rightX = getWidth() - 170;
        g.drawText("SHUFFLE", rightX, 6, 48, 12, juce::Justification::centred);
        g.drawText("FLAM", rightX + 55, 6, 48, 12, juce::Justification::centred);

        // Current shuffle number display
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.setColour(theme.accentPrimary);
        g.drawText(juce::String(sequencer.getShuffle()), rightX, 58, 48, 14, juce::Justification::centred);
        g.drawText(juce::String(sequencer.getFlamInterval()), rightX + 55, 58, 48, 14, juce::Justification::centred);
    }

    void StepSequencerComponent::resized()
    {
        // Row 1: Track Selector Buttons
        float trackBtnWidth = (static_cast<float>(getWidth()) - 180.0f) / static_cast<float>(NUM_TRACKS);
        for (int i = 0; i < NUM_TRACKS; ++i)
        {
            trackButtons[i]->setBounds(10 + static_cast<int>(i * trackBtnWidth), 20,
                                       static_cast<int>(trackBtnWidth - 2.0f), 20);
        }

        // Row 2: 16 Step Buttons
        float stepWidth = (static_cast<float>(getWidth()) - 180.0f) / static_cast<float>(NUM_STEPS);
        for (int s = 0; s < NUM_STEPS; ++s)
        {
            stepButtons[s]->setBounds(10 + static_cast<int>(s * stepWidth), 56,
                                      static_cast<int>(stepWidth - 3.0f), 48);
        }

        // Right side: Groove controls
        int rightX = getWidth() - 170;
        shuffleSlider.setBounds(rightX + 6, 20, 36, 36);
        flamSlider.setBounds(rightX + 61, 20, 36, 36);
        clearPatternButton.setBounds(rightX + 110, 32, 50, 24);
    }

    void StepSequencerComponent::updatePlayhead()
    {
        for (auto& sb : stepButtons)
            sb->repaint();
    }
}
