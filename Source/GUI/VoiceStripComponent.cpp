#include "VoiceStripComponent.h"
#include "LookAndFeel909.h"

namespace Replicant909
{
    SingleVoiceStrip::SingleVoiceStrip(int idx,
                                       juce::AudioProcessorValueTreeState& state,
                                       DrumEngine909& eng)
        : voiceIndex(idx), apvts(state), engine(eng)
    {
        auto setupRotary = [this](juce::Slider& s) {
            s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            s.setRange(0.0, 1.0, 0.01);
            addAndMakeVisible(s);
        };

        juce::String prefix = VOICE_SHORT_NAMES[voiceIndex];

        // 1. Level Slider (all voices have Level)
        setupRotary(levelSlider);
        levelSlider.setValue(0.85);
        levelSlider.onValueChange = [this]() {
            if (auto* v = engine.getVoice(voiceIndex))
                v->setLevel(static_cast<float>(levelSlider.getValue()));
        };
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, prefix + "_LEVEL", levelSlider));

        // 2. Pan Slider
        setupRotary(panSlider);
        if (auto* v = engine.getVoice(voiceIndex))
            panSlider.setValue(v->getPan());
        panSlider.onValueChange = [this]() {
            if (auto* v = engine.getVoice(voiceIndex))
                v->setPan(static_cast<float>(panSlider.getValue()));
        };
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, prefix + "_PAN", panSlider));

        // 3. Tune Slider (BD, SD, LT, MT, HT, CC, RC)
        if (voiceIndex == BD || voiceIndex == SD || voiceIndex == LT ||
            voiceIndex == MT || voiceIndex == HT || voiceIndex == CC || voiceIndex == RC)
        {
            setupRotary(tuneSlider);
            tuneSlider.setValue(0.5);
            tuneSlider.onValueChange = [this]() {
                if (auto* v = engine.getVoice(voiceIndex))
                    v->setTune(static_cast<float>(tuneSlider.getValue()));
            };
            attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                apvts, prefix + "_TUNE", tuneSlider));
        }

        // 4. Attack (BD) or Tone (SD)
        if (voiceIndex == BD)
        {
            setupRotary(attackOrToneSlider);
            attackOrToneSlider.setValue(0.5);
            attackOrToneSlider.onValueChange = [this]() {
                if (auto* v = engine.getVoice(voiceIndex))
                    v->setAttack(static_cast<float>(attackOrToneSlider.getValue()));
            };
            attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                apvts, prefix + "_ATTACK", attackOrToneSlider));
        }
        else if (voiceIndex == SD)
        {
            setupRotary(attackOrToneSlider);
            attackOrToneSlider.setValue(0.5);
            attackOrToneSlider.onValueChange = [this]() {
                if (auto* v = engine.getVoice(voiceIndex))
                    v->setTone(static_cast<float>(attackOrToneSlider.getValue()));
            };
            attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                apvts, prefix + "_TONE", attackOrToneSlider));
        }

        // 5. Decay (BD, LT, MT, HT, CH, OH, CC, RC) or Snappy (SD) or Reverb (CP)
        if (voiceIndex == SD)
        {
            setupRotary(decayOrSnappyOrReverbSlider);
            decayOrSnappyOrReverbSlider.setValue(0.5);
            decayOrSnappyOrReverbSlider.onValueChange = [this]() {
                if (auto* v = engine.getVoice(voiceIndex))
                    v->setSnappy(static_cast<float>(decayOrSnappyOrReverbSlider.getValue()));
            };
            attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                apvts, prefix + "_SNAPPY", decayOrSnappyOrReverbSlider));
        }
        else if (voiceIndex == CP)
        {
            setupRotary(decayOrSnappyOrReverbSlider);
            decayOrSnappyOrReverbSlider.setValue(0.5);
            decayOrSnappyOrReverbSlider.onValueChange = [this]() {
                if (auto* clap = dynamic_cast<HandClapVoice*>(engine.getVoice(voiceIndex)))
                    clap->setReverb(static_cast<float>(decayOrSnappyOrReverbSlider.getValue()));
            };
            attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                apvts, prefix + "_REVERB", decayOrSnappyOrReverbSlider));
        }
        else if (voiceIndex != RS)
        {
            setupRotary(decayOrSnappyOrReverbSlider);
            decayOrSnappyOrReverbSlider.setValue(0.5);
            decayOrSnappyOrReverbSlider.onValueChange = [this]() {
                if (auto* v = engine.getVoice(voiceIndex))
                    v->setDecay(static_cast<float>(decayOrSnappyOrReverbSlider.getValue()));
            };
            attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                apvts, prefix + "_DECAY", decayOrSnappyOrReverbSlider));
        }

        // 6. Manual Audition Trigger Button
        triggerButton.setButtonText(VOICE_SHORT_NAMES[voiceIndex]);
        triggerButton.onClick = [this]() {
            engine.triggerVoice(voiceIndex, 0.95f, 0.5f);
            repaint();
        };
        addAndMakeVisible(triggerButton);
    }

    void SingleVoiceStrip::paint(juce::Graphics& g)
    {
        const auto& theme = ThemeManager::getInstance().getCurrentTheme();
        auto bounds = getLocalBounds().toFloat();

        // Right vertical channel divider line
        g.setColour(theme.silkscreenLines);
        g.drawVerticalLine(getWidth() - 1, 0.0f, static_cast<float>(getHeight()));

        // Strip Header Banner
        auto headerRect = juce::Rectangle<float>(0, 0, bounds.getWidth() - 1, 24);
        g.setColour((voiceIndex % 2 == 0) ? theme.accentPrimary : theme.accentSecondary);
        g.fillRect(headerRect.removeFromTop(3.0f));

        g.setColour(theme.silkscreenText);
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.drawText(VOICE_SHORT_NAMES[voiceIndex], 0, 4, getWidth() - 1, 16, juce::Justification::centred);

        // Labels for knobs
        g.setFont(juce::FontOptions(8.5f, juce::Font::bold));
        g.setColour(theme.silkscreenText.withAlpha(0.85f));

        int yOffset = 24;
        if (tuneSlider.isVisible())
        {
            g.drawText("TUNE", 0, yOffset, getWidth(), 12, juce::Justification::centred);
            yOffset += 48;
        }

        if (attackOrToneSlider.isVisible())
        {
            juce::String lbl = (voiceIndex == BD) ? "ATTACK" : "TONE";
            g.drawText(lbl, 0, yOffset, getWidth(), 12, juce::Justification::centred);
            yOffset += 48;
        }

        if (decayOrSnappyOrReverbSlider.isVisible())
        {
            juce::String lbl = "DECAY";
            if (voiceIndex == SD) lbl = "SNAPPY";
            else if (voiceIndex == CP) lbl = "REVERB";
            g.drawText(lbl, 0, yOffset, getWidth(), 12, juce::Justification::centred);
            yOffset += 48;
        }

        g.drawText("PAN", 0, yOffset, getWidth(), 12, juce::Justification::centred);
        yOffset += 48;

        g.drawText("LEVEL", 0, yOffset, getWidth(), 12, juce::Justification::centred);

        // Real-time Peak LED Meter above the trigger button
        if (auto* v = engine.getVoice(voiceIndex))
        {
            float peak = v->getPeak();
            float meterCx = static_cast<float>(getWidth()) * 0.5f;
            float meterCy = triggerButton.getY() - 10.0f;
            juce::Colour meterCol = theme.meterGreen;
            if (peak > 0.85f) meterCol = theme.meterRed;
            else if (peak > 0.5f) meterCol = theme.meterYellow;

            LookAndFeel909::drawLed(g, meterCx, meterCy, 3.5f, (peak > 0.05f), meterCol, theme.ledOff);
        }
    }

    void SingleVoiceStrip::resized()
    {
        int y = 36;
        int knobSize = 36;
        int xPos = (getWidth() - knobSize) / 2;

        if (tuneSlider.isVisible())
        {
            tuneSlider.setBounds(xPos, y, knobSize, knobSize);
            y += 48;
        }

        if (attackOrToneSlider.isVisible())
        {
            attackOrToneSlider.setBounds(xPos, y, knobSize, knobSize);
            y += 48;
        }

        if (decayOrSnappyOrReverbSlider.isVisible())
        {
            decayOrSnappyOrReverbSlider.setBounds(xPos, y, knobSize, knobSize);
            y += 48;
        }

        // Pan slider
        panSlider.setBounds(xPos, y, knobSize, knobSize);
        y += 48;

        // Level slider
        levelSlider.setBounds(xPos, y, knobSize, knobSize);

        // Trigger button at bottom of strip
        triggerButton.setBounds(xPos - 4, getHeight() - 38, knobSize + 8, 28);
    }

    // =========================================================================
    // VoiceStripRackComponent
    // =========================================================================
    VoiceStripRackComponent::VoiceStripRackComponent(juce::AudioProcessorValueTreeState& state,
                                                     DrumEngine909& eng)
        : apvts(state), engine(eng)
    {
        for (int i = 0; i < NUM_VOICES; ++i)
        {
            strips[i] = std::make_unique<SingleVoiceStrip>(i, apvts, engine);
            addAndMakeVisible(strips[i].get());
        }
    }

    void VoiceStripRackComponent::paint(juce::Graphics& g)
    {
        const auto& theme = ThemeManager::getInstance().getCurrentTheme();
        g.setColour(theme.silkscreenLines);
        g.drawHorizontalLine(getHeight() - 1, 0.0f, static_cast<float>(getWidth()));
    }

    void VoiceStripRackComponent::resized()
    {
        float stripWidth = static_cast<float>(getWidth()) / static_cast<float>(NUM_VOICES);
        for (int i = 0; i < NUM_VOICES; ++i)
        {
            strips[i]->setBounds(static_cast<int>(i * stripWidth), 0,
                                 static_cast<int>(stripWidth), getHeight());
        }
    }

    void VoiceStripRackComponent::updateMeters()
    {
        for (int i = 0; i < NUM_VOICES; ++i)
        {
            if (strips[i] != nullptr)
                strips[i]->repaint();
        }
    }
}
