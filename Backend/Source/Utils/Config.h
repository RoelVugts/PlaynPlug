#pragma once

#include <JuceHeader.h>
#include "../Data/Properties.h"
#include "../Data/Data.h"

class Config {
public:

    struct IDs {
        static const juce::Identifier sliderID;
        static const juce::Identifier menuButtonID;
    };

    struct Parameter {

        enum class Type {
            slider,
            menu,
        };

        explicit Parameter(Type type) : type(type) {};

        virtual ~Parameter() {};

        juce::String id;
        juce::String name;
        juce::NormalisableRange<float> range;
        float defaultValue;
        juce::String suffix;
        Type type;
    };

    struct SliderConfig : public Parameter {

        SliderConfig();
        void setSliderStyle(const juce::String& text);

        juce::Rectangle<int> bounds;
        juce::Slider::SliderStyle style { juce::Slider::SliderStyle::RotaryHorizontalDrag };
    };

    struct MenuConfig : public Parameter {

        MenuConfig();

        juce::Rectangle<int> bounds;
        std::vector<juce::String> items;
    };

    explicit Config(DataSettings data);

    void setConfigFile(juce::File configFile);
    void findAndLoadConfig(juce::File dir);

    const std::vector<std::unique_ptr<Parameter>>& getParameters() const;

    std::function<void()> onReload;
    int width { 0 };
    int height { 0 };
    juce::Colour backgroundColour;
    std::vector<std::unique_ptr<Parameter>> parameters;

private:

    void parseTree();

    juce::ValueTree tree;
    juce::File file;
    DataSettings dataSettings;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Config);

};