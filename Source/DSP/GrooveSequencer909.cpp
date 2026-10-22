#include "GrooveSequencer909.h"
#include <algorithm>
#include <numeric>

namespace Replicant909
{
    GrooveSequencer909::GrooveSequencer909()
    {
        for (auto& bank : patternVault)
            for (auto& pat : bank)
                pat.clear();

        // Initialize default Factory Pattern 1 (Classic 909 4-on-the-floor House Groove)
        auto& p0 = patternVault[0][0];
        // BD on 1, 5, 9, 13 (0, 4, 8, 12)
        p0.setStep(BD, 0, STEP_ACCENT);
        p0.setStep(BD, 4, STEP_NORMAL);
        p0.setStep(BD, 8, STEP_ACCENT);
        p0.setStep(BD, 12, STEP_NORMAL);

        // SD on 4, 12 (snare backbeat on 2 and 4)
        p0.setStep(SD, 4, STEP_ACCENT);
        p0.setStep(SD, 12, STEP_ACCENT);

        // CH on off-beat 16ths: 2, 6, 10, 14
        p0.setStep(CH, 0, STEP_NORMAL);
        p0.setStep(CH, 1, STEP_NORMAL);
        p0.setStep(CH, 3, STEP_NORMAL);
        p0.setStep(CH, 4, STEP_NORMAL);
        p0.setStep(CH, 5, STEP_NORMAL);
        p0.setStep(CH, 7, STEP_NORMAL);
        p0.setStep(CH, 8, STEP_NORMAL);
        p0.setStep(CH, 9, STEP_NORMAL);
        p0.setStep(CH, 11, STEP_NORMAL);
        p0.setStep(CH, 12, STEP_NORMAL);
        p0.setStep(CH, 13, STEP_NORMAL);
        p0.setStep(CH, 15, STEP_NORMAL);

        // OH on upbeat 8ths: 2, 6, 10, 14
        p0.setStep(OH, 2, STEP_ACCENT);
        p0.setStep(OH, 6, STEP_ACCENT);
        p0.setStep(OH, 10, STEP_ACCENT);
        p0.setStep(OH, 14, STEP_ACCENT);

        // Clap on step 4 and 12
        p0.setStep(CP, 4, STEP_ACCENT);
        p0.setStep(CP, 12, STEP_ACCENT);

        // Rimshot syncopation
        p0.setStep(RS, 3, STEP_NORMAL);
        p0.setStep(RS, 11, STEP_NORMAL);

        // Total Accent track accents step 0, 4, 8, 12
        p0.setStep(TRACK_ACCENT, 0, STEP_ACCENT);
        p0.setStep(TRACK_ACCENT, 4, STEP_ACCENT);
        p0.setStep(TRACK_ACCENT, 8, STEP_ACCENT);
        p0.setStep(TRACK_ACCENT, 12, STEP_ACCENT);

        updateTiming();
    }

    void GrooveSequencer909::prepare(double newSampleRate)
    {
        sampleRate = std::max(8000.0, newSampleRate);
        updateTiming();
        reset();
    }

    void GrooveSequencer909::reset()
    {
        currentStep.store(0, std::memory_order_relaxed);
        lastHostStep = -1;
        tickAccumulator = 0.0;
        stepSubTick = 0;
        for (auto& f : pendingFlams)
            f.active = false;
    }

    void GrooveSequencer909::updateTiming()
    {
        // 16th note timing: 4 steps per beat
        // seconds per step = (60.0 / bpm) / 4.0 = 15.0 / bpm
        samplesPerTick = (15.0 / std::max(20.0, bpm)) * sampleRate;
    }

    void GrooveSequencer909::setBpm(double newBpm)
    {
        bpm = juce::jlimit(30.0, 300.0, newBpm);
        updateTiming();
    }

    void GrooveSequencer909::start()
    {
        reset();
        playing.store(true, std::memory_order_relaxed);
    }

    void GrooveSequencer909::stop()
    {
        playing.store(false, std::memory_order_relaxed);
        reset();
    }

    void GrooveSequencer909::selectPattern(int patternIndex)
    {
        selectedPattern = juce::jlimit(0, NUM_PATTERNS_PER_BANK - 1, patternIndex);
    }

