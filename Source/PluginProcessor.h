#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Common909.h"
#include "DSP/DrumEngine909.h"
#include "DSP/GrooveSequencer909.h"
#include "Presets/PresetBank909.h"

namespace Replicant909
{
    class PluginProcessor : public juce::AudioProcessor
    {
    public:
        PluginProcessor();
        ~PluginProcessor() override;

        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void releaseResources() override;

        bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

        using juce::AudioProcessor::processBlock;
        void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

        juce::AudioProcessorEditor* createEditor() override;
        bool hasEditor() const override { return true; }

#ifndef JucePlugin_Name
#define JucePlugin_Name "Replicant909"
#endif

        const juce::String getName() const override { return JucePlugin_Name; }

        bool acceptsMidi() const override { return true; }
        bool producesMidi() const override { return true; }
        bool isMidiEffect() const override { return false; }
        double getTailLengthSeconds() const override { return 0.0; }

        int getNumPrograms() override { return presetBank.getNumPresets(); }
        int getCurrentProgram() override { return currentProgram; }
        void setCurrentProgram(int index) override;
        const juce::String getProgramName(int index) override;
        void changeProgramName(int index, const juce::String& newName) override;

        void getStateInformation(juce::MemoryBlock& destData) override;
        void setStateInformation(const void* data, int sizeInBytes) override;

        juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts; }
        DrumEngine909& getEngine() noexcept { return engine; }
        GrooveSequencer909& getSequencer() noexcept { return sequencer; }
        PresetBank909& getPresetBank() noexcept { return presetBank; }

        uint32_t getActiveBusesMask() const noexcept { return activeBusesMask; }

        static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    private:
        juce::AudioProcessorValueTreeState apvts;
        DrumEngine909 engine;
        GrooveSequencer909 sequencer;
        PresetBank909 presetBank;

        int currentProgram = 0;
        int lastSkinIndex = -1;
        uint32_t activeBusesMask = 1u; // Bit 0 (Master) always active

        // Reused individual bus pointer vector
        std::vector<juce::AudioBuffer<float>*> individualBusPtrs;

        void syncParametersToEngine();

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
    };
}
