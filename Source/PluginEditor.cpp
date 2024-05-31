/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
RibbonToNotesAudioProcessorEditor::RibbonToNotesAudioProcessorEditor ( RibbonToNotesAudioProcessor& p)
    : AudioProcessorEditor (&p)
    , audioProcessor (p)
    , presetPanel(p.getPresetManager())
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (700, 500);
    CreateGui();
    SyncSliderValues();
    AddListeners();
    startTimerHz(24);
}

RibbonToNotesAudioProcessorEditor::~RibbonToNotesAudioProcessorEditor()
{
    sldMidiCCAttachment = nullptr;
    sldNumberOfZonesAttachment = nullptr;
    sldVelocityAttachment = nullptr;
    sldOctaveAttachment = nullptr;
    cmbPitchModesAttachment = nullptr;
    for(int i=0;i<MAX_SPLITS;i++)
    {
        if(i<MAX_ZONES)
        {
            cmbKeysAttachment[i] = nullptr;
        }
        sldSplitValuesAttachment[i] = nullptr;
    }
    sldSplitExtraValuesAttachment = nullptr;
    
}

void RibbonToNotesAudioProcessorEditor::CreateGui()
{
    addAndMakeVisible(presetPanel);
    
    CreateDial(sldMidiCC);
    sldMidiCCAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,MIDICC_ID, sldMidiCC);


    lblMidiCC.setText("Midi CC", juce::dontSendNotification);
    lblMidiCC.attachToComponent(&sldMidiCC, false);
    lblMidiCC.setJustificationType(juce::Justification::centred);

    CreateDial(sldNumberOfZones);
    sldNumberOfZonesAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,NUMBEROFZONES_ID, sldNumberOfZones);
    lblNumberOfZones.setText("Zones", juce::dontSendNotification);
    lblNumberOfZones.attachToComponent(&sldNumberOfZones, false);
    lblNumberOfZones.setJustificationType(juce::Justification::centred);

    CreateDial(sldVelocity);
    sldVelocityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, VELOCITY_ID, sldVelocity);

    lblVelocity.setText("Velocity", juce::dontSendNotification);
    lblVelocity.attachToComponent(&sldVelocity, false);
    lblVelocity.setJustificationType(juce::Justification::centred);

    CreateDial(sldOctave);
    sldOctaveAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, OCTAVES_ID, sldOctave);

    lblOctave.setText("Octave", juce::dontSendNotification);
    lblOctave.attachToComponent(&sldOctave, false);
    lblOctave.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(cmbPitchModes);
    cmbPitchModes.addItemList(pitchModesArray, 1);
    cmbPitchModesAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, PITCHMODES_ID, cmbPitchModes);
    cmbPitchModes.setEnabled(true);

    //put the rectangles underneath the controls, by adding them first.
    for(int i=0;i<MAX_SPLITS+1;i++)
    {
        addAndMakeVisible(ribbonZoneVisuals[i]);
    }

    for(int i=0;i<MAX_SPLITS;i++)
    {
        bool enabled = true;
        if(i>=(int)(*audioProcessor.numberOfZones))
        {
            enabled=false;
        }
        if(i<MAX_ZONES)
        {
            //cmbNotes[i] = new juce::ComboBox;
            addAndMakeVisible(cmbKeys[i]);
            cmbKeys[i].addItemList(keysArray, 1);
            cmbKeysAttachment[i]= std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, KEYS_ID + std::to_string(i), cmbKeys[i]);
            cmbKeys[i].setEnabled(enabled);

            addAndMakeVisible(cmbChords[i]);
            cmbChords[i].addItemList(chordsArray, 1);
            cmbChordsAttachment[i]= std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, CHORDS_ID + std::to_string(i), cmbChords[i]);
            cmbChords[i].setEnabled(enabled);
            cmbChords[i].onChange = [this, i] {SetChordBuildFromChord(i);};


            addAndMakeVisible(edtChordBuilder[i]);
            edtChordBuilder[i].setEnabled(enabled);
            edtChordBuilder[i].setEditable(true);
            edtChordBuilder[i].setColour (juce::Label::outlineColourId, juce::Colours::grey);
            juce::String chordBuildStr;
            for(int j=0;j<MAX_NOTES;j++)
            {
                chordBuildStr = chordBuildStr + std::to_string((int)*audioProcessor.chordNotes[i][j]);
                if(j+1==MAX_NOTES || (int)*audioProcessor.chordNotes[i][j+1] == 0)
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
        sldSplitValuesAttachment[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, SPLITS_ID + std::to_string(i), sldArSplitValues[i]);
        addAndMakeVisible(lblArSplitValues[i]);
    }
    CreateSlider(sldArSplitExtra);
    sldSplitExtraValuesAttachment= std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, SPLITEXTRA_ID, sldArSplitExtra);
    sldArSplitExtra.setValue(*audioProcessor.splitExtra);
    addAndMakeVisible(lblArSplitExtra);
}

