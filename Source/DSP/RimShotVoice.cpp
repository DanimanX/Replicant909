#include "RimShotVoice.h"
#include <algorithm>
#include <cmath>

namespace Replicant909
{
    RimShotVoice::RimShotVoice() = default;

    void RimShotVoice::prepare(double newSampleRate)
    {
        sampleRate = std::max(8000.0, newSampleRate);
        reset();
    }

    void RimShotVoice::reset()
    {
        active = false;
        envTime = 0.0f;
        phase1 = 0.0f;
        phase2 = 0.0f;
        phase3 = 0.0f;
        peakLevel.store(0.0f, std::memory_order_relaxed);
    }

    void RimShotVoice::trigger(float velocity, float totalAccent)
    {
        currentVelocity = juce::jlimit(0.05f, 1.0f, velocity);
        currentAccent = juce::jlimit(0.0f, 1.0f, totalAccent);
        envTime = 0.0f;
        phase1 = 0.0f;
        phase2 = 0.0f;
        phase3 = 0.0f;
        active = true;
    }

    float RimShotVoice::renderSample()
    {
        if (!active)
            return 0.0f;

        const float dt = 1.0f / static_cast<float>(sampleRate);

        // Rim Shot modal ringing frequencies
        const float f1 = 250.0f;
        const float f2 = 495.0f;
        const float f3 = 1580.0f;

        phase1 += f1 * dt; if (phase1 >= 1.0f) phase1 -= std::floor(phase1);
        phase2 += f2 * dt; if (phase2 >= 1.0f) phase2 -= std::floor(phase2);
        phase3 += f3 * dt; if (phase3 >= 1.0f) phase3 -= std::floor(phase3);

        // Amplitude envelope (fast decay ~45 ms)
        const float env = std::exp(-envTime * 65.0f);
        if (env < 0.0003f)
        {
            active = false;
            return 0.0f;
        }

        const float modalMix = std::sin(phase1 * TWO_PI) * 0.40f
                             + std::sin(phase2 * TWO_PI) * 0.60f
                             + std::sin(phase3 * TWO_PI) * 0.90f;

        const float accentBoost = 1.0f + currentAccent * 0.50f;
        const float raw = modalMix * env * currentVelocity * accentBoost * level * 2.1f;

        // Vintage diode limiter / soft clipper gives the crack
        const float out = fastTanh(raw * 1.4f);

        envTime += dt;
        updatePeak(out);

        return out;
    }
}
