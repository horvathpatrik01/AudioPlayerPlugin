/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPlayerPluginAudioProcessorEditor::AudioPlayerPluginAudioProcessorEditor (AudioPlayerPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), m_Openbutton("Open"), m_Playbutton("Play"), m_Stopbutton("Stop")
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    p.root = juce::File::getSpecialLocation(juce::File::userDesktopDirectory);
    m_Stopbutton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    m_Playbutton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    m_Playbutton.setEnabled(false);
    m_Stopbutton.setEnabled(false);
    m_Openbutton.onClick = [this] 
    {
        FileChooser = std::make_unique<juce::FileChooser>("Choose File", audioProcessor.root, "*");
        openbuttonclicked();
        m_Playbutton.setEnabled(true);
    };
    addAndMakeVisible(m_Openbutton);

    m_Playbutton.onClick = [this] 
    {
        playbuttonclicked();
    };
    addAndMakeVisible(m_Playbutton);
    m_Stopbutton.onClick = [this] 
    {
        stopbuttonclicked();
    };
    addAndMakeVisible(m_Stopbutton);
    setSize (400, 300);
    p.transport.addChangeListener(this);
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
    m_Playbutton.setBounds(10, 50, getWidth() - 20, 30);
    m_Stopbutton.setBounds(10, 90, getWidth() - 20, 30);

}

void AudioPlayerPluginAudioProcessorEditor::openbuttonclicked() 
{
    DBG("Clicked");
    //choose a file
    const auto fileChooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::canSelectDirectories;
    //if the user chooses a file
    FileChooser->launchAsync(fileChooserFlags, [&](const juce::FileChooser& chooser) 
    {
    //what did the user choose?
    juce::File result(chooser.getResult());

    if (result.getFileExtension() == ".wav" || result.getFileExtension() == ".mp3")
    {
        audioProcessor.savedFile = result;
        audioProcessor.root = result.getParentDirectory().getFullPathName();
        //read the file
        audioProcessor.mFormatReader = audioProcessor.formatManager.createReaderFor(result);
        //get the file ready to play
        if (audioProcessor.mFormatReader != nullptr)
        {
            std::unique_ptr<juce::AudioFormatReaderSource> tempSource(new juce::AudioFormatReaderSource(audioProcessor.mFormatReader, true));
            audioProcessor.transport.setSource(tempSource.get(),0,nullptr,audioProcessor.mFormatReader->sampleRate);
            transportStateChanged(AudioPlayerPluginAudioProcessor::Stopped);
            audioProcessor.playSource.reset(tempSource.release());
            
        }
    }
    DBG(result.getFileName());
    });
            
}

void AudioPlayerPluginAudioProcessorEditor::playbuttonclicked() 
{
    if ((audioProcessor.State == AudioPlayerPluginAudioProcessor::Stopped) || (audioProcessor.State == AudioPlayerPluginAudioProcessor::Paused))
    {
        transportStateChanged(AudioPlayerPluginAudioProcessor::Starting);
    }
        
    else if (audioProcessor.State == AudioPlayerPluginAudioProcessor::Playing)
    {
        transportStateChanged(AudioPlayerPluginAudioProcessor::Pausing);
    }
}

void AudioPlayerPluginAudioProcessorEditor::stopbuttonclicked()
{
    if (audioProcessor.State == AudioPlayerPluginAudioProcessor::Paused)
    {
        transportStateChanged(AudioPlayerPluginAudioProcessor::Stopped);
    }
    else
    {
        transportStateChanged(AudioPlayerPluginAudioProcessor::Stopping);
    }
}

void AudioPlayerPluginAudioProcessorEditor::transportStateChanged(AudioPlayerPluginAudioProcessor::TransportState newState)
{
    if (newState != audioProcessor.State)
    {
        audioProcessor.State = newState;

        switch (audioProcessor.State)
        {
            //Stopped
             //bring transport back to the beginning
        case AudioPlayerPluginAudioProcessor::Stopped:
            m_Playbutton.setButtonText("Play");
            m_Stopbutton.setButtonText("Stop");
            m_Playbutton.setEnabled(true);
            m_Stopbutton.setEnabled(false);
            audioProcessor.transport.setPosition(0.0);
            break;
            //Starting
            //stopbutton enable
            //transport play
        case AudioPlayerPluginAudioProcessor::Starting:
            m_Stopbutton.setEnabled(true);
            audioProcessor.transport.start();
            break;
        case AudioPlayerPluginAudioProcessor::Playing:
            m_Playbutton.setButtonText("Pause");
            m_Stopbutton.setButtonText("Stop");
            m_Stopbutton.setEnabled(true);
            break;
        case AudioPlayerPluginAudioProcessor::Pausing:
            audioProcessor.transport.stop();
            break;
        case AudioPlayerPluginAudioProcessor::Paused:
            m_Playbutton.setButtonText("Resume");
            m_Stopbutton.setButtonText("Return to Zero");
            break;
            //Stopping
           //playbutton enable
           //transport stop
        case AudioPlayerPluginAudioProcessor::Stopping:
            m_Playbutton.setEnabled(true);
            m_Stopbutton.setEnabled(false);
            audioProcessor.transport.stop();
            break;
        
        }
    }
}


void AudioPlayerPluginAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &audioProcessor.transport)
    {
        if (audioProcessor.transport.isPlaying())
        {
            transportStateChanged(AudioPlayerPluginAudioProcessor::Playing);
        }
        else if((audioProcessor.State == AudioPlayerPluginAudioProcessor::Stopping)|| 
            (audioProcessor.State == AudioPlayerPluginAudioProcessor::Playing))
        {
            transportStateChanged(AudioPlayerPluginAudioProcessor::Stopped);
        }
        else if (audioProcessor.State == AudioPlayerPluginAudioProcessor::Pausing)
        {
            transportStateChanged(AudioPlayerPluginAudioProcessor::Paused);
        }
    }
}