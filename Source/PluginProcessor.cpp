/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPlayerPluginAudioProcessor::AudioPlayerPluginAudioProcessor()
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
    root = juce::File::getSpecialLocation(juce::File::userDesktopDirectory);
    formatManager.registerBasicFormats();
    State = (Stopped);
    if (juce::Timer::isTimerRunning())
    {
        juce::Timer::stopTimer();
    }
}

AudioPlayerPluginAudioProcessor::~AudioPlayerPluginAudioProcessor()
{
    juce::Timer::stopTimer();
    mFormatReader = nullptr;
    transport.setSource(nullptr);
}

//==============================================================================
const juce::String AudioPlayerPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPlayerPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPlayerPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPlayerPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPlayerPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPlayerPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPlayerPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPlayerPluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AudioPlayerPluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void AudioPlayerPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AudioPlayerPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    transport.prepareToPlay(samplesPerBlock,sampleRate);
}

void AudioPlayerPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    transport.releaseResources();

}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AudioPlayerPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void AudioPlayerPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    transport.getNextAudioBlock(juce::AudioSourceChannelInfo(buffer));
    transport.setGain(juce::Decibels::decibelsToGain(gain-15.0f,-75.0f));
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
    }
    
}

//==============================================================================
bool AudioPlayerPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPlayerPluginAudioProcessor::createEditor()
{
    return new AudioPlayerPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPlayerPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void AudioPlayerPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void AudioPlayerPluginAudioProcessor::timerCallback() 
{
    //time/lengthinseconds needs to be compensated because of the timing delays
    //It's compensated with the lengthinseconds/60 which is subtracted from lengthinseconds
    time = time + 1.0/60.0;
    timeprogress = time / (lengthinseconds-(lengthinseconds / 60));
}

void AudioPlayerPluginAudioProcessor::startTimer()
{
    juce::Timer::startTimerHz(60);
}

void  AudioPlayerPluginAudioProcessor::stopTimer()
{
    if (juce::Timer::isTimerRunning())
    {
        juce::Timer::stopTimer();
    }
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPlayerPluginAudioProcessor();
}