    void GrooveSequencer909::selectBank(int bankIndex)
    {
        selectedBank = juce::jlimit(0, NUM_BANKS - 1, bankIndex);
    }

    Pattern& GrooveSequencer909::getActivePattern() noexcept
    {
        return patternVault[selectedBank][selectedPattern];
    }

    const Pattern& GrooveSequencer909::getActivePattern() const noexcept
    {
        return patternVault[selectedBank][selectedPattern];
    }

    Pattern& GrooveSequencer909::getPattern(int bank, int index) noexcept
    {
        return patternVault[juce::jlimit(0, NUM_BANKS - 1, bank)]
                           [juce::jlimit(0, NUM_PATTERNS_PER_BANK - 1, index)];
    }

    const Pattern& GrooveSequencer909::getPattern(int bank, int index) const noexcept
    {
        return patternVault[juce::jlimit(0, NUM_BANKS - 1, bank)]
                           [juce::jlimit(0, NUM_PATTERNS_PER_BANK - 1, index)];
    }

    void GrooveSequencer909::setShuffle(int shuffleVal)
    {
        getActivePattern().shuffle = static_cast<uint8_t>(juce::jlimit(1, 6, shuffleVal));
    }

    int GrooveSequencer909::getShuffle() const noexcept
    {
        return getActivePattern().shuffle;
    }

    void GrooveSequencer909::setFlamInterval(int flamVal)
    {
        getActivePattern().flamInterval = static_cast<uint8_t>(juce::jlimit(1, 8, flamVal));
    }

    int GrooveSequencer909::getFlamInterval() const noexcept
    {
        return getActivePattern().flamInterval;
    }

    void GrooveSequencer909::tapTempo()
    {
        auto now = juce::Time::currentTimeMillis();
        if (lastTapTimeMs > 0)
        {
            double diffSec = static_cast<double>(now - lastTapTimeMs) * 0.001;
            if (diffSec >= 0.18 && diffSec <= 2.0)
            {
                tapIntervals.push_back(diffSec);
                if (tapIntervals.size() > 5)
                    tapIntervals.erase(tapIntervals.begin());

                double avg = std::accumulate(tapIntervals.begin(), tapIntervals.end(), 0.0) / static_cast<double>(tapIntervals.size());
                double calculatedBpm = 60.0 / avg;
                setBpm(calculatedBpm);
            }
            else
            {
                tapIntervals.clear();
            }
        }
        lastTapTimeMs = now;
    }

