#include "PresetBank909.h"

namespace Replicant909
{
    PresetBank909::PresetBank909()
    {
        initFactoryPresets();
    }

    const Preset909& PresetBank909::getPreset(int index) const noexcept
    {
        int clamped = juce::jlimit(0, static_cast<int>(presets.size()) - 1, index);
        return presets[clamped];
    }

    Preset909& PresetBank909::getPreset(int index) noexcept
    {
        int clamped = juce::jlimit(0, static_cast<int>(presets.size()) - 1, index);
        return presets[clamped];
    }

    void PresetBank909::addPreset(const Preset909& p)
    {
        presets.push_back(p);
    }

    void PresetBank909::initFactoryPresets()
    {
        presets.clear();

        // Preset 1: Classic House (124 BPM)
        {
            Preset909 p;
            p.name = "Classic House";
            p.category = "House";
            p.bpm = 124.0;
            p.totalAccent = 0.75f;
            p.shuffle = 2;

            p.pattern.setStep(BD, 0, STEP_ACCENT);
            p.pattern.setStep(BD, 4, STEP_NORMAL);
            p.pattern.setStep(BD, 8, STEP_ACCENT);
            p.pattern.setStep(BD, 12, STEP_NORMAL);

            p.pattern.setStep(CP, 4, STEP_ACCENT);
            p.pattern.setStep(CP, 12, STEP_ACCENT);

            p.pattern.setStep(OH, 2, STEP_ACCENT);
            p.pattern.setStep(OH, 6, STEP_ACCENT);
            p.pattern.setStep(OH, 10, STEP_ACCENT);
            p.pattern.setStep(OH, 14, STEP_ACCENT);

            for (int s = 0; s < 16; ++s)
                if (s % 4 != 2)
                    p.pattern.setStep(CH, s, STEP_NORMAL);

            p.pattern.setStep(RS, 3, STEP_NORMAL);
            p.pattern.setStep(RS, 11, STEP_NORMAL);

            p.pattern.setStep(TRACK_ACCENT, 0, STEP_ACCENT);
            p.pattern.setStep(TRACK_ACCENT, 4, STEP_ACCENT);
            p.pattern.setStep(TRACK_ACCENT, 8, STEP_ACCENT);
            p.pattern.setStep(TRACK_ACCENT, 12, STEP_ACCENT);

            presets.push_back(p);
        }

        // Preset 2: Detroit Techno (132 BPM)
        {
            Preset909 p;
            p.name = "Detroit Techno";
            p.category = "Techno";
            p.bpm = 132.0;
            p.totalAccent = 0.85f;
            p.shuffle = 1;

            p.pattern.setStep(BD, 0, STEP_ACCENT);
            p.pattern.setStep(BD, 4, STEP_ACCENT);
            p.pattern.setStep(BD, 8, STEP_ACCENT);
            p.pattern.setStep(BD, 12, STEP_ACCENT);

            p.pattern.setStep(SD, 4, STEP_ACCENT);
            p.pattern.setStep(SD, 12, STEP_ACCENT);
            p.pattern.setStep(SD, 15, STEP_NORMAL);

            p.pattern.setStep(LT, 6, STEP_NORMAL);
            p.pattern.setStep(MT, 10, STEP_NORMAL);
            p.pattern.setStep(HT, 14, STEP_NORMAL);

            for (int s = 0; s < 16; s += 2)
                p.pattern.setStep(RC, s, STEP_NORMAL);
            p.pattern.setStep(RC, 2, STEP_ACCENT);
            p.pattern.setStep(RC, 10, STEP_ACCENT);

            p.pattern.setStep(OH, 2, STEP_ACCENT);
            p.pattern.setStep(OH, 6, STEP_ACCENT);
            p.pattern.setStep(OH, 10, STEP_ACCENT);
            p.pattern.setStep(OH, 14, STEP_ACCENT);

            presets.push_back(p);
        }

        // Preset 3: Acid Jack (128 BPM)
        {
            Preset909 p;
            p.name = "Acid Jack";
            p.category = "Acid";
            p.bpm = 128.0;
            p.totalAccent = 0.80f;
            p.shuffle = 3;

            p.pattern.setStep(BD, 0, STEP_ACCENT);
            p.pattern.setStep(BD, 4, STEP_NORMAL);
            p.pattern.setStep(BD, 8, STEP_ACCENT);
            p.pattern.setStep(BD, 12, STEP_NORMAL);

            p.pattern.setStep(SD, 4, STEP_ACCENT);
            p.pattern.setStep(SD, 10, STEP_NORMAL);
            p.pattern.setStep(SD, 12, STEP_ACCENT);

            p.pattern.setStep(RS, 2, STEP_NORMAL);
            p.pattern.setStep(RS, 7, STEP_NORMAL);
            p.pattern.setStep(RS, 14, STEP_NORMAL);

            for (int s = 0; s < 16; ++s)
                p.pattern.setStep(CH, s, (s % 2 == 0) ? STEP_NORMAL : STEP_ACCENT);

            presets.push_back(p);
        }

        // Preset 4: Hard Groove (138 BPM)
        {
            Preset909 p;
            p.name = "Hard Groove";
            p.category = "Techno";
            p.bpm = 138.0;
            p.totalAccent = 0.90f;
            p.shuffle = 2;

            for (int s = 0; s < 16; s += 4)
                p.pattern.setStep(BD, s, STEP_ACCENT);

            p.pattern.setStep(SD, 4, STEP_ACCENT);
            p.pattern.setStep(SD, 12, STEP_FLAM);

            p.pattern.setStep(LT, 3, STEP_NORMAL);
            p.pattern.setStep(LT, 11, STEP_NORMAL);
            p.pattern.setStep(MT, 7, STEP_NORMAL);

            for (int s = 0; s < 16; ++s)
                p.pattern.setStep(RC, s, STEP_NORMAL);

            presets.push_back(p);
        }

        // Preset 5: Electro Funk (126 BPM)
        {
            Preset909 p;
            p.name = "Electro Funk";
            p.category = "Electro";
            p.bpm = 126.0;
            p.totalAccent = 0.75f;
            p.shuffle = 4;

            p.pattern.setStep(BD, 0, STEP_ACCENT);
            p.pattern.setStep(BD, 6, STEP_NORMAL);
            p.pattern.setStep(BD, 8, STEP_NORMAL);
            p.pattern.setStep(BD, 10, STEP_ACCENT);

            p.pattern.setStep(CP, 4, STEP_ACCENT);
            p.pattern.setStep(CP, 12, STEP_ACCENT);

            p.pattern.setStep(RS, 2, STEP_NORMAL);
            p.pattern.setStep(RS, 14, STEP_NORMAL);

            for (int s = 0; s < 16; s += 2)
                p.pattern.setStep(CH, s, STEP_NORMAL);

            presets.push_back(p);
        }

        // Preset 6: Industrial Rave (145 BPM)
        {
            Preset909 p;
            p.name = "Industrial Rave";
            p.category = "Industrial";
            p.bpm = 145.0;
            p.totalAccent = 0.95f;
            p.shuffle = 1;

            for (int s = 0; s < 16; s += 4)
                p.pattern.setStep(BD, s, STEP_ACCENT);

            p.pattern.setStep(SD, 4, STEP_ACCENT);
            p.pattern.setStep(SD, 12, STEP_ACCENT);
            p.pattern.setStep(SD, 14, STEP_NORMAL);

            p.pattern.setStep(CC, 0, STEP_ACCENT);
            p.pattern.setStep(CC, 8, STEP_NORMAL);

            for (int s = 2; s < 16; s += 4)
                p.pattern.setStep(OH, s, STEP_ACCENT);

            presets.push_back(p);
        }

        // Preset 7: Minimal Techno (125 BPM)
        {
            Preset909 p;
            p.name = "Minimal Techno";
            p.category = "Minimal";
            p.bpm = 125.0;
            p.totalAccent = 0.60f;
            p.shuffle = 1;

            for (int s = 0; s < 16; s += 4)
                p.pattern.setStep(BD, s, STEP_NORMAL);

            p.pattern.setStep(RS, 4, STEP_NORMAL);
            p.pattern.setStep(RS, 12, STEP_NORMAL);
            p.pattern.setStep(RS, 7, STEP_NORMAL);

            p.pattern.setStep(CH, 2, STEP_NORMAL);
            p.pattern.setStep(CH, 6, STEP_NORMAL);
            p.pattern.setStep(CH, 10, STEP_NORMAL);
            p.pattern.setStep(CH, 14, STEP_NORMAL);

            presets.push_back(p);
        }

        // Preset 8: Synthwave 909 (118 BPM)
        {
            Preset909 p;
            p.name = "Synthwave 909";
            p.category = "Synthwave";
            p.bpm = 118.0;
            p.totalAccent = 0.70f;
            p.shuffle = 1;

            p.pattern.setStep(BD, 0, STEP_ACCENT);
            p.pattern.setStep(BD, 8, STEP_ACCENT);

            p.pattern.setStep(SD, 4, STEP_ACCENT);
            p.pattern.setStep(SD, 12, STEP_ACCENT);

            p.pattern.setStep(CP, 4, STEP_NORMAL);
            p.pattern.setStep(CP, 12, STEP_NORMAL);

            for (int s = 0; s < 16; ++s)
                p.pattern.setStep(CH, s, STEP_NORMAL);

            p.pattern.setStep(OH, 2, STEP_NORMAL);
            p.pattern.setStep(OH, 10, STEP_NORMAL);

            p.pattern.setStep(HT, 14, STEP_NORMAL);
            p.pattern.setStep(MT, 15, STEP_NORMAL);

            presets.push_back(p);
        }
    }
}
