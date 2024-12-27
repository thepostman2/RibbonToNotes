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
, sldVelocity(p, VELOCITY_ID)
, ribbonZeroZone(0,0)
, prevProgression(audioProcessor, MAX_PROGRESSIONS)
, nextProgression(audioProcessor, MAX_PROGRESSIONS+1)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    CreateProgressionSelectorKnobs();
    CreateRibbon();
    CreateGui();
    setSize (800, 400);
    RedistributeSplitRanges(true);
    AddListeners();
    startTimerHz(12);
}

//==============================================================================
// destructor. This is needed to make sure the attachments are not pointing
// to any destructed control.
//==============================================================================
RibbonToNotesAudioProcessorEditor::~RibbonToNotesAudioProcessorEditor()
{
    RemoveListeners();
    // clean up attachments first to prevent errors when the object is deconstructed before the attachment is deconstructed.
    sldMidiCCAttachment = nullptr;
    sldNumberOfZonesAttachment = nullptr;
    sldVelocityAttachment = nullptr;
    sldOctaveAttachment = nullptr;
    cmbChannelInAttachment = nullptr;
    cmbChannelOutAttachment = nullptr;
    cmbPitchModesAttachment = nullptr;
    cmbActiveProgressionAttachment = nullptr;
    for(int i=0;i<MAX_SPLITS;i++)
    {
        if(i<MAX_ZONES)
        {
            cmbKeysAttachment[i] = nullptr;
        }
        sldSplitValuesAttachment[i] = nullptr;
    }
}

//==============================================================================
// initialize ribbon key zones
//==============================================================================
void RibbonToNotesAudioProcessorEditor::CreateRibbon()
{
    for(int alt = 0;alt < MAX_PROGRESSIONS;alt++)
    {
        for(int zone=0;zone<MAX_ZONES;zone++)
        {
            ribbonKeyZone[alt].add(new KeyZone(audioProcessor, alt, zone));
            ribbonKeyZone[alt][zone]->ribbonToNotesAudioProcessorEditor = this;
            ribbonKeyZone[alt][zone]->BuildChordsFuncP = BuildChordsWrapper;
        }
    }
}

// initialize alternative selection knobs
void RibbonToNotesAudioProcessorEditor::CreateProgressionSelectorKnobs()
{
    for(int alt = 0;alt < MAX_PROGRESSIONS;alt++)
    {
        selectProgressionKnobs.add(new SelectionKnob(audioProcessor, alt));
    }
}

