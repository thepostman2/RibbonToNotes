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
    //juce::AudioProcessorValueTreeState::ParameterLayout params;
    
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    int versionHint1 = 1;
    
    
    params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{MIDICC_ID,versionHint1},
                                                               MIDICC_NAME,
                                                               1,
                                                               128,
                                                               22));
    
    params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{NUMBEROFZONES_ID,versionHint1},
                                                               NUMBEROFZONES_NAME,
                                                               1,
                                                               MAX_ZONES,
                                                               6));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{VELOCITY_ID,versionHint1},
                                                                 VELOCITY_NAME,
                                                                 0.0f,
                                                                 1.0f,
                                                                 90.0/127.0));
    
    params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{OCTAVES_ID,versionHint1},
                                                               OCTAVES_NAME,
                                                               -2,
                                                               8,
                                                               2));

    params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{CHANNELIN_ID,versionHint1},
                                                               CHANNELIN_NAME,
                                                               0,
                                                               16,
                                                               0));

    params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{CHANNELOUT_ID,versionHint1},
                                                               CHANNELOUT_NAME,
                                                               0,
                                                               16,
                                                               0));

    params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{PITCHMODES_ID,versionHint1},
                                                               PITCHMODES_NAME,
                                                               0,
                                                               1,
                                                               0));

    params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{ACTIVEALTERNATIVE_ID,versionHint1},
                                                               ACTIVEALTERNATIVE_NAME,
                                                               0,
                                                               7,
                                                               0));
    int stepSize = 127/DEFAULT_NUMBEROFZONES;
    bool enabled = true;
    
    for(int alt=0;alt<MAX_ALTERNATIVES;alt++)
    {
        for(int i=0;i<MAX_SPLITS;i++)
        {
            if(i<MAX_ZONES)
            {
                params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{KEYS_ID + std::to_string(alt) + "_" + std::to_string(i),versionHint1},
                                                                           KEYS_NAME,
                                                                           1,
                                                                           12,
                                                                           defaultNoteOrder[i]));
                params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{CHORDS_ID + std::to_string(alt) + "_" + std::to_string(i),versionHint1},
                                                                           CHORDS_NAME,
                                                                           1,
                                                                           chordsArray.size(),
                                                                           1));
                int chordBuildDefault = 1;//default only base note
                for(int j=0;j<MAX_NOTES;j++)
                {
                    params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{CHORDBUILDS_ID + std::to_string(alt) + "_" + std::to_string(i) + "_" + std::to_string(j),versionHint1},
                                                                               CHORDBUILDS_NAME,
                                                                               -128,
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
            if(alt == 0)
            {
                params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{SPLITS_ID + std::to_string(i),versionHint1},
                                                                           SPLITS_NAME,
                                                                           0,
                                                                           128,
                                                                           defaultsplit));
            }
        }
    }
    return {params.begin(), params.end()};
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
                  ), startTime (juce::Time::getMillisecondCounterHiRes() * 0.001)

