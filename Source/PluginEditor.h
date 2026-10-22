#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "GUI/LookAndFeel909.h"
#include "GUI/PanelHeaderComponent.h"
#include "GUI/VoiceStripComponent.h"
#include "GUI/StepSequencerComponent.h"
#include "GUI/RearPanelComponent.h"
#include "GUI/PatternDrawer909.h"

namespace Replicant909
{
    class PluginEditor : public juce::AudioProcessorEditor,
                         private juce::Timer
    {
    public:
        explicit PluginEditor(PluginProcessor& processor);
        ~PluginEditor() override;

        void paint(juce::Graphics& g) override;
        void resized() override;

        void setPatternDrawerOpen(bool shouldBeOpen);
        bool isPatternDrawerOpen() const noexcept { return drawerOpen; }

    private:
        PluginProcessor& processor;
        LookAndFeel909 lnf;

        // Subcomponents
        PanelHeaderComponent headerComponent;
        VoiceStripRackComponent voiceStripRack;
        StepSequencerComponent stepSequencer;
        RearPanelComponent rearPanel;
        std::unique_ptr<PatternDrawer909> patternDrawer;

        bool drawerOpen = false;

        // Toggle button for Rear Panel view
        juce::TextButton rearPanelToggleBtn;
        bool showRearPanel = false;

        void timerCallback() override;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
    };
}
