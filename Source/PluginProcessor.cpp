/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SCOPESCT002AudioProcessor::SCOPESCT002AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

SCOPESCT002AudioProcessor::~SCOPESCT002AudioProcessor()
{
}

//==============================================================================
const juce::String SCOPESCT002AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SCOPESCT002AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SCOPESCT002AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SCOPESCT002AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SCOPESCT002AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SCOPESCT002AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SCOPESCT002AudioProcessor::getCurrentProgram()
{
    return 0;
}

void SCOPESCT002AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SCOPESCT002AudioProcessor::getProgramName (int index)
{
    return {};
}

void SCOPESCT002AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SCOPESCT002AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    circularBuffer.setSize(2, bufferSize);
    circularBuffer.clear();
    circularBufferPosition = 0;
}

void SCOPESCT002AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SCOPESCT002AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SCOPESCT002AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Copy input data to circular buffer for oscilloscope display
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        for (int channel = 0; channel < juce::jmin(totalNumInputChannels, 2); ++channel)
        {
            auto* channelData = buffer.getReadPointer(channel);
            auto* circularData = circularBuffer.getWritePointer(channel);
            circularData[circularBufferPosition] = channelData[sample];
        }
        circularBufferPosition = (circularBufferPosition + 1) % bufferSize;
    }

    // Audio passes through unchanged (oscilloscope is analysis-only)
}

//==============================================================================
bool SCOPESCT002AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SCOPESCT002AudioProcessor::createEditor()
{
    return new SCOPESCT002AudioProcessorEditor (*this);
}

//==============================================================================
void SCOPESCT002AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SCOPESCT002AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SCOPESCT002AudioProcessor();
}
