/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class AudioPlayerPluginAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    AudioPlayerPluginAudioProcessorEditor (AudioPlayerPluginAudioProcessor&);
    ~AudioPlayerPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPlayerPluginAudioProcessor& audioProcessor;
    juce::TextButton m_Openbutton{"Open"};

    void openbuttonclicked();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPlayerPluginAudioProcessorEditor)
};
