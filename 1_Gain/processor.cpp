#include <Backend/API.h>

class Processor : public IAudioProcessor {
public:

    void prepareToPlay(float sampleRate, int maxSamplesPerBlock) override
    {

    }

    void process(AudioBuffer& audioBuffer, ParamFiFo& parameters, MidiFiFo& midi) override
    {
        for (int channel = 0; channel < audioBuffer.getNumChannels(); channel++) {
            for (int sample = 0; sample < audioBuffer.getNumSamples(); sample++) {

            }
        }
    }
};

DEFINE_CREATE_PROCESSOR(Processor);