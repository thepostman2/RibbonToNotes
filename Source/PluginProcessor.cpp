/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

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
    int defaultOctave = 2;
    params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{OCTAVES_ID,versionHint1},
                                                               OCTAVES_NAME,
                                                               -2,
                                                               8,
                                                               defaultOctave));

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

    params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{ACTIVEPROGRESSION_ID,versionHint1},
                                                               ACTIVEPROGRESSION_NAME,
                                                               0,
                                                               7,
                                                               0));
    int stepSize = 127/DEFAULT_NUMBEROFZONES;
    bool enabled = true;

    for(int prog=0;prog<MAX_PROGRESSIONSKNOBS;prog++)
    {
        if(prog<MAX_PROGRESSIONS)
        {
            for(int i=0;i<MAX_SPLITS;i++)
            {
                if(i<MAX_ZONES)
                {
                    params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{KEYS_ID + std::to_string(prog) + "_" + std::to_string(i),versionHint1},
                                                                               KEYS_NAME,
                                                                               1,
                                                                               12,
                                                                               defaultNoteOrder[i]));
                    params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{CHORDS_ID + std::to_string(prog) + "_" + std::to_string(i),versionHint1},
                                                                               CHORDS_NAME,
                                                                               1,
                                                                               chordsArray.size(),
                                                                               1));
                    int chordBuildDefault = 0;//default only base note
                    int noteDefault = defaultNoteOrder[i] + 24 -1 + defaultOctave * 12;

                    for(int j=0;j<MAX_NOTES;j++)
                    {
                        params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{CHORDBUILDS_ID + std::to_string(prog) + "_" + std::to_string(i) + "_" + std::to_string(j),versionHint1},
                                                                                   CHORDBUILDS_NAME,
                                                                                   -128,
                                                                                   1278,
                                                                                   chordBuildDefault));
                        chordBuildDefault = NONOTE;//default only the base note
                        
                        params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{NOTESTOPLAY_ID 
                            + std::to_string(prog) + "_"
                            + std::to_string(i) + "_"
                            + std::to_string(j), versionHint1},
                                                                                   NOTESTOPLAY_NAME,
                                                                                   0,
                                                                                   128,
                                                                                   noteDefault));
                        noteDefault = 0;//default no chords
                    }
                }
                if(i >= DEFAULT_NUMBEROFZONES)
                {
                    enabled=false;
                }
                int defaultsplit = enabled? 1 + i * stepSize:0;
                if(i>=DEFAULT_NUMBEROFZONES) defaultsplit = 128;
                if(prog == 0)
                {
                    params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{SPLITS_ID + std::to_string(i),versionHint1},
                                                                               SPLITS_NAME,
                                                                               0,
                                                                               128,
                                                                               defaultsplit));
                }
            }
        }
        params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{DEFCONCAT(MIDIINMESSAGETYPE_ID, PROGRESSION) + std::to_string(prog),versionHint1},
                                                                   MIDIINMESSAGETYPE_NAME,
                                                                   0,
                                                                   2,
                                                                   2));

        params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{DEFCONCAT(MIDIINCHANNEL_ID, PROGRESSION) + std::to_string(prog),versionHint1},
                                                                   MIDIINCHANNEL_NAME,
                                                                   0,
                                                                   16,
                                                                   0));

        params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{DEFCONCAT(MIDIINNUMBER_ID, PROGRESSION) + std::to_string(prog),versionHint1},
                                                                   MIDIINNUMBER_NAME,
                                                                   0,
                                                                   127,
                                                                   24+prog));

        params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{DEFCONCAT(MIDIINMINVALUE_ID, PROGRESSION) + std::to_string(prog),versionHint1},
                                                                   MIDIINMINVALUE_NAME,
                                                                   0,
                                                                   127,
                                                                   1));

        params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{DEFCONCAT(MIDIINMAXVALUE_ID, PROGRESSION) + std::to_string(prog),versionHint1},
                                                                   MIDIINMAXVALUE_NAME,
                                                                   0,
                                                                   127,
                                                                   127));
    }
    params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{DEFCONCAT(MIDIINMESSAGETYPE_ID, VELOCITY_ID),versionHint1},
                                                               MIDIINMESSAGETYPE_NAME,
                                                               0,
                                                               2,
                                                               0));

    params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{DEFCONCAT(MIDIINCHANNEL_ID, VELOCITY_ID),versionHint1},
                                                               MIDIINCHANNEL_NAME,
                                                               0,
                                                               16,
                                                               0));

    params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{DEFCONCAT(MIDIINNUMBER_ID, VELOCITY_ID),versionHint1},
                                                               MIDIINNUMBER_NAME,
                                                               0,
                                                               127,
                                                               11));

    params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{DEFCONCAT(MIDIINMINVALUE_ID, VELOCITY_ID),versionHint1},
                                                               MIDIINMINVALUE_NAME,
                                                               0,
                                                               127,
                                                               0));  
    params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{DEFCONCAT(MIDIINMAXVALUE_ID, VELOCITY_ID),versionHint1},
                                                               MIDIINMAXVALUE_NAME,
                                                               0,
                                                               127,
                                                               127));
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
    activeProgression = apvts.getRawParameterValue(ACTIVEPROGRESSION_ID);
    activeProgressionKnob = *activeProgression;

    for(int i=0;i<MAX_SPLITS;i++)
    {
        splitValues[i] = apvts.getRawParameterValue(SPLITS_ID + std::to_string(i));
    }
    *splitValues[0]=0;
    lastChannel = 1;

    for(int prog=0;prog<MAX_PROGRESSIONSKNOBS;prog++)
    {
        if(prog<MAX_PROGRESSIONS)
        {
            for(int i=0;i<MAX_ZONES;i++)
            {
                selectedKeys[prog][i] = apvts.getRawParameterValue(KEYS_ID + std::to_string(prog) + "_" + std::to_string(i));
                selectedChord[prog][i] = apvts.getRawParameterValue(CHORDS_ID + std::to_string(prog) + "_" + std::to_string(i));
                for(int j=0;j<MAX_NOTES;j++)
                {
                    chordNotes[prog][i][j] = apvts.getRawParameterValue(CHORDBUILDS_ID + std::to_string(prog) + "_" + std::to_string(i) + "_" + std::to_string(j));
                    notesToPlay[prog][i][j] = apvts.getRawParameterValue(NOTESTOPLAY_ID 
                                                                         + std::to_string(prog) + "_" 
                                                                         + std::to_string(i) + "_"
                                                                         + std::to_string(j));
                }
                notePressedChannel[i]=-1;
            }
        }
        midiInProgression[prog].MessageType = apvts.getRawParameterValue(DEFCONCAT(MIDIINMESSAGETYPE_ID, PROGRESSION) + std::to_string(prog));
        midiInProgression[prog].Channel = apvts.getRawParameterValue(DEFCONCAT(MIDIINCHANNEL_ID, PROGRESSION) + std::to_string(prog));
        midiInProgression[prog].Number = apvts.getRawParameterValue(DEFCONCAT(MIDIINNUMBER_ID, PROGRESSION) + std::to_string(prog));
        midiInProgression[prog].MinValue = apvts.getRawParameterValue(DEFCONCAT(MIDIINMINVALUE_ID, PROGRESSION) + std::to_string(prog));
        midiInProgression[prog].MaxValue = apvts.getRawParameterValue(DEFCONCAT(MIDIINMAXVALUE_ID, PROGRESSION) + std::to_string(prog));
    }
    midiInVelocity.MidiInfoID = DEFCONCAT(MIDIINMESSAGETYPE_ID, VELOCITY_ID);
    midiInVelocity.MessageType = apvts.getRawParameterValue(DEFCONCAT(MIDIINMESSAGETYPE_ID, VELOCITY_ID));
    midiInVelocity.Channel = apvts.getRawParameterValue(DEFCONCAT(MIDIINCHANNEL_ID, VELOCITY_ID));
    midiInVelocity.Number = apvts.getRawParameterValue(DEFCONCAT(MIDIINNUMBER_ID, VELOCITY_ID));
    midiInVelocity.MinValue = apvts.getRawParameterValue(DEFCONCAT(MIDIINMINVALUE_ID, VELOCITY_ID));
    midiInVelocity.MaxValue = apvts.getRawParameterValue(DEFCONCAT(MIDIINMAXVALUE_ID, VELOCITY_ID));
    
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