//==============================================================================
// this function only makes the controls, but does not position them.
// for the position of the controls, see resized function
//==============================================================================
void RibbonToNotesAudioProcessorEditor::CreateGui()
{
    // presets
    addAndMakeVisible(presetPanel);
    
    // main dials
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

    addAndMakeVisible(toggleShowMidiLearnSettings);
    lblShowMidiLearnSettings.setText(TOGGLEMIDI_NAME, juce::dontSendNotification);
    lblShowMidiLearnSettings.attachToComponent(&toggleShowMidiLearnSettings, true);
    lblShowMidiLearnSettings.setJustificationType(juce::Justification::left);

    addAndMakeVisible(cmbChannelIn);
    cmbChannelIn.addItemList(channelInArray, 1);
    cmbChannelInAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, CHANNELIN_ID, cmbChannelIn);
    lblChannelIn.setText(CHANNELIN_NAME, juce::dontSendNotification);
    lblChannelIn.attachToComponent(&cmbChannelIn, true);
    lblChannelIn.setJustificationType(juce::Justification::left);

    addAndMakeVisible(cmbChannelOut);
    cmbChannelOut.addItemList(channelOutArray, 1);
    cmbChannelOutAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, CHANNELOUT_ID, cmbChannelOut);
    lblChannelOut.setText(CHANNELOUT_NAME, juce::dontSendNotification);
    lblChannelOut.attachToComponent(&cmbChannelOut, true);
    lblChannelOut.setJustificationType(juce::Justification::left);

    // pitchmode
    addAndMakeVisible(cmbPitchModes);
    cmbPitchModes.addItemList(pitchModesArray, 1);
    cmbPitchModesAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, PITCHMODES_ID, cmbPitchModes);
    cmbPitchModes.setEnabled(true);
    lblPitchModes.setText(PITCHMODES_NAME, juce::dontSendNotification);
    lblPitchModes.attachToComponent(&cmbPitchModes, true);
    lblPitchModes.setJustificationType(juce::Justification::left);

    // progressions
    addAndMakeVisible(cmbActiveProgression);
    cmbActiveProgression.addItemList(progressionArray, 1);
    cmbActiveProgressionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, ACTIVEPROGRESSION_ID, cmbActiveProgression);
    cmbActiveProgression.setEnabled(false);
    lblActiveProgression.setText(ACTIVEPROGRESSION_NAME, juce::dontSendNotification);
    lblActiveProgression.attachToComponent(&cmbActiveProgression, true);
    lblActiveProgression.setJustificationType(juce::Justification::left);
    cmbActiveProgression.setVisible(false);
    lblActiveProgression.setVisible(false);

    // ribbon key zones
    // the first zone only sents notes off, so it is just a ZoneVisual without controls
    addAndMakeVisible(ribbonZeroZone);
    
    // the other zones ar ribbon key Zones with their individual settings
    for(int alt=0;alt < MAX_PROGRESSIONS; alt++)
    {
        rootKey[alt] = (int) *audioProcessor.selectedKeys[alt][0];
        for(int zone=0;zone<MAX_ZONES;zone++)
        {
            addChildComponent(ribbonKeyZone[alt][zone]);// do not make it visible here.
        }
        addAndMakeVisible(selectProgressionKnobs[alt]);
        selectProgressionKnobs[alt]->ColourOn = juce::Colours::blueviolet;
    }
    addAndMakeVisible(prevProgression);
    prevProgression.ColourOn = juce::Colours::blueviolet;
    addAndMakeVisible(nextProgression);
    nextProgression.ColourOn = juce::Colours::blueviolet;

    for(int i=0;i<MAX_SPLITS;i++)
    {
        CreateSlider(sldSplitValues[i]);
        sldSplitValuesAttachment[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, SPLITS_ID + std::to_string(i), sldSplitValues[i]);
        addAndMakeVisible(lblSplitValues[i]);
    }
}

//==============================================================================
// This functions shows the proper amount of key zones based on the selected
// number of zones/.
//==============================================================================
void RibbonToNotesAudioProcessorEditor::RedistributeSplitRanges(bool initSplitValues)
{
    int zones = (int)(*audioProcessor.numberOfZones);
    int lastSplit = zones - 1;
    int valueMin = (int) *audioProcessor.splitValues[0];
    int valueMax = fmax((int) *audioProcessor.splitValues[lastSplit], valueMin + zones);
    int stepSize = (valueMax - valueMin)/(zones-1);

    if(initSplitValues)
    {
        stepSize = (128 - valueMin)/(zones);
        valueMax = valueMin + (zones - 1) * stepSize;
    }
    
    int value = valueMin;
    sldSplitValues[0].setRange(fmax(value - 0.25 * stepSize,0), fmin(value + 0.25 * stepSize,128), 1);
    splitValuesSetFromCode = true;
    sldSplitValues[0].setValue(valueMin, juce::sendNotificationSync);
    
    int i=1;
    int prevValue = 0;
    for(;i < MAX_ZONES;i++)
    {
        if(i < zones)
        {
            value = i == lastSplit ? valueMax : valueMin + i * stepSize;
        }
        else
        {
            value =  128;
        }
        *audioProcessor.splitValues[i] = value;
        int min = fmin(fmax(value - 0.25 * stepSize, prevValue), value - 1);
        int max = fmin(fmax(value + 0.25 * stepSize, value + 1),128);
        sldSplitValues[i].setRange(min, max, 1);
        splitValuesSetFromCode = true;
        sldSplitValues[i].setValue(value, juce::sendNotificationSync);
        prevValue = value;
    }
    splitValuesSetFromCode = false;
}

//==============================================================================
// Functions to create dials and sliders with the same style.
//==============================================================================
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

