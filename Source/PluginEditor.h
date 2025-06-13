/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class OscilloscopeComponent : public juce::Component, public juce::Timer
{
public:
    OscilloscopeComponent(SCOPESCT002AudioProcessor& processor);
    ~OscilloscopeComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    
    void setTimeScale(float scale) { timeScale = scale; }
    void setAmplitudeScale(float scale) { amplitudeScale = scale; }
    void setTriggerLevel(float level) { triggerLevel = level; }
    void setChannelMode(int mode) { channelMode = mode; } // 0=left, 1=right, 2=stereo
    void setFrozen(bool frozen);

private:
    SCOPESCT002AudioProcessor& processor;
    
    float timeScale = 1.0f;
    float amplitudeScale = 1.0f;
    float triggerLevel = 0.0f;
    int channelMode = 2; // stereo by default
    bool isFrozen = false;
    bool triggerEnabled = true;
    
    juce::Path waveformPath[2];
    juce::Array<float> frozenBuffer[2];
    
    void drawWaveform(juce::Graphics& g, int channel, juce::Colour colour);
    void drawGrid(juce::Graphics& g);
    int findTriggerPoint(const float* data, int numSamples);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscilloscopeComponent)
};

//==============================================================================
class SCOPESCT002AudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SCOPESCT002AudioProcessorEditor (SCOPESCT002AudioProcessor&);
    ~SCOPESCT002AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    SCOPESCT002AudioProcessor& audioProcessor;
    
    OscilloscopeComponent oscilloscope;
    juce::Slider timeScaleSlider, amplitudeScaleSlider, triggerLevelSlider;
    juce::ComboBox channelSelector;
    juce::ToggleButton freezeButton;
    juce::Label timeScaleLabel, amplitudeScaleLabel, triggerLevelLabel, channelLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SCOPESCT002AudioProcessorEditor)
};
