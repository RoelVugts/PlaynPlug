#include "Properties.h"

Properties::Properties(juce::PropertiesFile::Options options) : juce::PropertiesFile(options)
{

}

juce::PropertiesFile::Options Properties::createOptions()
{
    juce::PropertiesFile::Options result;
    result.applicationName = ProjectInfo::projectName;
    result.folderName = /*juce::String (ProjectInfo::companyName) + "/" + */juce::String (ProjectInfo::projectName);
    result.filenameSuffix = ".settings";
    result.osxLibrarySubFolder = "Application Support/";
    result.doNotSave = false;
    result.millisecondsBeforeSaving = 0;
    result.commonToAllUsers = false;

    return result;
}
