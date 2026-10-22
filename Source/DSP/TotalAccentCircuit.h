#pragma once

#include "../Common909.h"
#include <algorithm>

namespace Replicant909
{
    class TotalAccentCircuit
    {
    public:
        TotalAccentCircuit() = default;

        void setAccentLevel(float level) noexcept
        {
            accentLevel = juce::jlimit(0.0f, 1.0f, level);
        }

        float getAccentLevel() const noexcept
        {
            return accentLevel;
        }

        // Calculates voice-specific accent scale factor
        // If step has accent, scales from 0.0 to accentLevel
        inline float computeAccentGain(bool stepAccented, float noteVelocity) const noexcept
        {
            // MIDI velocity threshold > 96 triggers accent, or explicit step accent flag
            bool accented = stepAccented || (noteVelocity > 0.75f);
            if (!accented)
                return 0.0f;

            // In TR-909, accent ranges from subtle +15% boost to aggressive +75% boost
            return accentLevel;
        }

    private:
        float accentLevel = 0.70f;
    };
}