//==============================================================================
// Add all the listeners to the sliders and dials
//==============================================================================
void RibbonToNotesAudioProcessorEditor::AddListeners()
{
    // add the listener to the slider
    sldMidiCC.addListener(this);
    sldNumberOfZones.addListener (this);
    sldVelocity.addListener(this);
    sldOctave.addListener(this);

    toggleShowMidiLearnSettings.addListener(this);
    
    cmbChannelIn.addListener(this);
    cmbChannelOut.addListener(this);
    cmbPitchModes.addListener(this);
    cmbActiveProgression.addListener(this);
    
    ribbonZeroZone.addListener(this);
    prevProgression.addListener(this);
    nextProgression.addListener(this);

    for(int i=0;i<MAX_SPLITS;i++)
    {
        sldSplitValues[i].addListener(this);
    }
    for(int alt=0;alt<MAX_PROGRESSIONS;alt++)
    {
        selectProgressionKnobs[alt]->addListener(this);
        for(int zone=0;zone<MAX_ZONES;zone++)
        {
            ribbonKeyZone[alt][zone]->addListener(this);
        }
    }
    for(int alt = 0;alt < MAX_PROGRESSIONS;alt++)
    {
        int zone = 0;
        //for(int zone=0;zone<MAX_ZONES;zone++)
        {
            ribbonKeyZone[alt][zone]->cmbKey.addListener(this);
//            ribbonKeyZone[alt][zone]->cmbKey.onChange = [this, alt, zone] {TransposeKeyAndChordModes(alt, ribbonKeyZone[alt][zone]->Transposed());};
        }
    }
}
void RibbonToNotesAudioProcessorEditor::RemoveListeners()
{
    // add the listener to the slider
    sldMidiCC.removeListener(this);
    sldNumberOfZones.removeListener (this);
    sldVelocity.removeListener(this);
    sldOctave.removeListener(this);
    
    toggleShowMidiLearnSettings.removeListener(this);
    
    cmbChannelIn.removeListener(this);
    cmbChannelOut.removeListener(this);
    cmbPitchModes.removeListener(this);
    cmbActiveProgression.removeListener(this);
    
    ribbonZeroZone.removeListener(this);
    prevProgression.removeListener(this);
    nextProgression.removeListener(this);

    for(int i=0;i<MAX_SPLITS;i++)
    {
        sldSplitValues[i].removeListener(this);
    }
    for(int alt=0;alt<MAX_PROGRESSIONS;alt++)
    {
        selectProgressionKnobs[alt]->removeListener(this);
        for(int zone=0;zone<MAX_ZONES;zone++)
        {
            ribbonKeyZone[alt][zone]->removeListener(this);
        }
    }
    for(int alt = 0;alt < MAX_PROGRESSIONS;alt++)
    {
        for(int zone=0;zone<MAX_ZONES;zone++)
        {
            ribbonKeyZone[alt][zone]->removeListener(this);
            ribbonKeyZone[alt][zone]->cmbKey.onChange = nullptr;
        }
    }
}
//==============================================================================
// set the font size and color.
//==============================================================================
void RibbonToNotesAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
}

