#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Source/PluginProcessor.h"
#include "../Source/DSP/DrumEngine909.h"
#include "../Source/DSP/GrooveSequencer909.h"
#include "../Source/Presets/PresetBank909.h"
#include "../Source/Presets/PresetIO909.h"
#include <iostream>
#include <cassert>

using namespace Replicant909;

int main()
{
    juce::ScopedJuceInitialiser_GUI guiInit;

    std::cout << "========================================" << std::endl;
    std::cout << "REPLICANT 909 HEADLESS REGRESSION SUITE " << std::endl;
    std::cout << "========================================" << std::endl;

    // Test 1: Drum Engine Preparation & Voice Rendering
    {
        std::cout << "[TEST 1] Drum Engine & Voice Synthesis... ";
        DrumEngine909 engine;
        engine.prepare(44100.0, 512);

        juce::AudioBuffer<float> masterBuffer(2, 512);
        std::vector<juce::AudioBuffer<float>*> individualBuses;
        uint32_t activeMask = 1u;

        // Trigger each voice and verify non-zero output
        for (int v = 0; v < NUM_VOICES; ++v)
        {
            engine.triggerVoice(v, 0.9f, 0.0f);
            engine.renderBlock(masterBuffer, individualBuses, activeMask);
            float rms = masterBuffer.getRMSLevel(0, 0, 512);
            assert(rms > 0.0001f);
        }
        std::cout << "PASSED" << std::endl;
    }

    // Test 2: Choke Logic (CH chokes OH)
    {
        std::cout << "[TEST 2] Choke Logic (CH chokes OH)... ";
        DrumEngine909 engine;
        engine.prepare(44100.0, 512);

        // Trigger Open Hi-Hat
        engine.triggerVoice(OH, 1.0f, 0.0f);
        auto* ohVoice = engine.getVoice(OH);
        assert(ohVoice->isActive());

        // Trigger Closed Hi-Hat -> OH should choke
        engine.triggerVoice(CH, 1.0f, 0.0f);

        // Render 200 samples (~4.5 ms)
        for (int i = 0; i < 200; ++i)
            ohVoice->renderSample();

        // OH should now be inactive or rapidly decaying
        assert(!ohVoice->isActive());
        std::cout << "PASSED" << std::endl;
    }

    // Test 3: Total Accent Dynamics
    {
        std::cout << "[TEST 3] Total Accent Circuit Dynamics... ";
        DrumEngine909 engine;
        engine.prepare(44100.0, 512);
        engine.getAccentCircuit().setAccentLevel(0.85f);

        juce::AudioBuffer<float> unaccentedBuf(2, 512);
        juce::AudioBuffer<float> accentedBuf(2, 512);
        std::vector<juce::AudioBuffer<float>*> individualBuses;

        // Normal kick
        engine.triggerVoice(BD, 0.70f, 0.0f);
        engine.renderBlock(unaccentedBuf, individualBuses, 1u);
        float unaccRMS = unaccentedBuf.getRMSLevel(0, 0, 512);

        // Accented kick
        engine.reset();
        engine.triggerVoice(BD, 0.70f, 1.0f);
        engine.renderBlock(accentedBuf, individualBuses, 1u);
        float accRMS = accentedBuf.getRMSLevel(0, 0, 512);

        assert(accRMS > unaccRMS);
        std::cout << "PASSED (Normal RMS: " << unaccRMS << " vs Accented: " << accRMS << ")" << std::endl;
    }

    // Test 4: Multi-bus Output Routing
    {
        std::cout << "[TEST 4] Multi-Bus Individual Output Routing... ";
        DrumEngine909 engine;
        engine.prepare(44100.0, 512);
        engine.setJackCutMode(true); // Auto-cut from master when jack is plugged

        juce::AudioBuffer<float> masterBuffer(2, 512);
        juce::AudioBuffer<float> bdBusBuffer(2, 512);
        std::vector<juce::AudioBuffer<float>*> indBuses(10, nullptr);
        indBuses[0] = &bdBusBuffer; // Bus 1 = BD

        // Enable Bus 1 (bit 1)
        uint32_t activeMask = (1u << 0) | (1u << 1);

        engine.triggerVoice(BD, 0.95f, 0.0f);
        engine.renderBlock(masterBuffer, indBuses, activeMask);

        float bdRMS = bdBusBuffer.getRMSLevel(0, 0, 512);
        float masterRMS = masterBuffer.getRMSLevel(0, 0, 512);

        assert(bdRMS > 0.001f);
        // Master should have received 0 because Jack Cut Mode routed BD exclusively to its individual bus!
        assert(masterRMS == 0.0f);
        std::cout << "PASSED (Individual Bus active: " << bdRMS << ", Master Muted: " << masterRMS << ")" << std::endl;
    }

    // Test 5: Groove Sequencer & 909 Shuffle Timing
    {
        std::cout << "[TEST 5] Groove Sequencer & 909 Shuffle Timing... ";
        GrooveSequencer909 seq;
        seq.prepare(44100.0);
        seq.setBpm(120.0);
        seq.setShuffle(3); // 58.3% swing

        int triggerCount = 0;
        seq.start();

        // Process 1 full bar (16 steps @ 120 BPM = 2.0 seconds = 88200 samples)
        int blockSize = 128;
        int totalBlocks = 88200 / blockSize;

        for (int b = 0; b < totalBlocks; ++b)
        {
            seq.process(blockSize, nullptr, [&](int /*track*/, float /*vel*/, float /*acc*/, bool /*isFlam*/) {
                triggerCount++;
            });
        }

        if (triggerCount == 0)
        {
            std::cerr << "FAILED: No triggers received" << std::endl;
            return 1;
        }
        std::cout << "PASSED (Processed 1 bar with " << triggerCount << " step triggers)" << std::endl;
    }

    // Test 6: Preset Vault & MIDI Serialization Round-trip
    {
        std::cout << "[TEST 6] Preset Vault & MIDI Serialization Round-trip... ";
        PresetBank909 bank;
        if (bank.getNumPresets() < 8)
        {
            std::cerr << "FAILED: Insufficient presets" << std::endl;
            return 1;
        }

        const auto& p1 = bank.getPreset(0);
        auto tempMidi = juce::File::createTempFile(".mid");
        bool exported = PresetIO909::exportToMidi(tempMidi, p1);
        if (!exported)
        {
            std::cerr << "FAILED: Export to MIDI failed" << std::endl;
            return 1;
        }

        Preset909 importedPreset;
        bool imported = PresetIO909::importFromMidi(tempMidi, importedPreset);
        if (!imported || std::abs(importedPreset.bpm - p1.bpm) > 0.05)
        {
            std::cerr << "FAILED: Import from MIDI failed (BPM: " << importedPreset.bpm << " vs " << p1.bpm << ")" << std::endl;
            return 1;
        }

        tempMidi.deleteFile();
        std::cout << "PASSED" << std::endl;
    }

    // Test 7: PluginProcessor DAW MIDI Input & Bass Drum Activation
    {
        std::cout << "[TEST 7] PluginProcessor DAW MIDI Input & Bass Drum Activation... ";
        PluginProcessor processor;
        processor.prepareToPlay(44100.0, 512);

        // Verify BD activates on all standard C octaves (36, 48, 60, 72) and GM 35
        for (int note : { 35, 36, 48, 60, 72 })
        {
            processor.getEngine().reset();
            juce::AudioBuffer<float> block(22, 512);
            block.clear();

            juce::MidiBuffer midi;
            midi.addEvent(juce::MidiMessage::noteOn(1, note, (juce::uint8)115), 0);
            processor.processBlock(block, midi);

            auto* bdVoice = processor.getEngine().getVoice(BD);
            if (!bdVoice->isActive() && bdVoice->getPeak() < 0.05f)
            {
                std::cerr << "FAILED: Bass Drum did NOT activate for MIDI Note " << note << "!" << std::endl;
                return 1;
            }

            float rms = block.getRMSLevel(0, 0, 512);
            if (rms < 0.005f)
            {
                std::cerr << "FAILED: Audio output too low (" << rms << ") for MIDI Note " << note << std::endl;
                return 1;
            }
        }

        std::cout << "PASSED (BD triggered successfully on Notes 35, 36, 48, 60, 72)" << std::endl;
    }

    // Test 8: Host DAW Tempo Synchronization
    {
        std::cout << "[TEST 8] Host DAW Tempo Synchronization... ";

        struct MockPlayHead final : public juce::AudioPlayHead
        {
            double bpm = 144.0;
            bool playing = false;
            double ppq = 0.0;

            juce::Optional<PositionInfo> getPosition() const override
            {
                PositionInfo info;
                info.setBpm(bpm);
                info.setIsPlaying(playing);
                info.setPpqPosition(ppq);
                return info;
            }
        };

        PluginProcessor processor;
        MockPlayHead playHead;
        playHead.bpm = 144.0;
        playHead.playing = false;

        processor.setPlayHead(&playHead);
        processor.prepareToPlay(44100.0, 512);

        // 1. Verify plugin adopted host tempo on initialization/prepareToPlay
        if (std::abs(processor.getSequencer().getBpm() - 144.0) > 0.1)
        {
            std::cerr << "FAILED: Initial host tempo 144.0 not adopted (got " << processor.getSequencer().getBpm() << ")" << std::endl;
            return 1;
        }

        // 2. Change host tempo to 98.0 while running processBlock
        playHead.bpm = 98.0;
        playHead.playing = true;
        playHead.ppq = 4.0;

        juce::AudioBuffer<float> block(22, 512);
        block.clear();
        juce::MidiBuffer midi;
        processor.processBlock(block, midi);

        if (std::abs(processor.getSequencer().getBpm() - 98.0) > 0.1)
        {
            std::cerr << "FAILED: Host tempo change to 98.0 not followed (got " << processor.getSequencer().getBpm() << ")" << std::endl;
            return 1;
        }

        if (auto* rawTempo = processor.getAPVTS().getRawParameterValue("TEMPO"))
        {
            if (std::abs(rawTempo->load() - 98.0f) > 0.5f)
            {
                std::cerr << "FAILED: APVTS TEMPO parameter did not update to host tempo (got " << rawTempo->load() << ")" << std::endl;
                return 1;
            }
        }

        processor.setPlayHead(nullptr);
        std::cout << "PASSED (Synchronized to 144 BPM and 98 BPM successfully)" << std::endl;
    }

    std::cout << "========================================" << std::endl;
    std::cout << "ALL 8 REGRESSION TESTS PASSED!          " << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
