#pragma once

#include "TreePropertyWrapper.h"
#include "TreeWrapper.h"

class DataSettings : public TreeWrapper {
public:

    struct IDs {
        static const juce::Identifier type;
        static const juce::Identifier lastLoadedCourse;
    };

    DataSettings(juce::ValueTree tree);

    static juce::Identifier getType();
    void setState(const DataSettings& other);

    TreePropertyWrapper<juce::String> lastLoadedCourse;

};