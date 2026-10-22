#pragma once

#include "../Common909.h"
#include "../DSP/Pattern909.h"
#include <juce_core/juce_core.h>
#include <vector>

namespace Replicant909
{
    struct VoiceParams
    {
        float tune = 0.5f;
        float level = 0.85f;
        float attack = 0.5f;
        float decay = 0.5f;
        float snappy = 0.5f;
        float tone = 0.5f;
        float pan = 0.5f;
    };

    struct Preset909
    {
        juce::String name = "Default 909";
        juce::String category = "House";
        double bpm = 128.0;
        float totalAccent = 0.70f;
        float masterVolume = 0.85f;
        int shuffle = 1;
        int flamInterval = 3;

        std::array<VoiceParams, NUM_VOICES> voices;
        Pattern pattern;
    };

    class PresetBank909
    {
    public:
        PresetBank909();
        ~PresetBank909() = default;

        int getNumPresets() const noexcept { return static_cast<int>(presets.size()); }
        const Preset909& getPreset(int index) const noexcept;
        Preset909& getPreset(int index) noexcept;

        void addPreset(const Preset909& p);

    private:
        std::vector<Preset909> presets;
        void initFactoryPresets();
    };
}
