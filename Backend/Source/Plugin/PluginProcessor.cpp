#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts(*this, nullptr, "apvts", createParameters()) , apvtsListener(this)
                        , hostInfoUpdater(*this), config(dataSettings)
{
    setParameterListeners();

    libFileWatcher.onChange = [this]() { libLoader.reloadLibrary(); };
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
    libLoader.unloadLibrary();
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);

    this->sampleRate = sampleRate;
    this->samplesPerBlock = samplesPerBlock;

    prepareProcessor();
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void printBinary(uint8_t value) {
    for (int i = 7; i >= 0; i--) { // Iterate through bits from most to least significant
        printf("%d", (value >> i) & 1); // Extract and print each bit
    }
    printf("\n");
}


void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{

    juce::MidiBufferIterator it = midiMessages.cbegin();
    while(it != midiMessages.cend()) {
        uint8_t bytes[3] = { 0, 0 ,0 };
        const uint8_t* data = (*it).data;
        for(int byte = 0; byte < (*it).numBytes; byte++) {

            /** MIDI packet expects 3 bytes: statusByte, dataByte1, dataByte2. */
            assert(byte < 3);

            uint8_t val = (*data++);
            bytes[byte] = val;
        }
        it++;
        MidiMessage msg = MidiMessage(bytes[0], bytes[1], bytes[2]);
        midiFifo.push(msg);
    }


    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    AudioBuffer audioBuffer(buffer.getArrayOfWritePointers(), buffer.getNumChannels(), buffer.getNumSamples());

    if (libLoader.suspendAudio)
    {
        for (int channel = 0; channel < totalNumOutputChannels; channel++) {
            float *channelData = buffer.getWritePointer(channel);
            for (int sample = 0; sample < buffer.getNumSamples(); sample++) {
                channelData[sample] = 0.0f;
            }
        }
    }
    else
    {
        if (auto* processor = libLoader.getProcessor())
            processor->process(audioBuffer, paramFifo, midiFifo);
    }

    // Empty queue if user did not
    ParamMessage msg;
    while (paramFifo.pop(msg));
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ValueTree audioState = apvts.copyState();
    juce::ValueTree dataState = dataSettings.getTree().createCopy();

    juce::ValueTree pluginState("PLUGIN");
    pluginState.appendChild(audioState, nullptr);
    pluginState.appendChild(dataState, nullptr);

    std::unique_ptr<juce::XmlElement> xml (pluginState.createXml());
    copyXmlToBinary(*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr)
    {
        const juce::ValueTree pluginState = juce::ValueTree::fromXml(*xml);
        if (pluginState.isValid())
        {
            juce::ValueTree audioState = pluginState.getChildWithName("apvts");
            if (audioState.isValid())
                apvts.replaceState(audioState);

            juce::ValueTree dataState = pluginState.getChildWithName(DataSettings::getType());
            if (dataState.isValid())
            {
                dataSettings.setState(dataState);

                // Load library
                if (! dataSettings.lastLoadedCourse.getValue().isEmpty()) {
                    juce::File dir(dataSettings.lastLoadedCourse.getValue());
                    const juce::String nameToLookFor = dir.getFileName() + libLoader.getExtension();
                    juce::Array<juce::File> libFiles = dir.findChildFiles(juce::File::TypesOfFileToFind::findFiles, true, nameToLookFor, juce::File::FollowSymlinks::no);

                    if (! libFiles.isEmpty()) {
                        juce::File libFile = libFiles[0]; // Get first found file
                        libLoader.loadLibrary(libFile);
                        prepareProcessor();
                        libFileWatcher.setFileToWatch(libFile);
                    }
                }
            }
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<PluginParameter>> params;
//
    for (int i = 0; i < NUM_PARAMS; i++)
        params.push_back(std::make_unique<PluginParameter>(juce::String(i+1), 0, 1, 0));

    return { params.begin(), params.end() };
}

void AudioPluginAudioProcessor::setParameterListeners()
{
    for (int i = 0; i < NUM_PARAMS; i++)
        apvts.addParameterListener(juce::String(i+1), &apvtsListener);
}

void AudioPluginAudioProcessor::ApvtsListener::parameterChanged(const juce::String &parameterID, float newValue)
{
    const int id = parameterID.getIntValue() - 1;
    auto& parameters = audioProcessor->config.getParameters();

    ParamMessage msg;
    //TODO: Check if id is found in the array, since this will result in false if the ID does not correspond with the param index
    if (id < parameters.size())
        msg.value = parameters[id]->range.convertFrom0to1(newValue);
    else
        msg.value = newValue;
    msg.id = parameterID.getIntValue();
    audioProcessor->paramFifo.push(msg);
}

void AudioPluginAudioProcessor::reloadParameters(bool setToDefaultValue)
{
    auto& params = config.getParameters();
    for (int i = 0; i < params.size(); i++) {
        auto* guiParam = params[i].get();
        auto* pluginParam = dynamic_cast<PluginParameter*>(getParameters()[i]);

        pluginParam->setName(guiParam->name);
        const float defaultValue = guiParam->range.convertTo0to1(guiParam->defaultValue);
        pluginParam->setDefaultValue(defaultValue);

        // Update parameters
        const float value = setToDefaultValue ? defaultValue : pluginParam->getValue();
        pluginParam->setValue(value);
        apvtsListener.parameterChanged(pluginParam->getParameterID(), value);
    }

    // Update names of parameters to the host
    if (juce::MessageManager::getInstance()->isThisTheMessageThread())
        hostInfoUpdater.handleAsyncUpdate();
    else
        hostInfoUpdater.triggerAsyncUpdate();
}

void AudioPluginAudioProcessor::prepareProcessor()
{
    if (auto* processor = libLoader.getProcessor())
        processor->prepareToPlay((float)sampleRate, samplesPerBlock);
}

void AudioPluginAudioProcessor::setNewLibrary(juce::File file)
{
    libLoader.loadLibrary(file);
    prepareProcessor();
    libFileWatcher.setFileToWatch(file);
}