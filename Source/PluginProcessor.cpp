/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

ChainSettings settingsGlobal;
//==============================================================================
TemperatureSliderAudioProcessor::TemperatureSliderAudioProcessor()

:AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                 .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
                 .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
                 ),
#ifndef JucePlugin_PreferredChannelConfigurations

#endif
chainSettings(settingsGlobal)
//audioSource(audioSource)
{
    
    
}

TemperatureSliderAudioProcessor::~TemperatureSliderAudioProcessor()
{
    
    
}

//==============================================================================
const juce::String TemperatureSliderAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TemperatureSliderAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TemperatureSliderAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TemperatureSliderAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TemperatureSliderAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TemperatureSliderAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TemperatureSliderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TemperatureSliderAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TemperatureSliderAudioProcessor::getProgramName (int index)
{
    return {};
}

void TemperatureSliderAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TemperatureSliderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
        
        spec.maximumBlockSize = samplesPerBlock;
        
        spec.numChannels = 1;
        
        spec.sampleRate = sampleRate;
        
        leftChain.prepare(spec);
        rightChain.prepare(spec);
        
        updateFilters();
        
        
        osc.initialise([](float x) { return std::sin(x); });
        
        spec.numChannels = getTotalNumOutputChannels();
        osc.prepare(spec);
        osc.setFrequency(440);

    
    
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void TemperatureSliderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TemperatureSliderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void TemperatureSliderAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    
    updateFilters();
   
   juce::dsp::AudioBlock<float> block(buffer);
       
    // This is the place where you'd normally do theguts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
    
   auto leftBlock = block.getSingleChannelBlock(0);
   auto rightBlock = block.getSingleChannelBlock(1);
   
   juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
   juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
   
   leftChain.process(leftContext);
   rightChain.process(rightContext);


    
    
    
}

//==============================================================================
bool TemperatureSliderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TemperatureSliderAudioProcessor::createEditor()
{
    return new TemperatureSliderAudioProcessorEditor (*this);
}

//==============================================================================
void TemperatureSliderAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void TemperatureSliderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
       if( tree.isValid() )
       {
           apvts.replaceState(tree);
           updateFilters();
       }
}


//void TemperatureSliderAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
//{
//
//}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TemperatureSliderAudioProcessor();
}


Audio::Audio(): forwardFFT(fftOrder), window(fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    DBG("hello");
    setOpaque (true);
    setAudioChannels (2, 0);  // we want a couple of input channels but no outputs
    
    setSize (700, 500);
}

Audio::~Audio()
{
    shutdownAudio();
}


void Audio::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
    {
        if (bufferToFill.buffer->getNumChannels() > 0)
        {
            auto* channelData = bufferToFill.buffer->getReadPointer (0, bufferToFill.startSample);
            
            
            
            for (auto i = 0; i < bufferToFill.numSamples; ++i)
                pushNextSampleIntoFifo (channelData[i]);
        }
    }

void Audio::pushNextSampleIntoFifo (float sample) noexcept
    {
        if (fifoIndex == fftSize)
        {
            if (! nextFFTBlockReady)
            {
                juce::zeromem (fftData, sizeof (fftData));
                memcpy (fftData, fifo, sizeof (fifo));
                nextFFTBlockReady = true;
            }
 
            fifoIndex = 0;
        }
 
        fifo[fifoIndex++] = sample;
    }
//





void Audio::prepareToPlay(int samplesPerBlock,double sampleRate)
{

}

void Audio::releaseResources()
{
    
}


void getChainSettings(juce::AudioProcessorValueTreeState& apvts, ChainSettings &settings)
{
    
    settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
    settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
    settings.peakFreq = apvts.getRawParameterValue("Peak Freq")->load();
    settings.peakGainInDecibels = apvts.getRawParameterValue("Peak Gain")->load();
    settings.peakQuality = apvts.getRawParameterValue("Peak Quality")->load();
    settings.lowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope")->load());
    settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("HighCut Slope")->load());
    settings.lowCutBypassed = apvts.getRawParameterValue("LowCut Bypassed")->load() > 0.5f;
    settings.peakBypassed = apvts.getRawParameterValue("Peak Bypassed")->load() > 0.5f;
    settings.highCutBypassed = apvts.getRawParameterValue("HighCut Bypassed")->load() > 0.5f;
    
    settings.highShelfBypassed = apvts.getRawParameterValue("HighShelf Bypassed")->load() > 0.5f;
    settings.highShelfPeakFreq = apvts.getRawParameterValue("High Shelf Peak Freq")->load();
    settings.highShelfGain = apvts.getRawParameterValue("High Shelf Peak Gain")->load();
    settings.highShelfQ = apvts.getRawParameterValue("High Shelf Peak Quality")->load();
    
}

