/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================

#include <array>

class Audio: public juce::AudioAppComponent
{
public:
    enum    
    {
        fftOrder = 11,
        fftSize = 1 << fftOrder,
        scopeSize = 512
    };
    Audio();
    ~Audio();
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void pushNextSampleIntoFifo (float sample) noexcept;
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    float fifo [fftSize];
    float fftData [2 * fftSize];
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;
    float scopeData [scopeSize];
    void prepareToPlay(int samplesPerBlock,double sampleRate) override;
    void releaseResources() override;
    
    
    
    

};

enum Slope
{
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48
};

struct ChainSettings
{
    float peakFreq { 0 }, peakGainInDecibels{ 0 }, peakQuality {1.f};
    float lowCutFreq { 0 }, highCutFreq { 0 };
    
    Slope lowCutSlope { Slope::Slope_12 }, highCutSlope { Slope::Slope_12 };
    
    float highShelfGain {0}, highShelfQ{1.f}, highShelfPeakFreq{0};
    
    
    
    bool lowCutBypassed { false }, peakBypassed { false }, highCutBypassed { false }, highShelfBypassed{false};
};

void getChainSettings(juce::AudioProcessorValueTreeState& apvts, ChainSettings &chainSettings);

using Filter = juce::dsp::IIR::Filter<float>;

using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;

using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter, Filter>;

enum ChainPositions
{
    LowCut,
    Peak,
    HighCut,
    HighShelf
};

using Coefficients = Filter::CoefficientsPtr;
void updateCoefficients(Coefficients& old, const Coefficients& replacements);

Coefficients makePeakFilter(const ChainSettings& chainSettings, double sampleRate);

template<int Index, typename ChainType, typename CoefficientType>
void update(ChainType& chain, const CoefficientType& coefficients)
{
    updateCoefficients(chain.template get<Index>().coefficients, coefficients[Index]);
    chain.template setBypassed<Index>(false);
}

template<typename ChainType, typename CoefficientType>
void updateCutFilter(ChainType& chain,
                     const CoefficientType& coefficients,
                     const Slope& slope)
{
    chain.template setBypassed<0>(true);
    chain.template setBypassed<1>(true);
    chain.template setBypassed<2>(true);
    chain.template setBypassed<3>(true);
    
    switch( slope )
    {
        case Slope_48:
        {
            update<3>(chain, coefficients);
        }
        case Slope_36:
        {
            update<2>(chain, coefficients);
        }
        case Slope_24:
        {
            update<1>(chain, coefficients);
        }
        case Slope_12:
        {
            update<0>(chain, coefficients);
        }
    }
}

inline auto makeLowCutFilter(const ChainSettings& chainSettings, double sampleRate )
{
    return juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq,
                                                                                       sampleRate,
                                                                                       2 * (chainSettings.lowCutSlope + 1));
}

inline auto makeHighCutFilter(const ChainSettings& chainSettings, double sampleRate )
{
    return juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq,
                                                                                      sampleRate,
                                                                                      2 * (chainSettings.highCutSlope + 1));
}

inline auto makeHighShelfFilter(const ChainSettings& chainSettings, double sampleRate)
{
    return juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, chainSettings.highShelfPeakFreq, chainSettings.highShelfQ, chainSettings.highShelfGain);
    
}


class TemperatureSliderAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    TemperatureSliderAudioProcessor();
    ~TemperatureSliderAudioProcessor() override;

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
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Parameters", createParameterLayout()};
    
    using BlockType = juce::AudioBuffer<float>;
    
    Audio audioSource;
    ChainSettings &chainSettings;
    void updateFilters();
    

private:
    //==============================================================================
    
    MonoChain leftChain, rightChain;
        
    void updatePeakFilter(const ChainSettings& chainSettings);

    
    void updateHighShelfFilter(const ChainSettings & chainSettings);
    
    void updateLowCutFilters(const ChainSettings& chainSettings);
    void updateHighCutFilters(const ChainSettings& chainSettings);
    
    
    
    juce::dsp::Oscillator<float> osc;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TemperatureSliderAudioProcessor)
};
