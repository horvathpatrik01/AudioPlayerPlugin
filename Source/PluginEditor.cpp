/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPlayerPluginAudioProcessorEditor::AudioPlayerPluginAudioProcessorEditor (AudioPlayerPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), openbutton("Open"), playbutton("Play"), stopbutton("Stop"),timeline(p.timeprogress)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    timeline.setColour(juce::ProgressBar::backgroundColourId, juce::Colours::black);
    timeline.setColour(juce::ProgressBar::foregroundColourId, juce::Colour::fromFloatRGBA(227,51,48,89));
    Component::addAndMakeVisible(timeline);
    stopbutton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    playbutton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    playbutton.setEnabled(false);
    stopbutton.setEnabled(false);
    openbutton.onClick = [this] 
    {
        audioProcessor.FileChooser = std::make_unique<juce::FileChooser>("Choose File", audioProcessor.root, "*");
        openbuttonclicked();
        playbutton.setEnabled(true);
    };
   addAndMakeVisible(openbutton);

    playbutton.onClick = [this] 
    {
        playbuttonclicked();
    };
    addAndMakeVisible(playbutton);
    stopbutton.onClick = [this] 
    {
        stopbuttonclicked();
    };
    addAndMakeVisible(stopbutton);
    loopbutton.setButtonText("Loop?");
    addAndMakeVisible(loopbutton);
    gainslider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    gainslider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
    gainslider.setRange(-60.0f, 6.0f, 0.01f);
    gainslider.setValue(0.0f);
    addAndMakeVisible(gainslider);
    TotalLength.setFont(15.0f);
    TotalLength.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(TotalLength);
    ActualTime.setFont(15.0F);
    ActualTime.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(ActualTime);
    Gainlabel.setFont(15.0F);
    Gainlabel.setText("Gain", juce::NotificationType::dontSendNotification);
    Gainlabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(Gainlabel);
    dBlabel.setFont(15.0f);
    dBlabel.setText("dB", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(dBlabel);
    setSize (400, 300);
    p.transport.addChangeListener(this);
    gainslider.addListener(this);
    loopbutton.addListener(this);
}

AudioPlayerPluginAudioProcessorEditor::~AudioPlayerPluginAudioProcessorEditor()
{
    juce::Timer::stopTimer();
}

//==============================================================================
void AudioPlayerPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void AudioPlayerPluginAudioProcessorEditor::resized()
{
    openbutton.setBounds(10, 10, getWidth() - 20, 30);
    playbutton.setBounds(10, 50, getWidth() - 20, 30);
    stopbutton.setBounds(10, 90, getWidth() - 20, 30);
    loopbutton.setBounds(10, 140, 30, 30);
    loopbutton.changeWidthToFitText();
    gainslider.setBounds(getWidth() / 2 - 40, 160, 80, 100);
    timeline.setBounds(10, getHeight() - 30, getWidth() - 20, 10);
    TotalLength.setBounds(getWidth() - 55, getHeight() - 50, 50, 20);
    ActualTime.setBounds(getWidth() - 100, getHeight() - 50, 50, 20);
    Gainlabel.setBounds(getWidth() / 2 - 20, 145, 40, 20);
    dBlabel.setBounds(getWidth()/2+25, 240, 30, 20);
}

void AudioPlayerPluginAudioProcessorEditor::openbuttonclicked() 
{
    //choose a file
    const auto fileChooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::canSelectDirectories;
    //if the user chooses a file
    audioProcessor.FileChooser->launchAsync(fileChooserFlags, [&](const juce::FileChooser& chooser) 
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
            //get the length of the file
            audioProcessor.lengthinseconds=audioProcessor.transport.getLengthInSeconds();
            transportStateChanged(AudioPlayerPluginAudioProcessor::Stopped);
            audioProcessor.playSource.reset(tempSource.release());
            audioProcessor.timeprogress = 0.0;
            int lengthinminutes = audioProcessor.lengthinseconds / 60;
            int remainingseconds = audioProcessor.lengthinseconds - (lengthinminutes * 60);
            juce::String slashwithlength = " / ";
            juce::String totallength =std::to_string(lengthinminutes).append(":").append(std::to_string(remainingseconds));
            slashwithlength.append(totallength, 1000);
            TotalLength.setText(slashwithlength, juce::NotificationType::dontSendNotification);
            setBoundsForTime(lengthinminutes, remainingseconds);
            ActualTime.setText("0:0", juce::NotificationType::dontSendNotification);
        }
    }
    });
}

