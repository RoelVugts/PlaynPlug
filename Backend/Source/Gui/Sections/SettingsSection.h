#pragma once

#include <JuceHeader.h>
#include "../../Plugin/PluginProcessor.h"
#include "../../Data/Properties.h"
#include "../../Utils/Config.h"
#include "../../Data/Data.h"

class SettingsSection : public juce::Component, private juce::Timer {
public:

    SettingsSection(AudioPluginAudioProcessor& p, Config& config_)
    : processor(p)
    , config(config_)
    , dataSettings(processor.dataSettings)
    {
        statusText.setFont(font);
        addAndMakeVisible(statusText);

        loadGuiButton.onClick = [this]()
        {
            juce::File lastDir(dataSettings.lastLoadedCourse.getValue());
            lastDir = lastDir.getParentDirectory();
            if (! lastDir.isDirectory()) lastDir = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userDocumentsDirectory);
            chooser = std::make_unique<juce::FileChooser>("Please select the course directory", lastDir);

            const int flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;

            chooser->launchAsync(flags, [this](const juce::FileChooser& fileChooser)
            {
                juce::File dir (fileChooser.getResult());
                if (dir.isDirectory())
                {
                    const juce::String libName = dir.getFileName() + processor.libLoader.getExtension();
                    juce::Array<juce::File> libFiles = dir.findChildFiles(juce::File::TypesOfFileToFind::findFiles, true, libName, juce::File::FollowSymlinks::no);
                    if (! libFiles.isEmpty()) {
                        juce::File libFile = libFiles[0]; // Get first found file
                        processor.setNewLibrary(libFile);
                    } else {
                        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                                                "No processor file found",
                                                                "Could not locate " + libName + ". Make sure to build it first.",
                                                                "OK");
                        processor.libLoader.unloadLibrary();
                    }

                    config.findAndLoadConfig(dir);
                    dataSettings.lastLoadedCourse.setValueExcludingListener(dir.getFullPathName(), &dataSettings);
                }
            });
        };

        addAndMakeVisible(loadGuiButton);

        startTimerHz(1);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::black);

        g.setColour (juce::Colours::white);
        g.drawText("Status: ", textMargin, 0, textWidth, getLocalBounds().getHeight(), juce::Justification::centredLeft, true);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        statusText.setBounds(textWidth, 0, 100, bounds.getHeight());
        loadGuiButton.setBounds(bounds.getWidth() - 100, 0, 100, bounds.getHeight());
    }


private:

    juce::File findLibFile(juce::File& dir)
    {
        const juce::String libName = dir.getFileName() + processor.libLoader.getExtension();
        juce::Array<juce::File> libFiles = dir.findChildFiles(juce::File::TypesOfFileToFind::findFiles, true, libName, juce::File::FollowSymlinks::no);
        if (! libFiles.isEmpty()) {
            juce::File libFile = libFiles[0]; // Get first found file
            return libFile;
        }

        return juce::File();
    }

    void timerCallback() override
    {
        bool newStatus = processor.libLoader.getLibStatus();

        if (newStatus) {
            statusText.setColour(juce::Label::ColourIds::textColourId, juce::Colours::lightgreen);
            statusText.setText("Connected", juce::dontSendNotification);
        } else {
            statusText.setColour(juce::Label::ColourIds::textColourId, juce::Colours::red);
            statusText.setText("Not connected", juce::dontSendNotification);
        }

        statusText.repaint();
        status = newStatus;
    }

    AudioPluginAudioProcessor& processor;
    DataSettings dataSettings;
    Config& config;

    juce::Label statusText;
    bool status { false };
    juce::FontOptions font { 15.0f, juce::Font::FontStyleFlags::bold};

    juce::TextButton loadGuiButton { "Load Course", "Load the GUI dynamically using a xml file"};
    std::unique_ptr<juce::FileChooser> chooser;

    static constexpr int textMargin { 5 };
    static constexpr int textWidth { 50 };


};