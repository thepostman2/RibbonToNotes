/*
 ==============================================================================
 
 PresetManager.cpp
 Created: 10 May 2024 10:56:15pm
 Author:  Peter
 
 ==============================================================================
 */

#include "PresetManager.h"

namespace Service
{
const File PresetManager::defaultDirectory
{
    File::getSpecialLocation(File::SpecialLocationType::commonDocumentsDirectory).getChildFile(ProjectInfo::companyName)
        .getChildFile(ProjectInfo::projectName)
};
const String PresetManager::extension{"preset"};
const String PresetManager::presetNameProperty{"presetName"};

PresetManager::PresetManager(AudioProcessorValueTreeState& apvts) : valueTreeState(apvts)
{
    if(!defaultDirectory.exists())
    {
        const auto result = defaultDirectory.createDirectory();
        if(result.failed())
        {
            DBG("Could not create preset directory:" + result.getErrorMessage());
            jassertfalse;
        }
    }
    //make sure that the currentPreset reference is updated via the listener when the valueTree changes.
    valueTreeState.state.addListener(this);
    //let currentPreset Value object point to the samen name as the presetNameProperty of the valueTree.
    currentPreset.referTo(valueTreeState.state.getPropertyAsValue(presetNameProperty, nullptr));
}

void PresetManager::savePreset(const String& presetName)
{
    if(presetName.isEmpty())
        return;
    //set the name first, because this is needed to write the name to the file
    currentPreset.setValue(presetName);
    const auto xml = valueTreeState.copyState().createXml();
    const auto presetFile = defaultDirectory.getChildFile(presetName + "." + extension);
    if(!xml->writeTo(presetFile))
    {
        DBG("Could not create preset file:" + presetFile.getFullPathName());
        jassertfalse;
    }
}
void PresetManager::deletePreset(const juce::String& presetName)
{
    if(presetName.isEmpty())
        return;
    
    const auto presetFile = defaultDirectory.getChildFile(presetName + "." + extension);
    if(presetFile.deleteFile()== false)
    {
        DBG("Preset file " + presetFile.getFullPathName() + " could not be deleted");
        jassertfalse;
        return;
    }
    currentPreset.setValue("");
}
void PresetManager::loadPreset(const juce::String& presetName)
{
    if(presetName.isEmpty())
        return;
    
    const auto presetFile = defaultDirectory.getChildFile(presetName + "." + extension);
    if(presetFile.existsAsFile() == false)
    {
        DBG("Preset file " + presetFile.getFullPathName() + " does not exist");
        jassertfalse;
        return;
    }
    //convert presetfile (XML) => (ValueTree)
    XmlDocument xmlDocument(presetFile);
    const auto valueTreeToLoad = ValueTree::fromXml(*xmlDocument.getDocumentElement());
    valueTreeState.replaceState(valueTreeToLoad);

    for(int i = 0 ; i<valueTreeToLoad.getNumChildren();++i)
    {
        const auto parameterChildToLoad = valueTreeToLoad.getChild(i);
        const auto parameterId = parameterChildToLoad.getProperty("id");
        auto parameterTree = valueTreeState.state.getChildWithProperty("id", parameterId);
        if(parameterTree.isValid())
        {
            parameterTree.copyPropertiesFrom(parameterChildToLoad, nullptr);
        }
    }
    currentPreset.setValue(presetName);
}
int PresetManager::loadNextPreset()
{
    const auto allPresets = getAllPresets();
    if(allPresets.isEmpty())
        return -1;
    const auto currentIndex = allPresets.indexOf(currentPreset.toString());
    const auto nextIndex = currentIndex + 1 > (allPresets.size() - 1) ? 0 : currentIndex + 1;
    loadPreset(allPresets.getReference(nextIndex));
    return nextIndex;
}
int PresetManager::loadPreviousPreset()
{
    const auto allPresets = getAllPresets();
    if(allPresets.isEmpty())
        return -1;
    const auto currentIndex = allPresets.indexOf(currentPreset.toString());
    const auto prevIndex = currentIndex - 1 < 0  ? allPresets.size() - 1 : currentIndex - 1;
    loadPreset(allPresets.getReference(prevIndex));
    return prevIndex;
}
juce::StringArray PresetManager::getAllPresets() const
{
    juce::StringArray presets;
    const auto fileArray = defaultDirectory.findChildFiles(juce::File::TypesOfFileToFind::findFiles, false, "*."+ extension);
    for(const auto& file : fileArray)
    {
        presets.add(file.getFileNameWithoutExtension());
    }
    return presets;
}
juce::String PresetManager::getCurrentPreset() const
{
    return currentPreset.toString();
}
//If the valueTree is replaced, the currentPreset must refer to the new valueTree.
void PresetManager::valueTreeRedirected(juce::ValueTree& treeWhichHasChanged)
{
    currentPreset.referTo(treeWhichHasChanged.getPropertyAsValue(presetNameProperty, nullptr));
}

}
