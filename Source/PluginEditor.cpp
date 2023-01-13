/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include<fstream>

std::string path = "../../Source/";

juce::Image colorMap = juce::ImageFileFormat::loadFrom(juce::File("/Users/stuartmccowan/Downloads/colormap.jpeg"));



//==============================================================================
TemperatureSliderAudioProcessorEditor::TemperatureSliderAudioProcessorEditor (TemperatureSliderAudioProcessor& p)
: AudioProcessorEditor (&p), trackNameIn("Track Name In", "Enter track name here"), setBtn("Set"), audioProcessorPtr (p)
{
    DBG("audio helper starting");
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (WIDTH, HEIGHT);
    startTimerHz(30);
    addAndMakeVisible(trackNameIn);
    trackNameIn.setBounds(0, 500, 300, 80);
    trackNameIn.setEditable (true);
    trackNameIn.setColour (juce::Label::backgroundColourId, juce::Colours::darkgrey);
    trackNameIn.setColour (juce::Label::outlineWhenEditingColourId, juce::Colours::purple);
    addAndMakeVisible(setBtn);
    setBtn.setBounds(getWidth()/2+ 230, getHeight()-100, 100, 80);
    setBtn.setButtonText("SET");
    setBtn.setHelpText("Click this when you are ready to save in database");
    trackNameIn.setColour (juce::TextButton::buttonColourId, juce::Colours::darkgrey);

    trackNameIn.onTextChange =[this](){setText();};
    setBtn.onClick = [this](){processString();};
    
}

TemperatureSliderAudioProcessorEditor::~TemperatureSliderAudioProcessorEditor()
{

}

//==============================================================================

void TemperatureSliderAudioProcessorEditor::setText()
{
    trackname = trackNameIn.getText();
    
}

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
    g.setColour (juce::Colours::black);
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
    float gainFactorMax = 1.5;
    float scaleFactor;
    
    if (xPos > cMapXLength/2+allowableNeutralRegion)
    {
        scaleFactor = (xPos - allowableNeutralRegion - cMapXLength/2);
        scaleFactor /= (cMapXLength/2);
        DBG("hot. processing low boost.");
        audioProcessorPtr.chainSettings.highShelfBypassed = true;
        audioProcessorPtr.chainSettings.peakBypassed = false;
        audioProcessorPtr.chainSettings.peakFreq = 500.f;
        audioProcessorPtr.chainSettings.peakQuality = qMax -(scaleFactor * qMax) + 0.001;
        audioProcessorPtr.chainSettings.peakGainInDecibels = scaleFactor * gainMax;
        //audioProcessorPtr.receivedInputFromCMap = true;
        
        audioProcessorPtr.chainSettings.highShelfPeakFreq = 1;
        audioProcessorPtr.chainSettings.highShelfQ = 1;
        audioProcessorPtr.chainSettings.highShelfGain = 1;
        
        
    }
    else if (xPos < cMapXLength/2-allowableNeutralRegion)
    {
        DBG("cold. processing high shelf");
        DBG(xPos);
        scaleFactor = cMapXLength/2 - xPos - allowableNeutralRegion;
        scaleFactor /= (cMapXLength/2);
        DBG(scaleFactor);
        audioProcessorPtr.chainSettings.highShelfBypassed = false;
        audioProcessorPtr.chainSettings.peakBypassed = true;
        audioProcessorPtr.chainSettings.highShelfPeakFreq = 4000.f;
        audioProcessorPtr.chainSettings.highShelfQ = scaleFactor * qMax;
        audioProcessorPtr.chainSettings.highShelfGain = 1+ scaleFactor * gainFactorMax;
        
//        audioProcessorPtr.chainSettings.peakFreq = 1;
//        audioProcessorPtr.chainSettings.peakQuality = 1;
//        audioProcessorPtr.chainSettings.peakGainInDecibels= 0;
        //audioProcessorPtr.receivedInputFromCMap = true;
    }
    else
    {
        DBG("neutral. doing nothing");
        audioProcessorPtr.chainSettings.highShelfBypassed = true;
        audioProcessorPtr.chainSettings.peakBypassed = true;
        
        
        
        
    }
    
    audioProcessorPtr.updateFilters();
    
    
}