void AudioPlayerPluginAudioProcessorEditor::playbuttonclicked() 
{
    if ((audioProcessor.State == AudioPlayerPluginAudioProcessor::Stopped) || (audioProcessor.State == AudioPlayerPluginAudioProcessor::Paused))
    {
        transportStateChanged(AudioPlayerPluginAudioProcessor::Starting);
    }
        
    else if (audioProcessor.State == AudioPlayerPluginAudioProcessor::Playing || audioProcessor.State == AudioPlayerPluginAudioProcessor::Looping)
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
                //stop timer
        case AudioPlayerPluginAudioProcessor::Stopped:
            playbutton.setButtonText("Play");
            stopbutton.setButtonText("Stop");
            playbutton.setEnabled(true);
            stopbutton.setEnabled(false);
            audioProcessor.transport.setPosition(0.0);
            audioProcessor.time = 0.0F;
            audioProcessor.timeprogress = 0.0;
            audioProcessor.stopTimer();
            stopTimer();
            ActualTime.setText("0:0", juce::NotificationType::dontSendNotification);
            break;
            //Starting
                //stopbutton enable
                //transport play
                //timer start
        case AudioPlayerPluginAudioProcessor::Starting:
            stopbutton.setEnabled(true);
            audioProcessor.startTimer();
            audioProcessor.transport.start();
            startTimerHz(60);
            break;
        case AudioPlayerPluginAudioProcessor::Playing:
            playbutton.setButtonText("Pause");
            stopbutton.setButtonText("Stop");
            stopbutton.setEnabled(true);
            break;
            //Pausing
                //transport stop
                //timer stop
        case AudioPlayerPluginAudioProcessor::Pausing:
            audioProcessor.transport.stop();
            audioProcessor.stopTimer();
            stopTimer();
            break;
        case AudioPlayerPluginAudioProcessor::Paused:
            playbutton.setButtonText("Resume");
            stopbutton.setButtonText("Back to Start");
            break;
            //Stopping
                //playbutton enable
                //transport stop
                //timer stop
        case AudioPlayerPluginAudioProcessor::Stopping:
            playbutton.setEnabled(true);
            stopbutton.setEnabled(false);
            audioProcessor.transport.stop();
            audioProcessor.stopTimer();
            stopTimer();
            break;
        case AudioPlayerPluginAudioProcessor::Looping:
            playbutton.setButtonText("Pause");
            stopbutton.setButtonText("Stop");
        }
    }
}
/*****************************************************************
                        Callback functions
*****************************************************************/
void AudioPlayerPluginAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &audioProcessor.transport)
    {
        if (audioProcessor.transport.isPlaying())
        {
            if (!isLooped) 
            {
                transportStateChanged(AudioPlayerPluginAudioProcessor::Playing);

            }
            else
            {
                transportStateChanged(AudioPlayerPluginAudioProcessor::Looping);
            }
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
        else if (audioProcessor.State == AudioPlayerPluginAudioProcessor::Looping)
        {
            //reset the progressbar and the time
            audioProcessor.timeprogress = 0.0;
            audioProcessor.time = 0.0F;
            //bring transport back to the beginning
            audioProcessor.transport.setPosition(0.0);
            transportStateChanged(AudioPlayerPluginAudioProcessor::Starting);
        }
    }
}

void AudioPlayerPluginAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &gainslider)
    {
        audioProcessor.gain = gainslider.getValue();
    }
}

void AudioPlayerPluginAudioProcessorEditor::buttonClicked(juce::Button* button) 
{
    if (button == &loopbutton)
    {
        isLooped = !isLooped;
        audioProcessor.transport.setLooping(isLooped);
        if (audioProcessor.transport.isPlaying())
        {
            //based on the current state of isLooped change the transportState to Looping
            // or Playing
            if (isLooped)
            {
                transportStateChanged(AudioPlayerPluginAudioProcessor::Looping);
            }
            else
            {
                transportStateChanged(AudioPlayerPluginAudioProcessor::Playing);
            }
        }
        else
        {
            //Do nothing...
        }
    }
}

void AudioPlayerPluginAudioProcessorEditor::timerCallback()
{
    int time = audioProcessor.timeprogress * audioProcessor.lengthinseconds;
    int timeinminutes = time/60;
    int remainingseconds = time-(timeinminutes*60);
    juce::String actualtime = std::to_string(timeinminutes).append(":").append(std::to_string(remainingseconds));
    ActualTime.setText(actualtime, juce::NotificationType::dontSendNotification);
}

void AudioPlayerPluginAudioProcessorEditor::setBoundsForTime(int minutes,int seconds)
{
    if (seconds >= 10 && minutes >=10)
    {
        TotalLength.setBounds(getWidth() - 60, getHeight() - 50, 50, 20);
        ActualTime.setBounds(getWidth() - 113, getHeight() - 50, 50, 20);
    }
    else if (seconds < 10 && minutes < 10)
    {
        TotalLength.setBounds(getWidth() - 60, getHeight() - 50, 50, 20);
        ActualTime.setBounds(getWidth() - 97, getHeight() - 50, 50, 20);
    }
    else
    {
        TotalLength.setBounds(getWidth() - 60, getHeight() - 50, 50, 20);
        ActualTime.setBounds(getWidth() - 105, getHeight() - 50, 50, 20);
    }
}