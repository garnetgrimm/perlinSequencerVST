/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DoomTestAudioProcessorEditor::DoomTestAudioProcessorEditor (DoomTestAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

    baseNoteSlider.setSliderStyle(juce::Slider::Slider::RotaryVerticalDrag);
    baseNoteSlider.setRange(0.0, 127.0, 1.0);
    baseNoteSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    baseNoteSlider.setPopupDisplayEnabled(true, false, this);
    baseNoteSlider.setTextValueSuffix(" Note");
    baseNoteSlider.setValue(processor.baseNote);
    baseNoteSlider.onValueChange = [this] { processor.baseNote = baseNoteSlider.getValue(); };
    addAndMakeVisible(&baseNoteSlider);

    numNotesSlider.setSliderStyle(juce::Slider::Slider::RotaryVerticalDrag);
    numNotesSlider.setRange(0.0, 100.0, 1.0);
    numNotesSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    numNotesSlider.setPopupDisplayEnabled(true, false, this);
    numNotesSlider.setTextValueSuffix(" Notes");
    numNotesSlider.setValue(processor.rampPower);
    numNotesSlider.onValueChange = [this] { processor.rampPower = numNotesSlider.getValue(); };
    addAndMakeVisible(&numNotesSlider);

    tempoSlider.setSliderStyle(juce::Slider::Slider::RotaryVerticalDrag);
    tempoSlider.setRange(60.0, 1000, 1.0);
    tempoSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    tempoSlider.setPopupDisplayEnabled(true, false, this);
    tempoSlider.setTextValueSuffix(" BPM");
    tempoSlider.setValue(60.0);
    tempoSlider.onValueChange = [this] { processor.BPM = tempoSlider.getValue(); };
    addAndMakeVisible(&tempoSlider);

    zSlider.setSliderStyle(juce::Slider::Slider::RotaryVerticalDrag);
    zSlider.setRange(0.1, 1.0, 0.1);
    zSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    zSlider.setPopupDisplayEnabled(true, false, this);
    zSlider.setValue(processor.spawnChance);
    zSlider.onValueChange = [this] { processor.noiseScale = zSlider.getValue(); };
    addAndMakeVisible(&zSlider);

    widthSlider.setSliderStyle(juce::Slider::Rotary);
    widthSlider.setRange(4.0, 64.0, 4.0);
    widthSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    widthSlider.setPopupDisplayEnabled(true, false, this);
    widthSlider.setTextValueSuffix(" Measures");
    widthSlider.setValue(processor.pixels->getNumColumns());
    widthSlider.onValueChange = [this] { 
           processor.doResize(processor.pixels->getNumRows(), widthSlider.getValue()); 
           resized();
    };
    addAndMakeVisible(&widthSlider);

    heightSlider.setSliderStyle(juce::Slider::Rotary);
    heightSlider.setRange(1.0, 64.0, 1.0);
    heightSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    heightSlider.setPopupDisplayEnabled(true, false, this);
    heightSlider.setTextValueSuffix(" Range");
    heightSlider.setValue(processor.pixels->getNumRows());
    heightSlider.onValueChange = [this] { 
        processor.doResize(heightSlider.getValue(), processor.pixels->getNumColumns()); 
        resized();
    };
    addAndMakeVisible(&heightSlider);

    addAndMakeVisible(randomConfig);
    randomConfig.setButtonText("Random");
    //randomConfig.onClick = [this] { processor.randomNew(); };

    //24 FPS
    startTimerHz(24);
}

DoomTestAudioProcessorEditor::~DoomTestAudioProcessorEditor()
{
}

//==============================================================================
void DoomTestAudioProcessorEditor::paint (Graphics& g)
{
    int rows = processor.pixels->getNumRows();
    int cols = processor.pixels->getNumColumns();

    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setOpacity(1);
    g.setColour(Colours::white);
    g.fillRect(xoffset, yoffset, xscale * cols, yscale * rows);
    g.setColour(Colours::black);
    g.drawRect(xoffset, yoffset, xscale * cols, yscale * rows);

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            g.setColour(Colours::black);
            g.setOpacity((*processor.pixels)(row, col));
            g.fillRect((col * xscale) + xoffset, (row * yscale) + yoffset, xscale, yscale);
        }
    }

    g.setColour(Colours::blue);
    g.fillRect((processor.scrubCol * xscale) + xoffset, (processor.strongestRow * yscale) + yoffset, xscale, yscale);

    g.setColour(Colours::red);
    g.setOpacity(0.3);
    g.fillRect(xoffset + (xscale * processor.scrubCol), yoffset, xscale, rows * yscale);

    g.setColour (Colours::white);
}

void DoomTestAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor.

    int sliderSize = 75;
    tempoSlider.setBounds(0, 0, sliderSize, sliderSize);
    widthSlider.setBounds(0, sliderSize, sliderSize, sliderSize);
    heightSlider.setBounds(0, 2 * sliderSize, sliderSize, sliderSize);
    zSlider.setBounds(0, 3 * sliderSize, sliderSize, sliderSize);
    baseNoteSlider.setBounds(getWidth() - sliderSize, 0 * sliderSize, sliderSize, sliderSize);
    numNotesSlider.setBounds(getWidth() - sliderSize, 1 * sliderSize, sliderSize, sliderSize);

    int buttonWidth = (getWidth() * 0.25);
    int buttonHeight = 40;
    randomConfig.setBounds((getWidth() - buttonWidth) / 2, getHeight() - (1.5*buttonHeight), buttonWidth, buttonHeight);

    xscale = (getWidth() * screenPercent) / (*processor.pixels).getNumColumns();
    xoffset = ((getWidth()) - (xscale * (*processor.pixels).getNumColumns())) / 2;
    yscale = (getHeight() * screenPercent) / (*processor.pixels).getNumRows();
    yoffset = ((getHeight()) - (yscale * (*processor.pixels).getNumRows())) / 2;
}

void DoomTestAudioProcessorEditor::timerCallback()
{
    repaint();
}