#pragma once

#include "VoiceBase.h"
#include "TR909RomData.h"

namespace Replicant909
{
    enum CymbalType : int
    {
        CYMBAL_CH = 0, // Closed Hi-Hat
        CYMBAL_OH,     // Open Hi-Hat
        CYMBAL_CRASH,  // Crash Cymbal
        CYMBAL_RIDE    // Ride Cymbal
    };

    class CymbalVoice : public VoiceBase
    {
    public:
        explicit CymbalVoice(CymbalType type);
        ~CymbalVoice() override = default;

        void prepare(double newSampleRate) override;
        void reset() override;
        void trigger(float velocity, float totalAccent) override;
        void choke() override;
        float renderSample() override;
        bool isActive() const override { return active; }

    private:
        CymbalType cymbalType;
        bool active = false;
        double playhead = 0.0;
        float envTime = 0.0f;
        float currentVelocity = 1.0f;
        float currentAccent = 0.0f;

        // Choke state
        bool choking = false;
        float chokeGain = 1.0f;

        // Analog reconstruction filter
        AnalogSvf reconFilter;
    };
}