//return true if de controller message is the selected controller message.
bool RibbonToNotesAudioProcessor::PlayMidi(int &ccval, int &channel, const juce::MidiMessage &message)
{
    if(message.isController() && message.getControllerNumber() == (int) *midiCC)
    {
        ccval = message.getControllerValue();
        channel = message.getChannel();
        return true;
    }
    //not a ccval linked to the ribbon
    else
    {
        if(SetControlByMidi(message) == false)
        {
            notesToPlayBuffer.addEvent(message, juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime);
        }
    }
    return false;
}

void RibbonToNotesAudioProcessor::LearnMidi(const juce::MidiMessage &message) 
{
    if(MidiLearnInterface::MidiLearnOn)
    {
        midiLearnBuffer.addEvent(message, juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime);
    }
    else
    {
        midiLearnBuffer.clear();
    }
    if(message.isNoteOff() || message.isAllNotesOff() || message.isNoteOn())
    {
        auto notemessageOrg = juce::MidiMessage(message);
        notemessageOrg.setVelocity(0.0);// just in case the note was pressed before midi learn was switched on.
        notesToPlayBuffer.addEvent(notemessageOrg, juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime);
        auto notemessage = juce::MidiMessage(message);
        notemessage.setChannel(fmax((int)*channelOut,1));
        notesToPlayBuffer.addEvent(notemessage, juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime);
    }
}

