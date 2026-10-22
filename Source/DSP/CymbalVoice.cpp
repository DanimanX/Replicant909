#include "CymbalVoice.h"
#include <algorithm>
#include <cmath>

namespace Replicant909
{
    CymbalVoice::CymbalVoice(CymbalType type)
        : cymbalType(type)
    {
    }

    void CymbalVoice::prepare(double newSampleRate)
    {
        sampleRate = std::max(8000.0, newSampleRate);
        reset();
    }

    void CymbalVoice::reset()
    {
        active = false;
        playhead = 0.0;
        envTime = 0.0f;
        choking = false;
        chokeGain = 1.0f;
        reconFilter.reset();
        peakLevel.store(0.0f, std::memory_order_relaxed);
    }

    void CymbalVoice::trigger(float velocity, float totalAccent)
    {
        currentVelocity = juce::jlimit(0.05f, 1.0f, velocity);
        currentAccent = juce::jlimit(0.0f, 1.0f, totalAccent);
        playhead = 0.0;
        envTime = 0.0f;
        choking = false;
        chokeGain = 1.0f;
        reconFilter.reset();
        active = true;
    }

    void CymbalVoice::choke()
    {
        if (active)
            choking = true;
    }

    float CymbalVoice::renderSample()
    {
        if (!active)
            return 0.0f;

        const auto& rom = TR909RomData::getInstance();
        const int8_t* pcmBuffer = nullptr;
        int pcmLength = 0;

        switch (cymbalType)
        {
            case CYMBAL_CH:
                pcmBuffer = rom.closedHiHat.data();
                pcmLength = static_cast<int>(rom.closedHiHat.size());
                break;
            case CYMBAL_OH:
                pcmBuffer = rom.openHiHat.data();
                pcmLength = static_cast<int>(rom.openHiHat.size());
                break;
            case CYMBAL_CRASH:
                pcmBuffer = rom.crashCymbal.data();
                pcmLength = static_cast<int>(rom.crashCymbal.size());
                break;
            case CYMBAL_RIDE:
                pcmBuffer = rom.rideCymbal.data();
                pcmLength = static_cast<int>(rom.rideCymbal.size());
                break;
        }

        if (pcmBuffer == nullptr || pcmLength <= 0)
        {
            active = false;
            return 0.0f;
        }

        // 1. Playback Clock / Pitch Tuning
        // Hi-Hats have fixed clock in hardware (tune ignored or subtle), Cymbals have front-panel Tune pot (+/- 1 octave)
        float tuneRatio = 1.0f;
        if (cymbalType == CYMBAL_CRASH || cymbalType == CYMBAL_RIDE)
            tuneRatio = std::pow(2.0f, (tune - 0.5f) * 1.6f);
        else
            tuneRatio = std::pow(2.0f, (tune - 0.5f) * 0.35f);

        const double samplePitchStep = (static_cast<double>(TR909RomData::ROM_SAMPLE_RATE) / sampleRate) * tuneRatio;

        // Linear interpolation across 6-bit PCM ROM samples
        int idx0 = static_cast<int>(playhead);
        if (idx0 >= pcmLength - 1)
        {
            active = false;
            return 0.0f;
        }

        float frac = static_cast<float>(playhead - idx0);
        float s0 = TR909RomData::decode6Bit(pcmBuffer[idx0]);
        float s1 = TR909RomData::decode6Bit(pcmBuffer[idx0 + 1]);
        float rawPcm = s0 + frac * (s1 - s0);

        playhead += samplePitchStep;

        // 2. Analog VCA Envelope
        float decayRate = 22.0f;
        const float decayScale = 0.22f + decay * 2.2f;

        switch (cymbalType)
        {
            case CYMBAL_CH:
                decayRate = 26.0f / decayScale; // 30ms - 120ms
                break;
            case CYMBAL_OH:
                decayRate = 3.6f / decayScale;  // 150ms - 850ms
                break;
            case CYMBAL_CRASH:
                decayRate = 1.35f / decayScale; // 0.6s - 3.5s
                break;
            case CYMBAL_RIDE:
                decayRate = 1.15f / decayScale; // 0.7s - 4.0s
                break;
        }

        const float dt = 1.0f / static_cast<float>(sampleRate);
        const float ampEnv = std::exp(-envTime * decayRate);

        if (ampEnv < 0.0003f)
        {
            active = false;
            return 0.0f;
        }

        // 3. Hardware Choke Behavior (~3ms fast exponential discharge)
        if (choking)
        {
            chokeGain *= 0.965f;
            if (chokeGain < 0.001f)
            {
                active = false;
                choking = false;
                return 0.0f;
            }
        }

        // 4. Analog Reconstruction / Voicing Board Filter
        // Active highpass / bandpass filtering
        float filterFreq = 6500.0f;
        if (cymbalType == CYMBAL_RIDE)
            filterFreq = 5200.0f;
        else if (cymbalType == CYMBAL_CRASH)
            filterFreq = 4800.0f;

        reconFilter.process(rawPcm, filterFreq * tuneRatio, 1.4f, static_cast<float>(sampleRate));
        const float filteredSignal = reconFilter.hp * 0.75f + reconFilter.bp * 0.45f;

        // 5. Dynamic Accent & Output Stage
        const float accentBoost = 1.0f + currentAccent * 0.45f;
        const float shaped = filteredSignal * ampEnv * chokeGain * currentVelocity * accentBoost * level * 1.55f;

        const float out = fastTanh(shaped * 1.25f);

        envTime += dt;
        updatePeak(out);

        return out;
    }
}
