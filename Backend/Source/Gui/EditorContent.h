#pragma once

#include <JuceHeader.h>
#include "Sections/SettingsSection.h"
#include "../Plugin/PluginProcessor.h"
#include "../Utils/Config.h"
#include "Components/MenuButton.h"
#include "Components/MenuButtonAttachment.h"

using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

class EditorContent : public juce::Component {
public:

    EditorContent(AudioPluginAudioProcessor& processor, Config& config) : processor(processor), config(config)
    {
        sliderAttachments.clear();
        buttonAttachments.clear();
        sliders.clear();
        sliderLabels.clear();

        for (const auto& param : config.parameters) {
            switch(param->type) {
                case Config::Parameter::Type::slider:
                {
                    auto *sliderConfig = dynamic_cast<Config::SliderConfig *>(param.get());
                    auto slider = createSlider(sliderConfig);

                    auto label = std::make_unique<juce::Label>(sliderConfig->name, sliderConfig->name);
                    label->attachToComponent(slider.get(), false);
                    label->setJustificationType(juce::Justification::centred);

                    addAndMakeVisible(*label);
                    addAndMakeVisible(*slider);
                    sliderLabels.emplace_back(std::move(label));
                    sliders.emplace_back(std::move(slider));
                    break;
                }
                case Config::Parameter::Type::menu:
                {
                    auto* menuConfig = dynamic_cast<Config::MenuConfig *>(param.get());
                    auto menuButton = createMenuButton(menuConfig);

                    addAndMakeVisible(*menuButton);
                    menuButtons.emplace_back(std::move(menuButton));
                    break;
                }
            }
        }
    }

    void paint(juce::Graphics& g) override
    {
        juce::Colour backgroundColour(config.backgroundColour);
        g.fillAll(backgroundColour);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
    }

private:

    std::unique_ptr<juce::Slider> createSlider(Config::SliderConfig* sliderConfig)
    {
        auto slider = std::make_unique<juce::Slider>(sliderConfig->style, juce::Slider::TextEntryBoxPosition::TextBoxBelow);

        std::unique_ptr<SliderAttachment> attachment = std::make_unique<SliderAttachment>(processor.apvts, sliderConfig->id, *slider);
        sliderAttachments.emplace_back(std::move(attachment));

        slider->setTextValueSuffix(sliderConfig->suffix);
        slider->valueFromTextFunction = [sliderConfig] (const juce::String& text) { return (double) sliderConfig->range.convertTo0to1(sliderConfig->range.snapToLegalValue(text.getFloatValue())); };
        slider->textFromValueFunction = [sliderConfig] (double value){ return juce::String(std::round(sliderConfig->range.convertFrom0to1(value) * 100.0f) /100.0f); };
        slider->setDoubleClickReturnValue (true, sliderConfig->range.convertTo0to1(sliderConfig->defaultValue));
        slider->updateText();

        slider->setColour(juce::Slider::ColourIds::textBoxOutlineColourId, juce::Colours::transparentWhite);
        slider->setBounds(sliderConfig->bounds);

        return slider;
    }

    std::unique_ptr<MenuButton> createMenuButton(Config::MenuConfig* menuConfig)
    {
        auto menuButton = std::make_unique<MenuButton>(menuConfig->name);
        menuButton->setItems(menuConfig->items);
        auto attachment = std::make_unique<MenuButtonAttachment>(processor.apvts, menuConfig->id, *menuButton);
        buttonAttachments.emplace_back(std::move(attachment));
        menuButton->setBounds(menuConfig->bounds);

        return menuButton;
    }

    AudioPluginAudioProcessor& processor;
    Config& config;

    std::vector<std::unique_ptr<juce::Label>> sliderLabels;
    std::vector<std::unique_ptr<juce::Slider>> sliders;
    std::vector<std::unique_ptr<MenuButton>> menuButtons;

    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<MenuButtonAttachment>> buttonAttachments;
};