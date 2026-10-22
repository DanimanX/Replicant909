#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../Common909.h"
#include "../DSP/DrumEngine909.h"
#include "Theme909.h"

namespace Replicant909
{
    class SingleVoiceStrip : public juce::Component
    {
    public:
        SingleVoiceStrip(int voiceIndex,
                         juce::AudioProcessorValueTreeState& apvts,
                         DrumEngine909& engine);
        ~SingleVoiceStrip() override = default;

        void paint(juce::Graphics& g) override;
        void resized() override;
        void mouseDown(const juce::MouseEvent& e) override;
        void lookAndFeelChanged() override { repaint(); }

        int getVoiceIndex() const noexcept { return voiceIndex; }

    private:
        int voiceIndex;
        juce::AudioProcessorValueTreeState& apvts;
        DrumEngine909& engine;

        // Sliders
        juce::Slider tuneSlider;
        juce::Slider levelSlider;
        juce::Slider attackOrToneSlider;
        juce::Slider decayOrSnappyOrReverbSlider;
        juce::Slider panSlider;

        // Trigger button
        juce::TextButton triggerButton;

        // Slider attachments
        std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SingleVoiceStrip)
    };

    class VoiceStripRackComponent : public juce::Component
    {
    public:
        VoiceStripRackComponent(juce::AudioProcessorValueTreeState& apvts,
                                DrumEngine909& engine);
        ~VoiceStripRackComponent() override = default;

        void paint(juce::Graphics& g) override;
        void resized() override;
        void lookAndFeelChanged() override;

        void updateMeters();

    private:
        juce::AudioProcessorValueTreeState& apvts;
        DrumEngine909& engine;
        std::array<std::unique_ptr<SingleVoiceStrip>, NUM_VOICES> strips;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoiceStripRackComponent)
    };
}
