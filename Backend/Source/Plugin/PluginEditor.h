#pragma once

#include "PluginProcessor.h"
#include "../Gui/EditorContent.h"
#include "../Data/Properties.h"
#include "../Utils/Config.h"
#include "../Data/Data.h"


//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void reload(bool setParametersToDefault = false);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

    std::unique_ptr<EditorContent> content;

    SettingsSection settingsSection;
    static constexpr int settingsHeight { 20 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
