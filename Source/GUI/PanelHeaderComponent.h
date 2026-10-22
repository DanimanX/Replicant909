#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "Theme909.h"
#include "../DSP/DrumEngine909.h"
#include "../DSP/GrooveSequencer909.h"
#include <functional>

namespace Replicant909
{
    class EditableTempoDisplay : public juce::Component,
                                 public juce::Label::Listener
    {
    public:
        EditableTempoDisplay(GrooveSequencer909& seq, juce::Slider& slider, juce::AudioProcessorValueTreeState& state);
        ~EditableTempoDisplay() override = default;

        void paint(juce::Graphics& g) override;
        void resized() override;

        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;

        void labelTextChanged(juce::Label* labelThatHasChanged) override;
        void editorShown(juce::Label* label, juce::TextEditor& ed) override;

        void updateDisplay();

    private:
        GrooveSequencer909& sequencer;
        juce::Slider& tempoSlider;
        juce::AudioProcessorValueTreeState& apvts;
        juce::Label tempoLabel;
        double dragStartBpm = 128.0;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EditableTempoDisplay)
    };

    class PanelHeaderComponent : public juce::Component
    {
    public:
        PanelHeaderComponent(juce::AudioProcessorValueTreeState& apvts,
                             DrumEngine909& engine,
                             GrooveSequencer909& sequencer);
        ~PanelHeaderComponent() override = default;

        void paint(juce::Graphics& g) override;
        void resized() override;

        void updateTempoDisplay();
        void setDrawerState(bool isOpen);

        std::function<void()> onToggleDrawer;

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

        // Drawer toggle button
        juce::TextButton patternDrawerButton;
        bool isDrawerOpen = false;

        // Editable tempo display
        std::unique_ptr<EditableTempoDisplay> tempoDisplay;

        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tempoAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterVolAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> totalAccentAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncHostAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> skinAttachment;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PanelHeaderComponent)
    };
}