    void GrooveSequencer909::process(int numSamples, const juce::AudioPlayHead* playHead, StepTriggerCallback onTrigger)
    {
        // 1. Sync with host transport if enabled
        if (syncToHost && playHead != nullptr)
        {
            if (auto pos = playHead->getPosition())
            {
                if (auto hostBpm = pos->getBpm())
                {
                    if (*hostBpm > 1.0 && std::abs(*hostBpm - bpm) > 0.01)
                        setBpm(*hostBpm);
                }

                if (pos->getIsPlaying())
                {
                    if (!playing.load(std::memory_order_relaxed))
                    {
                        start();
                        lastHostStep = -1;
                    }

                    if (auto ppq = pos->getPpqPosition())
                    {
                        // In 4/4 time, 1 beat = 1 quarter note = 4 16th notes
                        double total16ths = (*ppq) * 4.0;
                        int hostStep = static_cast<int>(std::floor(total16ths)) % getActivePattern().length;
                        if (hostStep < 0) hostStep = 0;
                        if (hostStep != lastHostStep)
                        {
                            lastHostStep = hostStep;
                            currentStep.store(hostStep, std::memory_order_relaxed);
                            advanceStep(onTrigger);
                        }
                    }
                    else if (auto timeInSec = pos->getTimeInSeconds())
                    {
                        double total16ths = (*timeInSec) * (bpm / 60.0) * 4.0;
                        int hostStep = static_cast<int>(std::floor(total16ths)) % getActivePattern().length;
                        if (hostStep < 0) hostStep = 0;
                        if (hostStep != lastHostStep)
                        {
                            lastHostStep = hostStep;
                            currentStep.store(hostStep, std::memory_order_relaxed);
                            advanceStep(onTrigger);
                        }
                    }
                }
                else
                {
                    if (playing.load(std::memory_order_relaxed))
                    {
                        stop();
                        lastHostStep = -1;
                    }
                }
            }
        }

        // 2. Process pending flams
        for (auto& f : pendingFlams)
        {
            if (f.active)
            {
                f.delaySamples -= numSamples;
                if (f.delaySamples <= 0)
                {
                    f.active = false;
                    if (onTrigger && f.track >= 0 && f.track < NUM_VOICES)
                        onTrigger(f.track, f.velocity, f.accent, false);
                }
            }
        }

        if (!playing.load(std::memory_order_relaxed))
            return;

        // If syncToHost is enabled and host playhead is active, host transport drove steps above
        if (syncToHost && playHead != nullptr)
            return;

        // 3. Internal Clock Step Scheduling with authentic TR-909 Shuffle
        // The TR-909 delays even 16th steps (1, 3, 5, 7, 9, 11, 13, 15)
        // Swing offsets for shuffle settings 1 to 6:
        // 1: 50.0% (0.00 offset)
        // 2: 54.2% (+0.084)
        // 3: 58.3% (+0.166)
        // 4: 62.5% (+0.250)
        // 5: 66.7% (+0.334)
        // 6: 70.8% (+0.416)
        static constexpr double shuffleDelays[6] = {
            0.0,
            0.084,
            0.166,
            0.250,
            0.334,
            0.416
        };

        const Pattern& pat = getActivePattern();
        int shuffleIdx = juce::jlimit(1, 6, static_cast<int>(pat.shuffle)) - 1;
        double swingFactor = shuffleDelays[shuffleIdx];

        int step = currentStep.load(std::memory_order_relaxed);
        bool isEvenStep = (step % 2) != 0; // 1, 3, 5... (0-indexed)

        double nominalStepLength = samplesPerTick;
        double currentStepLength = nominalStepLength;

        if (isEvenStep)
            currentStepLength = nominalStepLength * (1.0 + swingFactor);
        else
            currentStepLength = nominalStepLength * (1.0 - swingFactor);

        tickAccumulator += numSamples;
        while (tickAccumulator >= currentStepLength)
        {
            tickAccumulator -= currentStepLength;
            advanceStep(onTrigger);

            int nextStep = (currentStep.load(std::memory_order_relaxed) + 1) % pat.length;
            currentStep.store(nextStep, std::memory_order_relaxed);

            // Recompute step length for next step swing
            isEvenStep = (nextStep % 2) != 0;
            currentStepLength = isEvenStep ? nominalStepLength * (1.0 + swingFactor)
                                           : nominalStepLength * (1.0 - swingFactor);
        }
    }

    void GrooveSequencer909::advanceStep(StepTriggerCallback onTrigger)
    {
        if (!onTrigger)
            return;

        const Pattern& pat = getActivePattern();
        int step = currentStep.load(std::memory_order_relaxed);
        if (step < 0 || step >= pat.length)
            return;

        // Check if global Accent track is active on this step
        bool globalStepAccent = (pat.getStep(TRACK_ACCENT, step) == STEP_ACCENT);

        // Check and trigger each voice
        for (int track = 0; track < NUM_VOICES; ++track)
        {
            uint8_t state = pat.getStep(track, step);
            if (state == STEP_OFF)
                continue;

            float velocity = 0.85f;
            float stepAccent = globalStepAccent ? 1.0f : 0.0f;
            bool isFlam = (state == STEP_FLAM);

            if (state == STEP_ACCENT)
            {
                velocity = 1.0f;
                stepAccent = 1.0f;
            }

            // In TR-909, a Flam step triggers a softer first strike, followed by the main accented hit
            if (isFlam)
            {
                // Soft initial strike
                onTrigger(track, 0.55f, 0.0f, true);

                // Queue second accented strike delayed by flam interval (10ms to 45ms)
                int flamSamples = static_cast<int>(sampleRate * (0.010 + 0.005 * pat.flamInterval));
                pendingFlams[track] = { track, flamSamples, 1.0f, 1.0f, true };
            }
            else
            {
                onTrigger(track, velocity, stepAccent, false);
            }
        }
    }
}
