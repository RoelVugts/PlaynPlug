#pragma once

#include <JuceHeader.h>
#include "MenuButton.h"

class MenuButtonAttachment  {
public:

    MenuButtonAttachment(juce::AudioProcessorValueTreeState& apvts, const juce::String parameterID, MenuButton& button)
    : button(button)
    , attachment(*apvts.getParameter(parameterID), [this](float f) { setValue(f); }, nullptr)
    {
        sendInitialUpdate();
        button.onItemChange = [this]() { buttonItemChanged(); };
    }

    void sendInitialUpdate()
    {
        attachment.sendInitialUpdate();
    }

private:

    void setValue(float newValue)
    {
        const juce::ScopedValueSetter<bool> svs(ignoreCallbacks, true);
        const int intValue = newValue * (button.getNumItems() - 1) + 0.5f;
        button.setCurrentItem(intValue, juce::sendNotification);
    }

    void buttonItemChanged()
    {
        if(ignoreCallbacks)
            return;

        float value = (float)button.getCurrentItem() / (float)(button.getNumItems() - 1);
        attachment.setValueAsCompleteGesture(value);
    }

    MenuButton& button;
    juce::ParameterAttachment attachment;
    bool ignoreCallbacks { false };
};