void RibbonToNotesAudioProcessorEditor::SetSplitRanges()
{
    int stepSize = 127/(sldNumberOfZones.getValue());
    bool enabled = true;
    for(int i=0;i<MAX_SPLITS;i++)
    {
        enabled = i < (sldNumberOfZones.getValue());
        if(i<MAX_ZONES)
        {
            cmbKeys[i].setEnabled(enabled);
            cmbChords[i].setEnabled(enabled);
            edtChordBuilder[i].setEnabled(enabled);
        }
        int value = enabled? 1 + i * stepSize:0;
        if(i>=sldNumberOfZones.getValue()) value = 128;
        sldArSplitValues[i].setRange(fmax(value + 1 - stepSize,0), fmin(value - 1 + stepSize,128), 1);
        sldArSplitValues[i].setValue(value);
        sldArSplitValues[i].setEnabled(enabled);
        if(i == (MAX_ZONES/2))
        {
            sldArSplitExtra.setRange(fmax(value + 1 - stepSize,0), fmin(value - 1 + stepSize,128), 1);
            sldArSplitExtra.setValue(value);
            sldArSplitExtra.setEnabled(enabled);
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
        if(i == (MAX_ZONES/2))
        {
            sldArSplitExtra.setRange(fmax(value + 1 - stepSize,0), fmin(value - 1 + stepSize,128), 1);
            sldArSplitExtra.setValue(value);
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
    cmbPitchModes.addListener(this);
    for(int i=0;i<MAX_SPLITS;i++)
    {
        if(i<MAX_ZONES)
        {
            sldArNoteNumber[i].addListener(this);
            cmbKeys[i].addListener(this);
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
    auto controlWidth = (getWidth() / (0.5 * MAX_ZONES + 0.5)) - 2 * leftMargin;
    int textHeight = (getHeight() - controlWidth - 4 * topMargin) * 0.05f;
    auto dialHeight = textHeight + controlWidth;
    auto vsliderHeight = 2*textHeight;

    int topPresetMenu =  getHeight() * 0.1f;
    int topGeneralControls = topPresetMenu + topMargin + textHeight;
    int topRowA = topGeneralControls + dialHeight + 2 * topMargin;
    int topRowB = topRowA + topMargin + textHeight;
    int topRowC = topRowB + topMargin + textHeight;
    int topRowSplitSliders = topRowC + textHeight;
    int topOffsetRow2 = (topRowSplitSliders-topRowA) + vsliderHeight + textHeight + topMargin;

    presetPanel.setBounds(getLocalBounds().removeFromTop(proportionOfHeight(0.1f)));


    sldMidiCC.setBounds(leftMargin, topGeneralControls, controlWidth, dialHeight);
    sldMidiCC.setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);
    sldNumberOfZones.setBounds(leftMargin + 1*(leftMargin + controlWidth), topGeneralControls, controlWidth, dialHeight);
    sldNumberOfZones.setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);
    sldVelocity.setBounds(leftMargin + 2*(leftMargin + controlWidth), topGeneralControls, controlWidth, dialHeight);
    sldVelocity.setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);
    sldOctave.setBounds(leftMargin + 3*(leftMargin + controlWidth), topGeneralControls, controlWidth, dialHeight);
    sldOctave.setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);
    cmbPitchModes.setBounds(leftMargin + 4*(leftMargin + controlWidth), topGeneralControls, controlWidth, textHeight);

    int row = 0;
    ribbonZoneVisuals[0].setBounds(leftMargin, topRowA - topMargin + row * topOffsetRow2, 5*textHeight, controlWidth + leftMargin);//0.5*(controlWidth-leftMargin)); //Somehow making it smaller gives problems with the rendering.

    for(int i=0;i<MAX_SPLITS;i++)
    {
        if(i== MAX_ZONES/2)
        {
            sldArSplitExtra.setBounds(leftMargin + (i-row*6) * (leftMargin + controlWidth), topRowSplitSliders + row * topOffsetRow2, controlWidth, vsliderHeight);
            sldArSplitExtra.setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);
            row++;
        }
        if(i<MAX_ZONES)
        {
            ribbonZoneVisuals[i+1].setBounds(0.5 * leftMargin + 0.5 * controlWidth + (i-row*6) * (leftMargin + controlWidth), topRowA - topMargin + row * topOffsetRow2, 5*textHeight, controlWidth + leftMargin);

            cmbKeys[i].setBounds(leftMargin + 0.5 * controlWidth + (i-row*6) * (leftMargin + controlWidth), topRowA + row * topOffsetRow2, controlWidth, textHeight);

            cmbChords[i].setBounds(leftMargin + 0.5 * controlWidth + (i-row*6) * (leftMargin + controlWidth), topRowB + row * topOffsetRow2, controlWidth, textHeight);

            edtChordBuilder[i].setBounds(leftMargin + 0.5 * controlWidth + (i-row*6) * (leftMargin + controlWidth), topRowC + row * topOffsetRow2, controlWidth, textHeight);
        }
        sldArSplitValues[i].setBounds(leftMargin + (i-row*6) * (leftMargin + controlWidth), topRowSplitSliders + row * topOffsetRow2, controlWidth, vsliderHeight);
        sldArSplitValues[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);
    }


}

