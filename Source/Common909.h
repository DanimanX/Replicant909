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

    // Standard TR-909 MIDI Note Mapping (with multi-octave tolerance & chromatic mapping)
    // Primary TR-909 / General MIDI range:
    // BD: 35 & 36 (and all octaves of C: 12, 24, 36, 48, 60, 72, 84, 96, 108)
    // SD: 38 & 40
    // RS: 37
    // CP: 39
    // CH: 42 (and 44 pedal)
    // OH: 46
    // LT: 41 & 43
    // MT: 45 & 47
    // HT: 50
    // CC: 49
    // RC: 51
    inline int midiNoteToInstrument(int noteNumber) noexcept
    {
        if (noteNumber < 0 || noteNumber > 127)
            return -1;

        // 1. Any octave of C (note % 12 == 0) ALWAYS triggers Bass Drum!
        // This guarantees DAWs (FL Studio, Ableton, Reaper, Cubase, Logic) sending notes
        // on C1, C2, C3, C4, C5 (notes 24, 36, 48, 60, 72, 84) trigger the Kick drum.
        if (noteNumber % 12 == 0)
            return BD;

        // 2. Standard GM Acoustic Bass Drum (note 35) and lower octaves of B
        if (noteNumber == 35 || noteNumber == 23 || noteNumber == 11)
            return BD;

        // 3. Exact GM / TR-909 base notes (35..51)
        switch (noteNumber)
        {
            case 37:          return RS;
            case 38: case 40: return SD;
            case 39:          return CP;
            case 41: case 43: return LT;
            case 42: case 44: return CH;
            case 45: case 47: return MT;
            case 46:          return OH;
            case 49:          return CC;
            case 50:          return HT;
            case 51:          return RC;
        }

        // 4. Chromatic keyboard audition around Middle C (60..70)
        // 60: BD (handled above), 61: SD, 62: LT, 63: MT, 64: HT, 65: RS, 66: CP, 67: CH, 68: OH, 69: CC, 70: RC
        if (noteNumber >= 61 && noteNumber <= 70)
        {
            static constexpr int chromaticMap[10] = {
                SD, // 61: C#
                LT, // 62: D
                MT, // 63: D#
                HT, // 64: E
                RS, // 65: F
                CP, // 66: F#
                CH, // 67: G
                OH, // 68: G#
                CC, // 69: A
                RC  // 70: A#
            };
            return chromaticMap[noteNumber - 61];
        }

        // 5. Transposed GM octaves (+12, -12, +24, -24, +36) for transposed pads / keyboards
        for (int shift : { -24, -12, 12, 24, 36 })
        {
            int shifted = noteNumber - shift;
            switch (shifted)
            {
                case 37:          return RS;
                case 38: case 40: return SD;
                case 39:          return CP;
                case 41: case 43: return LT;
                case 42: case 44: return CH;
                case 45: case 47: return MT;
                case 46:          return OH;
                case 49:          return CC;
                case 50:          return HT;
                case 51:          return RC;
            }
        }

        return -1;
    }

    inline int instrumentToDefaultMidiNote(int inst) noexcept
    {
        switch (inst)
        {
            case BD: return 36;
            case SD: return 38;
            case LT: return 41;
            case MT: return 45;
            case HT: return 50;
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
