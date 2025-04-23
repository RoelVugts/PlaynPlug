#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "../Utils/Macros.h"
#include "../Utils/FileWatcher.h"
#include "PluginParameter.h"
#include "../Utils/LibraryLoader.h"
#include "../Data/Data.h"
#include "../Utils/Config.h"

#include <API.h>

#define NUM_PARAMS 512


//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void reloadParameters(bool setToDefaultValue = false);
    void setParameterListeners();

    void prepareProcessor();
    void setNewLibrary(juce::File file);


    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    class ApvtsListener : public juce::AudioProcessorValueTreeState::Listener {
    public:
        ApvtsListener(AudioPluginAudioProcessor* processor) : audioProcessor(processor) {};

        void parameterChanged(const juce::String& parameterID, float newValue) override;

    private:
        AudioPluginAudioProcessor* audioProcessor;
    };

    juce::AudioProcessorValueTreeState apvts;
    ApvtsListener apvtsListener;

    LibraryLoader libLoader;
    FileWatcher libFileWatcher;

    DataSettings dataSettings { juce::ValueTree(DataSettings::getType()) };

    Config config;


private:

    double sampleRate { 0 };
    int samplesPerBlock { 0 };

    ParamFiFo paramFifo;
    MidiFiFo midiFifo;

    class HostInfoUpdater : public juce::AsyncUpdater {
    public:

        explicit HostInfoUpdater(AudioPluginAudioProcessor& processor) : processor(processor) {}

        void handleAsyncUpdate() override
        {
            auto const details = juce::AudioProcessorListener::ChangeDetails {}.withParameterInfoChanged(true);
            processor.updateHostDisplay(details);
        }

        AudioPluginAudioProcessor& processor;
    };

    HostInfoUpdater hostInfoUpdater;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)


};
