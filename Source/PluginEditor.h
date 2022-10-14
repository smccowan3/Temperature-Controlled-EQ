/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#define WIDTH 800
#define HEIGHT 800

//==============================================================================
/**
*/
class TemperatureSliderAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    TemperatureSliderAudioProcessorEditor (TemperatureSliderAudioProcessor&);
    ~TemperatureSliderAudioProcessorEditor() override;
    void mouseEnter(const juce::MouseEvent &event) override;
    void mouseExit(const juce::MouseEvent &event) override;
    void mouseDown(const juce::MouseEvent &event) override;
    void mouseUp(const juce::MouseEvent &event) override;
    void mouseDrag(const juce::MouseEvent &event) override;
    void drawCMapDot (const juce::MouseEvent &event);
    
    int cMapStartX {0};
    int cMapXLength {WIDTH};
    int cMapStartY{500};
    int cMapYLength{HEIGHT/10};
    
    int cMapParam{1};
    
    
    int xPos{1};
    int yPos{500};
    int mouseX{0};
    int mouseY{0};
    bool isEntered{false};
    bool isClicked{false};
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TemperatureSliderAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TemperatureSliderAudioProcessorEditor)
};