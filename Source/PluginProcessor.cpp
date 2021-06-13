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
    pixels = new dsp::Matrix<bool>(16, 8);

    perlinMatrix();

    callAfterDelay(BPM_TO_MS(BPM), [&]{update(); });
}

DoomTestAudioProcessor::~DoomTestAudioProcessor()
{

}


void DoomTestAudioProcessor::test_smallMatrix() {
    (*pixels)(3, 3) = true;
    (*pixels)(3, 4) = true;
    (*pixels)(3, 5) = true;
    (*pixels)(5, 3) = true;
    (*pixels)(5, 4) = true;
    (*pixels)(6, 4) = true;
}

void DoomTestAudioProcessor::test_stringMatrix() {
    String s =
        "00000000\n"
        "000XX000\n"
        "00000000\n"
        "00XXXX00\n"
        "00000000\n"
        "0XXXXXX0\n"
        "00000000\n"
        "00XXXX00\n"
        "00000000\n"
        "000XX000\n"
        "00000000";

    StringArray tokens = StringArray::fromTokens(s, "\n");

    int initRows = tokens.size();
    int initCols = tokens[0].length();

    for (int i = 0; i < initRows; i++)
    {
        for (int n = 0; n < initCols; n++) {
            if (tokens[i][n] == 'X') (*pixels)(i, n) = true;
            else (*pixels)(i, n) = false;
        }
    }
}

void DoomTestAudioProcessor::perlinMatrix() {
    int rows = pixels->getNumRows();
    int cols = pixels->getNumColumns();
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            float noise = (perlin.noise(row * 0.1, col * 0.1, time) + 1.0) / 2.0;
            (*pixels)(row, col) = noise < spawnChance ? true : false;
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

void DoomTestAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    int majSize = sizeof(major) / sizeof(int);
    int rows = pixels->getNumRows();
    if (generateNotes) {
        std::set<int> lastOnNotes = onNotes;
        onNotes.clear();

        for (int row = 0; row < rows; row++) {
            int col = scrubCol;
            int offset = major[(row % majSize)] + (row / majSize);
            int note = baseNote + offset;
            if ((*pixels)(row, col)) {
                onNotes.insert(note);
            }
        }

        for (int note : lastOnNotes) {
            //if note was being played before and is no longer being played, kill it
            if (onNotes.find(note) == onNotes.end()) {
                setNoteNumber(note, false, midiMessages);
            }
        }

        std::set<int> lastChosenNotes = chosenNotes;
        chosenNotes.clear();
        DBG(onNotes.size() << " | " << lastChosenNotes.size() << " | " << chosenNotes.size());
        String test = "";
        for (int note : onNotes) {
            test.append(std::to_string(note), 3);
            test += " ";
        }
        DBG(test);
        

        if (onNotes.size() > 0) {
            for (int i = 0; i < numNotes; i++) {
                int idx = rand() % onNotes.size();
                //int idx = (onNotes.size()-i) % onNotes.size();
                if (idx > onNotes.size()) continue;
                std::set<int>::iterator it = onNotes.begin();
                std::advance(it, idx);
                int note = *it;
                chosenNotes.insert(note);
            }
        }

        for (int note : lastChosenNotes) {
            //if note was being played before and is no longer being played, kill it
            if (chosenNotes.find(note) == chosenNotes.end()) {
                setNoteNumber(note, false, midiMessages);
            }   
        }
        for (int note : chosenNotes) {
            //if note was not being played before but is now, turn it on
            if (lastChosenNotes.find(note) == lastChosenNotes.end()) {
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
            if ((*pixels)(currRow, currCol)) onCount++;
        }
    }
    return onCount;
}

void DoomTestAudioProcessor::step()
{
    int rows = pixels->getNumRows();
    int cols = pixels->getNumColumns();
    dsp::Matrix<bool> newPixels(rows, cols);
    for (int row = 0; row < rows-1; row++) {
        for (int col = 0; col < cols-1; col++) {
            int nearby = countOnNearby(row, col);
            if ((*pixels)(row, col)) {
                if (nearby <= 1 || nearby >= 4) newPixels(row, col) = false;
                else newPixels(row, col) = true;
            }
            else {
                if (nearby == 3) newPixels(row, col) = true;
            }
        }
    }
    (*pixels) = newPixels;
}

void DoomTestAudioProcessor::update() {
    scrubCol++;
    generateNotes = true;
    //step();
    if (scrubCol >= pixels->getNumColumns()) {
        //step();
        scrubCol = 0;
        time += timeSpeed;
    }
    perlinMatrix();
    callAfterDelay(BPM_TO_MS(BPM), [&] {update(); });
}

void DoomTestAudioProcessor::timerCallback()
{
    update();
}

void DoomTestAudioProcessor::randomNew()
{
    int rows = pixels->getNumRows();
    int cols = pixels->getNumColumns();

    for (int row = 0; row < rows - 1; row++) {
        for (int col = 0; col < cols - 1; col++) {
            (*pixels)(row, col) = !((random.nextInt() % 3) - 1);
            /*
            (*pixels)(3, 3 + testIdx) = true;
            (*pixels)(3, 4 + testIdx) = true;
            (*pixels)(3, 5 + testIdx) = true;
            (*pixels)(5, 3 + testIdx) = true;
            (*pixels)(5, 4 + testIdx) = true;
            (*pixels)(6, 4 + testIdx) = true;
            */
        }
    }
}

void DoomTestAudioProcessor::doResize(int newRows, int newCols) {
    int rows = pixels->getNumRows();
    int cols = pixels->getNumColumns();

    int xoffset = (newCols - cols) / 2;
    int yoffset = (newRows - rows) / 2;

    dsp::Matrix<bool> *newPixels = new dsp::Matrix<bool>(newRows, newCols);
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
