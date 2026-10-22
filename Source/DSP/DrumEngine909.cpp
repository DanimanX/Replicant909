#include "DrumEngine909.h"
#include <algorithm>
#include <cmath>

namespace Replicant909
{
    DrumEngine909::DrumEngine909()
    {
        voices[BD] = std::make_unique<BassDrumVoice>();
        voices[SD] = std::make_unique<SnareDrumVoice>();
        voices[LT] = std::make_unique<TomVoice>(TOM_LOW);
        voices[MT] = std::make_unique<TomVoice>(TOM_MID);
        voices[HT] = std::make_unique<TomVoice>(TOM_HI);
        voices[RS] = std::make_unique<RimShotVoice>();
        voices[CP] = std::make_unique<HandClapVoice>();
        voices[CH] = std::make_unique<CymbalVoice>(CYMBAL_CH);
        voices[OH] = std::make_unique<CymbalVoice>(CYMBAL_OH);
        voices[CC] = std::make_unique<CymbalVoice>(CYMBAL_CRASH);
        voices[RC] = std::make_unique<CymbalVoice>(CYMBAL_RIDE);

        // Default TR-909 Pan positions
        // BD, SD, RS, CP centered
        // Toms panned across stereo field
        // Hats slightly off-center
        // Crash left, Ride right
        voices[BD]->setPan(0.50f);
        voices[SD]->setPan(0.50f);
        voices[LT]->setPan(0.35f);
        voices[MT]->setPan(0.50f);
        voices[HT]->setPan(0.65f);
        voices[RS]->setPan(0.45f);
        voices[CP]->setPan(0.55f);
        voices[CH]->setPan(0.40f);
        voices[OH]->setPan(0.40f);
        voices[CC]->setPan(0.25f);
        voices[RC]->setPan(0.75f);
    }

    void DrumEngine909::prepare(double newSampleRate, int maxBlockSize)
    {
        sampleRate = std::max(8000.0, newSampleRate);
        for (auto& v : voices)
        {
            if (v != nullptr)
                v->prepare(sampleRate);
        }
        voiceScratchBuffer.setSize(1, std::max(64, maxBlockSize));
        reset();
    }

    void DrumEngine909::reset()
    {
        for (auto& v : voices)
        {
            if (v != nullptr)
                v->reset();
        }
        for (auto& q : queuedTriggers)
            q.active = false;
        for (auto& h : voiceHolds)
            h.isHeld = false;
    }

    VoiceBase* DrumEngine909::getVoice(int index) noexcept
    {
        if (index >= 0 && index < NUM_VOICES)
            return voices[index].get();
        return nullptr;
    }

    const VoiceBase* DrumEngine909::getVoice(int index) const noexcept
    {
        if (index >= 0 && index < NUM_VOICES)
            return voices[index].get();
        return nullptr;
    }

    void DrumEngine909::triggerVoice(int voiceIndex, float velocity, float stepAccent, bool /*isFlam*/)
    {
        if (voiceIndex < 0 || voiceIndex >= NUM_VOICES)
            return;

        if (!isVoiceAudible(voiceIndex))
            return;

        // Calculate dynamic accent scale
        bool hasAccent = (stepAccent > 0.0f) || (velocity > 0.75f);
        float accentVal = accentCircuit.computeAccentGain(hasAccent, velocity);

        // Choke handling: Closed Hi-Hat (CH) chokes Open Hi-Hat (OH)
        if (voiceIndex == CH)
        {
            if (voices[OH] != nullptr)
                voices[OH]->choke();
        }

        if (voices[voiceIndex] != nullptr)
            voices[voiceIndex]->trigger(velocity, accentVal);
    }

