/*
 ==============================================================================
 
 KeyZone.cpp
 Created: 7 Jun 2024 12:55:07pm
 Author:  PJP
 
 ==============================================================================
 */

#include "KeyZone.h"


KeyZone::KeyZone ( RibbonToNotesAudioProcessor& p, int progressionid, int zoneid)
: ZoneVisual(progressionid, zoneid),
audioProcessor (p)
{
    CreateGui();
    AddListeners();
    currentKey = cmbKey.getSelectedId();
    prevKey = currentKey;
}

KeyZone::~KeyZone()
{
    RemoveListeners();
    cmbKeysAttachment = nullptr;
    for(int j=0;j<MAX_NOTES;j++)
    {
        sldChordNotesHelpAttachment[j]=nullptr;
    }
}

void KeyZone::CreateGui()
{
    addAndMakeVisible(cmbKey);
    cmbKey.addItemList(keysArray, 1);
    cmbKeysAttachment= std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, KEYS_ID + std::to_string(PROGRESSION_ID) + "_" + std::to_string(ZONE_ID), cmbKey);
    cmbKey.setEnabled(true);
    
    addAndMakeVisible(cmbChord);
    cmbChord.addItemList(chordsArray, 1);
    cmbChordsAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, CHORDS_ID + std::to_string(PROGRESSION_ID) + "_" + std::to_string(ZONE_ID), cmbChord);
    cmbChord.setEnabled(true);
    
    
    addAndMakeVisible(edtChordBuilder);
    edtChordBuilder.setEnabled(true);
    edtChordBuilder.setEditable(true);
    edtChordBuilder.setColour (juce::Label::outlineColourId, juce::Colours::grey);
    juce::String chordBuildStr;
    for(int j=0;j<MAX_NOTES;j++)
    {
        chordBuildStr = chordBuildStr + std::to_string((int)*audioProcessor.chordNotes[PROGRESSION_ID][ZONE_ID][j]);
        if(j+1==MAX_NOTES || (int)*audioProcessor.chordNotes[PROGRESSION_ID][ZONE_ID][j+1] == 0)
        {
            break;
        }
        else
        {
            chordBuildStr = chordBuildStr + ",";
        }
    }
    edtChordBuilder.setText(chordBuildStr, juce::dontSendNotification);
    edtChordChanged=false;
    
    for(int j=0;j<MAX_NOTES;j++)
    {
        addAndMakeVisible(sldChordNotesHelp[j]);
        sldChordNotesHelp[j].setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
        sldChordNotesHelpAttachment[j] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, CHORDBUILDS_ID + std::to_string(PROGRESSION_ID) + "_" + std::to_string(ZONE_ID) + "_" + std::to_string(j), sldChordNotesHelp[j]);
        sldChordNotesHelp[j].setValue(*audioProcessor.chordNotes[PROGRESSION_ID][ZONE_ID][j], juce::sendNotificationSync);
    }
}

void KeyZone::AddListeners()
{
    cmbKey.addListener(this);
    cmbChord.addListener(this);
    for(int j=0;j<MAX_NOTES;j++)
    {
        sldChordNotesHelp[j].addListener(this);
    }
    edtChordBuilder.onTextChange = [this] {EdtChordBuilderOnChange();};
    //cmbChord.onChange = [this] {cmbChordBuilderOnChange();};
}
void KeyZone::RemoveListeners()
{
    cmbKey.removeListener(this);
    cmbChord.removeListener(this);
    for(int j=0;j<MAX_NOTES;j++)
    {
        sldChordNotesHelp[j].removeListener(this);
    }
    edtChordBuilder.onTextChange = nullptr;
    //cmbChord.onChange = nullptr;
}
void KeyZone::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto height = getHeight();
    auto width = getWidth();
    auto topMargin = height * 0.08;
    auto sideMargin = topMargin;
    auto controlWidth = width - 2 * sideMargin ;
    int textHeight = 3 * topMargin;
    
    int topRowA = topMargin;
    int topRowB = topRowA + topMargin + textHeight;
    int topRowC = topRowB + topMargin + textHeight;
    
    cmbKey.setEnabled(true);
    cmbChord.setEnabled(true);
    edtChordBuilder.setEnabled(true);
    
    cmbKey.setBounds(sideMargin, topRowA, controlWidth, textHeight);
    
    cmbChord.setBounds(sideMargin, topRowB, controlWidth, textHeight);
    
    edtChordBuilder.setBounds(sideMargin, topRowC, controlWidth, textHeight);
}

