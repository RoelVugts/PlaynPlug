#pragma once

#include <JuceHeader.h>

class PluginParameter : public juce::RangedAudioParameter {
public:

    enum Mode {
        Float,
        Integer,
        Log,
        Exp
    };

    PluginParameter(const juce::String name, const float min, const float max, const float defaultValue)
        : juce::RangedAudioParameter(juce::ParameterID(name, 1), name)
        , value(defaultValue)
        , defaultValue(defaultValue)
        , rangeStart(min)
        , rangeEnd(max)
        , interval(0)
        , skew(1)
        , parameterName(name)
        , mode(Float)
    {

        range = { rangeStart, rangeEnd, interval, skew };

        if (stringFromValueFunction == nullptr) {

            auto numDecimalPlacesToDisplay = [this] {
                int numDecimalPlaces = 7;
                if (!juce::approximatelyEqual(range.interval, 0.0f)) {
                    if (juce::approximatelyEqual(std::abs(range.interval - std::floor(range.interval)), 0.0f))
                        return 0;

                    auto v = std::abs(juce::roundToInt(range.interval * pow(10, numDecimalPlaces)));

                    while ((v % 10) == 0 && numDecimalPlaces > 0) {
                        --numDecimalPlaces;
                        v /= 10;
                    }
                }

                return numDecimalPlaces;
            }();

            stringFromValueFunction = [numDecimalPlacesToDisplay](float v, int length) {
                juce::String asText(v, numDecimalPlacesToDisplay);
                return length > 0 ? asText.substring(0, length) : asText;
            };
        }

        if (valueFromStringFunction == nullptr)
            valueFromStringFunction = [] (const juce::String& text) { return text.getFloatValue(); };
    }

    void setMode(Mode newMode)
    {
        mode = newMode;
        switch(mode) {
            case Float:
                interval = 0.0f;
                skew = 1.0f;
                break;
            case Integer:
                rangeStart = std::floor(rangeStart);
                rangeEnd = std::floor(rangeEnd);
                interval = 1.0f;
                skew = 1.0f;
                break;
            case Log:
                skew = 4.0f;
                interval = 0.0f;
                break;
            case Exp:
                skew = 0.25f;
                interval = 0.0f;
                break;
        }

        range = { rangeStart, rangeEnd, interval, skew };

        if (mode == Integer)
            setValue(getValue());

    }


    void setName(const juce::String& newName)
    {
        // TODO: Not thread safe
        parameterName = newName;
    }

    juce::String getName(int maximumStringLength) const override
    {
        return parameterName.substring(0, maximumStringLength - 1);
    }

    void setRange(const float min, const float max)
    {
        rangeStart = min;
        rangeEnd = max;
        range = { rangeStart, rangeEnd, interval, skew };
        setValue(getValue());
    }

    const juce::NormalisableRange<float>& getNormalisableRange() const override
    {
        return range;
    }

    int getNumSteps() const override
    {
        if(mode == Integer)
        {
            return (int)(range.end - range.start) + 1;
        }

        return static_cast<int>((range.end - range.start) / std::numeric_limits<float>::epsilon()) + 1;
    }

    float getValue() const override
    {
        return range.convertTo0to1(value);
    }

    void setValue(const float newValue) override
    {
        value = range.convertFrom0to1(newValue);
    }

    void setDefaultValue(const float newDefaultValue)
    {
        defaultValue = newDefaultValue;
    }

    float getDefaultValue() const override
    {
        return range.snapToLegalValue(defaultValue);
    }

    float getValueForText(const juce::String& text) const override
    {
        return range.convertTo0to1(text.getFloatValue());
    }

private:

    juce::String parameterName;
    std::atomic<float> rangeStart;
    std::atomic<float> rangeEnd;
    std::atomic<float> interval;
    std::atomic<float> skew;
    juce::NormalisableRange<float> range;
    std::atomic<Mode> mode;
    std::atomic<float> value;
    std::atomic<float> defaultValue;

    std::function<juce::String (float, int)> stringFromValueFunction { nullptr };
    std::function<float (const juce::String&)> valueFromStringFunction { nullptr };
};