/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

std::string path = "../../Source/";

juce::Image colorMap = juce::ImageFileFormat::loadFrom(juce::File("/Users/stuartmccowan/Downloads/colormap.jpeg"));




//==============================================================================
TemperatureSliderAudioProcessorEditor::TemperatureSliderAudioProcessorEditor (TemperatureSliderAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessorPtr (p)
{
    DBG("audio helper starting");
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (WIDTH, HEIGHT);
    startTimerHz(30);
}

TemperatureSliderAudioProcessorEditor::~TemperatureSliderAudioProcessorEditor()
{

}

//==============================================================================
void TemperatureSliderAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    
    
    

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);

    g.drawText ("Temperature Input", 100, 700, 200, 100, juce::Justification::centred, 1);
    g.drawText ("X: " + std::to_string(xPos), 80, 720, 200, 100, juce::Justification::centred, 1);
    g.drawText ("Y: " + std::to_string(yPos), 150, 720, 200, 100, juce::Justification::centred, 1);
    
    g.drawImage(colorMap,cMapStartX, cMapStartY, cMapXLength, cMapYLength, 0, 0, colorMap.getWidth(), colorMap.getHeight(), false);
    g.drawEllipse(xPos, yPos, 10, 10, 2);
    currentParam = xPos;
    //g.fillAll(juce::Colours::darkcyan);
    g.setColour(juce::Colours::darkslategrey);
    g.fillRect(20, 20, WIDTH-40, cMapStartY -140);
    g.setColour(juce::Colours::darkmagenta);
    //lines around the rectangle
    g.drawRect(20, 20, WIDTH-40, cMapStartY -140);
    g.setColour (juce::Colours::white);
    g.drawText("Frequency (Hz)", WIDTH/2-100, cMapStartY+cMapYLength-230, 200,  100, juce::Justification::centred);

    //g.fillAll (juce::Colours::black);
    g.setOpacity (1.0f);
    g.setColour (juce::Colours::white);
    drawFrame(g);
}
void TemperatureSliderAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

void TemperatureSliderAudioProcessorEditor::mouseEnter(const juce::MouseEvent &event)
{
    isEntered = true;
}


void TemperatureSliderAudioProcessorEditor::mouseExit(const juce::MouseEvent &event)
{
    isEntered = false;
}

void TemperatureSliderAudioProcessorEditor::mouseDown(const juce::MouseEvent &event)
{
    drawCMapDot(event);
 
}

void TemperatureSliderAudioProcessorEditor::mouseUp(const juce::MouseEvent &event)
{
}

void TemperatureSliderAudioProcessorEditor::mouseDrag(const juce::MouseEvent &event)
{
    drawCMapDot(event);
}

void TemperatureSliderAudioProcessorEditor::drawCMapDot (const juce::MouseEvent &event)
{
    if ((cMapStartX < event.x && event.x < cMapStartX + cMapXLength-10) && (cMapStartY < event.y && event.y < cMapStartY + cMapYLength-10))
    {
        xPos = event.x;
        yPos = event.y;
        currentParam = xPos;
        cMapParam = event.x - cMapStartX;
        repaint();
    }
    processParameter();
}

void TemperatureSliderAudioProcessorEditor::processParameter()
{
    int allowableNeutralRegion = 10;
    int qMax = 18;
    int gainMax = 12;
    float scaleFactor;
    
    if (xPos > cMapXLength/2+allowableNeutralRegion)
    {
        scaleFactor = (xPos - cMapXLength/2+allowableNeutralRegion)/cMapXLength;
        DBG("hot. processing low boost.");
        audioProcessorPtr.chainSettings.peakFreq = 500.f;
        audioProcessorPtr.chainSettings.peakQuality = scaleFactor * qMax;
        audioProcessorPtr.chainSettings.peakGainInDecibels = scaleFactor * gainMax;
        
        
        
    }
    else if (xPos < cMapXLength/2-allowableNeutralRegion)
    {
        DBG("cold. processing high shelf");
        scaleFactor = (cMapXLength/2-allowableNeutralRegion-xPos)/cMapXLength;
        audioProcessorPtr.chainSettings.highShelfPeakFreq = 4000.f;
        audioProcessorPtr.chainSettings.highShelfQ = scaleFactor * qMax;
        audioProcessorPtr.chainSettings.highShelfGain = scaleFactor * gainMax;
    }
    else
    {
        DBG("neutral. doing nothing");
        audioProcessorPtr.chainSettings.highShelfGain = 0;
        audioProcessorPtr.chainSettings.peakGainInDecibels = 0;
        
        
    }
    
    audioProcessorPtr.updateFilters();
}

