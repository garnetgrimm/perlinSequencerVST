/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DoomTestAudioProcessor::DoomTestAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    pixels = new dsp::Matrix <float> (16, 8);

    perlinMatrix();

    callAfterDelay(BPM_TO_MS(BPM), [&]{update(); });
}

DoomTestAudioProcessor::~DoomTestAudioProcessor()
{

}

float DoomTestAudioProcessor::ramp(float x) {
    int power = rampPower;
    float exp = pow(2, -power * (x - 0.5));
    return 1.0/(1.0+exp);
};

void DoomTestAudioProcessor::perlinMatrix() {
    int rows = pixels->getNumRows();
    int cols = pixels->getNumColumns();
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            float noise = (perlin.noise(row * noiseScale, col * noiseScale, sin(time)) + 1.0) / 2.0;
            noise *= sin((float(row)/float(rows)) * juce::MathConstants<float>::pi);
            noise = ramp(noise);
            (*pixels)(row, col) = noise;
        }
    }
}

//==============================================================================
const String DoomTestAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DoomTestAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DoomTestAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DoomTestAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DoomTestAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DoomTestAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DoomTestAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DoomTestAudioProcessor::setCurrentProgram (int index)
{
}

const String DoomTestAudioProcessor::getProgramName (int index)
{
    return {};
}

void DoomTestAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void DoomTestAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void DoomTestAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DoomTestAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

float DoomTestAudioProcessor::limitSample(float sample) {
    return reverbGain * atan(sample * reverbGain) / MathConstants<float>::halfPi;
}

// utility comparator function to pass to the sort() module
bool sortByVal(const std::pair<int, float>& a,
    const std::pair<int, float>& b)
{
    return (a.second < b.second);
}

void DoomTestAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    int majSize = sizeof(major) / sizeof(int);
    int rows = pixels->getNumRows();
    if (generateNotes) {
        std::map<int, float> noteMap;

        for (int row = 0; row < rows; row++) {
            int col = scrubCol;
            int fixedRow = row - (rows / 2);
            int offset = major[(fixedRow % majSize)] + (fixedRow / majSize);
            int note = baseNote + offset;
            noteMap[row] = (*pixels)(row,col);
        }

        std::vector<std::pair<int, float>> sortVec;

        std::map<int, float> ::iterator it2;
        for (it2 = noteMap.begin(); it2 != noteMap.end(); it2++)
        {
            sortVec.push_back(std::make_pair(it2->first, it2->second));
        }
        std::sort(sortVec.begin(), sortVec.end(), sortByVal);

        //float heighestVal = sortVec[sortVec.size() - 1].second;
        strongestRow = sortVec[sortVec.size() - 1].first;

        std::set<int> lastOnNotes = onNotes;
        onNotes.clear();

        int newNote = major[(strongestRow % majSize)] + (strongestRow / majSize) + baseNote;
        onNotes.insert(newNote);

        for (int note : lastOnNotes) {
            //if note was being played before and is no longer being played, kill it
            if (onNotes.find(note) == onNotes.end()) {
                setNoteNumber(note, false, midiMessages);
            }
        }

        for (int note : onNotes) {
            //if note was not being played before but is now, turn it on
            if (lastOnNotes.find(note) == lastOnNotes.end()) {
                setNoteNumber(note, true, midiMessages);
            }
        }

        generateNotes = false;
    }
}

void DoomTestAudioProcessor::addMessageToBuffer(const juce::MidiMessage& message, MidiBuffer& midiBuffer)
{
    auto timestamp = message.getTimeStamp();
    auto sampleNumber = (int)(timestamp * getSampleRate());
    midiBuffer.addEvent(message, sampleNumber);
}

void DoomTestAudioProcessor::setNoteNumber(int noteNumber, bool on, MidiBuffer& midiMessages)
{
    if (on) {
        auto message = juce::MidiMessage::noteOn(1, noteNumber, (juce::uint8)100);
        message.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
        addMessageToBuffer(message, midiMessages);
    }
    else {
        auto message = juce::MidiMessage::noteOff(1, noteNumber, (juce::uint8)100);
        message.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
        addMessageToBuffer(message, midiMessages);
    }
}

//==============================================================================
bool DoomTestAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* DoomTestAudioProcessor::createEditor()
{
    return new DoomTestAudioProcessorEditor (*this);
}

//==============================================================================
void DoomTestAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DoomTestAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

bool DoomTestAudioProcessor::isPixelOn(int row, int col) {
    return (*pixels)(row, col) < spawnChance ? true : false;
}


int DoomTestAudioProcessor::countOnNearby(int targetRow, int targetCol) {
    int rows = pixels->getNumRows();
    int cols = pixels->getNumRows();
    int onCount = 0;
    for (int row = -1; row <= 1; row++) {
        for (int col = -1; col <= 1; col++) {
            int currRow = targetRow + row;
            int currCol = targetCol + col;
            if (row == 0 && col == 0) continue;
            if (currRow > rows || currCol > cols) continue;
            if (currRow < 0 || currCol < 0) continue;
            if (isPixelOn(currRow, currCol)) onCount++;
        }
    }
    return onCount;
}

void DoomTestAudioProcessor::update() {
    scrubCol++;
    generateNotes = true;
    if (scrubCol >= pixels->getNumColumns()) {
        scrubCol = 0;
        //time += timeSpeed;
    }
    time += timeSpeed;
    perlinMatrix();
    callAfterDelay(BPM_TO_MS(BPM), [&] {update(); });
}

void DoomTestAudioProcessor::timerCallback()
{
    update();
}

void DoomTestAudioProcessor::doResize(int newRows, int newCols) {
    int rows = pixels->getNumRows();
    int cols = pixels->getNumColumns();

    int xoffset = (newCols - cols) / 2;
    int yoffset = (newRows - rows) / 2;

    dsp::Matrix<float> *newPixels = new dsp::Matrix<float>(newRows, newCols);
    for (int row = 0; row < rows - 1; row++) {
        for (int col = 0; col < cols - 1; col++) {
            int currCol = col + xoffset;
            int currRow = row - yoffset;
            if (currCol > newCols - 1) continue;
            if (currRow > newRows - 1) continue;
            if (currCol < 0) continue;
            if (currRow < 0) continue;
            (*newPixels)(currRow, currCol) = (*pixels)(row, col);
        }
    }
    pixels = newPixels;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DoomTestAudioProcessor();
}
