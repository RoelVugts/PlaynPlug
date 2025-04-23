#pragma once

#include <JuceHeader.h>

class MenuButton : public juce::TextButton {
public:

    MenuButton(juce::String name) : juce::TextButton(name)
    {
        onClick = [this]()
        {
            juce::PopupMenu popupMenu;
            for(int i = 0; i < items.size(); i++) {
                popupMenu.addItem(items[i], [i, this]()
                {
                    setCurrentItem(i, juce::NotificationType::sendNotification);
                });
            }
            popupMenu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this));
        };
    }

    void addItem(juce::String name)
    {
        items.push_back(name);
    }

    void setItems(std::vector<juce::String> names)
    {
        items = names;
    }

    int getCurrentItem()
    {
        return currentItem;
    }

    void setCurrentItem(const int index, juce::NotificationType notificationType)
    {
        jassert(index < items.size());

        currentItem = index;
        setButtonText(items[index]);

        if (notificationType != juce::dontSendNotification) {
            //TODO: Call listeners
            juce::NullCheckedInvocation::invoke(onItemChange);
        }
    }

    int getNumItems() const { return (int)items.size(); }

    std::function<void()> onItemChange;

private:

    std::vector<juce::String> items;
    int currentItem { 0 };
};