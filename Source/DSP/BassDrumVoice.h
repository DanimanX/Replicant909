#pragma once

#include "VoiceBase.h"

namespace Replicant909
{
    class BassDrumVoice : public VoiceBase
    {
    public:
        BassDrumVoice();
        ~BassDrumVoice() override = default;

        void prepare(double newSampleRate) override;
        void reset() override;
        void trigger(float velocity, float totalAccent) override;
        float renderSample() override;
        bool isActive() const override { return active; }

    private:
        bool active = false;
        float envTime = 0.0f;
        float phase = 0.0f;
        float currentVelocity = 1.0f;
        float currentAccent = 0.0f;

        // Click generator noise state
        FastNoise rng;
        AnalogSvf clickFilter;
    };
}
