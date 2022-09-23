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
class AudioPlayerPluginAudioProcessorEditor  : public juce::AudioProcessorEditor, juce::ChangeListener, juce::Slider::Listener
{
public:
    AudioPlayerPluginAudioProcessorEditor (AudioPlayerPluginAudioProcessor&);
    ~AudioPlayerPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void transportStateChanged(AudioPlayerPluginAudioProcessor::TransportState newState);
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void sliderValueChanged(juce::Slider* slider)override;
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPlayerPluginAudioProcessor& audioProcessor;
    juce::Slider     gainslider;
    juce::TextButton m_Openbutton;
    juce::TextButton m_Playbutton;
    juce::TextButton m_Stopbutton;
    std::unique_ptr<juce::FileChooser> FileChooser;

    void openbuttonclicked();
    void playbuttonclicked();
    void stopbuttonclicked();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPlayerPluginAudioProcessorEditor)
};