void TemperatureSliderAudioProcessorEditor::drawNextFrameOfSpectrum()
    {
    // first apply a windowing function to our data
    
        audioProcessorPtr.audioSource.window.multiplyWithWindowingTable (audioProcessorPtr.audioSource.fftData, audioProcessorPtr.audioSource.fftSize);
     
            // then render our FFT data..
        audioProcessorPtr.audioSource.forwardFFT.performFrequencyOnlyForwardTransform (audioProcessorPtr.audioSource.fftData);
     
            auto mindB = -100.0f;
            auto maxdB =    0.0f;
            auto scopeSize = audioProcessorPtr.audioSource.scopeSize;
            auto fftSize = audioProcessorPtr.audioSource.fftSize;
            for (int i = 0; i < scopeSize; ++i)                         // [3]
            {
                
                auto skewedProportionX = 1.0f - std::exp (std::log (1.0f - (float) i / (float) scopeSize) * 0.2f);
                auto fftDataIndex = juce::jlimit (0, fftSize / 2, (int) (skewedProportionX * (float) fftSize * 0.5f));
                //DBG(audioProcessorPtr.audioSource.fftData[fftDataIndex]);
                auto level = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (audioProcessorPtr.audioSource.fftData[fftDataIndex])
                                                                   - juce::Decibels::gainToDecibels ((float) fftSize)),
                                         mindB, maxdB, 0.0f, 1.0f);
     
                audioProcessorPtr.audioSource.scopeData[i] = level;
                // [4]
                //DBG(level);
            }
    }




void TemperatureSliderAudioProcessorEditor::timerCallback()
{
    //DBG("timer callback");
    if (audioProcessorPtr.audioSource.nextFFTBlockReady)
    {
        drawNextFrameOfSpectrum();
        audioProcessorPtr.audioSource.nextFFTBlockReady = false;
        repaint();
    }
}

void TemperatureSliderAudioProcessorEditor::drawFrame (juce::Graphics& g)
    {
    //DBG("Drawing frames");
    g.setColour(juce::Colours::white);
        for (int i = 1; i < audioProcessorPtr.audioSource.scopeSize; ++i)
        {
            auto width  = WIDTH-50;
            auto height = HEIGHT-40;
 
            g.drawLine ({ (float) juce::jmap (i - 1+20, 0, audioProcessorPtr.audioSource.scopeSize - 1, 0, width),
                                  juce::jmap (audioProcessorPtr.audioSource.scopeData[i - 1], 0.0f, 1.0f, (float) height, 0.0f)-300,
                          (float) juce::jmap (i+20,     0, audioProcessorPtr.audioSource.scopeSize - 1, 0, width),
                                  juce::jmap (audioProcessorPtr.audioSource.scopeData[i],     0.0f, 1.0f, (float) height, 0.0f)-300 });
//            DBG("x1: ");
//            DBG((float) juce::jmap (i - 1, 0, audioProcessorPtr.audioSource.scopeSize - 1, 0, width));
//            DBG("y1: ");
//            DBG(juce::jmap (audioProcessorPtr.audioSource.scopeData[i - 1], 0.0f, 1.0f, (float) height, 0.0f));
//            DBG("x2: ");
//            DBG((float) juce::jmap (i,     0, audioProcessorPtr.audioSource.scopeSize - 1, 0, width));
//            DBG("y2: ");
//            DBG(juce::jmap (audioProcessorPtr.audioSource.scopeData[i],     0.0f, 1.0f, (float) height, 0.0f));
            
        }
    }