    void DrumEngine909::triggerVoiceMultiplied(int voiceIndex, float velocity, float stepAccent, int count)
    {
        if (voiceIndex < 0 || voiceIndex >= NUM_VOICES)
            return;

        int mult = (count > 0) ? count : multiplierCount;
        if (mult <= 1)
        {
            triggerVoice(voiceIndex, velocity, stepAccent);
            return;
        }

        // Cancel previous pending triggers for this specific voice
        for (auto& q : queuedTriggers)
        {
            if (q.active && q.voiceIndex == voiceIndex)
                q.active = false;
        }

        // Trigger hit 0 immediately
        triggerVoice(voiceIndex, velocity, stepAccent);

        // Subdivision timing: 1 step (16th note) divided by multiplier
        double bpm = (currentBpm >= 30.0) ? currentBpm : 128.0;
        double stepSeconds = 15.0 / bpm;
        double stepSamples = stepSeconds * sampleRate;
        double intervalSamples = stepSamples / static_cast<double>(mult);

        if (intervalSamples < 64.0)
            intervalSamples = 64.0;

        for (int k = 1; k < mult; ++k)
        {
            int delay = static_cast<int>(std::round(k * intervalSamples));
            float subVel = juce::jlimit(0.20f, 1.0f, velocity * std::pow(0.93f, static_cast<float>(k)));
            float subAcc = (stepAccent > 0.0f) ? (stepAccent * std::pow(0.90f, static_cast<float>(k))) : 0.0f;

            for (auto& q : queuedTriggers)
            {
                if (!q.active)
                {
                    q.voiceIndex = voiceIndex;
                    q.velocity = subVel;
                    q.stepAccent = subAcc;
                    q.samplesUntilTrigger = delay;
                    q.active = true;
                    break;
                }
            }
        }
    }

    void DrumEngine909::triggerSelectedVoice(float velocity, float stepAccent)
    {
        if (selectedVoice < 0 || selectedVoice >= NUM_VOICES)
            selectedVoice = BD;

        if (multiplierEnabled && multiplierCount > 1)
            triggerVoiceMultiplied(selectedVoice, velocity, stepAccent, multiplierCount);
        else
            triggerVoice(selectedVoice, velocity, stepAccent);
    }

    void DrumEngine909::startVoiceHold(int voiceIndex, float velocity, float stepAccent)
    {
        if (voiceIndex < 0 || voiceIndex >= NUM_VOICES)
            return;

        selectedVoice = voiceIndex;

        // Clear any previous queued triggers for this specific voice
        for (auto& q : queuedTriggers)
        {
            if (q.voiceIndex == voiceIndex)
                q.active = false;
        }

        // Trigger hit 0 immediately
        triggerVoice(voiceIndex, velocity, stepAccent);

        if (!multiplierEnabled || multiplierCount <= 1)
            return;

        double bpm = (currentBpm >= 30.0) ? currentBpm : 128.0;
        double stepSeconds = 15.0 / bpm;
        double stepSamples = stepSeconds * sampleRate;
        int intervalSamples = static_cast<int>(std::round(stepSamples / static_cast<double>(multiplierCount)));
        if (intervalSamples < 64)
            intervalSamples = 64;

        auto& h = voiceHolds[voiceIndex];
        h.isHeld = true;
        h.velocity = velocity;
        h.stepAccent = stepAccent;
        h.samplesUntilNextHit = intervalSamples;
        h.rollCounter = 1;
    }

    void DrumEngine909::stopVoiceHold(int voiceIndex)
    {
        if (voiceIndex >= 0 && voiceIndex < NUM_VOICES)
        {
            voiceHolds[voiceIndex].isHeld = false;
            voiceHolds[voiceIndex].samplesUntilNextHit = 0;
            for (auto& q : queuedTriggers)
            {
                if (q.voiceIndex == voiceIndex)
                    q.active = false;
            }
        }
    }

    void DrumEngine909::stopAllVoiceHolds()
    {
        for (int v = 0; v < NUM_VOICES; ++v)
            stopVoiceHold(v);
    }

    bool DrumEngine909::isVoiceHeld(int voiceIndex) const noexcept
    {
        if (voiceIndex >= 0 && voiceIndex < NUM_VOICES)
            return voiceHolds[voiceIndex].isHeld;
        return false;
    }