//==============================================================================
void RibbonToNotesAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // A pure MIDI plugin shouldn't be provided any audio data
    //jassert (buffer.getNumChannels() == 0);
    
    // however we use the buffer to get timing information
    auto numSamples = buffer.getNumSamples();
    
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    buffer.clear();
        
    int ccval = lastCCValue;
    int channel = lastChannel;
    bool ccPlayNotes = false;
    //filter the cc mesagges of the selected midiCC
    for(const auto metadata : midiMessages)
    {
        const auto message = metadata.getMessage();
        //auto time = metadata.samplePosition;
        if(MidiLearnInterface::MidiSettingOn == false && MidiLearnInterface::MidiLearnOn == false)
        {
            //ccval linked to the ribbon?
            if(PlayMidi(ccval, channel, message))
            {
                ccPlayNotes = true;
            }
        }
        else
        {
            LearnMidi(message);
        }
    }
    midiMessages.clear();

    //play notes or stop playing notes based on the cc value
    if(ccPlayNotes && HasChanged(ccval))
    {
        AddNotesToPlayToBuffer(ccval, channel, notesToPlayBuffer);
    }
    lastCCValue = ccval;
    
    // some programs do not except multiple messages added to the buffer.
    // so adding the notes one by one solves this problem
    // Mind you that processblock fires each sample time, so for the user
    // it is as if the notes are played immediately.
    PlayNextMidiMessages(midiMessages,0,numSamples);
}

