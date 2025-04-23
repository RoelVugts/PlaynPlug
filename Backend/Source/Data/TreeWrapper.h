#pragma once

#include <JuceHeader.h>

class TreeWrapper : public juce::ValueTree::Listener {

   public:

        TreeWrapper(const juce::ValueTree& tree) : tree(tree)
        {
            this->tree.addListener(this);
        }

        ~TreeWrapper() override
        {
            tree.removeListener(this);
        }

        TreeWrapper (const TreeWrapper& other)
            : TreeWrapper (other.tree)
        {
        }

        TreeWrapper& operator== (const TreeWrapper& other) = delete; // ambiguous

        using CallbackFn = std::function<void()>;

        void setOnPropertyChanged(const juce::Identifier ID, CallbackFn f)
        {
            propertyChangedLambdas[ID].push_back(f);
        }

        juce::ValueTree& getTree() { return tree; }

   protected:

    void valueTreePropertyChanged(juce::ValueTree &tree_, const juce::Identifier &ID) override
    {
        if (tree == tree_)
        {
            if (auto it = propertyChangedLambdas.find(ID); it != propertyChangedLambdas.end()) {
                for (CallbackFn& f : it->second)
                    juce::NullCheckedInvocation::invoke(f);
            }
        }
    }

        juce::ValueTree tree;
        std::map<juce::Identifier, std::vector<CallbackFn>> propertyChangedLambdas;


};