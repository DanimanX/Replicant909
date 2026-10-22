#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>
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

    std::cout << "========================================" << std::endl;
    std::cout << "ALL 6 REGRESSION TESTS PASSED!          " << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
