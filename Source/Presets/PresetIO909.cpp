#include "PresetIO909.h"
#include <juce_audio_basics/juce_audio_basics.h>

namespace Replicant909
{
    bool PresetIO909::exportToMidi(const juce::File& file, const Preset909& preset)
    {
        juce::MidiFile midiFile;
        midiFile.setTicksPerQuarterNote(960);

        juce::MidiMessageSequence trackSeq;

        // Set Tempo meta event (microsecond per quarter note = 60,000,000 / BPM)
        int tempoMicrosec = static_cast<int>(60000000.0 / std::max(20.0, preset.bpm));
        trackSeq.addEvent(juce::MidiMessage::tempoMetaEvent(tempoMicrosec), 0.0);

        // Embed preset metadata text event
        juce::String metaStr = "REPLICANT909_PRESET:" + preset.name + "|" + preset.category +
                               "|BPM=" + juce::String(preset.bpm) +
                               "|SHUFFLE=" + juce::String(preset.shuffle);
        trackSeq.addEvent(juce::MidiMessage::textMetaEvent(1, metaStr), 0.0);

        // 16th note = 960 / 4 = 240 ticks
        const double ticksPer16th = 240.0;
        const double noteDuration = 120.0;

        for (int track = 0; track < NUM_VOICES; ++track)
        {
            int midiNote = instrumentToDefaultMidiNote(track);

            for (int step = 0; step < preset.pattern.length; ++step)
            {
                uint8_t state = preset.pattern.getStep(track, step);
                if (state == STEP_OFF)
                    continue;

                double noteStart = static_cast<double>(step) * ticksPer16th;
                float vel = (state == STEP_ACCENT) ? 1.0f : (state == STEP_FLAM ? 0.75f : 0.85f);

                trackSeq.addEvent(juce::MidiMessage::noteOn(10, midiNote, vel), noteStart);
                trackSeq.addEvent(juce::MidiMessage::noteOff(10, midiNote, 0.0f), noteStart + noteDuration);
            }
        }

        midiFile.addTrack(trackSeq);

        juce::FileOutputStream stream(file);
        if (!stream.openedOk())
            return false;

        return midiFile.writeTo(stream);
    }

    bool PresetIO909::importFromMidi(const juce::File& file, Preset909& preset)
    {
        juce::FileInputStream stream(file);
        if (!stream.openedOk())
            return false;

        juce::MidiFile midiFile;
        if (!midiFile.readFrom(stream))
            return false;

        preset.pattern.clear();

        short timeFormat = midiFile.getTimeFormat();
        double ticksPerQuarter = (timeFormat > 0) ? static_cast<double>(timeFormat) : 960.0;
        double ticksPer16th = ticksPerQuarter / 4.0;

        for (int t = 0; t < midiFile.getNumTracks(); ++t)
        {
            const auto* seq = midiFile.getTrack(t);
            if (seq == nullptr)
                continue;

            for (int i = 0; i < seq->getNumEvents(); ++i)
            {
                const auto& msg = seq->getEventPointer(i)->message;
                if (msg.isTempoMetaEvent())
                {
                    double tempoBpm = 60.0 / std::max(0.001, msg.getTempoSecondsPerQuarterNote());
                    preset.bpm = juce::jlimit(30.0, 300.0, tempoBpm);
                }
                else if (msg.isTextMetaEvent())
                {
                    juce::String txt = msg.getTextFromTextMetaEvent();
                    if (txt.startsWith("REPLICANT909_PRESET:"))
                    {
                        txt = txt.substring(20);
                        auto parts = juce::StringArray::fromTokens(txt, "|", "");
                        if (parts.size() >= 1) preset.name = parts[0];
                        if (parts.size() >= 2) preset.category = parts[1];
                        for (int p = 2; p < parts.size(); ++p)
                        {
                            if (parts[p].startsWith("BPM="))
                                preset.bpm = parts[p].substring(4).getDoubleValue();
                            else if (parts[p].startsWith("SHUFFLE="))
                                preset.shuffle = parts[p].substring(8).getIntValue();
                        }
                    }
                }
                else if (msg.isNoteOn())
                {
                    int inst = midiNoteToInstrument(msg.getNoteNumber());
                    if (inst >= 0 && inst < NUM_VOICES)
                    {
                        double tickPos = seq->getEventTime(i);
                        int step = static_cast<int>(std::round(tickPos / ticksPer16th)) % 16;
                        uint8_t st = (msg.getVelocity() > 100) ? STEP_ACCENT : STEP_NORMAL;
                        preset.pattern.setStep(inst, step, st);
                    }
                }
            }
        }

        return true;
    }

    std::unique_ptr<juce::XmlElement> PresetIO909::toXml(const Preset909& preset)
    {
        auto xml = std::make_unique<juce::XmlElement>("REPLICANT_909_PRESET");
        xml->setAttribute("name", preset.name);
        xml->setAttribute("category", preset.category);
        xml->setAttribute("bpm", preset.bpm);
        xml->setAttribute("totalAccent", preset.totalAccent);
        xml->setAttribute("masterVolume", preset.masterVolume);
        xml->setAttribute("shuffle", preset.shuffle);
        xml->setAttribute("flamInterval", preset.flamInterval);

        auto* patXml = xml->createNewChildElement("PATTERN");
        patXml->setAttribute("length", preset.pattern.length);

        for (int t = 0; t < NUM_TRACKS; ++t)
        {
            juce::String trackStr;
            for (int s = 0; s < NUM_STEPS; ++s)
                trackStr += juce::String(preset.pattern.getStep(t, s)) + ",";
            patXml->setAttribute("track_" + juce::String(t), trackStr);
        }

        return xml;
    }

    bool PresetIO909::fromXml(const juce::XmlElement& xml, Preset909& preset)
    {
        if (!xml.hasTagName("REPLICANT_909_PRESET"))
            return false;

        preset.name = xml.getStringAttribute("name", "Imported 909");
        preset.category = xml.getStringAttribute("category", "User");
        preset.bpm = xml.getDoubleAttribute("bpm", 128.0);
        preset.totalAccent = static_cast<float>(xml.getDoubleAttribute("totalAccent", 0.75));
        preset.masterVolume = static_cast<float>(xml.getDoubleAttribute("masterVolume", 0.85));
        preset.shuffle = xml.getIntAttribute("shuffle", 1);
        preset.flamInterval = xml.getIntAttribute("flamInterval", 3);

        if (auto* patXml = xml.getChildByName("PATTERN"))
        {
            preset.pattern.length = static_cast<uint8_t>(patXml->getIntAttribute("length", 16));
            for (int t = 0; t < NUM_TRACKS; ++t)
            {
                juce::String trackStr = patXml->getStringAttribute("track_" + juce::String(t));
                auto tokens = juce::StringArray::fromTokens(trackStr, ",", "");
                for (int s = 0; s < std::min(NUM_STEPS, tokens.size()); ++s)
                {
                    preset.pattern.setStep(t, s, static_cast<uint8_t>(tokens[s].getIntValue()));
                }
            }
        }

        return true;
    }
}
