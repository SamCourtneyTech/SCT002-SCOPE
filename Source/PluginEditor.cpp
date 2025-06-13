/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
OscilloscopeComponent::OscilloscopeComponent(SCOPESCT002AudioProcessor& proc)
    : processor(proc)
{
    // Don't start timer immediately - wait until component is properly set up
}

OscilloscopeComponent::~OscilloscopeComponent()
{
    stopTimer();
}

void OscilloscopeComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    if (bounds.isEmpty())
        return;
        
    g.fillAll(juce::Colours::black);
    
    drawGrid(g);
    
    if (channelMode == 0 || channelMode == 2) // Left or Stereo
        drawWaveform(g, 0, juce::Colours::cyan);
    
    if (channelMode == 1 || channelMode == 2) // Right or Stereo
        drawWaveform(g, 1, juce::Colours::yellow);
    
    // Draw trigger level line
    if (triggerEnabled)
    {
        g.setColour(juce::Colours::red);
        float y = getHeight() * 0.5f - (triggerLevel * amplitudeScale * getHeight() * 0.4f);
        g.drawHorizontalLine(juce::roundToInt(y), 0.0f, (float)getWidth());
    }
}

void OscilloscopeComponent::drawGrid(juce::Graphics& g)
{
    int width = getWidth();
    int height = getHeight();
    
    if (width <= 0 || height <= 0)
        return;
        
    g.setColour(juce::Colours::darkgrey);
    
    // Vertical grid lines
    for (int i = 1; i < 10; ++i)
    {
        float x = width * i / 10.0f;
        g.drawVerticalLine(juce::roundToInt(x), 0.0f, (float)height);
    }
    
    // Horizontal grid lines
    for (int i = 1; i < 8; ++i)
    {
        float y = height * i / 8.0f;
        g.drawHorizontalLine(juce::roundToInt(y), 0.0f, (float)width);
    }
    
    // Center lines
    g.setColour(juce::Colours::grey);
    g.drawVerticalLine(width / 2, 0.0f, (float)height);
    g.drawHorizontalLine(height / 2, 0.0f, (float)width);
}

void OscilloscopeComponent::drawWaveform(juce::Graphics& g, int channel, juce::Colour colour)
{
    int width = getWidth();
    int height = getHeight();
    
    if (width <= 0 || height <= 0 || channel < 0 || channel >= 2)
        return;
        
    const float* data;
    int numSamples;
    bool useProcessorData = !isFrozen;
    
    if (useProcessorData)
    {
        data = processor.getCircularBufferData(channel);
        numSamples = processor.getCircularBufferSize();
    }
    else
    {
        if (frozenBuffer[channel].size() == 0) return;
        data = frozenBuffer[channel].getRawDataPointer();
        numSamples = frozenBuffer[channel].size();
    }
    
    if (numSamples == 0 || data == nullptr) return;
    
    g.setColour(colour);
    
    waveformPath[channel].clear();
    
    int samplesToDisplay = juce::jmin(numSamples, juce::roundToInt(width * timeScale));
    int startSample = 0;
    
    if (triggerEnabled && !isFrozen)
    {
        startSample = findTriggerPoint(data, numSamples);
    }
    else if (useProcessorData)
    {
        startSample = processor.getCircularBufferPosition();
    }
    
    for (int i = 0; i < samplesToDisplay && i < width; ++i)
    {
        int sampleIndex = (startSample + i) % numSamples;
        float sample = data[sampleIndex];
        
        float x = (float)i * width / samplesToDisplay;
        float y = height * 0.5f - (sample * amplitudeScale * height * 0.4f);
        
        if (i == 0)
            waveformPath[channel].startNewSubPath(x, y);
        else
            waveformPath[channel].lineTo(x, y);
    }
    
    g.strokePath(waveformPath[channel], juce::PathStrokeType(1.0f));
}

int OscilloscopeComponent::findTriggerPoint(const float* data, int numSamples)
{
    int currentPos = processor.getCircularBufferPosition();
    int searchStart = (currentPos - numSamples / 4 + numSamples) % numSamples;
    
    for (int i = 0; i < numSamples / 2; ++i)
    {
        int index = (searchStart + i) % numSamples;
        int nextIndex = (index + 1) % numSamples;
        
        if (data[index] <= triggerLevel && data[nextIndex] > triggerLevel)
        {
            return index;
        }
    }
    
    return currentPos;
}

void OscilloscopeComponent::timerCallback()
{
    if (!isFrozen && isShowing() && getWidth() > 0 && getHeight() > 0)
    {
        repaint();
    }
}

