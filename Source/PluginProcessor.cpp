/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <Carbon/Carbon.h>

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout CreateParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout params;
    int versionHint1 = 1;
    
    
    params.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"midicc",versionHint1},
                                                           "MidiCC",
                                                           0.0f,
                                                           127.0f,
                                                           22.0f));
    
    params.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"numberofzones",versionHint1},
                                                         "NumberOfZones",
                                                         0,
                                                         MAX_NOTES,
                                                         6));
    
    params.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"velocity",versionHint1},
                                                           "Velocity",
                                                           0.0f,
                                                           1.0f,
                                                           90.0/127.0));
    
    params.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"octaves",versionHint1},
                                                         "Octaves",
                                                         0,
                                                         8,
                                                         2));
    int stepSize = 127/DEFAULT_NUMBEROFZONES;
    bool enabled = true;
    
    for(int i=0;i<MAX_SPLITS;i++)
    {
        if(i<MAX_NOTES)
        {
            params.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"notes" + std::to_string(i),versionHint1},
                                                                 "Notes",
                                                                 0,
                                                                 128,
                                                                 47 + defaultNoteOrder[i]));
            params.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"chords" + std::to_string(i),versionHint1},
                                                                 "Chords",
                                                                 1,
                                                                 128,
                                                                 1));
            int chordBuildDefault = 1;
            for(int j=0;j<MAX_NOTES;j++)
            {
                params.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"chordbuilds" + std::to_string(i) + "_" + std::to_string(j),versionHint1},
                                                                     "Chord Builds",
                                                                     0,
                                                                     128,
                                                                     chordBuildDefault));
                chordBuildDefault = 0;//default only the base note
            }
        }
        if(i >= DEFAULT_NUMBEROFZONES)
        {
            enabled=false;
        }
        int defaultsplit = enabled? 1 + i * stepSize:0;
        if(i>=DEFAULT_NUMBEROFZONES) defaultsplit = 128;
        params.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"splits" + std::to_string(i),versionHint1},
                                                             "Splits",
                                                             0,
                                                             128,
                                                             defaultsplit));
    }
    params.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"splitextra",versionHint1},
                                                         "Split Extra",
                                                         0,
                                                         128,
                                                         128));
    return params;
}
//==============================================================================
RibbonToNotesAudioProcessor::RibbonToNotesAudioProcessor()
: AudioProcessor (BusesProperties()
                  //#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                  .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
                  .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                  //#endif
                  ), StartTime (juce::Time::getMillisecondCounterHiRes() * 0.001)

#ifndef JucePlugin_PreferredChannelConfigurations
, valueTreeState(*this, nullptr, juce::Identifier("RibbonToNotes"), CreateParameterLayout())
#endif
{
    valueTreeState.state.setProperty(Service::PresetManager::presetNameProperty, "", nullptr);
    valueTreeState.state.setProperty("version", ProjectInfo::versionString, nullptr);
    
    midiCC = valueTreeState.getRawParameterValue("midicc");
    numberOfZones = valueTreeState.getRawParameterValue("numberofzones");
    noteVelocity = valueTreeState.getRawParameterValue("velocity");
    octaves = valueTreeState.getRawParameterValue("octaves");
    splitExtra = valueTreeState.getRawParameterValue("splitextra");
    for(int i=0;i<MAX_NOTES;i++)
    {
        noteValues[i] = valueTreeState.getRawParameterValue("notes" + std::to_string(i));
        chordValues[i] = valueTreeState.getRawParameterValue("chords" + std::to_string(i));
        for(int j=0;j<MAX_NOTES;j++)
        {
            chordBuilds[i][j] = valueTreeState.getRawParameterValue("chordbuilds" + std::to_string(i) + "_" + std::to_string(j));
        }
        notePressedChannel[i]=-1;
    }
    
    //int cnt = sizeof(notePressedChannel)/sizeof(notePressedChannel[0]);
    for(int i=0;i<MAX_SPLITS;i++)
    {
        splitValues[i] = valueTreeState.getRawParameterValue("splits" + std::to_string(i));
    }
    *splitValues[0]=0;
    
    presetManager = std::make_unique<Service::PresetManager>(valueTreeState);
}