void RibbonToNotesAudioProcessorEditor::SyncNotesAndSplits()
{
    audioProcessor.BuildChords();
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
    for(int i=0 ; i < MAX_ZONES; i++)
    {
        if(i<((int)(*audioProcessor.numberOfZones)))
        {
            *audioProcessor.splitValues[i] = sldArSplitValues[i].getValue();
        }
        else
        {
            *audioProcessor.splitValues[i] = 128;
        }
        if(i == (MAX_ZONES/2))
        {
            sldArSplitExtra.setValue(sldArSplitValues[i].getValue(), juce::dontSendNotification);
        }
    }
    SyncNotesAndSplits();
}

void RibbonToNotesAudioProcessorEditor::SyncComboBoxValues()
{
    *audioProcessor.pitchMode = cmbPitchModes.getSelectedId();

    //determine if the first key has been transposed to another key.
    int transpose = cmbKeys[0].getSelectedId() - noteOrder[0];

    for(int i=0 ; i < MAX_ZONES; i++)
    {
        int key = cmbKeys[i].getSelectedId();
        *audioProcessor.chordValues[i] = cmbChords[i].getSelectedId();

        //in case the first key has changed, transpose other keys accordingly
        if(transpose != 0 && i > 0)
        {
            key = ((key + 11 + transpose) % 12)+1;
            noteOrder[i] = key;
            cmbKeys[i].setSelectedId(key, juce::dontSendNotification);
        }
        noteOrder[i] = key;
        *audioProcessor.selectedKeys[i] = key;
    }
    SyncNotesAndSplits();
}

void RibbonToNotesAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if(slider == &sldArSplitExtra)
    {
        sldArSplitValues[MAX_ZONES/2].setValue(sldArSplitExtra.getValue());
    }
    SyncSliderValues();
}
void RibbonToNotesAudioProcessorEditor::comboBoxChanged(juce::ComboBox* combobox)
{
    SyncComboBoxValues();
}

void RibbonToNotesAudioProcessorEditor::SetChordBuildFromChord(int i)
{
    edtChordBuilder[i].setText(chordbuildsArray[cmbChords[i].getSelectedId()], juce::sendNotification);
}
void RibbonToNotesAudioProcessorEditor::GetChordBuild(int i)
{
    
    juce::StringArray chordbuildarray;
    chordbuildarray.addTokens(edtChordBuilder[i].getText(),",","");
    for(int j = 0;j < MAX_NOTES ; j++)
    {
        if(j<chordbuildarray.size() && is_validnotenumber(chordbuildarray[j]))
        {
            *audioProcessor.chordNotes[i][j] = chordbuildarray[j].getIntValue();
        }
        else
        {
            *audioProcessor.chordNotes[i][j] = 0;
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

void RibbonToNotesAudioProcessorEditor::ShowRibbonZone(int area)
{
    for(int i=0;i<MAX_SPLITS;i++)
    {
        ribbonZoneVisuals[i].FillColourOn = i == area;
        ribbonZoneVisuals[i].repaint();
    }
}

void RibbonToNotesAudioProcessorEditor::timerCallback()
{
    ShowRibbonZone(audioProcessor.getActiveZone());
}
