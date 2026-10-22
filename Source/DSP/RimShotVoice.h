#pragma once

#include "VoiceBase.h"

namespace Replicant909
{
    class RimShotVoice : public VoiceBase
    {
    public:
        RimShotVoice();
        ~RimShotVoice() override = default;

        void prepare(double newSampleRate) override;
        void reset() override;
        void trigger(float velocity, float totalAccent) override;
        float renderSample() override;
        bool isActive() const override { return active; }

    private:
        bool active = false;
        float envTime = 0.0f;
        float phase1 = 0.0f;
        float phase2 = 0.0f;
        float phase3 = 0.0f;
        float currentVelocity = 1.0f;
        float currentAccent = 0.0f;
    };
}
