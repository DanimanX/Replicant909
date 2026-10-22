#pragma once

#include "PresetBank909.h"
#include <juce_audio_basics/juce_audio_basics.h>

namespace Replicant909
{
    class PresetIO909
    {
    public:
        // Export 909 pattern to Standard MIDI File (.mid) with embedded patch metadata
        static bool exportToMidi(const juce::File& file, const Preset909& preset);

        // Import Standard MIDI File into 909 pattern
        static bool importFromMidi(const juce::File& file, Preset909& preset);

        // XML state serialization for APVTS & DAW state
        static std::unique_ptr<juce::XmlElement> toXml(const Preset909& preset);
        static bool fromXml(const juce::XmlElement& xml, Preset909& preset);
    };
}
