/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include <array>

//==============================================================================
/**
*/
class DoomTestAudioProcessorEditor  : public AudioProcessorEditor, public Timer
{
public:
    DoomTestAudioProcessorEditor (DoomTestAudioProcessor&);
    ~DoomTestAudioProcessorEditor();

    float screenPercent = 0.5;

    int xoffset = 0;
    int yoffset = 0;
    int xscale = 1;
    int yscale = 1;

    void timerCallback() override;
    juce::Slider tempoSlider;
    juce::Slider widthSlider;
    juce::Slider heightSlider;
    juce::Slider zSlider;
    juce::Slider baseNoteSlider;
    juce::Slider numNotesSlider;
    juce::TextButton randomConfig;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DoomTestAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DoomTestAudioProcessorEditor)
};
