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
    CreateGui();
    SetSplitRanges();
    SyncSliderValues();
    AddListeners();
}

RibbonToNotesAudioProcessorEditor::~RibbonToNotesAudioProcessorEditor()
{
    sldMidiCCAttachment = nullptr;
    sldNumberOfZonesAttachment = nullptr;
    sldVelocityAttachment = nullptr;
    sldOctaveAttachment = nullptr;
    for(int i=0;i<MAX_SPLITS;i++)
    {
        if(i<MAX_NOTES)
        {
            cmbNotesAttachment[i] = nullptr;
        }
        sldSplitValuesAttachment[i] = nullptr;
    }
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
    sldNumberOfZonesAttachment.reset(new SliderAttachment(valueTreeState, "numberofzones", sldNumberOfZones));
    sldNumberOfZones.setRange(1, 12,1.0);
    sldNumberOfZones.setValue(*audioProcessor.numberOfZones);
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
    sldOctaveAttachment.reset(new SliderAttachment(valueTreeState, "octaves", sldOctave));
    sldOctave.setRange(0, 8,1.0);
    sldOctave.setValue(*audioProcessor.octaves);
    lblOctave.setText("Octave", juce::dontSendNotification);
    lblOctave.attachToComponent(&sldOctave, false);
    lblOctave.setJustificationType(juce::Justification::centred);
    

    for(int i=0;i<MAX_SPLITS;i++)
    {
        bool enabled = true;
        if(i>=(int)(*audioProcessor.numberOfZones))
        {
            enabled=false;
        }
        if(i<MAX_NOTES)
        {
            //cmbNotes[i] = new juce::ComboBox;
            addAndMakeVisible(cmbNotes[i]);
            cmbNotesAttachment[i].reset(new ComboBoxAttachment(valueTreeState, "notes" + std::to_string(i), cmbNotes[i]));
            cmbNotes[i].addItemList(notesArray, 1);
            cmbNotes[i].setSelectedId((((int)*audioProcessor.noteValues[i]) % 12)+1);
            cmbNotes[i].setEnabled(enabled);

            addAndMakeVisible(cmbChords[i]);
            cmbChordsAttachment[i].reset(new ComboBoxAttachment(valueTreeState, "chords" + std::to_string(i), cmbChords[i]));
            cmbChords[i].addItemList(chordsArray, 1);
            cmbChords[i].setSelectedId((int)*audioProcessor.chordValues[i]);
            cmbChords[i].setEnabled(enabled);
            cmbChords[i].onChange = [this, i] {SetChordBuildFromChord(i);};


            addAndMakeVisible(edtChordBuilder[i]);
            edtChordBuilder[i].setEnabled(enabled);
            edtChordBuilder[i].setEditable(true);
            juce::String chordBuildStr;
            for(int j=0;j<MAX_NOTES;j++)
            {
                chordBuildStr = chordBuildStr + std::to_string((int)*audioProcessor.chordBuilds[i][j]);
                if(j+1==MAX_NOTES || (int)*audioProcessor.chordBuilds[i][j+1] == 0)
                {
                    break;
                }
                else
                {
                    chordBuildStr = chordBuildStr + ",";
                }
            }
            edtChordBuilder[i].setText(chordBuildStr, juce::dontSendNotification);
            edtChordBuilder[i].onTextChange = [this, i] {GetChordBuild(i);};
        }
        CreateSlider(sldArSplitValues[i]);
        sldSplitValuesAttachment[i].reset(new SliderAttachment(valueTreeState, "splits" + std::to_string(i), sldArSplitValues[i]));
        addAndMakeVisible(lblArSplitValues[i]);
    }
    CreateSlider(sldArSplitExtra);
    addAndMakeVisible(sldArSplitExtra);
}

void RibbonToNotesAudioProcessorEditor::SetSplitRanges()
{
    int stepSize = 127/((int)(*audioProcessor.numberOfZones));
    bool enabled = true;
    for(int i=0;i<MAX_SPLITS;i++)
    {
        enabled = i < ((int)(*audioProcessor.numberOfZones));
        if(i<MAX_NOTES)
        {
            cmbNotes[i].setEnabled(enabled);
            cmbChords[i].setEnabled(enabled);
            edtChordBuilder[i].setEnabled(enabled);
        }
        int value = enabled? 1 + i * stepSize:0;
        if(i>=((int)(*audioProcessor.numberOfZones))) value = 128;
        sldArSplitValues[i].setRange(fmax(value + 1 - stepSize,0), fmin(value - 1 + stepSize,128), 1);
        sldArSplitValues[i].setValue(value);
        sldArSplitValues[i].setEnabled(enabled);
        if(i == (MAX_NOTES/2)-1)
        {
            sldArSplitExtra.setRange(fmax(value + 1 - stepSize,0), fmin(value - 1 + stepSize,128), 1);
            sldArSplitValues[i].setValue(value);
            sldArSplitValues[i].setEnabled(enabled);
        }
    }
}

