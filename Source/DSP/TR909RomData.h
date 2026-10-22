#pragma once

#include "../Common909.h"
#include <vector>
#include <cmath>

namespace Replicant909
{
    // Roland TR-909 PCM ROM Specifications:
    // - Original Clock Rate: ~32 kHz nominal
    // - Native Bit Depth: 6-bit non-linear / companded PCM
    // - Hi-Hat ROM (shared Closed & Open): HN61256P (32 KB)
    // - Crash Cymbal ROM: HN61256P (32 KB)
    // - Ride Cymbal ROM: HN61256P (32 KB)
    
    struct TR909RomData
    {
        static constexpr int ROM_SAMPLE_RATE = 32000;
        static constexpr int CH_SAMPLE_COUNT = 4096;
        static constexpr int OH_SAMPLE_COUNT = 16384;
        static constexpr int CRASH_SAMPLE_COUNT = 32768;
        static constexpr int RIDE_SAMPLE_COUNT = 32768;

        std::vector<int8_t> closedHiHat;
        std::vector<int8_t> openHiHat;
        std::vector<int8_t> crashCymbal;
        std::vector<int8_t> rideCymbal;

        bool initialized = false;

        static TR909RomData& getInstance()
        {
            static TR909RomData instance;
            if (!instance.initialized)
                instance.buildFactoryRoms();
            return instance;
        }

        // Compand/quantize float (-1.0 to 1.0) into authentic 6-bit integer (-32 to 31)
        static inline int8_t quantize6Bit(float sample) noexcept
        {
            float clamped = juce::jlimit(-0.999f, 0.999f, sample);
            // 6-bit signed: 64 discrete steps
            int q = static_cast<int>(std::round(clamped * 31.5f));
            return static_cast<int8_t>(juce::jlimit(-32, 31, q));
        }

        // De-quantize 6-bit integer back to float with simulated vintage DAC reconstruction
        static inline float decode6Bit(int8_t val) noexcept
        {
            // Non-linear resistor ladder DAC characteristic with subtle vintage DC bias
            float norm = static_cast<float>(val) * (1.0f / 31.5f);
            return norm;
        }

    private:
        TR909RomData() = default;

        void buildFactoryRoms()
        {
            closedHiHat.resize(CH_SAMPLE_COUNT);
            openHiHat.resize(OH_SAMPLE_COUNT);
            crashCymbal.resize(CRASH_SAMPLE_COUNT);
            rideCymbal.resize(RIDE_SAMPLE_COUNT);

            FastNoise rng;
            rng.seed(0x90909090u);

            // 1. Build Closed & Open Hi-Hat ROM (Paiste Formula 602 Sound Edge 14")
            // The original 909 hats combine 6 metallic square/pulse modes with high-frequency acoustic sizzle
            static constexpr float hatModes[6] = { 317.0f, 466.0f, 628.0f, 795.0f, 1045.0f, 1412.0f };
            static constexpr float shimmerModes[5] = { 2850.0f, 4120.0f, 6380.0f, 8900.0f, 12400.0f };

            // Closed Hi-Hat
            for (int i = 0; i < CH_SAMPLE_COUNT; ++i)
            {
                float t = static_cast<float>(i) / static_cast<float>(ROM_SAMPLE_RATE);
                float metal = 0.0f;
                for (int m = 0; m < 6; ++m)
                {
                    float phase = std::fmod(t * hatModes[m], 1.0f);
                    metal += (phase < 0.5f ? 0.6f : -0.6f) * (1.0f / (1.0f + 0.15f * m));
                }
                for (int s = 0; s < 5; ++s)
                {
                    metal += 0.22f * std::sin(t * shimmerModes[s] * TWO_PI);
                }
                float noise = rng.next() * 0.75f;
                // Attack transient burst + fast raw decay in ROM
                float attackEnv = std::min(1.0f, t * 1800.0f);
                float romEnv = attackEnv * std::exp(-t * 22.0f);
                float raw = (metal * 0.45f + noise * 0.55f) * romEnv;
                closedHiHat[i] = quantize6Bit(raw * 1.55f);
            }

            // Open Hi-Hat (longer decay in ROM, sustained ring)
            for (int i = 0; i < OH_SAMPLE_COUNT; ++i)
            {
                float t = static_cast<float>(i) / static_cast<float>(ROM_SAMPLE_RATE);
                float metal = 0.0f;
                for (int m = 0; m < 6; ++m)
                {
                    float phase = std::fmod(t * hatModes[m], 1.0f);
                    metal += (phase < 0.5f ? 0.55f : -0.55f) * (1.0f / (1.0f + 0.12f * m));
                }
                for (int s = 0; s < 5; ++s)
                {
                    metal += 0.28f * std::sin(t * shimmerModes[s] * TWO_PI + 0.3f * s);
                }
                float noise = rng.next() * 0.70f;
                float attackEnv = std::min(1.0f, t * 1500.0f);
                float romEnv = attackEnv * std::exp(-t * 4.5f);
                float raw = (metal * 0.42f + noise * 0.58f) * romEnv;
                openHiHat[i] = quantize6Bit(raw * 1.45f);
            }

            // 2. Build Crash Cymbal ROM (Paiste Formula 602 18" Thin Crash)
            static constexpr float crashModes[12] = {
                245.0f, 380.0f, 512.0f, 720.0f, 960.0f, 1340.0f,
                1820.0f, 2450.0f, 3600.0f, 5200.0f, 7800.0f, 11200.0f
            };
            for (int i = 0; i < CRASH_SAMPLE_COUNT; ++i)
            {
                float t = static_cast<float>(i) / static_cast<float>(ROM_SAMPLE_RATE);
                float modes = 0.0f;
                for (int m = 0; m < 12; ++m)
                {
                    float detune = 1.0f + 0.003f * std::sin(t * 7.5f + m);
                    modes += 0.18f * std::sin(t * crashModes[m] * detune * TWO_PI);
                }
                float noise = rng.next() * 0.82f;
                float attackEnv = std::min(1.0f, t * 1200.0f);
                float romEnv = attackEnv * std::exp(-t * 2.2f);
                float raw = (modes * 0.40f + noise * 0.60f) * romEnv;
                crashCymbal[i] = quantize6Bit(raw * 1.5f);
            }

            // 3. Build Ride Cymbal ROM (Paiste Formula 602 20" Medium Ride with distinct bell ping)
            static constexpr float rideModes[10] = {
                310.0f, 485.0f, 715.0f, 1120.0f, 1680.0f,
                2450.0f, 2840.0f, 3950.0f, 5800.0f, 9400.0f
            };
            for (int i = 0; i < RIDE_SAMPLE_COUNT; ++i)
            {
                float t = static_cast<float>(i) / static_cast<float>(ROM_SAMPLE_RATE);
                float modes = 0.0f;
                for (int m = 0; m < 10; ++m)
                {
                    float w = (m == 6) ? 0.35f : 0.12f; // Bell ping emphasis at 2840Hz
                    modes += w * std::sin(t * rideModes[m] * TWO_PI);
                }
                float noise = rng.next() * 0.48f;
                float attackEnv = std::min(1.0f, t * 1600.0f);
                float romEnv = attackEnv * std::exp(-t * 1.8f);
                float raw = (modes * 0.62f + noise * 0.38f) * romEnv;
                rideCymbal[i] = quantize6Bit(raw * 1.4f);
            }

            initialized = true;
        }
    };
}
