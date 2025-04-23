#pragma once

#include <JuceHeader.h>

class Properties : public juce::PropertiesFile, private juce::DeletedAtShutdown {
public:
    using SharedPtr = juce::SharedResourcePointer<Properties>;

    Properties(juce::PropertiesFile::Options options = createOptions());
protected:

    static juce::PropertiesFile::Options createOptions();

};