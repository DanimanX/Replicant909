#pragma once

#include "Pattern909.h"
#include <functional>
#include <atomic>

namespace Replicant909
{
    class GrooveSequencer909
    {
    public:
        using StepTriggerCallback = std::function<void(int track, float velocity, float accent, bool isFlam)>;

        GrooveSequencer909();
        ~GrooveSequencer909() = default;

        void prepare(double sampleRate);
        void reset();

        // Process a block of samples (delivers trigger callbacks synchronously)
        void process(int numSamples, const juce::AudioPlayHead* playHead, StepTriggerCallback onTrigger);

        void start();
        void stop();
        bool isPlaying() const noexcept { return playing.load(std::memory_order_relaxed); }
        bool isTransportArmed() const noexcept { return playRequested.load(std::memory_order_relaxed); }

        void setBpm(double newBpm);
        double getBpm() const noexcept { return bpm; }

        void setSyncToHost(bool sync) noexcept { syncToHost = sync; }
        bool isSyncToHost() const noexcept    { return syncToHost; }

        int getCurrentStep() const noexcept
        {
            return currentStep.load(std::memory_order_relaxed);
        }

        int getSelectedPattern() const noexcept { return selectedPattern; }
        int getSelectedBank() const noexcept    { return selectedBank; }

        void selectPattern(int patternIndex);
        void selectBank(int bankIndex);

        Pattern& getActivePattern() noexcept;
        const Pattern& getActivePattern() const noexcept;

        Pattern& getPattern(int bank, int index) noexcept;
        const Pattern& getPattern(int bank, int index) const noexcept;

        void setShuffle(int shuffleVal); // 1..6
        int getShuffle() const noexcept;

        void setFlamInterval(int flamVal); // 1..8
        int getFlamInterval() const noexcept;

        // Tap tempo handler
        void tapTempo();

    private:
        double sampleRate = 44100.0;
        double bpm = 128.0;
        bool syncToHost = true;
        int lastHostStep = -1;

        std::atomic<bool> playRequested { false };
        std::atomic<bool> manualPlayWhileHostStopped { false };
        std::atomic<bool> playing { false };
        std::atomic<int> currentStep { 0 };

        int selectedBank = 0;
        int selectedPattern = 0;

        // 2 Banks of 16 Patterns (32 patterns total)
        std::array<std::array<Pattern, NUM_PATTERNS_PER_BANK>, NUM_BANKS> patternVault;

        // Clock phase tracking
        double samplesPerTick = 0.0;
        double tickAccumulator = 0.0;
        int stepSubTick = 0;

        // Flam pending triggers
        struct PendingFlam
        {
            int track = -1;
            int delaySamples = 0;
            float velocity = 1.0f;
            float accent = 0.0f;
            bool active = false;
        };
        std::array<PendingFlam, NUM_VOICES> pendingFlams;

        // Tap tempo tracking
        juce::int64 lastTapTimeMs = 0;
        std::vector<double> tapIntervals;

        void updateTiming();
        void advanceStep(StepTriggerCallback onTrigger);
    };
}
