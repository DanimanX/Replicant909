#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets/PresetIO909.h"
#include "GUI/Theme909.h"

namespace Replicant909
{
    PluginProcessor::PluginProcessor()
        : AudioProcessor(BusesProperties()
              .withOutput("Master", juce::AudioChannelSet::stereo(), true)
              .withOutput("Bass Drum", juce::AudioChannelSet::stereo(), false)
              .withOutput("Snare Drum", juce::AudioChannelSet::stereo(), false)
              .withOutput("Low Tom", juce::AudioChannelSet::stereo(), false)
              .withOutput("Mid Tom", juce::AudioChannelSet::stereo(), false)
              .withOutput("Hi Tom", juce::AudioChannelSet::stereo(), false)
              .withOutput("Rim Shot", juce::AudioChannelSet::stereo(), false)
              .withOutput("Hand Clap", juce::AudioChannelSet::stereo(), false)
              .withOutput("Hi-Hat", juce::AudioChannelSet::stereo(), false)
              .withOutput("Crash", juce::AudioChannelSet::stereo(), false)
              .withOutput("Ride", juce::AudioChannelSet::stereo(), false)),
          apvts(*this, nullptr, "REPLICANT909_PARAMS", createParameterLayout())
    {
        individualBusPtrs.resize(10, nullptr);
    }

    PluginProcessor::~PluginProcessor() = default;

    bool PluginProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
    {
        // An instrument synthesizer has no input channels
        if (!layouts.getMainInputChannelSet().isDisabled())
            return false;

        // Master bus must be stereo or mono
        const auto& master = layouts.getMainOutputChannelSet();
        if (master != juce::AudioChannelSet::stereo() && master != juce::AudioChannelSet::mono())
            return false;

        // Individual buses can be disabled, mono, or stereo
        for (int i = 1; i < layouts.outputBuses.size(); ++i)
        {
            const auto& bus = layouts.outputBuses[i];
            if (!bus.isDisabled() && bus != juce::AudioChannelSet::stereo() && bus != juce::AudioChannelSet::mono())
                return false;
        }

        return true;
    }