//==============================================================================
RibbonToNotesAudioProcessor::~RibbonToNotesAudioProcessor()
{
}

//==============================================================================
const juce::String RibbonToNotesAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RibbonToNotesAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool RibbonToNotesAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool RibbonToNotesAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double RibbonToNotesAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RibbonToNotesAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int RibbonToNotesAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RibbonToNotesAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RibbonToNotesAudioProcessor::getProgramName (int index)
{
    return {};
}

void RibbonToNotesAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RibbonToNotesAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void RibbonToNotesAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RibbonToNotesAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void RibbonToNotesAudioProcessor::PlayNotes(int ccval, int channel, juce::MidiBuffer &midiMessages)
{
    for(int i=0 ; i < ((int)(*numberOfZones)) ;i++)
    {
        if(ccval <= *splitValues[0])
        {
            //if ccval is 0, then stop any note from sounding
            //AddSentAllNotesOff(midiMessages,lastChannel);
            AddPreviousNotesSentNotesOff(midiMessages, lastChannel);
            //processedMidi.addEvent(juce::MidiMessage::allNotesOff(channel), juce::Time::getMillisecondCounterHiRes() * 0.001 - StartTime);
            activeZone = 0;
            break;
        }
        //if ccval is in range according to spliValues array, start the note
        if(ccval < *splitValues[i+1])
        {
            //only do something if the same note is not already pressed
            if(activeZone != i+1)//notePressedChannel[i] != channel)
            {
                activeZone = i+1;
                //first sent noteOff for previous notes.
                AddPreviousChannelNotesSentNotesOff(midiMessages, i);
                //create new noteOn
                notePressedChannel[i] = channel;
                AddSentNotesOn(midiMessages,i);
            }
            //stop the loop as soon as a range was valid
            break;
        }
    }
}

void RibbonToNotesAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // A pure MIDI plugin shouldn't be provided any audio data
    jassert (buffer.getNumChannels() == 0);
    
    // however we use the buffer to get timing information
    //auto numSamples = buffer.getNumSamples();
    
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    buffer.clear();
    
    //create a new buffer
    juce::MidiBuffer processedMidi;
    
    int ccval = lastCCValue;
    int channel =lastChannel;
    //filter the cc mesagges of the selected midiCC
    for(const auto metadata : midiMessages)
    {
        const auto message = metadata.getMessage();
        //auto time = metadata.samplePosition;
        if(message.isController() && message.getControllerNumber() == (int) *midiCC)
        {
            ccval = message.getControllerValue();
            channel = message.getChannel();
        }
        else
        {
            processedMidi.addEvent(message, juce::Time::getMillisecondCounterHiRes() * 0.001 - StartTime);
        }
    }
    //swap the original buffer with the new created one
    //midiMessages.swapWith(processedMidi);
    
    //play notes or stop playing notes based on the cc value
    if(HasChanged(ccval, channel))
    {
        PlayNotes(ccval, channel, midiMessages);
    }
    lastCCValue = ccval;
    lastChannel = channel;
}

bool RibbonToNotesAudioProcessor::HasChanged(int ccval, int channel)
{
    if(channel != lastChannel) return true;
    for(int i=0 ; i < ((int)(*numberOfZones)) ;i++)
    {
        if(lastCCValue <= *splitValues[i])
        {
            if(i == 0 && ccval <= *splitValues[i])
            {
                return false;
            }
            if(i > 0 && ccval <= *splitValues[i])
            {
                return (ccval > *splitValues[i-1] == false);
            }
            else
            {
                return true;
            }
        }
    }
    return true;
}