void TemperatureSliderAudioProcessorEditor::drawNextFrameOfSpectrum()
    {
    // first apply a windowing function to our data
    
//LEFT FIRSTTTTTT
    
    
        audioProcessorPtr.audioSource.window.multiplyWithWindowingTable (audioProcessorPtr.audioSource.leftfftData, audioProcessorPtr.audioSource.fftSize);
     
            // then render our FFT data..
        audioProcessorPtr.audioSource.forwardFFT.performFrequencyOnlyForwardTransform (audioProcessorPtr.audioSource.leftfftData);
     
            auto mindB = -100.0f;
            auto maxdB =    0.0f;
            auto scopeSize = audioProcessorPtr.audioSource.scopeSize;
            auto fftSize = audioProcessorPtr.audioSource.fftSize;
            for (int i = 0; i < scopeSize; ++i)                         // [3]
            {
                
                auto skewedProportionX = 1.0f - std::exp (std::log (1.0f - (float) i / (float) scopeSize) * 0.2f);
                auto fftDataIndex = juce::jlimit (0, fftSize / 2, (int) (skewedProportionX * (float) fftSize * 0.5f));
                //DBG(audioProcessorPtr.audioSource.fftData[fftDataIndex]);
                auto level = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (audioProcessorPtr.audioSource.leftfftData[fftDataIndex])
                                                                   - juce::Decibels::gainToDecibels ((float) fftSize)),
                                         mindB, maxdB, 0.0f, 1.0f);
     
                audioProcessorPtr.audioSource.leftscopeData[i] = level;
                // [4]
                //DBG(level);
            }
    //RIGHT FIRSTTTTTT
    audioProcessorPtr.audioSource.window.multiplyWithWindowingTable (audioProcessorPtr.audioSource.rightfftData, audioProcessorPtr.audioSource.fftSize);
 
        // then render our FFT data..
    audioProcessorPtr.audioSource.forwardFFT.performFrequencyOnlyForwardTransform (audioProcessorPtr.audioSource.rightfftData);
 
        for (int i = 0; i < scopeSize; ++i)                         // [3]
        {
            
            auto skewedProportionX = 1.0f - std::exp (std::log (1.0f - (float) i / (float) scopeSize) * 0.2f);
            auto fftDataIndex = juce::jlimit (0, fftSize / 2, (int) (skewedProportionX * (float) fftSize * 0.5f));
            //DBG(audioProcessorPtr.audioSource.fftData[fftDataIndex]);
            auto level = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (audioProcessorPtr.audioSource.rightfftData[fftDataIndex])
                                                               - juce::Decibels::gainToDecibels ((float) fftSize)),
                                     mindB, maxdB, 0.0f, 1.0f);
 
            audioProcessorPtr.audioSource.rightscopeData[i] = level;
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

void TemperatureSliderAudioProcessorEditor::processString()
{
//    std::string output;
//
//    output.append(trackname.toStdString());
//    output.append(",");
//    output.append(std::to_string(xPos));
    
    
    writeFile(trackname.toStdString(), std::to_string(xPos));
}


void TemperatureSliderAudioProcessorEditor::writeFile(std::string trackname, std::string xPos)
{
    // adapted from https://docs.juce.com/master/classXmlElement.html#details
    //more on filepath at https://docs.juce.com/master/classFile.html#a3e19cafabb03c5838160263a6e76313d

     auto filePath = juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile ("test123.xml");
    
    
    
    juce::XmlElement output ("DATA");
    //juce::XmlElement* datainput = new juce::XmlElement datainput ("Trial_Data");
    output.setAttribute("TRACKNAME", trackname);
    output.setAttribute("TEMPERATURE", xPos );
    //output.addChildElement(datainput);
    juce::File file ("../text.xml");
    juce::FileOutputStream outputstream (file);
    output.writeTo(filePath, juce::XmlElement::TextFormat());
    DBG("wrote file");
    
    
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
                                  juce::jmap (audioProcessorPtr.audioSource.leftscopeData[i - 1], 0.0f, 1.0f, (float) height, 0.0f)-300,
                          (float) juce::jmap (i+20,     0, audioProcessorPtr.audioSource.scopeSize - 1, 0, width),
                                  juce::jmap (audioProcessorPtr.audioSource.leftscopeData[i],     0.0f, 1.0f, (float) height, 0.0f)-300 });
//            DBG("x1: ");
//            DBG((float) juce::jmap (i - 1, 0, audioProcessorPtr.audioSource.scopeSize - 1, 0, width));
//            DBG("y1: ");
//            DBG(juce::jmap (audioProcessorPtr.audioSource.scopeData[i - 1], 0.0f, 1.0f, (float) height, 0.0f));
//            DBG("x2: ");
//            DBG((float) juce::jmap (i,     0, audioProcessorPtr.audioSource.scopeSize - 1, 0, width));
//            DBG("y2: ");
//            DBG(juce::jmap (audioProcessorPtr.audioSource.scopeData[i],     0.0f, 1.0f, (float) height, 0.0f));
            
        }
    
    /*for (int i = 1; i < audioProcessorPtr.audioSource.scopeSize; ++i)
    {
        auto width  = WIDTH-50;
        auto height = HEIGHT-40;

        g.drawLine ({ (float) juce::jmap (i - 1+20, 0, audioProcessorPtr.audioSource.scopeSize - 1, 0, width),
                              juce::jmap (audioProcessorPtr.audioSource.rightscopeData[i - 1], 0.0f, 1.0f, (float) height, 0.0f)-300,
                      (float) juce::jmap (i+20,     0, audioProcessorPtr.audioSource.scopeSize - 1, 0, width),
                              juce::jmap (audioProcessorPtr.audioSource.rightscopeData[i],     0.0f, 1.0f, (float) height, 0.0f)-300 });
//            DBG("x1: ");
//            DBG((float) juce::jmap (i - 1, 0, audioProcessorPtr.audioSource.scopeSize - 1, 0, width));
//            DBG("y1: ");
//            DBG(juce::jmap (audioProcessorPtr.audioSource.scopeData[i - 1], 0.0f, 1.0f, (float) height, 0.0f));
//            DBG("x2: ");
//            DBG((float) juce::jmap (i,     0, audioProcessorPtr.audioSource.scopeSize - 1, 0, width));
//            DBG("y2: ");
//            DBG(juce::jmap (audioProcessorPtr.audioSource.scopeData[i],     0.0f, 1.0f, (float) height, 0.0f));
        
    }
    */
    
    
    
    }