#ifndef JucePlugin_PreferredChannelConfigurations
, apvts(*this, nullptr, juce::Identifier("RibbonToNotes"), CreateParameterLayout())
#endif
{
    apvts.state.setProperty(Service::PresetManager::presetNameProperty, "", nullptr);
    apvts.state.setProperty("version", ProjectInfo::versionString, nullptr);
    
    midiCC = apvts.getRawParameterValue(MIDICC_ID);
    numberOfZones = apvts.getRawParameterValue(NUMBEROFZONES_ID);
    noteVelocity = apvts.getRawParameterValue(VELOCITY_ID);
    octaves = apvts.getRawParameterValue(OCTAVES_ID);
    channelIn = apvts.getRawParameterValue(CHANNELIN_ID);
    channelOut = apvts.getRawParameterValue(CHANNELOUT_ID);
    pitchMode = apvts.getRawParameterValue(PITCHMODES_ID);
    activeAlternative = apvts.getRawParameterValue(ACTIVEALTERNATIVE_ID);

    for(int i=0;i<MAX_SPLITS;i++)
    {
        splitValues[i] = apvts.getRawParameterValue(SPLITS_ID + std::to_string(i));
    }
    *splitValues[0]=0;
    lastChannel = 1;

    for(int alt=0;alt<MAX_ALTERNATIVES;alt++)
    {
        for(int i=0;i<MAX_ZONES;i++)
        {
            selectedKeys[alt][i] = apvts.getRawParameterValue(KEYS_ID + std::to_string(alt) + "_" + std::to_string(i));
            selectedChord[alt][i] = apvts.getRawParameterValue(CHORDS_ID + std::to_string(alt) + "_" + std::to_string(i));
            for(int j=0;j<MAX_NOTES;j++)
            {
                chordNotes[alt][i][j] = apvts.getRawParameterValue(CHORDBUILDS_ID + std::to_string(alt) + "_" + std::to_string(i) + "_" + std::to_string(j));
            }
            notePressedChannel[i]=-1;
        }
        BuildChords(alt);
    }
    
    
    presetManager = std::make_unique<Service::PresetManager>(apvts);
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


//==============================================================================
void RibbonToNotesAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // A pure MIDI plugin shouldn't be provided any audio data
    //jassert (buffer.getNumChannels() == 0);
    
    // however we use the buffer to get timing information
    //auto numSamples = buffer.getNumSamples();
    
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    buffer.clear();
        
    int ccval = lastCCValue;
    int channel = lastChannel;
    //filter the cc mesagges of the selected midiCC
    for(const auto metadata : midiMessages)
    {
        const auto message = metadata.getMessage();
        //auto time = metadata.samplePosition;
        if(message.isController() && message.getControllerNumber() == (int) *midiCC)
        {
            ccval = message.getControllerValue();
            channel = message.getChannel();
            midiMessages.clear();//remove from midibuffer
        }
    }

    //play notes or stop playing notes based on the cc value
    if(HasChanged(ccval))
    {
        AddNotesToPlayToBuffer(ccval, channel, notesToPlayBuffer);
    }
    lastCCValue = ccval;
    
    // some programs do not except multiple messages added to the buffer.
    // so adding the notes one by one solves this problem
    // Mind you that processblock fires each sample time, so for the user
    // it is as if the notes are played immediately.
    PlayNextNote(midiMessages);
}

// plays the first of the buffered notes, and removes it from the buffer
void RibbonToNotesAudioProcessor::PlayNextNote(juce::MidiBuffer &midiMessages)
{
    juce::MidiBuffer tmpBuffer;
    int i = 0;
    for(const auto metadata : notesToPlayBuffer)
    {
        auto message = metadata.getMessage();
        if(i<1)
        {
            // first message of the buffer is excecuted
            midiMessages.addEvent(message, juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime);
        }
        else
        {
            // other messages are copied to temporary buffer
            tmpBuffer.addEvent(message, juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime);
        }
        i++;
    }
    // swap the buffer, so the excuted message is removed from the buffer.
    notesToPlayBuffer.swapWith(tmpBuffer);
}//==============================================================================
bool RibbonToNotesAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RibbonToNotesAudioProcessor::createEditor()
{
    return new RibbonToNotesAudioProcessorEditor (*this);
}

//==============================================================================
void RibbonToNotesAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void RibbonToNotesAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

