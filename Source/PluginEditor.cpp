/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
RibbonToNotesAudioProcessorEditor::RibbonToNotesAudioProcessorEditor (RibbonToNotesAudioProcessor& p,juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 400);
    numberOfZones = 6;//sizeof(noteOrder)/sizeof(noteOrder[0]);
    CreateGui();
    SetSplitRanges();
    SyncSliderValues();
    AddListeners();
}

RibbonToNotesAudioProcessorEditor::~RibbonToNotesAudioProcessorEditor()
{
    sldMidiCCAttachment = nullptr;
    sldVelocityAttachment = nullptr;
}

void RibbonToNotesAudioProcessorEditor::CreateGui()
{
    CreateDial(sldMidiCC);
    sldMidiCCAttachment.reset (new SliderAttachment (valueTreeState, "midicc", sldMidiCC));
    sldMidiCC.setRange(1, 127,1);
    sldMidiCC.setValue(*audioProcessor.midiCC);
    lblMidiCC.setText("Midi CC", juce::dontSendNotification);
    lblMidiCC.attachToComponent(&sldMidiCC, false);
    lblMidiCC.setJustificationType(juce::Justification::centred);

    CreateDial(sldNumberOfZones);
    sldNumberOfZones.setRange(1, 12,1.0);
    sldNumberOfZones.setValue(numberOfZones);
    lblNumberOfZones.setText("Zones", juce::dontSendNotification);
    lblNumberOfZones.attachToComponent(&sldNumberOfZones, false);
    lblNumberOfZones.setJustificationType(juce::Justification::centred);

    CreateDial(sldVelocity);
    sldVelocityAttachment.reset(new SliderAttachment (valueTreeState, "velocity", sldVelocity));
    sldVelocity.setRange(0.0, 1.0, 1/127.0);
    sldVelocity.setValue(*audioProcessor.noteVelocity);
    lblVelocity.setText("Velocity", juce::dontSendNotification);
    lblVelocity.attachToComponent(&sldVelocity, false);
    lblVelocity.setJustificationType(juce::Justification::centred);

    
    CreateDial(sldOctave);
    sldOctave.setRange(0, 8,1.0);
    sldOctave.setValue(2);
    lblOctave.setText("Octave", juce::dontSendNotification);
    lblOctave.attachToComponent(&sldOctave, false);
    lblOctave.setJustificationType(juce::Justification::centred);

    for(int i=0;i<MAX_NOTES;i++)
    {
        //cmbNotes[i] = new juce::ComboBox;
        addAndMakeVisible(cmbNotes[i]);
        if(i < MAX_NOTES-1)
        {
            CreateSlider(sldArSplitValues[i]);
            addAndMakeVisible(lblArSplitValues[i]);
        }
    }
}

void RibbonToNotesAudioProcessorEditor::SetSplitRanges()
{
    int stepSize = 127/numberOfZones;
    bool enabled = true;
    for(int i=0;i<MAX_NOTES;i++)
    {
        if(i>=numberOfZones)
        {
            enabled=false;
        }

        cmbNotes[i].addItemList(notesArray, 1);
        cmbNotes[i].setSelectedId(noteOrder[i]);
        cmbNotes[i].setEnabled(enabled);
        if(i < MAX_NOTES-1)
        {
            int value = enabled? 1 + i * stepSize:0;
            if(i>=numberOfZones) value = 128;
            sldArSplitValues[i].setRange(fmax(value + 1 - stepSize,0), fmin(value - 1 + stepSize,128), 1);
            sldArSplitValues[i].setValue(value);
            sldArSplitValues[i].setEnabled(enabled);
        }
    }
}

void RibbonToNotesAudioProcessorEditor::RedistributeSplitRanges()
{
    int stepSize = (audioProcessor.splitValues[numberOfZones-1] - audioProcessor.splitValues[0])/(numberOfZones-1);
    bool enabled = true;
    for(int i=1;i<numberOfZones-1;i++)
    {
        int value = audioProcessor.splitValues[0] + i * stepSize;
        audioProcessor.splitValues[i] = value;
        sldArSplitValues[i].setRange(fmax(value + 1 - stepSize,0), fmin(value - 1 + stepSize,128), 1);
        sldArSplitValues[i].setValue(value);
    }
}
void RibbonToNotesAudioProcessorEditor::CreateDial(juce::Slider& sld)
{
    addAndMakeVisible(sld);
    sld.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
}
void RibbonToNotesAudioProcessorEditor::CreateSlider(juce::Slider& sld)
{
    addAndMakeVisible(sld);
    sld.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
}

