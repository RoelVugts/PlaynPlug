#include "Config.h"

#include <unordered_map>

const juce::Identifier Config::IDs::sliderID { "Slider" };
const juce::Identifier Config::IDs::menuButtonID { "Menu" };

Config::SliderConfig::SliderConfig() : Parameter(Type::slider) {};

void Config::SliderConfig::setSliderStyle(const juce::String& text)
{
    const std::unordered_map<juce::String, juce::Slider::SliderStyle> sliderStyleMap =
            {
                    {"LinearHorizontal",             juce::Slider::SliderStyle::LinearHorizontal},
                    {"LinearVertical",               juce::Slider::SliderStyle::LinearVertical},
                    {"LinearBar",                    juce::Slider::SliderStyle::LinearBar},
                    {"LinearBarVertical",            juce::Slider::SliderStyle::LinearBarVertical},
                    {"Rotary",                       juce::Slider::SliderStyle::Rotary},
                    {"RotaryHorizontalDrag",         juce::Slider::SliderStyle::RotaryHorizontalDrag},
                    {"RotaryVerticalDrag",           juce::Slider::SliderStyle::RotaryVerticalDrag},
                    {"RotaryHorizontalVerticalDrag", juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag},
                    {"IncDecButtons",                juce::Slider::SliderStyle::IncDecButtons},
                    {"TwoValueHorizontal",           juce::Slider::SliderStyle::TwoValueHorizontal},
                    {"TwoValueVertical",             juce::Slider::SliderStyle::TwoValueVertical},
                    {"ThreeValueHorizontal",         juce::Slider::SliderStyle::ThreeValueHorizontal},
                    {"ThreeValueVertical",           juce::Slider::SliderStyle::ThreeValueVertical}
            };

    auto it = sliderStyleMap.find(text.trim());
    if (it != sliderStyleMap.end())
        style = it->second;
}

Config::MenuConfig::MenuConfig() : Parameter(Type::menu) {};

Config::Config(DataSettings data) : dataSettings(data)
{
    // Load default values
    width = 600;
    height = 400;
    backgroundColour = juce::Colour(26, 39, 54);
    parameters.clear();

    // Initialize
    if (! dataSettings.lastLoadedCourse.getValue().isEmpty())
        findAndLoadConfig(dataSettings.lastLoadedCourse.getValue());

    dataSettings.setOnPropertyChanged(DataSettings::IDs::lastLoadedCourse, [this]()
    {
        if (! dataSettings.lastLoadedCourse.getValue().isEmpty())
            findAndLoadConfig(dataSettings.lastLoadedCourse.getValue());
    });
}

void Config::setConfigFile(juce::File configFile)
{
    tree = juce::ValueTree::fromXml(configFile.loadFileAsString());

    if(tree.isValid())
    {
        if(tree.hasType("Config"))
        {
            file = configFile;
            parseTree();
            juce::NullCheckedInvocation::invoke(onReload);
            return;
        }
    }

    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Invalid Gui File!", "The selected file is not a valid GUI configuration. Please check the file.", "Ok");
}

const std::vector<std::unique_ptr<Config::Parameter>>& Config::getParameters() const
{
    return parameters;
}

void Config::findAndLoadConfig(juce::File dir)
{
    juce::Array<juce::File> guiConfigFiles = dir.findChildFiles(juce::File::TypesOfFileToFind::findFiles, true, "Config.xml", juce::File::FollowSymlinks::no);

    if (! guiConfigFiles.isEmpty()) {
        juce::File guiFile = guiConfigFiles[0]; // Get first found file
        setConfigFile(guiFile);
    } else {
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                                "No Config File Found",
                                                "No valid config file (Config.xml) could not be found in the directory.",
                                                "OK");
    }
}

juce::Rectangle<int> getComponentBounds(juce::ValueTree comp)
{
    const int x = comp.getProperty("x");
    const int y = comp.getProperty("y");
    const int width = comp.getProperty("width");
    const int height = comp.getProperty("height");
    return { x, y, width, height };
}

void Config::parseTree()
{
    //TODO: Revert to default values when nothing found
    width = tree.getChildWithName("MainUI").getProperty("width");
    height = tree.getChildWithName("MainUI").getProperty("width");
    backgroundColour = juce::Colour::fromString(tree.getChildWithName("Colours").getProperty("mainBackground").toString());

    const juce::ValueTree componentsTree = tree.getChildWithName("Components");
    if (componentsTree.isValid())
    {
        parameters.clear();

        for (int i = 0; i < componentsTree.getNumChildren(); i++) {
            juce::ValueTree comp = componentsTree.getChild(i);

            if (comp.getType() == IDs::sliderID)
            {
                auto config = std::make_unique<SliderConfig>();
                config->id = comp.getProperty("id");
                config->name = comp.getProperty("name");
                juce::String suffix = comp.getProperty("suffix");
                config->suffix = " " + suffix;

                const float min = comp.getProperty("min");
                const float max = comp.getProperty("max");
                config->range = { min, max };
                config->defaultValue = comp.getProperty("defaultValue");
                config->bounds = getComponentBounds(comp);
                config->setSliderStyle(comp.getProperty("sliderStyle"));
                parameters.emplace_back(std::move(config));
            }
            else if (comp.getType() == IDs::menuButtonID)
            {
                auto config = std::make_unique<MenuConfig>();
                config->id = comp.getProperty("id");
                config->name = comp.getProperty("name");
                config->defaultValue = 0;
                config->bounds = getComponentBounds(comp);

                for (int idx = 0; idx < comp.getNumChildren(); idx++) {
                    auto item = comp.getChild(idx);
                    if (item.getType() == juce::Identifier("Item")) {
                        config->items.push_back(item.getProperty("name"));
                    }
                }
                if (config->items.size() > 0)
                    config->range = { 0, (float)(config->items.size() - 1), 1.0f };

                parameters.emplace_back(std::move(config));
            }
        }
    }
}
