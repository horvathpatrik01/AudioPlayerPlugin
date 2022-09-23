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
class AudioPlayerPluginAudioProcessorEditor  : public juce::AudioProcessorEditor, juce::ChangeListener, juce::Slider::Listener,juce::Button::Listener
{
public:
    AudioPlayerPluginAudioProcessorEditor (AudioPlayerPluginAudioProcessor&);
    ~AudioPlayerPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    //Callback function for the AudioTransportSource 
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    //Callback function for the Slider
    void sliderValueChanged(juce::Slider* slider)override;
    //Callback function for the ToggleButton
    void buttonClicked(juce::Button* button)override;
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPlayerPluginAudioProcessor& audioProcessor;
    juce::Slider     gainslider;
    juce::TextButton openbutton;
    juce::TextButton playbutton;
    juce::TextButton stopbutton;
    juce::ToggleButton loopbutton;
    bool isLooped=false;
    std::unique_ptr<juce::FileChooser> FileChooser;
    juce::ProgressBar timeline;

    //Called when a state is changed
    void transportStateChanged(AudioPlayerPluginAudioProcessor::TransportState newState);
    //On Clicked events for TextButtons
    void openbuttonclicked();
    void playbuttonclicked();
    void stopbuttonclicked();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPlayerPluginAudioProcessorEditor)
};
