#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), settingsSection(p, processorRef.config)
{
    juce::ignoreUnused (processorRef);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 450);

    addAndMakeVisible(settingsSection);

    processorRef.config.onReload = [this]() { reload(true); };
    reload(false);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void AudioPluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    settingsSection.setBounds(0, bounds.getHeight() - settingsHeight, bounds.getWidth(), settingsHeight);
}

void AudioPluginAudioProcessorEditor::reload(bool setParametersToDefault)
{
    // Reload parameters first so we can attach to them in the editor
    processorRef.reloadParameters(setParametersToDefault);

    content = std::make_unique<EditorContent>(processorRef, processorRef.config);
    addAndMakeVisible(*content, 0);

    setSize(processorRef.config.width, processorRef.config.height);
    content->setBounds(0, 0, processorRef.config.width, processorRef.config.height);
}