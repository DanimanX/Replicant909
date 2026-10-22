#include "BassDrumVoice.h"
#include <algorithm>
#include <cmath>

namespace Replicant909
{
    BassDrumVoice::BassDrumVoice()
    {
        rng.seed(0xbd909bd1u);
    }

    void BassDrumVoice::prepare(double newSampleRate)
    {
        sampleRate = std::max(8000.0, newSampleRate);
        reset();
    }

    void BassDrumVoice::reset()
    {
        active = false;
        envTime = 0.0f;
        phase = 0.0f;
        clickFilter.reset();
        peakLevel.store(0.0f, std::memory_order_relaxed);
    }

    void BassDrumVoice::trigger(float velocity, float totalAccent)
    {
        currentVelocity = juce::jlimit(0.05f, 1.0f, velocity);
        currentAccent = juce::jlimit(0.0f, 1.0f, totalAccent);
        envTime = 0.0f;
        phase = 0.0f;
        clickFilter.reset();
        active = true;
    }

    float BassDrumVoice::renderSample()
    {
        if (!active)
            return 0.0f;

        const float dt = 1.0f / static_cast<float>(sampleRate);

        // 1. Tuning & Pitch Envelope
        // Base frequency: 42 Hz (low) to 75 Hz (high)
        const float tuneRatio = std::pow(2.0f, (tune - 0.5f) * 1.4f);
        const float baseFreq = 52.0f * tuneRatio;

        // Pitch envelope: Fast initial strike down-chirp + medium body settling
        // Accent dynamically deepens the initial chirp pitch!
        const float chirpDepth = (1.0f + 0.45f * currentAccent);
        const float fastEnv = std::exp(-envTime * 110.0f);
        const float medEnv  = std::exp(-envTime * 22.0f);
        const float instFreq = baseFreq + (230.0f * fastEnv * chirpDepth + 65.0f * medEnv) * tuneRatio;

        // Phase accumulation
        phase += instFreq * dt;
        if (phase >= 1.0f)
            phase -= std::floor(phase);

        // 2. Triangle-wave oscillator
        const float tri = (phase < 0.5f) ? (4.0f * phase - 1.0f) : (3.0f - 4.0f * phase);

        // 3. Diode-transistor waveshaping into warm, round sine with saturation harmonics
        const float body = diodeWaveshaper(tri);

        // 4. Amplitude Envelope (VCA)
        // Decay time: 0.15s to 1.1s
        const float decayScale = 0.18f + decay * 1.95f;
        const float ampEnv = std::exp(-envTime * (2.8f / decayScale));

        if (ampEnv < 0.0003f)
        {
            active = false;
            return 0.0f;
        }

        // 5. Transient Attack Click Generator (RC pulse + filtered noise)
        // Attack knob governs the click impact
        const float clickEnv = std::exp(-envTime * 380.0f);
        const float rawNoise = rng.next();
        clickFilter.process(rawNoise, 2800.0f * tuneRatio, 1.8f, static_cast<float>(sampleRate));
        const float clickNoise = clickFilter.bp * std::exp(-envTime * 520.0f);

        const float clickAmp = (0.15f + attack * 1.35f) * (1.0f + 0.5f * currentAccent);
        const float click = (clickEnv * 0.95f + clickNoise * 0.55f) * clickAmp;

        // 6. Summing, Accent Dynamic Scaling, and Output Drive
        const float accentBoost = 1.0f + currentAccent * 0.45f;
        const float rawOut = (body * ampEnv + click) * currentVelocity * accentBoost * level * 1.35f;

        // Discrete op-amp soft clipping
        const float out = fastTanh(rawOut * 1.25f);

        envTime += dt;
        updatePeak(out);

        return out;
    }
}
