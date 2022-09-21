/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPlayerPluginAudioProcessorEditor::AudioPlayerPluginAudioProcessorEditor (AudioPlayerPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    m_Openbutton.onClick = [this] {openbuttonclicked(); };
    addAndMakeVisible(m_Openbutton);
    setSize (400, 300);
}

AudioPlayerPluginAudioProcessorEditor::~AudioPlayerPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPlayerPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void AudioPlayerPluginAudioProcessorEditor::resized()
{
    m_Openbutton.setBounds(10, 10, getWidth() - 20, 30);
}

void AudioPlayerPluginAudioProcessorEditor::openbuttonclicked() {
    DBG("Clicked");
}