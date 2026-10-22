#include "HandClapVoice.h"
#include <algorithm>
#include <cmath>

namespace Replicant909
{
    HandClapVoice::HandClapVoice()
    {
        rng.seed(0xc1909a11u);
    }

    void HandClapVoice::prepare(double newSampleRate)
    {
        sampleRate = std::max(8000.0, newSampleRate);
        reset();
    }

    void HandClapVoice::reset()
    {
        active = false;
        envTime = 0.0f;
        clapFilter.reset();
        peakLevel.store(0.0f, std::memory_order_relaxed);
    }

    void HandClapVoice::trigger(float velocity, float totalAccent)
    {
        currentVelocity = juce::jlimit(0.05f, 1.0f, velocity);
        currentAccent = juce::jlimit(0.0f, 1.0f, totalAccent);
        envTime = 0.0f;
        clapFilter.reset();
        active = true;
    }

    float HandClapVoice::renderSample()
    {
        if (!active)
            return 0.0f;

        const float dt = 1.0f / static_cast<float>(sampleRate);

        // Envelope Generator:
        // 3 pre-bursts (spaced by 10ms = 0.010s, 0.020s), followed by main sustained clap at 0.030s
        float ampEnv = 0.0f;
        const float decayScale = 0.25f + reverb * 2.2f;

        if (envTime < 0.030f)
        {
            // Micro-bursts
            const float burstPhase = std::fmod(envTime, 0.010f) / 0.010f;
            ampEnv = std::exp(-burstPhase * 4.5f);
        }
        else
        {
            // Main clap and reverb tail
            const float tailTime = envTime - 0.030f;
            ampEnv = 0.75f * std::exp(-tailTime * (7.0f / decayScale));
        }

        if (envTime > 0.035f && ampEnv < 0.0003f)
        {
            active = false;
            return 0.0f;
        }

        // Transistor white noise through analog bandpass filter @ ~1180 Hz
        const float rawNoise = rng.next();
        clapFilter.process(rawNoise, 1180.0f, 1.8f, static_cast<float>(sampleRate));
        const float bandpassed = clapFilter.bp;

        const float accentBoost = 1.0f + currentAccent * 0.45f;
        const float raw = bandpassed * ampEnv * currentVelocity * accentBoost * level * 2.3f;

        // VCA soft saturation
        const float out = fastTanh(raw * 1.3f);

        envTime += dt;
        updatePeak(out);

        return out;
    }
}
