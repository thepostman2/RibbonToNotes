/*
 ==============================================================================
 
 KeyZone.cpp
 Created: 7 Jun 2024 12:55:07pm
 Author:  Peter
 
 ==============================================================================
 */

#include "KeyZone.h"


KeyZone::KeyZone ( RibbonToNotesAudioProcessor& p, int zoneid)
: ZoneVisual(zoneid),
audioProcessor (p)
{
    CreateGui();
    AddListeners();
}

KeyZone::~KeyZone()
{
    cmbKeysAttachment = nullptr;
    //sldSplitValuesAttachment = nullptr;
    for(int j=0;j<MAX_NOTES;j++)
    {
        sldChordNotesHelpAttachment[j]=nullptr;
    }
}

void KeyZone::CreateGui()
{
    addAndMakeVisible(cmbKey);
    cmbKey.addItemList(keysArray, 1);
    cmbKeysAttachment= std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, KEYS_ID + std::to_string(ID), cmbKey);
    cmbKey.setEnabled(true);
    
    addAndMakeVisible(cmbChord);
    cmbChord.addItemList(chordsArray, 1);
    cmbChordsAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, CHORDS_ID + std::to_string(ID), cmbChord);
    cmbChord.setEnabled(true);
    cmbChord.onChange = [this] {cmbChordBuilderOnChange();};
    
    
    addAndMakeVisible(edtChordBuilder);
    edtChordBuilder.setEnabled(true);
    edtChordBuilder.setEditable(true);
    edtChordBuilder.setColour (juce::Label::outlineColourId, juce::Colours::grey);
    juce::String chordBuildStr;
    for(int j=0;j<MAX_NOTES;j++)
    {
        chordBuildStr = chordBuildStr + std::to_string((int)*audioProcessor.chordNotes[ID][j]);
        if(j+1==MAX_NOTES || (int)*audioProcessor.chordNotes[ID][j+1] == 0)
        {
            break;
        }
        else
        {
            chordBuildStr = chordBuildStr + ",";
        }
    }
    edtChordBuilder.setText(chordBuildStr, juce::dontSendNotification);
    edtChordBuilder.onTextChange = [this] {EdtChordBuilderOnChange();};
    edtChordChanged=false;
    
//    addAndMakeVisible(sldArSplitValue);
//    sldArSplitValue.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
//    sldSplitValuesAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, SPLITS_ID + std::to_string(ID), sldArSplitValue);
//    addAndMakeVisible(lblArSplitValue);
    
    
    for(int j=0;j<MAX_NOTES;j++)
    {
        addAndMakeVisible(sldChordNotesHelp[j]);
        sldChordNotesHelp[j].setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
        sldChordNotesHelpAttachment[j] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, CHORDBUILDS_ID + std::to_string(ID)+ "_" + std::to_string(j), sldChordNotesHelp[j]);
        sldChordNotesHelp[j].setValue(*audioProcessor.chordNotes[ID][j], juce::sendNotificationSync);
    }
}

void KeyZone::AddListeners()
{
    // add the listener to the slider
    sldArNoteNumber.addListener(this);
    cmbKey.addListener(this);
    cmbChord.addListener(this);
    for(int j=0;j<MAX_NOTES;j++)
    {
        sldChordNotesHelp[j].addListener(this);
    }
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
    auto splitSldrWidth = controlWidth * 0.5;
    int textHeight = 3 * topMargin;
    auto vsliderHeight = 2 * textHeight;
    
    int topRowA = topMargin;
    int topRowB = topRowA + topMargin + textHeight;
    int topRowC = topRowB + topMargin + textHeight;
    int topRowSplitSliders = topRowC + topMargin + textHeight;
    
    cmbKey.setVisible(true);
    cmbChord.setVisible(true);
    edtChordBuilder.setVisible(true);
//    sldArSplitValue.setVisible(true);
    
    cmbKey.setEnabled(true);
    cmbChord.setEnabled(true);
    edtChordBuilder.setEnabled(true);
//    sldArSplitValue.setEnabled(true);
    
    cmbKey.setBounds(sideMargin, topRowA, controlWidth, textHeight);
    
    cmbChord.setBounds(sideMargin, topRowB, controlWidth, textHeight);
    
    edtChordBuilder.setBounds(sideMargin, topRowC, controlWidth, textHeight);
//    sldArSplitValue.setBounds(0, topRowSplitSliders, splitSldrWidth, vsliderHeight);
//    sldArSplitValue.setTextBoxStyle(juce::Slider::TextBoxBelow, false, splitSldrWidth, textHeight);
}

void KeyZone::cmbKeyOnChange()
{
    ChordBuild();
}

void KeyZone::cmbChordBuilderOnChange()
{
    int selectedChord = cmbChord.getSelectedId();
    if(selectedChord < chordbuildsArray.size())
    {
        edtChordBuilder.setText(chordbuildsArray[selectedChord], juce::dontSendNotification);
        ChordBuild();
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
                *audioProcessor.chordNotes[ID][j] = notenr;
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
}

void KeyZone::EdtChordBuilderOnChange()
{
    edtChordChanged = true;
    *audioProcessor.selectedChord[ID] = chordbuildsArray.size();//set selectedChord to "Custom"
    ChordBuild();
    cmbChord.setSelectedId(chordbuildsArray.size(), juce::sendNotificationSync);//set cmbChord to "Custom"
}

void KeyZone::ChordBuild()
{
    juce::StringArray chordStringArray;
    chordStringArray.addTokens(edtChordBuilder.getText(),",","");
    for(int j = 0;j < MAX_NOTES ; j++)
    {
        if(j<chordStringArray.size() && is_validnotenumber(chordStringArray[j]))
        {
            auto value =chordStringArray[j].getIntValue();
            *audioProcessor.chordNotes[ID][j] =value;
            setChordParameter(j,value);
        }
        else
        {
            *audioProcessor.chordNotes[ID][j] = 0;
            setChordParameter(j,0);
        }
    }
}

void KeyZone::setChordParameter(int j, float value)
{
    //    juce::AudioProcessorParameterWithID* pParam = audioProcessor.apvts.getParameter(CHORDBUILDS_ID + std::to_string(key) + "_" + std::to_string(j));
    //    pParam->beginChangeGesture();
    //    pParam->setValue(value);
    //    pParam->endChangeGesture();
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

void KeyZone::sliderValueChanged(juce::Slider* slider)
{
    ;
//    *audioProcessor.splitValues[ID] = sldArSplitValue.getValue();
}

void KeyZone::comboBoxChanged(juce::ComboBox* combobox)
{
    *audioProcessor.selectedKeys[ID] = cmbKey.getSelectedId();;
    *audioProcessor.selectedChord[ID] = cmbChord.getSelectedId();
}
