#include "SnareDrumVoice.h"
#include <algorithm>
#include <cmath>

namespace Replicant909
{
    SnareDrumVoice::SnareDrumVoice()
    {
        rng.seed(0x5d9095d1u);
    }

    void SnareDrumVoice::prepare(double newSampleRate)
    {
        sampleRate = std::max(8000.0, newSampleRate);
        reset();
    }

    void SnareDrumVoice::reset()
    {
        active = false;
        envTime = 0.0f;
        phase1 = 0.0f;
        phase2 = 0.0f;
        snappyFilter.reset();
        peakLevel.store(0.0f, std::memory_order_relaxed);
    }

    void SnareDrumVoice::trigger(float velocity, float totalAccent)
    {
        currentVelocity = juce::jlimit(0.05f, 1.0f, velocity);
        currentAccent = juce::jlimit(0.0f, 1.0f, totalAccent);
        envTime = 0.0f;
        phase1 = 0.0f;
        phase2 = 0.0f;
        snappyFilter.reset();
        active = true;
    }

    float SnareDrumVoice::renderSample()
    {
        if (!active)
            return 0.0f;

        const float dt = 1.0f / static_cast<float>(sampleRate);

        // 1. Tuning & Shell Resonances
        const float tuneRatio = std::pow(2.0f, (tune - 0.5f) * 1.5f);
        const float sweep = 1.0f + 0.60f * std::exp(-envTime * 90.0f);

        const float f1 = 185.0f * tuneRatio * sweep;
        const float f2 = 340.0f * tuneRatio * sweep;

        phase1 += f1 * dt;
        if (phase1 >= 1.0f) phase1 -= std::floor(phase1);

        phase2 += f2 * dt;
        if (phase2 >= 1.0f) phase2 -= std::floor(phase2);

        // 2. Shell Body Amplitude Envelope
        // Dual sine resonant modes with 909 shell balance (65% low mode, 35% high mode)
        const float bodyEnv = std::exp(-envTime * 17.5f);
        const float shell = (std::sin(phase1 * TWO_PI) * 0.65f + std::sin(phase2 * TWO_PI) * 0.35f) * bodyEnv;

        // 3. Snappy Noise Engine
        // Snappy decay envelope: fast initial crack + body rattle
        const float snappyDecayScale = 0.35f + snappy * 1.65f;
        const float snapEnv = 0.65f * std::exp(-envTime * (24.0f / snappyDecayScale))
                            + 0.35f * std::exp(-envTime * (8.0f / snappyDecayScale));

        if (bodyEnv < 0.0003f && snapEnv < 0.0003f)
        {
            active = false;
            return 0.0f;
        }

        // Noise filtering: High-pass and bandpass shaping controlled by Tone
        const float noiseCutoff = 1600.0f + tone * 2200.0f;
        const float rawNoise = rng.next();
        snappyFilter.process(rawNoise, noiseCutoff, 1.5f, static_cast<float>(sampleRate));
        const float filteredNoise = (snappyFilter.hp * 0.65f + snappyFilter.bp * 0.55f);

        // 4. Tone Balance: Tone knob weights body vs snappy brightness
        const float bodyGain = (1.15f - tone * 0.35f);
        const float snappyGain = (0.20f + snappy * 1.45f) * (0.6f + tone * 0.8f);

        const float accentBoost = 1.0f + currentAccent * 0.5f;
        const float mixed = (shell * bodyGain + filteredNoise * snapEnv * snappyGain)
                            * currentVelocity * accentBoost * level * 1.25f;

        const float out = fastTanh(mixed * 1.35f);

        envTime += dt;
        updatePeak(out);

        return out;
    }
}