// plays the first of the buffered notes, and removes it from the buffer
void RibbonToNotesAudioProcessor::PlayNextMidiMessages(juce::MidiBuffer &midiMessages,
                                               const int startSample,
                                               const int numSamples)
{
    juce::MidiBuffer tmpBuffer;
    int i = 0;

    const int firstEventToAdd = notesToPlayBuffer.getFirstEventTime();
    const double scaleFactor = numSamples / (double) (notesToPlayBuffer.getLastEventTime() + 1 - firstEventToAdd);

    for(const auto metadata : notesToPlayBuffer)
    {
        const auto pos = juce::jlimit (0, numSamples - 1, juce::roundToInt ((metadata.samplePosition - firstEventToAdd) * scaleFactor));
        auto message = metadata.getMessage();
        
        //workaround to avoid problems in Blue Cat's Patchwork. Not to happy with this, because it introduces
        //unecessary latency. That is why I only apply it for small buffers. With large buffer sizes, the latency gets
        //to much noticeable.
        if(i<1 || numSamples > 256)
        {
            // add message to be excecuted
            midiMessages.addEvent (message, startSample + pos);
        }
        else
        {
            // other messages are not executed but copied to temporary buffer. This is needed for Blue Cat's Patchwork
            // In Logic it is not a problem to have a buffer with multiple messages.
            tmpBuffer.addEvent(message, juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime);
        }
        i++;
    }
    // swap the buffer, so the excuted message is removed from the buffer.
    notesToPlayBuffer.swapWith(tmpBuffer);
}
//==============================================================================
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

int RibbonToNotesAudioProcessor::getActiveProgression() const
{
    return (int) *activeProgression;
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
void RibbonToNotesAudioProcessor::AddNotesToPlayToBuffer(int ccval)
{
    AddNotesToPlayToBuffer(ccval, std::max((int) *channelOut,1), notesToPlayBuffer);
}

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
                AddSentNotesOn(midiMessages, getActiveProgression(), zone, channel);
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
        int note = (int) *notesToPlay[selectedAlt][selectedZone][j];
        if(((int)(*chordNotes[selectedAlt][selectedZone][j]))==NONOTE) break;
        auto message = juce::MidiMessage::noteOn(channel,note,*noteVelocity);
        processedMidi.addEvent(message, juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime);
        notesPressed.add(note);
    }
}
//==============================================================================
// Select progression
//==============================================================================
bool RibbonToNotesAudioProcessor::SetControlByMidi(const juce::MidiMessage &midiMessage)
{
    auto messageType = midiMessage.isController() ? 1 : midiMessage.isNoteOn() ? 2 : 0;
    if(messageType == 0) return false;
 
    //controller value that sets the midi velocity?
    if(midiInVelocity.MidiMessageComplies(midiMessage))
    {
        auto messageValue = messageType == 1 ? midiMessage.getControllerValue() : midiMessage.getVelocity();
        *noteVelocity = (*midiInVelocity.MinValue + (*midiInVelocity.MaxValue - *midiInVelocity.MinValue) * messageValue / 127.0) / 127.0;
        return true;
    }

    int ap = ((int) *activeProgression);
    for(int i=0; i < MAX_PROGRESSIONSKNOBS;i++)
    {
        if(midiInProgression[i].MidiMessageComplies(midiMessage))
        {
            if(i < MAX_PROGRESSIONS)
            {
                ap = i;
            }
            else if(i == MAX_PROGRESSIONS)
            {
                ap--;
                ap = ap < 0 ? MAX_PROGRESSIONS-1 : ap;
            }
            else
            {
                ap++;
                ap = ap < MAX_PROGRESSIONS ? ap : 0;
            }
            UpdateParameter(ap, ACTIVEPROGRESSION_ID);
            return true;
        }
    }
    return false;
}

//==============================================================================
// Utility functions
//==============================================================================
void RibbonToNotesAudioProcessor::UpdateParameter(int value, juce::String parameterID)
{
    auto pParam = apvts.getParameter(parameterID);
    pParam->beginChangeGesture();
    pParam->setValueNotifyingHost(pParam->convertTo0to1(value));
    pParam->endChangeGesture();
}

bool RibbonToNotesAudioProcessor::HasChanged(int ccval)
{
    int zoneUpper = (int) *splitValues[activeZone];
    int zoneLower = activeZone > 0 ? (int) *splitValues[activeZone-1] : 0;
    return ccval > zoneUpper || ccval < zoneLower;
}