int RibbonToNotesAudioProcessor::getActiveAlternative() const
{
    return (int) *activeAlternative;
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

//==============================================================================
// functions for adding the note on and offs to the buffer
//==============================================================================
void RibbonToNotesAudioProcessor::AddNotesToPlayToBuffer(int ccval, int channel, juce::MidiBuffer &midiMessages)
{
    // if listening to specific channels and channel is not the same, do nothing
    if((int) *channelIn != 0 && channel != (int) *channelIn)
    {
        return;
    }
    
    // if specific channel out has been set, change the channel
    if((int) *channelOut != 0)
    {
        channel = (int) *channelOut;
    }
    
    // determine selected zone
    for(int zone=0 ; zone < ((int)(*numberOfZones)) ;zone++)
    {
        if(ccval <= *splitValues[0])
        {
            if(channel != lastChannel)
            {
                AddSentAllNotesOff(midiMessages,lastChannel);
            }
            AddPreviousNotesSentNotesOff(midiMessages, channel);
            activeZone = 0;
            break;
        }
        
        //if ccval is in range according to spliValues array, start the note
        if(ccval < *splitValues[zone+1])
        {
            //only do something if the same note is not already pressed
            if(activeZone != zone+1)//notePressedChannel[i] != channel)
            {
                activeZone = zone+1;
                //first sent noteOff for previous notes.
                AddPreviousNotesSentNotesOff(midiMessages, channel);
                //create new noteOn
                notePressedChannel[zone] = channel;
                AddSentNotesOn(midiMessages, getActiveAlternative(), zone, channel);
            }
            
            //stop the loop as soon as a range was valid
            break;
        }
    }
    lastChannel = channel;
}
// add an all notes off to the buffer for given channel and any other channel that was previously used
void RibbonToNotesAudioProcessor::AddSentAllNotesOff(juce::MidiBuffer& processedMidi, int channel)
{
    processedMidi.addEvent(juce::MidiMessage::allNotesOff(channel), juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime);
    
    //loop through array
    for(int i=0;i<MAX_ZONES;i++)
    {
        //if note was pressed, the channel was set.
        if(notePressedChannel[i]>0)
        {
            processedMidi.addEvent(juce::MidiMessage::allNotesOff(notePressedChannel[i]), juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime);
            notePressedChannel[i]=-1; //remove the channel setting, because all notes have been cleared
        }
    }
}

// add notes off for previous played notes on the selected channel
void RibbonToNotesAudioProcessor::AddPreviousNotesSentNotesOff(juce::MidiBuffer& processedMidi, int channel)
{
    //loop through array
    for(int i = 0; i < notesPressed.size(); i++)
    {
        auto note =notesPressed[i];
        
        auto message1 = juce::MidiMessage::noteOn(channel, note, 0.0f);
        processedMidi.addEvent(message1, juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime);
        
        auto message2 = juce::MidiMessage::noteOff(channel,note);
        processedMidi.addEvent(message2, juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime);
    }
    notesPressed.removeRange(0,notesPressed.size());
}

// add notes on for the selected zone on the given channel
void RibbonToNotesAudioProcessor::AddSentNotesOn(juce::MidiBuffer& processedMidi, int selectedAlt, int selectedZone, int channel)
{
    //loop through array
    for(int j=0;j<MAX_NOTES;j++)
    {
        int note = notesToPlay[selectedAlt][selectedZone][j];
        if(((int)(*chordNotes[selectedAlt][selectedZone][j]))==0) break;
        auto message = juce::MidiMessage::noteOn(channel,note,*noteVelocity);
        processedMidi.addEvent(message, juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime);
        notesPressed.add(note);
    }
}

//==============================================================================
// Utility functions
//==============================================================================
// build a chord for each zone based on the key and the chord notes
void RibbonToNotesAudioProcessor::BuildChords(int alternative)
{
    int maxNote = 0;
    int key = 0;
    int octave = (int) *octaves;
    int addOctaves=0;
    
    for(int zone=0 ; zone < MAX_ZONES; zone++)
    {
        key = (int) *selectedKeys[alternative][zone];
        
        //pitchMode 0 = up
        if(*pitchMode == 0)
        {
            //if the notevalue is lower then the highest note, just add an octave to it.
            if(key <= maxNote && (key + ( octave + addOctaves + 1) * 12) < 128)
            {
                addOctaves++;
            }
            maxNote = key;
        }
        GetNoteNumbersForChord(octave + addOctaves, alternative, zone, key);
    }
}

// calculate the notes to be played for a specific zone
// since key equals to one instead of zero, the counting is a bit strange
void RibbonToNotesAudioProcessor::GetNoteNumbersForChord(int addOctaves, int alternative, int zone, int key)
{
    for(int j=0;j<MAX_NOTES;j++)
    {
        int note = (int) *chordNotes[alternative][zone][j];
        if(note == 0)
        {
            notesToPlay[alternative][zone][j] = note;
        }
        else
        {
            if(note > 0) note = note - 1; //offset for positive notes is +1. Correct it here.
            int keynote = key + 24 - 1; //Since addoctaves starts at -2, offset for key is -24. Also there is an offset of +1, because C corresponds to 1 in the list instead of 0. Both are corrected here.
            if(keynote + note > 8 && addOctaves > 7) addOctaves = 7; //do not go past G8
            notesToPlay[alternative][zone][j]= (keynote + note + addOctaves * 12);
        }
    }
}
// determine if the cc value has changed to another zone
bool RibbonToNotesAudioProcessor::HasChanged(int ccval)
{
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
