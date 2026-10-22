#include "TomVoice.h"
#include <algorithm>
#include <cmath>

namespace Replicant909
{
    TomVoice::TomVoice(TomType type)
        : tomType(type)
    {
        uint32_t seed = 0x70909000u + static_cast<uint32_t>(type);
        rng.seed(seed);
    }

    void TomVoice::prepare(double newSampleRate)
    {
        sampleRate = std::max(8000.0, newSampleRate);
        reset();
    }

    void TomVoice::reset()
    {
        active = false;
        envTime = 0.0f;
        phase = 0.0f;
        peakLevel.store(0.0f, std::memory_order_relaxed);
    }

    void TomVoice::trigger(float velocity, float totalAccent)
    {
        currentVelocity = juce::jlimit(0.05f, 1.0f, velocity);
        currentAccent = juce::jlimit(0.0f, 1.0f, totalAccent);
        envTime = 0.0f;
        phase = 0.0f;
        active = true;
    }

    float TomVoice::renderSample()
    {
        if (!active)
            return 0.0f;

        const float dt = 1.0f / static_cast<float>(sampleRate);

        // 1. Base Frequencies per tom type
        float nominalFreq = 92.0f;
        if (tomType == TOM_MID)
            nominalFreq = 132.0f;
        else if (tomType == TOM_HI)
            nominalFreq = 185.0f;

        const float tuneRatio = std::pow(2.0f, (tune - 0.5f) * 1.4f);
        const float baseFreq = nominalFreq * tuneRatio;

        // Pitch envelope: Downward skin-tension drop
        const float pitchSweep = 1.0f + (0.80f + 0.35f * currentAccent) * std::exp(-envTime * 30.0f);
        const float instFreq = baseFreq * pitchSweep;

        phase += instFreq * dt;
        if (phase >= 1.0f) phase -= std::floor(phase);

        // 2. Amplitude Envelope (VCA)
        const float decayScale = 0.25f + decay * 2.1f;
        const float ampEnv = std::exp(-envTime * (6.0f / decayScale));

        if (ampEnv < 0.0003f)
        {
            active = false;
            return 0.0f;
        }

        // Resonant bridged-T body sine + 2nd harmonic richness
        const float body = std::sin(phase * TWO_PI) + 0.08f * std::sin(phase * TWO_PI * 2.0f);

        // Transient stick click
        const float click = rng.next() * std::exp(-envTime * 175.0f) * 0.28f;

        const float accentBoost = 1.0f + currentAccent * 0.45f;
        const float raw = (body * ampEnv + click) * currentVelocity * accentBoost * level * 1.35f;

        const float out = fastTanh(raw * 1.2f);

        envTime += dt;
        updatePeak(out);

        return out;
    }
}
