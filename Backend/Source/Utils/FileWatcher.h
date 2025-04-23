#pragma once

#include <JuceHeader.h>

class FileWatcher : private juce::Timer, juce::AsyncUpdater {
public:

    FileWatcher() = default;

    void setFileToWatch(juce::File file)
    {
        if (file.existsAsFile())
        {
            fileToWatch = file;
            lastModTime = fileToWatch.getLastModificationTime();
            startTimer(msBetweenCheck);
        }
    }

    FileWatcher(juce::File file, int msBetweenCheck = 500) : fileToWatch(file), msBetweenCheck(msBetweenCheck)
    {
        lastModTime = fileToWatch.getLastModificationTime();
        startTimer(msBetweenCheck);
    }

    void stopWatching()
    {
        stopTimer();
    }

    std::function<void()> onChange { nullptr };

private:

    void timerCallback() override
    {
        if(fileToWatch.getLastModificationTime() != lastModTime)
        {
            triggerAsyncUpdate();
            lastModTime = fileToWatch.getLastModificationTime();
        }
    }

    void handleAsyncUpdate() override
    {
        if (fileToWatch.existsAsFile())
            juce::NullCheckedInvocation::invoke(onChange);
    }

    int msBetweenCheck { 500 };
    juce::File fileToWatch;
    juce::Time lastModTime { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileWatcher)
};