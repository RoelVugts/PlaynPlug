#include "Data.h"

const juce::Identifier DataSettings::IDs::type { "Data" };
const juce::Identifier DataSettings::IDs::lastLoadedCourse { "LastLoadedCourse" };

DataSettings::DataSettings(juce::ValueTree tree)
    : TreeWrapper(tree),
    lastLoadedCourse(tree, IDs::lastLoadedCourse, "")
{

}

juce::Identifier DataSettings::getType()
{
    return IDs::type;
}

void DataSettings::setState(const DataSettings& other)
{
    lastLoadedCourse = other.lastLoadedCourse;
}