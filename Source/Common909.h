#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>
#include <array>
#include <cstdint>

namespace Replicant909
{
    // Mathematical constants
    inline constexpr float PI         = 3.14159265358979323846f;
    inline constexpr float TWO_PI     = 6.28318530717958647692f;
    inline constexpr float HALF_PI    = 1.57079632679489661923f;
    inline constexpr float INV_TWO_PI = 0.15915494309189533576f;

    // Instruments count and enumeration
    enum InstrumentId : int
    {
        BD = 0, // Bass Drum
        SD,     // Snare Drum
        LT,     // Low Tom
        MT,     // Mid Tom
        HT,     // Hi Tom
        RS,     // Rim Shot
        CP,     // Hand Clap
        CH,     // Closed Hi-Hat
        OH,     // Open Hi-Hat
        CC,     // Crash Cymbal
        RC,     // Ride Cymbal
        NUM_VOICES,

        TRACK_ACCENT = NUM_VOICES, // Sequencer Accent Track
        NUM_TRACKS
    };

    // Output bus identifiers (Master + 10 Individual Outs)
    enum BusId : int
    {
        BUS_MASTER = 0,
        BUS_BD,
        BUS_SD,
        BUS_LT,
        BUS_MT,
        BUS_HT,
        BUS_RS,
        BUS_CP,
        BUS_HH, // Shared CH & OH
        BUS_CC,
        BUS_RC,
        NUM_BUSES
    };

    // Display labels for each voice
    inline constexpr const char* VOICE_SHORT_NAMES[NUM_VOICES] = {
        "BD", "SD", "LT", "MT", "HT", "RS", "CP", "CH", "OH", "CC", "RC"
    };

    inline constexpr const char* VOICE_FULL_NAMES[NUM_VOICES] = {
        "BASS DRUM", "SNARE DRUM", "LOW TOM", "MID TOM", "HI TOM",
        "RIM SHOT", "HAND CLAP", "CLOSED HI-HAT", "OPEN HI-HAT", "CRASH CYMBAL", "RIDE CYMBAL"
    };

    // Standard TR-909 MIDI Note Mapping
    // BD: 35 & 36
    // SD: 38 & 40
    // RS: 37
    // CP: 39
    // CH: 42 (and 44 pedal)
    // OH: 46
    // LT: 41 & 43
    // MT: 45 & 47
    // HT: 48 & 50
    // CC: 49
    // RC: 51
    inline int midiNoteToInstrument(int noteNumber) noexcept
    {
        switch (noteNumber)
        {
            case 35: case 36: return BD;
            case 38: case 40: return SD;
            case 37:          return RS;
            case 39:          return CP;
            case 42: case 44: return CH;
            case 46:          return OH;
            case 41: case 43: return LT;
            case 45: case 47: return MT;
            case 48: case 50: return HT;
            case 49:          return CC;
            case 51:          return RC;
            default:          return -1;
        }
    }

    inline int instrumentToDefaultMidiNote(int inst) noexcept
    {
        switch (inst)
        {
            case BD: return 36;
            case SD: return 38;
            case LT: return 41;
            case MT: return 45;
            case HT: return 48;
            case RS: return 37;
            case CP: return 39;
            case CH: return 42;
            case OH: return 46;
            case CC: return 49;
            case RC: return 51;
            default: return 36;
        }
    }

    // High performance fast polynomial approximation for tanh (waveshaping / soft clipping)
    inline float fastTanh(float x) noexcept
    {
        if (x < -3.0f) return -1.0f;
        if (x > 3.0f)  return 1.0f;
        const float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

    // Diode-transistor waveshaper for TR-909 triangle-to-sine conversion
    // Models the non-linear soft limiting with subtle second-harmonic thermal asymmetry
    inline float diodeWaveshaper(float x) noexcept
    {
        // Add subtle DC offset to mimic BJT Vbe mismatch
        const float v = x + 0.035f * (x * x - 1.0f);
        return fastTanh(1.45f * v);
    }

    // Linear congruential / Xorshift white noise generator
    struct FastNoise
    {
        uint32_t state = 0x13379091u;

        inline void seed(uint32_t s) noexcept
        {
            state = (s != 0) ? s : 0x13379091u;
        }

        inline float next() noexcept
        {
            state ^= state << 13;
            state ^= state >> 17;
            state ^= state << 5;
            return static_cast<float>(static_cast<int32_t>(state)) * (1.0f / 2147483648.0f);
        }
    };

    // State Variable Filter (SVF) for analog circuit simulation (Chamberlin / biquad)
    struct AnalogSvf
    {
        float lp = 0.0f;
        float bp = 0.0f;
        float hp = 0.0f;

        inline void reset() noexcept
        {
            lp = 0.0f;
            bp = 0.0f;
            hp = 0.0f;
        }

        inline void process(float input, float cutoffHz, float q, float sampleRate) noexcept
        {
            const float f = 2.0f * std::sin(PI * std::min(0.48f, cutoffHz / sampleRate));
            const float damp = 1.0f / std::max(0.2f, q);
            hp = input - lp - damp * bp;
            bp += f * hp;
            lp += f * bp;
        }
    };
}
