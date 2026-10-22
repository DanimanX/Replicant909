#pragma once

#include "../Common909.h"
#include <array>
#include <string>

namespace Replicant909
{
    static constexpr int NUM_STEPS = 16;
    static constexpr int NUM_PATTERNS_PER_BANK = 16;
    static constexpr int NUM_BANKS = 2; // Bank A and Bank B (32 patterns total)

    enum StepState : uint8_t
    {
        STEP_OFF = 0,
        STEP_NORMAL = 1,
        STEP_ACCENT = 2,
        STEP_FLAM = 3
    };

    struct Pattern
    {
        char name[32] = "Init Pattern";
        uint8_t length = 16;          // 1..16
        uint8_t scale = 0;           // 0: 1/16, 1: 1/32, 2: 1/12 (triplet), 3: 1/24
        uint8_t shuffle = 1;         // 1..6 (1 = 50% straight, 6 = heavy 70.8% swing)
        uint8_t flamInterval = 3;    // 1..8 (10ms to 45ms flam spacing)

        // Steps grid: [Track][Step]
        // Tracks 0..10: Instruments, Track 11: Total Accent track
        std::array<std::array<uint8_t, NUM_STEPS>, NUM_TRACKS> steps {};

        Pattern()
        {
            clear();
        }

        void clear()
        {
            for (auto& track : steps)
                track.fill(STEP_OFF);
            length = 16;
            scale = 0;
            shuffle = 1;
            flamInterval = 3;
        }

        inline uint8_t getStep(int track, int step) const noexcept
        {
            if (track >= 0 && track < NUM_TRACKS && step >= 0 && step < NUM_STEPS)
                return steps[track][step];
            return STEP_OFF;
        }

        inline void setStep(int track, int step, uint8_t state) noexcept
        {
            if (track >= 0 && track < NUM_TRACKS && step >= 0 && step < NUM_STEPS)
                steps[track][step] = state;
        }

        inline void toggleStep(int track, int step) noexcept
        {
            if (track >= 0 && track < NUM_TRACKS && step >= 0 && step < NUM_STEPS)
            {
                uint8_t current = steps[track][step];
                // Cycle: Off -> Normal -> Accent -> Flam -> Off
                if (track == TRACK_ACCENT)
                {
                    // Accent track cycles Off -> On -> Off
                    steps[track][step] = (current == STEP_OFF) ? STEP_ACCENT : STEP_OFF;
                }
                else
                {
                    if (current == STEP_OFF)
                        steps[track][step] = STEP_NORMAL;
                    else if (current == STEP_NORMAL)
                        steps[track][step] = STEP_ACCENT;
                    else if (current == STEP_ACCENT)
                        steps[track][step] = STEP_FLAM;
                    else
                        steps[track][step] = STEP_OFF;
                }
            }
        }
    };
}
