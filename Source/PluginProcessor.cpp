/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <Carbon/Carbon.h>

//==============================================================================
RibbonToNotesAudioProcessor::RibbonToNotesAudioProcessor()
: AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                  .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
                  .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
                  )

#ifndef JucePlugin_PreferredChannelConfigurations
, parameters(*this, nullptr, juce::Identifier("RibbonToNotes"),
             {
    std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"midicc",1},
                                                            "MidiCC",
                                                            0.0f,
                                                            127.0f,
                                                            22.0f),
    std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"velocity",2},
                                                            "Velocity",
                                                            0.0f,
                                                            1.0f,
                                                            90.0/127.0)
            })
#endif
{
    midiCC = parameters.getRawParameterValue("midicc");
    noteVelocity = parameters.getRawParameterValue("velocity");
    int cnt = sizeof(notePressedChannel)/sizeof(notePressedChannel[0]);
    for(int i=0;i<cnt;i++)
    {
        notePressedChannel[i]=-1;
        splitValues[i]=128;
    }
    splitValues[0]=0;
}

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

void RibbonToNotesAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // A pure MIDI plugin shouldn't be provided any audio data
    jassert (buffer.getNumChannels() == 0);
    
    // however we use the buffer to get timing information
    auto numSamples = buffer.getNumSamples();
    
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    buffer.clear();
    
    //create a new buffer
    juce::MidiBuffer processedMidi;
    
    for(const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        auto time = metadata.samplePosition;
        
        if(message.isController() && message.getControllerNumber() == (int) *midiCC)
        {
            auto ccval = message.getControllerValue();
            auto channel = message.getChannel();
            for(int i=0 ; i < numberOfZones ;i++)
            {
                if(ccval < splitValues[i])
                {
                    //if ccval is 0, then stop any note from sounding
                    SentNotesOff(processedMidi,-1,time);
                    break;
                }
                //if ccval is in range according to spliValues array, start the note
                if(ccval < splitValues[i+1])
                {
                    //only do something if the same note is not already pressed
                    if(notePressedChannel[i] != channel)
                    {
                        //first send noteOff for previous note.
                        SentNotesOff(processedMidi, i, time);
                        //create new noteOn
                        message = juce::MidiMessage::noteOn(channel,
                                                            noteValues[i],
                                                            *noteVelocity);
                        notePressedChannel[i] = channel;
                    }
                    //stop the loop as soon as a range was valid
                    break;
                }
            }
            
        }
        //add the noteOn event to the buffer
        processedMidi.addEvent(message,time);
    }
    //swap the original buffer with the new created one
    midiMessages.swapWith(processedMidi);
}

void RibbonToNotesAudioProcessor::SentNotesOff(juce::MidiBuffer& processedMidi, int exceptNote, int time)
{
    //loop through array
    for(int i=0;i<MAX_NOTES;i++)
    {
        //if note was pressed, the channel was set.
        if(notePressedChannel[i]>0 && i!=exceptNote)
        {
            auto message = juce::MidiMessage::noteOff(notePressedChannel[i],noteValues[i]);
            processedMidi.addEvent(message, time+10*i);
            auto message2 = juce::MidiMessage::noteOn(notePressedChannel[i], noteValues[i], 0.0f);
            processedMidi.addEvent(message2, time+20*i);
            notePressedChannel[i]=-1;
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
    return new RibbonToNotesAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void RibbonToNotesAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void RibbonToNotesAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RibbonToNotesAudioProcessor();
}