void KeyZone::setVisible(bool visible)
{
    ZoneVisual::setVisible(visible);
    cmbKey.setVisible(visible);
    cmbChord.setVisible(visible);
    edtChordBuilder.setVisible(visible);
}

void KeyZone::cmbChordBuilderOnChange()
{
    int selectedChord = cmbChord.getSelectedId();

    if(selectedChord < chordbuildsArray.size())
    {
        edtChordBuilder.setText(chordbuildsArray[selectedChord], juce::dontSendNotification);
        GetChordFromChordString();
    }
    else
    {
        //check if the change came from a user edit
        if(edtChordChanged==false)
        {
            juce::String chord;
            juce::String sep = "";
            for(int j=0;j<MAX_NOTES;j++)
            {
                int notenr = sldChordNotesHelp[j].getValue();
                *audioProcessor.chordNotes[PROGRESSION_ID][ZONE_ID][j] = notenr;
                if(notenr != 0)
                {
                    chord = chord + sep + std::to_string(notenr);
                    sep = ",";
                }
            }
            edtChordBuilder.setText(chord,juce::dontSendNotification);
        }
        //since the change came from a user edit, do nothing, except for setting user edit back to false
        else
        {
            edtChordChanged=false;
        }
    }
    audioProcessor.BuildChords(PROGRESSION_ID);
}

void KeyZone::EdtChordBuilderOnChange()
{
    edtChordChanged = true;
    *audioProcessor.selectedChord[PROGRESSION_ID][ZONE_ID] = chordbuildsArray.size();//set selectedChord to "Custom"
    GetChordFromChordString();
    audioProcessor.BuildChords(PROGRESSION_ID);
    cmbChord.setSelectedId(chordbuildsArray.size(), juce::sendNotificationSync);//set cmbChord to "Custom"
}

void KeyZone::GetChordFromChordString()
{
    juce::StringArray chordStringArray;
    chordStringArray.addTokens(edtChordBuilder.getText(),",","");
    for(int j = 0;j < MAX_NOTES ; j++)
    {
        if(j<chordStringArray.size() && is_validnotenumber(chordStringArray[j]))
        {
            auto value =chordStringArray[j].getIntValue();
            *audioProcessor.chordNotes[PROGRESSION_ID][ZONE_ID][j] =value;
            SetChordParameter(j,value);
        }
        else
        {
            *audioProcessor.chordNotes[PROGRESSION_ID][ZONE_ID][j] = 0;
            SetChordParameter(j,0);
        }
    }
}

void KeyZone::SetChordParameter(int j, float value)
{
    sldChordNotesHelp[j].setValue(value, juce::sendNotificationSync);
}

bool KeyZone::is_validnotenumber(const juce::String& str)
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

int KeyZone::Transposed()
{
    return currentKey-prevKey;
}
//==============================================================================
// listeners
//==============================================================================
void KeyZone::sliderValueChanged(juce::Slider* slider)
{
}

void KeyZone::comboBoxChanged(juce::ComboBox* combobox)
{
    if(combobox == &cmbKey)
    {
        prevKey = currentKey;
        currentKey = cmbKey.getSelectedId();
        audioProcessor.BuildChords(PROGRESSION_ID);
        return;
    }
    if(combobox == &cmbChord)
    {
        cmbChordBuilderOnChange();
        return;
    }
}