void OscilloscopeComponent::setFrozen(bool frozen)
{
    if (frozen && !isFrozen)
    {
        // Capture current waveform data
        for (int ch = 0; ch < 2; ++ch)
        {
            const float* data = processor.getCircularBufferData(ch);
            int numSamples = processor.getCircularBufferSize();
            frozenBuffer[ch].clearQuick();
            frozenBuffer[ch].addArray(data, numSamples);
        }
    }
    isFrozen = frozen;
    repaint();
}

void OscilloscopeComponent::resized()
{
    // Start timer only after component is properly sized
    if (getWidth() > 0 && getHeight() > 0 && !isTimerRunning())
    {
        startTimerHz(60); // 60 FPS refresh rate
    }
}

//==============================================================================
SCOPESCT002AudioProcessorEditor::SCOPESCT002AudioProcessorEditor (SCOPESCT002AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), oscilloscope(audioProcessor)
{
    setSize (800, 600);
    
    // Add oscilloscope
    addAndMakeVisible(oscilloscope);
    
    // Time scale controls
    timeScaleLabel.setText("Time Scale", juce::dontSendNotification);
    addAndMakeVisible(timeScaleLabel);
    
    timeScaleSlider.setRange(0.1, 5.0, 0.1);
    timeScaleSlider.setValue(1.0);
    timeScaleSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    timeScaleSlider.onValueChange = [this] { 
        oscilloscope.setTimeScale((float)timeScaleSlider.getValue()); 
    };
    addAndMakeVisible(timeScaleSlider);
    
    // Amplitude scale controls
    amplitudeScaleLabel.setText("Amplitude Scale", juce::dontSendNotification);
    addAndMakeVisible(amplitudeScaleLabel);
    
    amplitudeScaleSlider.setRange(0.1, 10.0, 0.1);
    amplitudeScaleSlider.setValue(1.0);
    amplitudeScaleSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    amplitudeScaleSlider.onValueChange = [this] { 
        oscilloscope.setAmplitudeScale((float)amplitudeScaleSlider.getValue()); 
    };
    addAndMakeVisible(amplitudeScaleSlider);
    
    // Trigger level controls
    triggerLevelLabel.setText("Trigger Level", juce::dontSendNotification);
    addAndMakeVisible(triggerLevelLabel);
    
    triggerLevelSlider.setRange(-1.0, 1.0, 0.01);
    triggerLevelSlider.setValue(0.0);
    triggerLevelSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    triggerLevelSlider.onValueChange = [this] { 
        oscilloscope.setTriggerLevel((float)triggerLevelSlider.getValue()); 
    };
    addAndMakeVisible(triggerLevelSlider);
    
    // Channel selector
    channelLabel.setText("Channel", juce::dontSendNotification);
    addAndMakeVisible(channelLabel);
    
    channelSelector.addItem("Left", 1);
    channelSelector.addItem("Right", 2);
    channelSelector.addItem("Stereo", 3);
    channelSelector.setSelectedId(3);
    channelSelector.onChange = [this] { 
        oscilloscope.setChannelMode(channelSelector.getSelectedId() - 1); 
    };
    addAndMakeVisible(channelSelector);
    
    // Freeze button
    freezeButton.setButtonText("Freeze");
    freezeButton.onClick = [this] { 
        oscilloscope.setFrozen(freezeButton.getToggleState()); 
    };
    addAndMakeVisible(freezeButton);
}

SCOPESCT002AudioProcessorEditor::~SCOPESCT002AudioProcessorEditor()
{
}

//==============================================================================
void SCOPESCT002AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void SCOPESCT002AudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Controls panel at the bottom
    auto controlsArea = bounds.removeFromBottom(120);
    controlsArea = controlsArea.reduced(10);
    
    // Split controls into rows
    auto row1 = controlsArea.removeFromTop(30);
    auto row2 = controlsArea.removeFromTop(30);
    auto row3 = controlsArea.removeFromTop(30);
    auto row4 = controlsArea.removeFromTop(30);
    
    // Time scale row
    timeScaleLabel.setBounds(row1.removeFromLeft(100));
    timeScaleSlider.setBounds(row1.removeFromLeft(200));
    
    // Amplitude scale row  
    amplitudeScaleLabel.setBounds(row2.removeFromLeft(100));
    amplitudeScaleSlider.setBounds(row2.removeFromLeft(200));
    
    // Trigger level row
    triggerLevelLabel.setBounds(row3.removeFromLeft(100));
    triggerLevelSlider.setBounds(row3.removeFromLeft(200));
    
    // Channel selector and freeze button row
    channelLabel.setBounds(row4.removeFromLeft(60));
    channelSelector.setBounds(row4.removeFromLeft(100));
    row4.removeFromLeft(20); // spacing
    freezeButton.setBounds(row4.removeFromLeft(80));
    
    // Oscilloscope takes the remaining space
    oscilloscope.setBounds(bounds.reduced(10));
}