    void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        engine.prepare(sampleRate, samplesPerBlock);
        sequencer.prepare(sampleRate);
        syncHostPlayHead();
        syncParametersToEngine();
    }

    void PluginProcessor::releaseResources()
    {
        engine.reset();
        sequencer.reset();
    }

    void PluginProcessor::syncHostPlayHead() noexcept
    {
        double hostBpm = 0.0;

        if (auto* hostPlayHead = getPlayHead())
        {
            if (auto position = hostPlayHead->getPosition())
            {
                if (auto bpm = position->getBpm())
                {
                    if (*bpm > 1.0)
                        hostBpm = *bpm;
                }
            }
        }

        const bool syncHost = sequencer.isSyncToHost();

        if (hostBpm > 1.0)
        {
            const bool hostBpmChanged = !hasSyncedInitialHostTempo
                                        || std::abs(hostBpm - lastHostBpm) > 0.01;
            hasSyncedInitialHostTempo = true;
            lastHostBpm = hostBpm;

            if (syncHost || hostBpmChanged)
            {
                const float clampedHostBpm = juce::jlimit(30.0f, 300.0f, static_cast<float>(hostBpm));
                sequencer.setBpm(clampedHostBpm);

                if (auto* tempoParam = apvts.getParameter("TEMPO"))
                {
                    if (auto* rawVal = apvts.getRawParameterValue("TEMPO"))
                    {
                        if (std::abs(*rawVal - clampedHostBpm) > 0.05f)
                        {
                            tempoParam->setValueNotifyingHost(tempoParam->convertTo0to1(clampedHostBpm));
                        }
                    }
                }
            }
        }
    }

    void PluginProcessor::syncParametersToEngine()
    {
        // Global parameters
        if (auto* p = apvts.getRawParameterValue("SYNC_HOST"))
            sequencer.setSyncToHost(*p > 0.5f);

        if (!sequencer.isSyncToHost())
        {
            if (auto* p = apvts.getRawParameterValue("TEMPO"))
                sequencer.setBpm(static_cast<double>(*p));
        }

        if (auto* p = apvts.getRawParameterValue("TOTAL_ACCENT"))
            engine.getAccentCircuit().setAccentLevel(*p);

        if (auto* p = apvts.getRawParameterValue("MASTER_VOL"))
            engine.setMasterVolume(*p);

        if (auto* p = apvts.getRawParameterValue("SHUFFLE"))
            sequencer.setShuffle(static_cast<int>(*p));

        if (auto* p = apvts.getRawParameterValue("FLAM"))
            sequencer.setFlamInterval(static_cast<int>(*p));

        if (auto* p = apvts.getRawParameterValue("JACK_CUT"))
            engine.setJackCutMode(*p > 0.5f);

        if (auto* p = apvts.getRawParameterValue("TRIGGER_MULTI"))
            engine.setMultiplierEnabled(*p > 0.5f);

        if (auto* p = apvts.getRawParameterValue("MULTI_COUNT"))
        {
            int choiceIdx = juce::jlimit(0, 6, static_cast<int>(*p));
            static constexpr int MULTIPLIER_VALUES[7] = { 1, 2, 3, 4, 6, 8, 16 };
            engine.setMultiplierCount(MULTIPLIER_VALUES[choiceIdx]);
        }

        engine.setBpm(sequencer.getBpm());

        if (auto* p = apvts.getRawParameterValue("SKIN"))
        {
            int skinIdx = static_cast<int>(*p);
            if (skinIdx != lastSkinIndex)
            {
                lastSkinIndex = skinIdx;
                ThemeManager::getInstance().setSkin(static_cast<SkinId>(skinIdx));
            }
        }

        // Voice parameters
        for (int i = 0; i < NUM_VOICES; ++i)
        {
            auto* voice = engine.getVoice(i);
            if (voice == nullptr)
                continue;

            juce::String prefix = VOICE_SHORT_NAMES[i];

            if (auto* p = apvts.getRawParameterValue(prefix + "_LEVEL"))
                voice->setLevel(*p);

            if (auto* p = apvts.getRawParameterValue(prefix + "_PAN"))
                voice->setPan(*p);

            if (auto* p = apvts.getRawParameterValue(prefix + "_TUNE"))
                voice->setTune(*p);

            if (auto* p = apvts.getRawParameterValue(prefix + "_ATTACK"))
                voice->setAttack(*p);

            if (auto* p = apvts.getRawParameterValue(prefix + "_TONE"))
                voice->setTone(*p);

            if (auto* p = apvts.getRawParameterValue(prefix + "_DECAY"))
                voice->setDecay(*p);

            if (auto* p = apvts.getRawParameterValue(prefix + "_SNAPPY"))
                voice->setSnappy(*p);

            if (auto* p = apvts.getRawParameterValue(prefix + "_REVERB"))
            {
                if (auto* clap = dynamic_cast<HandClapVoice*>(voice))
                    clap->setReverb(*p);
            }
        }
    }

    void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
    {
        juce::ScopedNoDenormals noDenormals;

        syncHostPlayHead();
        syncParametersToEngine();

        // 1. Process MIDI Note triggers
        engine.processMidi(midiMessages);

        // 2. Advance Groove Sequencer & trigger voices
        sequencer.process(buffer.getNumSamples(), getPlayHead(),
            [this](int track, float velocity, float accent, bool isFlam) {
                engine.triggerVoice(track, velocity, accent, isFlam);
            });

        // 3. Multi-bus output management
        // Master stereo buffer
        auto masterBusBuffer = getBusBuffer(buffer, false, 0);

        // Determine which output buses are enabled by the host
        activeBusesMask = 1u; // Master is bit 0
        int numOutputBuses = getBusCount(false);

        std::array<juce::AudioBuffer<float>, 10> indBusBuffers;
        for (int b = 1; b < numOutputBuses && b <= 10; ++b)
        {
            if (getBus(false, b)->isEnabled())
            {
                indBusBuffers[b - 1] = getBusBuffer(buffer, false, b);
                if (indBusBuffers[b - 1].getNumChannels() > 0)
                {
                    activeBusesMask |= (1u << b);
                    individualBusPtrs[b - 1] = &indBusBuffers[b - 1];
                }
                else
                {
                    individualBusPtrs[b - 1] = nullptr;
                }
            }
            else
            {
                individualBusPtrs[b - 1] = nullptr;
            }
        }

        // Render voices to master and individual buses
        engine.renderBlock(masterBusBuffer, individualBusPtrs, activeBusesMask);
    }

    juce::AudioProcessorEditor* PluginProcessor::createEditor()
    {
        return new PluginEditor(*this);
    }

    void PluginProcessor::setCurrentProgram(int index)
    {
        if (index >= 0 && index < presetBank.getNumPresets())
        {
            currentProgram = index;
            const auto& p = presetBank.getPreset(index);
            sequencer.getActivePattern() = p.pattern;
            if (!sequencer.isSyncToHost())
            {
                sequencer.setBpm(p.bpm);
                if (auto* param = apvts.getParameter("TEMPO"))
                    param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(p.bpm)));
            }
            sequencer.setShuffle(p.shuffle);
            sequencer.setFlamInterval(p.flamInterval);
            engine.getAccentCircuit().setAccentLevel(p.totalAccent);
            engine.setMasterVolume(p.masterVolume);

            // Update APVTS parameters
            if (auto* param = apvts.getParameter("TOTAL_ACCENT"))
                param->setValueNotifyingHost(p.totalAccent);
            if (auto* param = apvts.getParameter("MASTER_VOL"))
                param->setValueNotifyingHost(p.masterVolume);
            if (auto* param = apvts.getParameter("SHUFFLE"))
                param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(p.shuffle)));
        }
    }

    const juce::String PluginProcessor::getProgramName(int index)
    {
        if (index >= 0 && index < presetBank.getNumPresets())
            return presetBank.getPreset(index).name;
        return {};
    }

    void PluginProcessor::changeProgramName(int index, const juce::String& newName)
    {
        if (index >= 0 && index < presetBank.getNumPresets())
            presetBank.getPreset(index).name = newName;
    }

    void PluginProcessor::getStateInformation(juce::MemoryBlock& destData)
    {
        auto state = apvts.copyState();
        std::unique_ptr<juce::XmlElement> xml(state.createXml());

        // Save active pattern data
        Preset909 currentPatch;
        currentPatch.pattern = sequencer.getActivePattern();
        currentPatch.bpm = sequencer.getBpm();
        currentPatch.shuffle = sequencer.getShuffle();
        currentPatch.flamInterval = sequencer.getFlamInterval();
        currentPatch.totalAccent = engine.getAccentCircuit().getAccentLevel();
        currentPatch.masterVolume = engine.getMasterVolume();

        auto patXml = PresetIO909::toXml(currentPatch);
        if (patXml != nullptr)
            xml->addChildElement(patXml.release());

        copyXmlToBinary(*xml, destData);
    }

    void PluginProcessor::setStateInformation(const void* data, int sizeInBytes)
    {
        std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
        if (xmlState != nullptr)
        {
            if (auto* patXml = xmlState->getChildByName("REPLICANT_909_PRESET"))
            {
                Preset909 currentPatch;
                if (PresetIO909::fromXml(*patXml, currentPatch))
                {
                    sequencer.getActivePattern() = currentPatch.pattern;
                    if (!sequencer.isSyncToHost())
                        sequencer.setBpm(currentPatch.bpm);
                    sequencer.setShuffle(currentPatch.shuffle);
                    sequencer.setFlamInterval(currentPatch.flamInterval);
                    engine.getAccentCircuit().setAccentLevel(currentPatch.totalAccent);
                    engine.setMasterVolume(currentPatch.masterVolume);
                }
            }

            if (xmlState->hasTagName(apvts.state.getType()))
                apvts.replaceState(juce::ValueTree::fromXml(*xmlState));

            syncHostPlayHead();
        }
    }

    juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

        // Global Parameters
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ "TEMPO", 1 }, "Tempo BPM",
            juce::NormalisableRange<float>(30.0f, 300.0f, 0.1f), 128.0f));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ "TOTAL_ACCENT", 1 }, "Total Accent",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.70f));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ "MASTER_VOL", 1 }, "Master Volume",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.85f));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{ "SHUFFLE", 1 }, "909 Shuffle",
            juce::StringArray{ "1 (50.0%)", "2 (54.2%)", "3 (58.3%)", "4 (62.5%)", "5 (66.7%)", "6 (70.8%)" }, 0));

        params.push_back(std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID{ "FLAM", 1 }, "Flam Interval", 1, 8, 3));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ "SYNC_HOST", 1 }, "Sync to Host", true));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ "JACK_CUT", 1 }, "Individual Jack Auto-Cut", false));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ "TRIGGER_MULTI", 1 }, "Note Multiplier", false));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{ "MULTI_COUNT", 1 }, "Multiplier Count",
            juce::StringArray{ "1x (Off)", "2x (Double)", "3x (Triplet)", "4x (Quad)", "6x (Sextuplet)", "8x (Octa Roll)", "16x (Fast Roll)" }, 1));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{ "SKIN", 1 }, "Panel Skin",
            juce::StringArray{ "Classic 909", "Modern Obsidian" }, 0));

        // Voice Parameters
        for (int i = 0; i < NUM_VOICES; ++i)
        {
            juce::String prefix = VOICE_SHORT_NAMES[i];
            juce::String fullName = VOICE_FULL_NAMES[i];

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID{ prefix + "_LEVEL", 1 }, fullName + " Level",
                juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.85f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID{ prefix + "_PAN", 1 }, fullName + " Pan",
                juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.50f));

            if (i == BD || i == SD || i == LT || i == MT || i == HT || i == CC || i == RC)
            {
                params.push_back(std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID{ prefix + "_TUNE", 1 }, fullName + " Tune",
                    juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.50f));
            }

            if (i == BD)
            {
                params.push_back(std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID{ prefix + "_ATTACK", 1 }, fullName + " Attack Click",
                    juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.50f));
            }
            else if (i == SD)
            {
                params.push_back(std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID{ prefix + "_TONE", 1 }, fullName + " Tone",
                    juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.50f));
            }

            if (i == SD)
            {
                params.push_back(std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID{ prefix + "_SNAPPY", 1 }, fullName + " Snappy",
                    juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.50f));
            }
            else if (i == CP)
            {
                params.push_back(std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID{ prefix + "_REVERB", 1 }, fullName + " Reverb Tail",
                    juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.50f));
            }
            else if (i != RS)
            {
                params.push_back(std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID{ prefix + "_DECAY", 1 }, fullName + " Decay",
                    juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.50f));
            }
        }

        return { params.begin(), params.end() };
    }
}

// Juce entry point
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Replicant909::PluginProcessor();
}
