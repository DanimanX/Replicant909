#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../DSP/GrooveSequencer909.h"
#include "Theme909.h"

namespace Replicant909
{
    class StepButtonComponent : public juce::Component
    {
    public:
        StepButtonComponent(int stepIndex, GrooveSequencer909& sequencer);
        ~StepButtonComponent() override = default;

        void paint(juce::Graphics& g) override;
        void mouseDown(const juce::MouseEvent& e) override;

        void setSelectedTrack(int track) noexcept { currentTrack = track; }

    private:
        int step;
        int currentTrack = 0;
        GrooveSequencer909& sequencer;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StepButtonComponent)
    };

    class StepSequencerComponent : public juce::Component
    {
    public:
        StepSequencerComponent(juce::AudioProcessorValueTreeState& apvts,
                               GrooveSequencer909& sequencer);
        ~StepSequencerComponent() override = default;

        void paint(juce::Graphics& g) override;
        void resized() override;
        void lookAndFeelChanged() override;

        void updatePlayhead();

    private:
        juce::AudioProcessorValueTreeState& apvts;
        GrooveSequencer909& sequencer;
        int activeTrack = 0;

        // Track selector buttons (BD..RC, ACCENT)
        std::array<std::unique_ptr<juce::TextButton>, NUM_TRACKS> trackButtons;

        // 16 Step buttons
        std::array<std::unique_ptr<StepButtonComponent>, NUM_STEPS> stepButtons;

        // Groove controls
        juce::Slider shuffleSlider;
        juce::Slider flamSlider;
        juce::TextButton clearPatternButton;

        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> shuffleAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> flamAttachment;

        void updateTrackSelection();

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StepSequencerComponent)
    };
}
