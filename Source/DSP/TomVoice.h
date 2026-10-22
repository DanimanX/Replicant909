#pragma once

#include "VoiceBase.h"

namespace Replicant909
{
    enum TomType : int
    {
        TOM_LOW = 0,
        TOM_MID,
        TOM_HI
    };

    class TomVoice : public VoiceBase
    {
    public:
        explicit TomVoice(TomType type);
        ~TomVoice() override = default;

        void prepare(double newSampleRate) override;
        void reset() override;
        void trigger(float velocity, float totalAccent) override;
        float renderSample() override;
        bool isActive() const override { return active; }

    private:
        TomType tomType;
        bool active = false;
        float envTime = 0.0f;
        float phase = 0.0f;
        float currentVelocity = 1.0f;
        float currentAccent = 0.0f;

        FastNoise rng;
    };
}