void RibbonToNotesAudioProcessorEditor::AddListeners()
{
    // add the listener to the slider
    sldMidiCC.addListener(this);
    sldNumberOfZones.addListener (this);
    sldVelocity.addListener(this);
    sldOctave.addListener(this);
    for(int i=0;i<MAX_NOTES;i++)
    {
        sldArNoteNumber[i].addListener(this);
        sldArSplitValues[i].addListener(this);
        cmbNotes[i].addListener(this);
    }
}

int RibbonToNotesAudioProcessorEditor::GetNumberOfZones()
{
    int zones = sldNumberOfZones.getValue();
    zones = zones > MAX_NOTES ? MAX_NOTES : zones;
    zones = zones > sizeof(noteOrder)/sizeof(noteOrder[0]) ? sizeof(noteOrder)/sizeof(noteOrder[0]) : zones;
    return zones;
}
//==============================================================================
void RibbonToNotesAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
}

void RibbonToNotesAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto leftMargin = getWidth() * 0.02;
    auto topMargin = getHeight() * 0.02;
    auto controlWidth = (getWidth() / (0.5 * MAX_NOTES)) - 2 * leftMargin;
    int textHeight = 22;
    auto dialHeight = textHeight + controlWidth;
    auto vsliderHeight = 2*textHeight;

    int top1 = topMargin + textHeight;
    int top2 = top1 + dialHeight + topMargin;
    int top3 = top2 + topMargin + textHeight;
    int topOffsetRow2 =vsliderHeight + topMargin+topMargin + textHeight;

    sldMidiCC.setBounds(leftMargin, top1, controlWidth, dialHeight);
    sldMidiCC.setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);
    sldNumberOfZones.setBounds(leftMargin + 1*(leftMargin + controlWidth), top1, controlWidth, dialHeight);
    sldNumberOfZones.setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);
    sldVelocity.setBounds(leftMargin + 2*(leftMargin + controlWidth), top1, controlWidth, dialHeight);
    sldVelocity.setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);
    sldOctave.setBounds(leftMargin + 3*(leftMargin + controlWidth), top1, controlWidth, dialHeight);
    sldOctave.setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);

    int row = 0;
    for(int i=0;i<MAX_NOTES;i++)
    {
        if(i== MAX_NOTES/2)
        {
            row++;
        }
        cmbNotes[i].setBounds(leftMargin + 0.5 * controlWidth + (i-row*6) * (leftMargin + controlWidth), top2 + row * topOffsetRow2, controlWidth, textHeight);
        sldArSplitValues[i].setBounds(leftMargin + (i-row*6) * (leftMargin + controlWidth), top3 + row * topOffsetRow2, controlWidth, vsliderHeight);
        sldArSplitValues[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);
    }
}
void RibbonToNotesAudioProcessorEditor::SyncNotesAndSplits()
{
    int maxNote = 0;
    int note = 0;
    int addOctaves=0;
    for(int i=0 ; i < numberOfZones; i++)
    {
        note = cmbNotes[i].getSelectedId();
        if(note <= maxNote)
        {
            addOctaves++;
        }
        audioProcessor.noteValues[i] = note + 23 + (sldOctave.getValue()+addOctaves)*12;
        maxNote = note;
        if(i<numberOfZones)
        {
            audioProcessor.splitValues[i] = sldArSplitValues[i].getValue();
        }
        else
            audioProcessor.splitValues[i] = 128;
    }
}


void RibbonToNotesAudioProcessorEditor::SyncSliderValues()
{
    auto zones = GetNumberOfZones();
    audioProcessor.numberOfZones = zones;
    
    if(numberOfZones!=zones)
    {
        numberOfZones=zones;
        SetSplitRanges();
    }
    else
    {
        RedistributeSplitRanges();
    }
    SyncNotesAndSplits();
}

void RibbonToNotesAudioProcessorEditor::SyncComboBoxValues()
{
    for(int i=0 ; i < numberOfZones; i++)
    {
        noteOrder[i] = cmbNotes[i].getSelectedId();
    }
    SyncNotesAndSplits();
}

void RibbonToNotesAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    SyncSliderValues();
}
void RibbonToNotesAudioProcessorEditor::comboBoxChanged(juce::ComboBox* combobox)
{
    SyncSliderValues();
}