Coefficients makePeakFilter(const ChainSettings& chainSettings, double sampleRate)
{
    return juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate,
                                                               chainSettings.peakFreq,
                                                               chainSettings.peakQuality,
                                                               juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));
}

void TemperatureSliderAudioProcessor::updatePeakFilter(const ChainSettings &chainSettings)
{
    auto peakCoefficients = makePeakFilter(chainSettings, getSampleRate());
    
    leftChain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypassed);
    rightChain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypassed);
    
    updateCoefficients(leftChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
}




void updateCoefficients(Coefficients &old, const Coefficients &replacements)
{
    *old = *replacements;
}

void TemperatureSliderAudioProcessor::updateLowCutFilters(const ChainSettings &chainSettings)
{
    auto cutCoefficients = makeLowCutFilter(chainSettings, getSampleRate());
    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();
    
    leftChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
    rightChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
    
    updateCutFilter(rightLowCut, cutCoefficients, chainSettings.lowCutSlope);
    updateCutFilter(leftLowCut, cutCoefficients, chainSettings.lowCutSlope);
}

void TemperatureSliderAudioProcessor::updateHighCutFilters(const ChainSettings &chainSettings)
{
    auto highCutCoefficients = makeHighCutFilter(chainSettings, getSampleRate());
    
    auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();
    auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();
    
    leftChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);
    rightChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);
    
    updateCutFilter(leftHighCut, highCutCoefficients, chainSettings.highCutSlope);
    updateCutFilter(rightHighCut, highCutCoefficients, chainSettings.highCutSlope);
}

void TemperatureSliderAudioProcessor::updateHighShelfFilter(const ChainSettings & chainSettings)
{
    auto coefficients = makeHighShelfFilter(chainSettings, getSampleRate());
    
    leftChain.setBypassed<ChainPositions::HighShelf>(chainSettings.highShelfBypassed);
    rightChain.setBypassed<ChainPositions::HighShelf>(chainSettings.highShelfBypassed);
    
    updateCoefficients(leftChain.get<ChainPositions::HighShelf>().coefficients, coefficients);
    updateCoefficients(rightChain.get<ChainPositions::HighShelf>().coefficients, coefficients);
    
    
}



void TemperatureSliderAudioProcessor::updateFilters()
{
    getChainSettings(apvts,chainSettings);
    updateLowCutFilters(chainSettings);
    updatePeakFilter(chainSettings);
    updateHighCutFilters(chainSettings);
    updateHighShelfFilter(chainSettings);
    
}

juce::AudioProcessorValueTreeState::ParameterLayout TemperatureSliderAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Freq",
                                                           "LowCut Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                           20.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Freq",
                                                           "HighCut Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                           20000.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Freq",
                                                           "Peak Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                           750.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Gain",
                                                           "Peak Gain",
                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
                                                           0.0f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Quality",
                                                           "Peak Quality",
                                                           juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
                                                           1.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("High Shelf Peak Freq",
                                                           "High Shelf Peak Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                           750.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("High Shelf Peak Gain",
                                                           "High Shelf Peak Gain",
                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
                                                           0.0f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("High Shelf Peak Quality",
                                                           "High Shelf Peak Quality",
                                                           juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
                                                           1.f));
    
    juce::StringArray stringArray;
    for( int i = 0; i < 4; ++i )
    {
        juce::String str;
        str << (12 + i*12);
        str << " db/Oct";
        stringArray.add(str);
    }
    
    layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope", "LowCut Slope", stringArray, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope", "HighCut Slope", stringArray, 0));
    layout.add(std::make_unique<juce::AudioParameterBool>("LowCut Bypassed", "LowCut Bypassed", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("Peak Bypassed", "Peak Bypassed", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("HighCut Bypassed", "HighCut Bypassed", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("HighShelf Bypassed", "HighShelf Bypassed", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("Analyzer Enabled", "Analyzer Enabled", true));
    
    return layout;
}



