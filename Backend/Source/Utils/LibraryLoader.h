#pragma once

#include <JuceHeader.h>
#include "Macros.h"
#include "../../API.h"

typedef IAudioProcessor* (*CreateProcessorFunc)();
typedef void (*DeleteProcessorFunc)(IAudioProcessor*);

class LibraryLoader {
public:

    /** Will try to load the AudioProcessor library. Don't forget to initialize the processor by calling prepareToPlay().
     *
     * @param file      The dynamic library file
     */
    void loadLibrary(const juce::File& file)
    {
        if (file.existsAsFile()) {
            std::cout << "Library found, last modified: " << file.getLastModificationTime().toString(true, true, true, true) << std::endl;
        } else {
            std::cerr << "ERROR: Unable to locate " << file.getFullPathName() << std::endl;
            return;
        }

        // Avoid calling the processor when loading a new processor
        suspendAudio = true;

        // Unload if one was already loaded
        if (libraryLoaded)
            unloadLibrary();

        const juce::String parentPath = file.getParentDirectory().getFullPathName();
        tempFilePath = parentPath + "\\" + file.getFileNameWithoutExtension() + "_temp" + extension;

        // Copy the library
        file.copyFileTo(tempFilePath);

        // Load the copied library (So we can change the original library at runtime)
        dllHandle = DL_OPEN(tempFilePath.toRawUTF8());

        if (dllHandle)
        {
            createProcessor = (CreateProcessorFunc)DL_SYM(dllHandle, "createProcessor");

            if (createProcessor)
                processor = createProcessor();

            libraryLoaded.store(true);
            lastLoadedFile = file;

        } else
        {
            DL_ERROR;
            libraryLoaded.store(false);
        }

        suspendAudio = false;
    }

    /** Will try to unload the library if it's currently loaded.*/
    bool unloadLibrary()
    {
        if (dllHandle)
        {
            suspendAudio = true;
            if (processor)
                delete processor;

            // Avoid dangling pointers
            processor = nullptr;
            createProcessor = nullptr;

            if (! DL_CLOSE(dllHandle)) {
                DL_ERROR;
                return false;
            }

            dllHandle = nullptr;
            libraryLoaded.store(false);
        }

        juce::File tempFile(tempFilePath);
        tempFile.deleteFile();

        suspendAudio = false;

        return true;
    }

    bool getLibStatus() const
    {
        return libraryLoaded.load();
    }

    void reloadLibrary()
    {
        suspendAudio = true;
        unloadLibrary();
        loadLibrary(lastLoadedFile);
        suspendAudio = false;
    }

    IAudioProcessor* getProcessor() const noexcept { return processor; }
    const juce::String& getExtension() const { return extension; }
    std::atomic<bool> suspendAudio { false };

private:

    #if JUCE_WINDOWS
        HINSTANCE dllHandle { nullptr };
        const juce::String extension { ".dll" };
    #elif JUCE_MAC
        void* dllHandle { nullptr };
        const juce::String extension { ".dylib" };
    #endif

    IAudioProcessor* processor { nullptr };
    CreateProcessorFunc createProcessor;
    DeleteProcessorFunc deleteProcessor;

    std::atomic<bool> libraryLoaded { false };

    juce::File lastLoadedFile;
    juce::String tempFilePath;
};