void RibbonToNotesAudioProcessor::AddSentAllNotesOff(juce::MidiBuffer& processedMidi, int channel)
{
    processedMidi.addEvent(juce::MidiMessage::allNotesOff(channel), juce::Time::getMillisecondCounterHiRes() * 0.001 - StartTime);
    
    //loop through array
    for(int i=0;i<MAX_NOTES;i++)
    {
        //if note was pressed, the channel was set.
        if(notePressedChannel[i]>0)
        {
            processedMidi.addEvent(juce::MidiMessage::allNotesOff(notePressedChannel[i]), juce::Time::getMillisecondCounterHiRes() * 0.001 - StartTime);
            notePressedChannel[i]=-1; //remove the channel setting, because all notes have been cleared
        }
    }
}

void RibbonToNotesAudioProcessor::AddPreviousNotesSentNotesOff(juce::MidiBuffer& processedMidi, int channel)
{
    //loop through array
    for(int i = 0; i < notesPressed.size(); i++)
    {
        auto message1 = juce::MidiMessage::noteOn(channel, notesPressed[i], 0.0f);
        processedMidi.addEvent(message1, juce::Time::getMillisecondCounterHiRes() * 0.001 - StartTime);
        
        auto message2 = juce::MidiMessage::noteOff(channel,notesPressed[i]);
        processedMidi.addEvent(message2, juce::Time::getMillisecondCounterHiRes() * 0.001 - StartTime);
    }
    notesPressed.removeRange(0,notesPressed.size());
}

void RibbonToNotesAudioProcessor::AddPreviousChannelNotesSentNotesOff(juce::MidiBuffer& processedMidi, int exceptNote)
{
    for(int i=0;i<MAX_NOTES;i++)
    {
        //if note was pressed, the channel was set.
        if(notePressedChannel[i]>0 && i!=exceptNote)
        {
            for(int j=0;j<MAX_NOTES;j++)
            {
                //get j note of the current selected chord
                int note = ((int)(*noteValues[i])) + ((int)(*chordBuilds[i][j])) - 1;
                if(note < 1){note += 12;}
                //create  a noteon message with velocity=0 (for devices that do not respond to note off).
                if(note < 128)
                {
                    auto message1 = juce::MidiMessage::noteOn(notePressedChannel[i], note, 0.0f);
                    processedMidi.addEvent(message1, juce::Time::getMillisecondCounterHiRes() * 0.001 - StartTime);
                    
                    //create note off message for this note
                    auto message2 = juce::MidiMessage::noteOff(notePressedChannel[i],note);
                    processedMidi.addEvent(message2, juce::Time::getMillisecondCounterHiRes() * 0.001 - StartTime);
                }
                
            }
            notePressedChannel[i]=-1; //remove the channel setting, because all notes have been cleared
        }
    }
}
void RibbonToNotesAudioProcessor::AddSentNotesOn(juce::MidiBuffer& processedMidi, int selectedZone)
{
    //loop through array
    for(int j=0;j<MAX_NOTES;j++)
    {
        if(((int)(*chordBuilds[selectedZone][j]))==0) break;
        int note = ((int)(*noteValues[selectedZone])) + ((int)(*chordBuilds[selectedZone][j])) - 1;
        if(note < 1){note += 12;}
        if(note < 128)
        {
            auto message = juce::MidiMessage::noteOn(notePressedChannel[selectedZone],note,*noteVelocity);
            processedMidi.addEvent(message, juce::Time::getMillisecondCounterHiRes() * 0.001 - StartTime);
            notesPressed.add(note);
        }
    }
}
//==============================================================================
bool RibbonToNotesAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RibbonToNotesAudioProcessor::createEditor()
{
    return new RibbonToNotesAudioProcessorEditor (*this, valueTreeState);
}

//==============================================================================
void RibbonToNotesAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = valueTreeState.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void RibbonToNotesAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (valueTreeState.state.getType()))
            valueTreeState.replaceState (juce::ValueTree::fromXml (*xmlState));
}

int RibbonToNotesAudioProcessor::getActiveZone() const
{
    return activeZone;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RibbonToNotesAudioProcessor();
}
