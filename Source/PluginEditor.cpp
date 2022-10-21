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
    startTimerHz(60);
}

TemperatureSliderAudioProcessorEditor::~TemperatureSliderAudioProcessorEditor()
{

}

//==============================================================================
void TemperatureSliderAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    
    
    
//
//    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
//
//    g.setColour (juce::Colours::white);
//    g.setFont (15.0f);
//
//    g.drawText ("Temperature Input", 100, 700, 200, 100, juce::Justification::centred, 1);
//    g.drawText ("X: " + std::to_string(xPos), 80, 720, 200, 100, juce::Justification::centred, 1);
//    g.drawText ("Y: " + std::to_string(yPos), 150, 720, 200, 100, juce::Justification::centred, 1);
//
//    g.drawImage(colorMap,cMapStartX, cMapStartY, cMapXLength, cMapYLength, 0, 0, colorMap.getWidth(), colorMap.getHeight(), false);
//    g.drawEllipse(xPos, yPos, 10, 10, 2);
//    //g.fillAll(juce::Colours::darkcyan);
//    g.setColour(juce::Colours::darkslategrey);
//    g.fillRect(20, 20, WIDTH -40, cMapStartY -40);
//    g.setColour(juce::Colours::darkmagenta);
//    //lines around the rectangle
//    g.drawRect(20, 20, WIDTH-40, cMapStartY -40);
//
    g.fillAll (juce::Colours::black);

    g.setOpacity (1.0f);
    g.drawImage (audioProcessorPtr.audioSource.spectrogramImage, getLocalBounds().toFloat());
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
        cMapParam = event.x - cMapStartX;
        repaint();
    }
}


void TemperatureSliderAudioProcessorEditor::drawNextLineOfSpectrogram()
    {
    DBG("drawingNextLine");
        
        auto rightHandEdge = audioProcessorPtr.audioSource.spectrogramImage.getWidth() - 1;
        auto imageHeight   = audioProcessorPtr.audioSource.spectrogramImage.getHeight();

        // first, shuffle our image leftwards by 1 pixel..
    audioProcessorPtr.audioSource.spectrogramImage.moveImageSection (0, 0, 1, 0, rightHandEdge, imageHeight);         // [1]

        // then render our FFT data..
    audioProcessorPtr.audioSource.forwardFFT.performFrequencyOnlyForwardTransform (audioProcessorPtr.audioSource.fftData.data());                   // [2]

        // find the range of values produced, so we can scale our rendering to
        // show up the detail clearly
        auto maxLevel = juce::FloatVectorOperations::findMinAndMax (audioProcessorPtr.audioSource.fftData.data(), audioProcessorPtr.audioSource.fftSize / 2); // [3]

        for (auto y = 1; y < imageHeight; ++y)                                              // [4]
        {
            auto skewedProportionY = 1.0f - std::exp (std::log ((float) y / (float) imageHeight) * 0.2f);
            auto fftDataIndex = (size_t) juce::jlimit (0, audioProcessorPtr.audioSource.fftSize / 2, (int) (skewedProportionY * audioProcessorPtr.audioSource.fftSize / 2));
            auto level = juce::jmap (audioProcessorPtr.audioSource.fftData[fftDataIndex], 0.0f, juce::jmax (maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);

            audioProcessorPtr.audioSource.spectrogramImage.setPixelAt (rightHandEdge, y, juce::Colour::fromHSV (level, 1.0f, level, 1.0f)); // [5]
        }
    }




void TemperatureSliderAudioProcessorEditor::timerCallback()
{
    DBG("timer callback");
    if (audioProcessorPtr.audioSource.nextFFTBlockReady)
    {
        drawNextLineOfSpectrogram();
        audioProcessorPtr.audioSource.nextFFTBlockReady = false;
        repaint();
    }
}
