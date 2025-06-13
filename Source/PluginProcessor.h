/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class SCOPESCT002AudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SCOPESCT002AudioProcessor();
    ~SCOPESCT002AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    const float* getCircularBufferData(int channel) const { return circularBuffer.getReadPointer(channel); }
    int getCircularBufferSize() const { return circularBuffer.getNumSamples(); }
    int getCircularBufferPosition() const { return circularBufferPosition; }
    double getSampleRate() const { return currentSampleRate; }

private:
    //==============================================================================
    juce::AudioBuffer<float> circularBuffer;
    int circularBufferPosition = 0;
    double currentSampleRate = 44100.0;
    
    static constexpr int bufferSize = 4096;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SCOPESCT002AudioProcessor)
};