//==============================================================================
// Position and size all controls based on the size of the GUI.
//==============================================================================
void RibbonToNotesAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    int activezones = (int) *audioProcessor.numberOfZones;
    int activeAlternative = audioProcessor.getActiveProgression();

    auto topMargin = getHeight() * 0.0145;
    int textHeight = 3 * topMargin;
    auto sideMargin = topMargin;
    auto controlWidth = 4 * (textHeight + topMargin);// getWidth() / 8 - 2 * sideMargin;
    auto splitSldrWidth = controlWidth * 0.5;
    auto firstZoneWidth = 0.5 * splitSldrWidth + sideMargin;
    auto zoneWidth = (getWidth()-2 * sideMargin - firstZoneWidth) / (activezones);
    auto dialHeight =  textHeight + controlWidth;
    auto vsliderHeight = 2*textHeight;
    auto rightX = getWidth() - sideMargin;

    
    int topPresetMenu =  getHeight() * 0.1f;
    int topGeneralControls = topPresetMenu + topMargin + textHeight;
    int topZone = topGeneralControls + dialHeight + topMargin;//topRowA - topMargin + row * topOffsetRow2;
    int zoneX = 0;
    int zoneHeight = 3*textHeight + 4 * topMargin;//getHeight() * 0.25;//
    int topRowSplitSliders = topZone + zoneHeight;
    int topRowSelectionKnobs = topRowSplitSliders + 2*(textHeight + topMargin);
    int selectionKnobsSize = zoneWidth * activezones / 8 -sideMargin ;//controlWidth - sideMargin;

    presetPanel.setBounds(getLocalBounds().removeFromTop(proportionOfHeight(0.1f)));
    
    sldMidiCC.setBounds(sideMargin, topGeneralControls, controlWidth, dialHeight);
    sldMidiCC.setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);
    sldNumberOfZones.setBounds(sideMargin + 1*(sideMargin + controlWidth), topGeneralControls, controlWidth, dialHeight);
    sldNumberOfZones.setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);
    sldVelocity.setBounds(sideMargin + 2*(sideMargin + controlWidth), topGeneralControls, controlWidth, dialHeight);
    sldVelocity.setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);
    sldOctave.setBounds(sideMargin + 3*(sideMargin + controlWidth), topGeneralControls, controlWidth, dialHeight);
    sldOctave.setTextBoxStyle(juce::Slider::TextBoxBelow, false, controlWidth, textHeight);
    
    toggleShowMidiLearnSettings.setBounds(rightX - controlWidth, topGeneralControls , controlWidth,  textHeight);
    
    cmbChannelIn.setBounds(rightX - controlWidth, topGeneralControls + 1* (textHeight + topMargin), controlWidth,  textHeight);
    cmbChannelOut.setBounds(rightX - controlWidth, topGeneralControls + 2* (textHeight + topMargin), controlWidth,  textHeight);
    cmbPitchModes.setBounds(rightX - controlWidth, topGeneralControls + 3* (textHeight + topMargin), controlWidth, textHeight);
    cmbActiveProgression.setBounds(rightX - controlWidth, topGeneralControls + 4* (textHeight + topMargin), controlWidth, textHeight);

    ribbonZeroZone.setBounds(zoneX + sideMargin, topZone, firstZoneWidth, zoneHeight);//0.5*zoneWidth); //Somehow making it smaller gives problems with the rendering.
    
    for(int prog=0;prog<MAX_PROGRESSIONS;prog++)
    {
        int splitCtrlX = 0;
        zoneX = firstZoneWidth + sideMargin;

        if(prog < selectProgressionKnobs.size())
        {
            if(prog == 0)
            {
                prevProgression.setBounds(zoneX + prog * (selectionKnobsSize + sideMargin), topRowSelectionKnobs, selectionKnobsSize, selectionKnobsSize );
                nextProgression.setBounds(zoneX + (selectProgressionKnobs.size()+1) * (selectionKnobsSize + sideMargin), topRowSelectionKnobs, selectionKnobsSize, selectionKnobsSize );
            }
            selectProgressionKnobs[prog]->setBounds(zoneX + (prog+1) * (selectionKnobsSize + sideMargin), topRowSelectionKnobs, selectionKnobsSize, selectionKnobsSize );
        }

        for(int zone=0;zone<MAX_ZONES;zone++)
        {
            ribbonKeyZone[prog][zone]->setBounds(zoneX, topZone, zoneWidth, zoneHeight);
            ribbonKeyZone[prog][zone]->setVisible(prog == activeAlternative && zone < activezones);
            ribbonKeyZone[prog][zone]->setEnabled(prog == activeAlternative && zone < activezones);
            if(zone<activezones)
            {
                if(prog == 0) //sliders have to be set only once. There are no alternatives
                {
                    sldSplitValues[zone].setVisible(true);
                    sldSplitValues[zone].setEnabled(true);
                    sldSplitValues[zone].setBounds(splitCtrlX + sideMargin, topRowSplitSliders, splitSldrWidth, vsliderHeight);
                    sldSplitValues[zone].setTextBoxStyle(juce::Slider::TextBoxBelow, false, splitSldrWidth, textHeight);
                }
                zoneX += zoneWidth;
                splitCtrlX += zoneWidth;
            }
            else
            {
                if(prog == 0)
                {
                    sldSplitValues[zone].setVisible(false);
                }
            }
        }
    }
}



