#pragma once

#include <JuceHeader.h>

template <class T>
class TreePropertyWrapper {
   public:

    TreePropertyWrapper() = default;

    TreePropertyWrapper(const juce::ValueTree& tree, const juce::Identifier ID,  const T defaultValue = {})
    : tree(tree), ID(ID), defaultValue(defaultValue)
    {
        jassert(tree.isValid());
    }

    TreePropertyWrapper (const TreePropertyWrapper& other)
        : TreePropertyWrapper (other.tree, other.ID, other.defaultValue) {}

    void setValue(T value)
    {
        tree.setProperty(ID, value, nullptr);
    }

    void setValueExcludingListener(T value, juce::ValueTree::Listener* listenerToExclude)
    {
        tree.setPropertyExcludingListener(listenerToExclude, ID, value, nullptr);
    }

    TreePropertyWrapper& operator= (const TreePropertyWrapper& other)
    {
        setValue(other.getValue());
        return *this;
    }

    TreePropertyWrapper& operator= (const T& newValue)
    {
        setValue(newValue);
        return *this;
    }

    const juce::Identifier getPropertyID() const { return ID; }

    T getValue() const
    {
        T value = defaultValue;
        if (tree.hasProperty(ID))
            value = juce::VariantConverter<T>::fromVar(getVar());
        return value;
    }

    juce::var getVar() const { return tree.getProperty(ID); }

   private:
    juce::ValueTree tree;
    juce::Identifier ID;
    T defaultValue = {};
};