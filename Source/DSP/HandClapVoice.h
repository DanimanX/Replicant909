#pragma once

#include "VoiceBase.h"

namespace Replicant909
{
    class HandClapVoice : public VoiceBase
    {
    public:
        HandClapVoice();
        ~HandClapVoice() override = default;

        void prepare(double newSampleRate) override;
        void reset() override;
        void trigger(float velocity, float totalAccent) override;
        float renderSample() override;
        bool isActive() const override { return active; }

        void setReverb(float value) { reverb = value; }
        float getReverb() const noexcept { return reverb; }

    private:
        bool active = false;
        float envTime = 0.0f;
        float reverb = 0.5f;
        float currentVelocity = 1.0f;
        float currentAccent = 0.0f;

        FastNoise rng;
        AnalogSvf clapFilter;
    };
}