//==============================================================================
// gets the values of the sliders and makes necessary changes accordingly
//==============================================================================
void RibbonToNotesAudioProcessorEditor::SyncZoneSliderValues()
{
    // get the split value settings
    for(int i=0 ; i < MAX_ZONES; i++)
    {
        *audioProcessor.splitValues[i] = sldSplitValues[i].getValue();
    }
}

//==============================================================================
// Sync the selected settings for the key and chord modes
//==============================================================================
void RibbonToNotesAudioProcessorEditor::SyncKeyAndChordModes(int progression, int zone)
{
    //determine if the first key has been transposed to another key.
    
    if(zone == 0)
    {
        int key = ribbonKeyZone[progression][0]->cmbKey.getSelectedId();
        int transpose = key - rootKey[progression];
        rootKey[progression] = key;
        
        // get the selected key and chord mode for each zone
        for(int z=0 ; z < MAX_ZONES; z++)
        {
            // get the selected key
            key = ribbonKeyZone[progression][z]->cmbKey.getSelectedId();
            
            // get the selected chord mode
            *audioProcessor.selectedChord[progression][z] = ribbonKeyZone[progression][z]->cmbChord.getSelectedId();
            
            //in case the first key has changed, transpose the key accordingly
            if(transpose != 0 && z > 0)
            {
                key = ((key + 11 + transpose) % 12)+1;
                ribbonKeyZone[progression][z]->cmbKey.setSelectedId(key, juce::dontSendNotification);
            }
            
            // set the selected key
            *audioProcessor.selectedKeys[progression][z] = key;
        }
    }
    else
    {
        // get the selected chord mode
        *audioProcessor.selectedChord[progression][zone] = ribbonKeyZone[progression][zone]->cmbChord.getSelectedId();
        // set the selected key
        *audioProcessor.selectedKeys[progression][zone] = ribbonKeyZone[progression][zone]->cmbKey.getSelectedId();
    }
    
    // rebuild the chord for each zone based on key en chord mode setting
    BuildChords(progression);
}
void RibbonToNotesAudioProcessorEditor::TransposeKeyAndChordModes(int progression, int transpose)
{
    //determine if the first key has been transposed to another key.
    
        int key = ribbonKeyZone[progression][0]->cmbKey.getSelectedId();
        rootKey[progression] = key;
        
        // get the selected key and chord mode for each zone
        for(int z=1 ; z < MAX_ZONES; z++)
        {
            // get the selected key
            key = ribbonKeyZone[progression][z]->cmbKey.getSelectedId();
            
            // get the selected chord mode
            *audioProcessor.selectedChord[progression][z] = ribbonKeyZone[progression][z]->cmbChord.getSelectedId();
            
            //in case the first key has changed, transpose the key accordingly
            if(transpose != 0 && z > 0)
            {
                key = ((key + 11 + transpose) % 12)+1;
                ribbonKeyZone[progression][z]->cmbKey.setSelectedId(key, juce::dontSendNotification);
            }
            
            // set the selected key
            *audioProcessor.selectedKeys[progression][z] = key;
        }
    
    // rebuild the chord for each zone based on key en chord mode setting
    BuildChords(progression);
}
//==============================================================================
// Listeners for the controls
//==============================================================================
void RibbonToNotesAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    auto zones = (int) sldNumberOfZones.getValue();
    int lastSplit = zones - 1;

    if(splitValuesSetFromCode == true)
    {
        splitValuesSetFromCode = false;
        return;
    }
    if(slider == &sldOctave)
    {
        *audioProcessor.octaves = sldOctave.getValue();
        BuildChordsForAllProgressions();
        return;
    }
        // check if number of zones has changed. If so, update the GUI.
    else if(zones != lastNumberOfZones)
    {
        *audioProcessor.numberOfZones = zones; //necessary if preset is loaded
        lastNumberOfZones=zones;
        RedistributeSplitRanges(true);
        resized();
    }
    else if(slider == &sldSplitValues[0] || slider == &sldSplitValues[zones-1])
    {
        *audioProcessor.splitValues[0] = (int) sldSplitValues[0].getValue();
        *audioProcessor.splitValues[lastSplit] = (int) sldSplitValues[lastSplit].getValue();
        *audioProcessor.splitValues[zones] = 128;
        RedistributeSplitRanges(false);
    }
    SyncZoneSliderValues();
}
//==============================================================================
void RibbonToNotesAudioProcessorEditor::comboBoxChanged(juce::ComboBox* combobox)
{
    if(combobox == &cmbPitchModes)
    {
        BuildChordsForAllProgressions();
        return;
    }
    if(combobox == &cmbActiveProgression)
    {
        ShowActiveAlternative();
        return;
    }
    int transpose = 0;
    for(int prog=0;prog < MAX_PROGRESSIONS;prog++)
    {
        if(transpose != 0)
        {
            //apparently first progression has been transposed, so transpose other progressions as well
            auto key = ribbonKeyZone[prog][0]->cmbKey.getSelectedId();
            key = ((key + 11 + transpose) % 12)+1;
            ribbonKeyZone[prog][0]->cmbKey.setSelectedId(key, juce::sendNotification);//this will transpose the all progression.
            if(prog == MAX_PROGRESSIONS - 1)return;
        }
        else if(combobox == &ribbonKeyZone[prog][0]->cmbKey)
        {
            TransposeKeyAndChordModes(prog, ribbonKeyZone[prog][0]->Transposed());
            if(prog == 0)
            {
                //if first progression is transposed, then transpose other progressions as well.
                transpose = ribbonKeyZone[prog][0]->Transposed();
            }
            else
            {
                return;
            }
        }
    }
}
//==============================================================================
void RibbonToNotesAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if(button == &ribbonZeroZone)
    {
        audioProcessor.AddNotesToPlayToBuffer(sldSplitValues[0].getValue());
        return;
    }
    if(button == &prevProgression)
    {
        audioProcessor.activeProgressionKnob = MAX_PROGRESSIONS;
        return;
    }
    if(button == &nextProgression)
    {
        audioProcessor.activeProgressionKnob = MAX_PROGRESSIONS+1;
        return;
    }
    if(button == &toggleShowMidiLearnSettings)
    {
        MidiLearnInterface::MidiSettingOn = button->getToggleState();
    }

    //check if one of the progression selection knobs is pressed
    if(auto castProgressionKnob = dynamic_cast<SelectionKnob*>(button))
    {
        auto index = selectProgressionKnobs.indexOf(castProgressionKnob);
        if(index > -1)
        {
            audioProcessor.activeProgressionKnob = index;
            return;
        }
    }

    //check if one of the ribbonzone knobs is pressed
    for(int i=0;i<MAX_ZONES;i++)
    {
        if(auto castKeyZone = dynamic_cast<KeyZone*>(button))
        {
            auto index = ribbonKeyZone[i].indexOf(castKeyZone);
            if(index > -1)
            {
                auto ccval = (*audioProcessor.splitValues[index] + *audioProcessor.splitValues[index+1])/2;
                audioProcessor.AddNotesToPlayToBuffer(ccval);
                return;
            }
        }
    }
    
}

