#pragma once

#include "../Common909.h"
#include <atomic>

namespace Replicant909
{
    class VoiceBase
    {
    public:
        virtual ~VoiceBase() = default;

        virtual void prepare(double sampleRate) = 0;
        virtual void reset() = 0;
        virtual void trigger(float velocity, float totalAccent) = 0;
        virtual void choke() {}
        virtual float renderSample() = 0;
        virtual bool isActive() const = 0;

        virtual void setTune(float value)   { tune = value; }
        virtual void setLevel(float value)  { level = value; }
        virtual void setAttack(float value) { attack = value; }
        virtual void setDecay(float value)  { decay = value; }
        virtual void setSnappy(float value) { snappy = value; }
        virtual void setTone(float value)   { tone = value; }
        virtual void setPan(float value)    { pan = value; }

        float getLevel() const noexcept { return level; }
        float getPan() const noexcept   { return pan; }

        float getPeak() const noexcept
        {
            return peakLevel.load(std::memory_order_relaxed);
        }

        void decayPeak() noexcept
        {
            float p = peakLevel.load(std::memory_order_relaxed);
            peakLevel.store(p * 0.92f, std::memory_order_relaxed);
        }

    protected:
        double sampleRate = 44100.0;
        float tune   = 0.5f;
        float level  = 0.8f;
        float attack = 0.5f;
        float decay  = 0.5f;
        float snappy = 0.5f;
        float tone   = 0.5f;
        float pan    = 0.5f; // 0 = Left, 0.5 = Center, 1 = Right

        std::atomic<float> peakLevel { 0.0f };

        inline void updatePeak(float sample) noexcept
        {
            const float absS = std::abs(sample);
            float current = peakLevel.load(std::memory_order_relaxed);
            if (absS > current)
                peakLevel.store(absS, std::memory_order_relaxed);
        }
    };
}