    void DrumEngine909::processMidi(const juce::MidiBuffer& midiMessages)
    {
        for (const auto metadata : midiMessages)
        {
            const auto msg = metadata.getMessage();
            if (msg.isNoteOn())
            {
                int inst = midiNoteToInstrument(msg.getNoteNumber());
                if (inst >= 0 && inst < NUM_VOICES)
                {
                    selectedVoice = inst;
                    float vel = static_cast<float>(msg.getVelocity()) / 127.0f;
                    float stepAcc = (vel > 0.75f ? 1.0f : 0.0f);
                    if (multiplierEnabled && multiplierCount > 1)
                        startVoiceHold(inst, vel, stepAcc);
                    else
                        triggerVoice(inst, vel, stepAcc);
                }
            }
            else if (msg.isNoteOff())
            {
                int inst = midiNoteToInstrument(msg.getNoteNumber());
                if (inst >= 0 && inst < NUM_VOICES)
                {
                    stopVoiceHold(inst);
                }
            }
        }
    }

    void DrumEngine909::renderBlock(juce::AudioBuffer<float>& masterBuffer,
                                    const std::vector<juce::AudioBuffer<float>*>& individualBuses,
                                    uint32_t activeBusesMask)
    {
        const int numSamples = masterBuffer.getNumSamples();
        const int numMasterChannels = masterBuffer.getNumChannels();

        if (voiceScratchBuffer.getNumSamples() < numSamples)
            voiceScratchBuffer.setSize(1, numSamples, false, false, true);

        // Clear master stereo outputs initially
        masterBuffer.clear();

        // Clear individual bus buffers
        for (size_t b = 0; b < individualBuses.size(); ++b)
        {
            if (individualBuses[b] != nullptr && (activeBusesMask & (1u << (b + 1))))
                individualBuses[b]->clear();
        }

        // Map voices to individual bus indices:
        // Bus 1: BD (inst 0)
        // Bus 2: SD (inst 1)
        // Bus 3: LT (inst 2)
        // Bus 4: MT (inst 3)
        // Bus 5: HT (inst 4)
        // Bus 6: RS (inst 5)
        // Bus 7: CP (inst 6)
        // Bus 8: HH (inst 7 [CH] and inst 8 [OH] share Bus 8!)
        // Bus 9: CC (inst 9)
        // Bus 10: RC (inst 10)
        static constexpr int voiceToBusIndex[NUM_VOICES] = {
            1, // BD
            2, // SD
            3, // LT
            4, // MT
            5, // HT
            6, // RS
            7, // CP
            8, // CH -> HH
            8, // OH -> HH
            9, // CC
            10 // RC
        };

        float* scratchPtr = voiceScratchBuffer.getWritePointer(0);

        for (int v = 0; v < NUM_VOICES; ++v)
        {
            auto* voice = voices[v].get();
            if (voice == nullptr)
                continue;

            // Decay recent peak level for UI LED meter
            voice->decayPeak();

            // Collect triggers for voice v that occur in this audio block
            struct TriggerEvent {
                int sampleOffset;
                float velocity;
                float stepAccent;
            };
            std::array<TriggerEvent, 16> blockEvents;
            int numEvents = 0;

            // 1. Collect one-shot triggers
            for (auto& q : queuedTriggers)
            {
                if (q.active && q.voiceIndex == v && q.samplesUntilTrigger < numSamples)
                {
                    if (numEvents < static_cast<int>(blockEvents.size()))
                    {
                        blockEvents[numEvents++] = {
                            std::max(0, q.samplesUntilTrigger),
                            q.velocity,
                            q.stepAccent
                        };
                    }
                    q.active = false;
                }
            }

            // 2. Collect sustained hold roll triggers
            if (voiceHolds[v].isHeld && multiplierEnabled && multiplierCount > 1)
            {
                double bpm = (currentBpm >= 30.0) ? currentBpm : 128.0;
                double stepSeconds = 15.0 / bpm;
                double stepSamples = stepSeconds * sampleRate;
                int intervalSamples = static_cast<int>(std::round(stepSamples / static_cast<double>(multiplierCount)));
                if (intervalSamples < 64)
                    intervalSamples = 64;

                auto& h = voiceHolds[v];
                while (h.samplesUntilNextHit < numSamples && numEvents < static_cast<int>(blockEvents.size()))
                {
                    bool isMainHit = (h.rollCounter % multiplierCount == 0);
                    float nextVel = isMainHit ? h.velocity : (h.velocity * 0.88f);
                    float nextAcc = isMainHit ? h.stepAccent : 0.0f;

                    blockEvents[numEvents++] = {
                        std::max(0, h.samplesUntilNextHit),
                        nextVel,
                        nextAcc
                    };

                    h.rollCounter++;
                    h.samplesUntilNextHit += intervalSamples;
                }

                h.samplesUntilNextHit -= numSamples;
            }

            if (numEvents > 1)
            {
                std::sort(blockEvents.begin(), blockEvents.begin() + numEvents,
                          [](const TriggerEvent& a, const TriggerEvent& b) {
                              return a.sampleOffset < b.sampleOffset;
                          });
            }

            if (numEvents == 0 && !voice->isActive())
                continue;

            // Render voice mono audio into scratch buffer
            if (numEvents == 0)
            {
                for (int s = 0; s < numSamples; ++s)
                    scratchPtr[s] = voice->renderSample();
            }
            else
            {
                int curSample = 0;
                for (int e = 0; e < numEvents; ++e)
                {
                    int eventOffset = blockEvents[e].sampleOffset;
                    if (eventOffset > curSample)
                    {
                        if (voice->isActive())
                        {
                            for (int s = curSample; s < eventOffset; ++s)
                                scratchPtr[s] = voice->renderSample();
                        }
                        else
                        {
                            for (int s = curSample; s < eventOffset; ++s)
                                scratchPtr[s] = 0.0f;
                        }
                        curSample = eventOffset;
                    }

                    triggerVoice(v, blockEvents[e].velocity, blockEvents[e].stepAccent);
                }

                for (int s = curSample; s < numSamples; ++s)
                    scratchPtr[s] = voice->renderSample();
            }

            int busIdx = voiceToBusIndex[v]; // 1..10
            bool busActive = (activeBusesMask & (1u << busIdx)) != 0;

            // Route to Individual Bus if enabled
            if (busActive && (busIdx - 1) < static_cast<int>(individualBuses.size()))
            {
                auto* indBuf = individualBuses[busIdx - 1];
                if (indBuf != nullptr)
                {
                    int indChans = indBuf->getNumChannels();
                    for (int ch = 0; ch < indChans; ++ch)
                        indBuf->addFrom(ch, 0, voiceScratchBuffer, 0, 0, numSamples);
                }
            }

            // Route to Master Stereo Mix
            // If jackCutMode is enabled and the individual bus is active, we mute it from Master (TR-909 jack behavior)
            bool routeToMaster = !(jackCutMode && busActive);
            if (routeToMaster)
            {
                float pan = voice->getPan();
                float panAngle = juce::jlimit(0.0f, 1.0f, pan) * HALF_PI;
                float gainL = std::cos(panAngle);
                float gainR = std::sin(panAngle);

                if (numMasterChannels >= 2)
                {
                    masterBuffer.addFrom(0, 0, voiceScratchBuffer, 0, 0, numSamples, gainL);
                    masterBuffer.addFrom(1, 0, voiceScratchBuffer, 0, 0, numSamples, gainR);
                }
                else if (numMasterChannels == 1)
                {
                    masterBuffer.addFrom(0, 0, voiceScratchBuffer, 0, 0, numSamples, 0.707f);
                }
            }
        }

        // Advance remaining queued triggers past this block
        for (auto& q : queuedTriggers)
        {
            if (q.active)
                q.samplesUntilTrigger -= numSamples;
        }

        // Apply Master Volume to Master output bus
        masterBuffer.applyGain(masterVolume);
    }
}