void RibbonToNotesAudioProcessorEditor::RedistributeSplitRanges()
{
    int stepSize = ((*audioProcessor.splitValues[((int)(*audioProcessor.numberOfZones))-1]) - (*audioProcessor.splitValues[0]))/(((int)(*audioProcessor.numberOfZones))-1);
    //bool enabled = true;
    for(int i=1;i<((int)(*audioProcessor.numberOfZones))-1;i++)
    {
        int value = (*audioProcessor.splitValues[0]) + i * stepSize;
        *audioProcessor.splitValues[i] = value;
        sldArSplitValues[i].setRange(fmax(value + 1 - stepSize,0), fmin(value - 1 + stepSize,128), 1);
        sldArSplitValues[i].setValue(value);
        if(i == (MAX_NOTES/2)-1)
        {
            sldArSplitExtra.setRange(fmax(value + 1 - stepSize,0), fmin(value - 1 + stepSize,128), 1);
            sldArSplitValues[i].setValue(value);
        }
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
    for(int i=0;i<MAX_SPLITS;i++)
    {
        if(i<MAX_NOTES)
        {
            sldArNoteNumber[i].addListener(this);
            cmbNotes[i].addListener(this);
            cmbChords[i].addListener(this);
        }
        sldArSplitValues[i].addListener(this);
    }
    sldArSplitExtra.addListener(this);
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
    auto controlWidth = (getWidth() / (0.5 * MAX_NOTES + 0.5)) - 2 * leftMargin;
    int textHeight = 22;
    auto dialHeight = textHeight + controlWidth;
    auto vsliderHeight = 2*textHeight;

    int topGeneralControls =  + textHeight;
    int topRowA = topGeneralControls + dialHeight + topMargin;
    int topRowB = topRowA + topMargin + textHeight;
    int topRowC = topRowB + topMargin + textHeight;
    int topRowSplitSliders = topRowC + topMargin + textHeight;
    int topOffsetRow2 = (topRowSplitSliders-topRowA) + vsliderHeight + textHeight - topMargin;

    sldMidiCC.setBounds(leftMargin, topGeneralControls, controlWidth, dialHeight);
    sldMidiCC.setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);
    sldNumberOfZones.setBounds(leftMargin + 1*(leftMargin + controlWidth), topGeneralControls, controlWidth, dialHeight);
    sldNumberOfZones.setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);
    sldVelocity.setBounds(leftMargin + 2*(leftMargin + controlWidth), topGeneralControls, controlWidth, dialHeight);
    sldVelocity.setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);
    sldOctave.setBounds(leftMargin + 3*(leftMargin + controlWidth), topGeneralControls, controlWidth, dialHeight);
    sldOctave.setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);

    int row = 0;
    for(int i=0;i<MAX_SPLITS;i++)
    {
        if(i== MAX_NOTES/2)
        {
            sldArSplitExtra.setBounds(leftMargin + (i-row*6) * (leftMargin + controlWidth), topRowSplitSliders + row * topOffsetRow2, controlWidth, vsliderHeight);
            sldArSplitExtra.setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);
            row++;
        }
        if(i<MAX_NOTES)
        {
            cmbNotes[i].setBounds(leftMargin + 0.5 * controlWidth + (i-row*6) * (leftMargin + controlWidth), topRowA + row * topOffsetRow2, controlWidth, textHeight);

            cmbChords[i].setBounds(leftMargin + 0.5 * controlWidth + (i-row*6) * (leftMargin + controlWidth), topRowB + row * topOffsetRow2, controlWidth, textHeight);

            edtChordBuilder[i].setBounds(leftMargin + 0.5 * controlWidth + (i-row*6) * (leftMargin + controlWidth), topRowC + row * topOffsetRow2, controlWidth, textHeight);
        }
        sldArSplitValues[i].setBounds(leftMargin + (i-row*6) * (leftMargin + controlWidth), topRowSplitSliders + row * topOffsetRow2, controlWidth, vsliderHeight);
        sldArSplitValues[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);
    }
}
void RibbonToNotesAudioProcessorEditor::SyncNotesAndSplits()
{
    int maxNote = 0;
    int note = 0;
    int addOctaves=0;
    for(int i=0 ; i < MAX_NOTES; i++)
    {
        note = cmbNotes[i].getSelectedId();
        //if the notevalue is lower then the highest note, just add an octave to it.
        if(note <= maxNote && (note + 23 + (sldOctave.getValue()+addOctaves+1)*12) < 128)
        {
            addOctaves++;
        }
        
        *audioProcessor.noteValues[i] = note + 23 + (sldOctave.getValue()+addOctaves)*12;
        *audioProcessor.chordValues[i] = cmbChords[i].getSelectedId();
        maxNote = note;
        if(i<((int)(*audioProcessor.numberOfZones)))
        {
            *audioProcessor.splitValues[i] = sldArSplitValues[i].getValue();
        }
        else
            *audioProcessor.splitValues[i] = 128;
    }
}

void RibbonToNotesAudioProcessorEditor::SyncSliderValues()
{
    auto zones = sldNumberOfZones.getValue();
    
    if(zones != lastNumberOfZones)
    {
        lastNumberOfZones=zones;
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
    for(int i=0 ; i < MAX_NOTES; i++)
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
    SyncComboBoxValues();
}

void RibbonToNotesAudioProcessorEditor::SetChordBuildFromChord(int i)
{
    edtChordBuilder[i].setText(chordbuilds[cmbChords[i].getSelectedId()], juce::sendNotification);
}
void RibbonToNotesAudioProcessorEditor::GetChordBuild(int i)
{
    
    juce::StringArray chordbuildarray;
    chordbuildarray.addTokens(edtChordBuilder[i].getText(),",","");
    for(int j = 0;j < MAX_NOTES ; j++)
    {
        if(j<chordbuildarray.size() && is_validnotenumber(chordbuildarray[j]))
        {
            *audioProcessor.chordBuilds[i][j] = chordbuildarray[j].getIntValue();
        }
        else
        {
            *audioProcessor.chordBuilds[i][j] = 0;
        }
    }
}
//check if it is a valid integer
bool RibbonToNotesAudioProcessorEditor::is_validnotenumber(const juce::String& str)
{
    auto isvalid = str.containsOnly("0123456789-");
    if(isvalid)
    {
        int notenumber = str.getIntValue();
        if(notenumber < -103 || notenumber > 103)
        {
            isvalid = false;
        }
    }
    return isvalid;
}

