#include <juce_core/juce_core.h>
#include "../Source/Presets/PresetBank909.h"
#include "../Source/Presets/PresetIO909.h"
#include <iostream>

using namespace Replicant909;

int main()
{
    std::cout << "Exporting Replicant909 Factory Presets to MIDI..." << std::endl;

    PresetBank909 bank;
    juce::File presetDir("/PROCESOS/Replicant909/presets");
    presetDir.createDirectory();

    static constexpr const char* fileNames[8] = {
        "01_Classic_House.mid",
        "02_Detroit_Techno.mid",
        "03_Acid_Jack.mid",
        "04_Hard_Groove.mid",
        "05_Electro_Funk.mid",
        "06_Industrial_Rave.mid",
        "07_Minimal_Techno.mid",
        "08_Synthwave_909.mid"
    };

    for (int i = 0; i < 8 && i < bank.getNumPresets(); ++i)
    {
        const auto& p = bank.getPreset(i);
        juce::File outFile = presetDir.getChildFile(fileNames[i]);
        if (PresetIO909::exportToMidi(outFile, p))
        {
            std::cout << "Exported: " << outFile.getFileName() << " (" << p.name << " @ " << p.bpm << " BPM)" << std::endl;
        }
    }

    std::cout << "Done!" << std::endl;
    return 0;
}
