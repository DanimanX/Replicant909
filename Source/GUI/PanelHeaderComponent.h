#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "Theme909.h"
#include "../DSP/DrumEngine909.h"
#include "../DSP/GrooveSequencer909.h"

namespace Replicant909
{
    class PanelHeaderComponent : public juce::Component
    {
    public:
        PanelHeaderComponent(juce::AudioProcessorValueTreeState& apvts,
                             DrumEngine909& engine,
                             GrooveSequencer909& sequencer);
        ~PanelHeaderComponent() override = default;

        void paint(juce::Graphics& g) override;
        void resized() override;

    private:
        juce::AudioProcessorValueTreeState& apvts;
        DrumEngine909& engine;
        GrooveSequencer909& sequencer;

        // Header Controls
        juce::Slider tempoSlider;
        juce::Slider masterVolumeSlider;
        juce::Slider totalAccentSlider;

        juce::TextButton playStopButton;
        juce::TextButton tapTempoButton;
        juce::ToggleButton syncHostToggle;
        juce::ToggleButton jackCutToggle;

        juce::ComboBox skinSelector;
        juce::ComboBox bankSelector;
        juce::ComboBox patternSelector;

        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tempoAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterVolAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> totalAccentAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncHostAttachment;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PanelHeaderComponent)
    };
}
