#pragma once

#if defined (__APPLE_CPP__) || defined (__APPLE_CC__)
    #define EXPORT __attribute__((visibility("default")))
#elif defined (_WIN32) || defined (_WIN64)
    #define EXPORT __declspec(dllexport)
#endif

#include <vector>
#include <assert.h>
#include <boost/lockfree/queue.hpp>

/** AudioBuffer is a class that represents an audio buffer containing multiple channels of audio samples.
 *  It allows easy access to the audio data.*/
class AudioBuffer {
public:

    /** Sets the data to where this AudioBuffer should point to.
     *  This does not copy the audio data.
     * @param data          2D Array of pointers to the auido data
     * @param numChannels   Amount of channels in the buffer
     * @param numSamples    Amount of samples per channel
     */
    AudioBuffer(float* const* data, const int numChannels, const int numSamples)
    : data(data)
    , numChannels(numChannels)
    , numSamples(numSamples)
    {

    }

    // Returns the amount of channels
    int getNumChannels() const { return numChannels; }

     // Returns the amount of samples per channel
    int getNumSamples() const { return numSamples; }

    /** Returns an audio channel. This is an easy way to access the audio samples.
     *  A specific sample from a specific channel can be accessed like buffer[channel][sample].
     * @param channel       The channel index
     * @return              Array containing one channel of audio samples
     */
    float* operator[](int channel) { return data[channel]; };

private:
    float* const* data { nullptr };
    int numChannels { 0 };
    int numSamples { 0 };
};

/** Parameter value change message. */
struct ParamMessage {
    ParamMessage(int id, float value) : id(id), value(value) {};
    ParamMessage() = default;

    /** The parameter ID.*/
    int id { -1 };

    /** The parameter value.*/
    float value { 0.0f };
};

/** An array of parameter change events.*/
class ParamFiFo {
public:

    /** Add a message to the fifo queue.
     *
     *
     * @param msg   Message to add.
     * @return
     */
    bool push(ParamMessage& msg)
    {
        bool err = queue.push(msg);
        assert(err);
        return err;
    }

    /** Get and delete a message from the queue.*/
    bool pop(ParamMessage& msg)
    {
        return queue.pop(msg);
    }

protected:
    boost::lockfree::queue<ParamMessage> queue { 100 };
};

/** A MIDI event. This can be a noteOn, noteOff, aftertouch, etc.. */
struct MidiMessage {

    /** MIDI Event type.*/
    enum class Type {
        noteOff = 0b1000,
        noteOn = 0b1001,
        aftertouch = 0b1010,
        controlChange = 0b1011,
        programChange = 0b1100,
        channelPressure = 0b1101,
        pitchBend = 0b1110,
    };

    MidiMessage() = default;

    MidiMessage(uint8_t statusByte, uint8_t dataByte1, uint8_t dataByte2)
    {
        type = static_cast<Type>((statusByte & 0b11110000) >> 4);
        channel = (statusByte & 0b00001111);
        note = dataByte1;
        value = dataByte2;
    }

    /** The MIDI event type.
     * @see MidiMessage::Type
     */
    Type type;

    /** MIDI Channel*/
    uint8_t channel;

    /** MIDI note value. If the event is a program change this corresponds to program number.*/
    uint8_t note;

    /** MIDI Event value. In case of a noteOn / noteOff this is velocity. If aftertouch it is pressure, etc..*/
    uint8_t value;
};

/** An array of MIDI events.*/
class MidiFiFo {
public:

    /** Add a message to the fifo queue.*/
    bool push(MidiMessage& msg)
    {
        bool err = queue.push(msg);
        assert(err);
        return err;
    }

    /** Get and delete a message from the queue.*/
    bool pop(MidiMessage& msg)
    {
        return queue.pop(msg);
    }

protected:
    boost::lockfree::queue<MidiMessage> queue { 100 };
};

/** Audio Processor Interface. The plugin will call the methods of this class when
 *  processing audio.
 */
class IAudioProcessor {
public:

    /** Prepares the processor for playing. Initialize your processor here.
     *
     * @param sampleRate        The sample rate used by the DAW.
     * @param samplesPerBlock   The block size (in samples) used by the DAW.
     */
    virtual void prepareToPlay(float sampleRate, int samplesPerBlock) = 0;

    /** Here you do your audio processing. The plugin calls this method every time a new block
     *  of audio arrives.
     * @param audioBuffer       Audio buffer containg channels of audio samples.
     * @param parameters        A First In First Out queue containing parameter changes.
     * @param midi              A First In First Out queue containing midi messages.
     */
    virtual void process(AudioBuffer& audioBuffer, ParamFiFo& parameters, MidiFiFo& midi) = 0;

    virtual ~IAudioProcessor() = default;

};

extern "C" EXPORT IAudioProcessor* createProcessor();

/** Makes sure this returns a new instance of your processor.*/
#define DEFINE_CREATE_PROCESSOR(ClassName) \
    IAudioProcessor* createProcessor()      \
    {                                       \
        return new ClassName();             \
    }                                       \