//==============================================================================
// build notes to play
//==============================================================================
void RibbonToNotesAudioProcessorEditor::BuildChordsForAllProgressions()
{
    for(int prog=0;prog<MAX_PROGRESSIONS;prog++)
    {
        BuildChords(prog);
    }
}

void RibbonToNotesAudioProcessorEditor::BuildChords(int progression)
{
    int maxNote = 0;
    int key = 0;
    int octave = (int) *audioProcessor.octaves;
    int addOctaves=0;
    
    for(int zone=0 ; zone < MAX_ZONES; zone++)
    {
        key = (int) *audioProcessor.selectedKeys[progression][zone];
        
        //pitchMode 0 = up
        if(*audioProcessor.pitchMode == 0)
        {
            //if the notevalue is lower then the highest note, just add an octave to it.
            if(key <= maxNote && (key + ( octave + addOctaves + 1) * 12) < 128)
            {
                addOctaves++;
            }
            maxNote = key;
        }
        GetNoteNumbersForChord(octave + addOctaves, progression, zone, key);
    }
}
// calculate the notes to be played for a specific zone
// since key equals to one instead of zero, the counting is a bit strange
void RibbonToNotesAudioProcessorEditor::GetNoteNumbersForChord(int addOctaves, int alternative, int zone, int key)
{
    for(int j=0;j<MAX_NOTES;j++)
    {
        int note = (int) *audioProcessor.chordNotes[alternative][zone][j];
        if(note != 0)
        {
            if(note > 0) note = note - 1; //offset for positive notes is +1. Correct it here.
            int keynote = key + 24 - 1; //Since addoctaves starts at -2, offset for key is -24. Also there is an offset of +1, because C corresponds to 1 in the list instead of 0. Both are corrected here.
            if(keynote + note > 8 && addOctaves > 7) addOctaves = 7; //do not go past G8
            note = keynote + note + addOctaves * 12;
        }
        ribbonKeyZone[alternative][zone]->SetNoteParameter(j, note);//this is for saving it to the valuetreestate
        *audioProcessor.notesToPlay[alternative][zone][j]=note;
    }
}
//==============================================================================
// Update functions for the visuals
//==============================================================================
void RibbonToNotesAudioProcessorEditor::ShowActiveAlternative()
{
    int activeProgression = audioProcessor.getActiveProgression();
    int activeProgressionKnob = audioProcessor.activeProgressionKnob;
    int activezones = (int) *audioProcessor.numberOfZones;

    prevProgression.FillColourOn = activeProgressionKnob == MAX_PROGRESSIONS;
    prevProgression.repaint();
    nextProgression.FillColourOn = activeProgressionKnob == MAX_PROGRESSIONS+1;
    nextProgression.repaint();
    for(int alt=0;alt<MAX_PROGRESSIONS;alt++)
    {
        for(int zone=0;zone<MAX_ZONES;zone++)
        {
            ribbonKeyZone[alt][zone]->setVisible(alt == activeProgressionKnob && zone < activezones);
            ribbonKeyZone[alt][zone]->setEnabled(alt == activeProgressionKnob && zone < activezones);
        }
        selectProgressionKnobs[alt]->FillColourOn = alt == activeProgressionKnob;
        selectProgressionKnobs[alt]->repaint();
    }
    
    if(activeProgression == activeProgressionKnob) return; //progression selection wasn't changed

    if(activeProgressionKnob >= MAX_PROGRESSIONS)
    {
        //previous or next progression button is clicked.
        auto moveProgression = activeProgressionKnob == MAX_PROGRESSIONS? -1 : 1;

        activeProgression = cmbActiveProgression.getSelectedId() - 1  + moveProgression;
        if(activeProgression >= MAX_PROGRESSIONS)
        {
            activeProgression = 0;
        }
        else if (activeProgression < 0)
        {
            activeProgression = MAX_PROGRESSIONS - 1;
        }
        cmbActiveProgression.setSelectedId(activeProgression + 1, juce::dontSendNotification);
    }
    else
    {
        // progression selection button is clicked
        cmbActiveProgression.setSelectedId(activeProgressionKnob + 1, juce::dontSendNotification);
        activeProgression = activeProgressionKnob;
    }
    audioProcessor.activeProgressionKnob = activeProgression;
    *audioProcessor.activeProgression = activeProgression;
}

// Colors the selected zone when a user presses the ribbon
void RibbonToNotesAudioProcessorEditor::ShowRibbonZone(int area)
{
    if(area == 0)
    {
        ribbonZeroZone.FillColourOn = true;
    }
    else
    {
        ribbonZeroZone.FillColourOn = false;
    }
    ribbonZeroZone.repaint();
    int activeAlternative = audioProcessor.getActiveProgression();
    for(int zone=0;zone<MAX_ZONES;zone++)
    {
        ribbonKeyZone[activeAlternative][zone]->FillColourOn = zone == (area-1);
        ribbonKeyZone[activeAlternative][zone]->repaint();
    }
}

void RibbonToNotesAudioProcessorEditor::UpdateMidiLearnControls()
{
    sldVelocity.setValue(*audioProcessor.noteVelocity, juce::dontSendNotification);
    sldVelocity.repaint();
}
//==============================================================================
// Timer call back function. In this case only used to update the GUI when
// the user has pressed somewhere on the ribbon.
//==============================================================================
void RibbonToNotesAudioProcessorEditor::timerCallback()
{
    UpdateMidiLearnControls();
    ShowRibbonZone(audioProcessor.getActiveZone());
    ShowActiveAlternative();
}
