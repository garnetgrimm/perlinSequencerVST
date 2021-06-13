/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Perlin.h"
#include <set>

#define BPM_TO_MS(bpm) { int( (1.0/(bpm))*(60000.0) ) }

//==============================================================================
/**
*/
class DoomTestAudioProcessor  : public AudioProcessor, public Timer
{
public:
    //==============================================================================
    DoomTestAudioProcessor();
    ~DoomTestAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    dsp::Matrix<float> *pixels;

    int major[8] = { 0,2,4,5,7,8 };

    Perlin perlin;
    float BPM = 60;
    float time = 1.5;
    float timeSpeed = 0.1;
    float spawnChance = 0.4;
    void perlinMatrix();
    void test_stringMatrix();
    void test_smallMatrix();
    void doResize(int newRows, int newCols);
    Random random = juce::Random::getSystemRandom();
    void randomNew();
    std::set<int> onNotes;
    std::set<int> chosenNotes;
    int baseNote = 50;
    int numNotes = 1;
    int strongestRow = 0;
    float noiseScale = 0.1;
    float ramp(float x);
    int rampPower = 10;

    int scrubCol = 0;
    bool generateNotes = 0;
    bool isPixelOn(int row, int col);
    int countOnNearby(int targetRow, int targetCol);
    void step();
    void update();
    void timerCallback() override;

    void addMessageToBuffer(const juce::MidiMessage& message, MidiBuffer& midiBuffer);
    void setNoteNumber(int noteNumber, bool on, MidiBuffer& midiMessages);

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    AudioSampleBuffer reverbBuffer;
    int reverbBufferIdx = 0;
    float reverbTime = 0.01;
    float reverbGain = 1.25;
    float limitSample(float sample);

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DoomTestAudioProcessor)
};
