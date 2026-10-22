#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../Common909.h"
#include "../DSP/DrumEngine909.h"
#include "../DSP/GrooveSequencer909.h"
#include "Theme909.h"

namespace Replicant909
{
    class PatternDrawer909 : public juce::Component
    {
    public:
        static constexpr int DRAWER_HEIGHT = 380;

        PatternDrawer909(juce::AudioProcessorValueTreeState& apvts,
                         DrumEngine909& engine,
                         GrooveSequencer909& sequencer);
        ~PatternDrawer909() override = default;

        void paint(juce::Graphics& g) override;
        void resized() override;

        void mouseDown(const juce::MouseEvent& e) override;
        void lookAndFeelChanged() override { repaint(); }

        void refresh();

        std::function<void()> onCloseRequested;

    private:
        juce::AudioProcessorValueTreeState& apvts;
        DrumEngine909& engine;
        GrooveSequencer909& sequencer;

        // --- Algorithmic Pattern Generator Controls ---
        juce::ComboBox genreSelector;
        juce::Slider kickDensitySlider;
        juce::Slider snareDensitySlider;
        juce::Slider hatDensitySlider;
        juce::Slider percDensitySlider;
        juce::Slider accentProbSlider;
        juce::Slider flamProbSlider;

        juce::TextButton generateButton;
        juce::TextButton mutateButton;
        juce::TextButton clearButton;

        // --- Multi-Track Matrix Quick Tools ---
        juce::TextButton fill4x4KickBtn;
        juce::TextButton fillOffbeatHatBtn;
        juce::TextButton fillBackbeatClapBtn;
        juce::TextButton shiftLeftBtn;
        juce::TextButton shiftRightBtn;
        juce::TextButton exportMidiBtn;
        juce::TextButton closeDrawerBtn;

        // Selected track for quick actions
        int selectedTrack = 0;

        // Cell layout cache for matrix grid
        juce::Rectangle<float> matrixBounds;
        float rowHeight = 22.0f;
        float colWidth = 36.0f;
        float headerWidth = 72.0f;

        void generateAlgorithmicPattern(int genre);
        void mutatePattern(float mutationRate);
        void shiftPattern(int trackIndex, int stepsShift);
        void exportActivePatternToMidi();

        void drawMatrixGrid(juce::Graphics& g);
        void drawGeneratorPanel(juce::Graphics& g);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatternDrawer909)
    };
}
