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

        // Note Multiplier / Ratchet Roll controls
        void setMultiplierEnabled(bool enabled) noexcept { multiplierEnabled = enabled; }
        bool isMultiplierEnabled() const noexcept        { return multiplierEnabled; }

        void setMultiplierCount(int count) noexcept      { multiplierCount = juce::jlimit(1, 16, count); }
        int getMultiplierCount() const noexcept          { return multiplierCount; }

        void setSelectedVoice(int voiceIndex) noexcept
        {
            if (voiceIndex >= 0 && voiceIndex < NUM_VOICES)
                selectedVoice = voiceIndex;
        }
        int getSelectedVoice() const noexcept            { return selectedVoice; }

        void setBpm(double bpm) noexcept
        {
            if (bpm >= 30.0 && bpm <= 300.0)
                currentBpm = bpm;
        }
        double getBpm() const noexcept                   { return currentBpm; }

        // Trigger voice with ratchet / roll multiplier
        void triggerVoiceMultiplied(int voiceIndex, float velocity, float stepAccent = 0.0f, int count = -1);

        // Trigger currently selected voice with multiplier if enabled
        void triggerSelectedVoice(float velocity = 0.95f, float stepAccent = 0.5f);

        // Continuous Hold Roll / Sustained Multiplier
        void startVoiceHold(int voiceIndex, float velocity = 0.95f, float stepAccent = 0.5f);
        void stopVoiceHold(int voiceIndex);
        void stopAllVoiceHolds();
        bool isVoiceHeld(int voiceIndex) const noexcept;

    private:
        struct MultiplierHit
        {
            int voiceIndex = 0;
            float velocity = 0.8f;
            float stepAccent = 0.0f;
            int samplesUntilTrigger = 0;
            bool active = false;
        };

        struct VoiceHoldState
        {
            bool isHeld = false;
            float velocity = 0.8f;
            float stepAccent = 0.0f;
            int samplesUntilNextHit = 0;
            int rollCounter = 0;
        };

        static constexpr int MAX_QUEUED_TRIGGERS = 128;
        std::array<MultiplierHit, MAX_QUEUED_TRIGGERS> queuedTriggers {};
        std::array<VoiceHoldState, NUM_VOICES> voiceHolds {};

        bool multiplierEnabled = false;
        int multiplierCount = 2; // Default 2x
        int selectedVoice = BD;  // Default BD
        double currentBpm = 128.0;

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
