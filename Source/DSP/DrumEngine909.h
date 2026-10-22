#pragma once

#include "../Common909.h"
#include "VoiceBase.h"
#include "BassDrumVoice.h"
#include "SnareDrumVoice.h"
#include "TomVoice.h"
#include "RimShotVoice.h"
#include "HandClapVoice.h"
#include "CymbalVoice.h"
#include "TotalAccentCircuit.h"
#include <memory>
#include <vector>
#include <array>

namespace Replicant909
{
    class DrumEngine909
    {
    public:
        DrumEngine909();
        ~DrumEngine909() = default;

        void prepare(double sampleRate, int maxBlockSize);
        void reset();

        // Trigger an instrument voice directly (from sequencer or manual UI key)
        void triggerVoice(int voiceIndex, float velocity, float stepAccent = 0.0f, bool isFlam = false);

        // Process incoming MIDI buffer
        void processMidi(const juce::MidiBuffer& midiMessages);

        // Multi-bus render process:
        // masterBuffer: Main Stereo Output (Bus 0)
        // individualBuses: Array of pointers to individual output buffers (Bus 1..10)
        // activeBusesMask: Bitmask indicating which individual buses are enabled by the host
        void renderBlock(juce::AudioBuffer<float>& masterBuffer,
                         const std::vector<juce::AudioBuffer<float>*>& individualBuses,
                         uint32_t activeBusesMask);

        // Access to individual voices
        VoiceBase* getVoice(int index) noexcept;
        const VoiceBase* getVoice(int index) const noexcept;

        TotalAccentCircuit& getAccentCircuit() noexcept { return accentCircuit; }
        const TotalAccentCircuit& getAccentCircuit() const noexcept { return accentCircuit; }

        void setMasterVolume(float vol) noexcept { masterVolume = juce::jlimit(0.0f, 1.0f, vol); }
        float getMasterVolume() const noexcept   { return masterVolume; }

        void setJackCutMode(bool cut) noexcept   { jackCutMode = cut; }
        bool getJackCutMode() const noexcept     { return jackCutMode; }

        // Mute & Solo
        void setMuted(int voiceIndex, bool mute) noexcept
        {
            if (voiceIndex >= 0 && voiceIndex < NUM_VOICES)
                muted[voiceIndex] = mute;
        }

        bool isMuted(int voiceIndex) const noexcept
        {
            if (voiceIndex >= 0 && voiceIndex < NUM_VOICES)
                return muted[voiceIndex];
            return false;
        }

        void setSoloed(int voiceIndex, bool solo) noexcept
        {
            if (voiceIndex >= 0 && voiceIndex < NUM_VOICES)
                soloed[voiceIndex] = solo;
        }

        bool isSoloed(int voiceIndex) const noexcept
        {
            if (voiceIndex >= 0 && voiceIndex < NUM_VOICES)
                return soloed[voiceIndex];
            return false;
        }

        bool hasAnySolo() const noexcept
        {
            for (bool s : soloed)
                if (s) return true;
            return false;
        }

        bool isVoiceAudible(int voiceIndex) const noexcept
        {
            if (voiceIndex < 0 || voiceIndex >= NUM_VOICES)
                return false;
            if (muted[voiceIndex])
                return false;
            if (hasAnySolo())
                return soloed[voiceIndex];
            return true;
        }

    private:
        double sampleRate = 44100.0;
        float masterVolume = 0.85f;
        bool jackCutMode = false; // Default OFF so Master bus always outputs full mix in DAWs

        std::array<std::unique_ptr<VoiceBase>, NUM_VOICES> voices;
        std::array<bool, NUM_VOICES> muted {};
        std::array<bool, NUM_VOICES> soloed {};
        TotalAccentCircuit accentCircuit;

        // Temporary scratch buffer for rendering single voice across the block
        juce::AudioBuffer<float> voiceScratchBuffer;
    };
}
