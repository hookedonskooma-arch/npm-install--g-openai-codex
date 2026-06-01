#include "PluginProcessor.h"
#include "PluginEditor.h"

AlfredPromptEngineAudioProcessor::AlfredPromptEngineAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)) {}

void AlfredPromptEngineAudioProcessor::prepareToPlay(double, int) {}

void AlfredPromptEngineAudioProcessor::releaseResources() {}

bool AlfredPromptEngineAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet();
}

void AlfredPromptEngineAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;
    for (int channel = getTotalNumInputChannels(); channel < getTotalNumOutputChannels(); ++channel) {
        buffer.clear(channel, 0, buffer.getNumSamples());
    }
}

juce::AudioProcessorEditor* AlfredPromptEngineAudioProcessor::createEditor() {
    return new AlfredPromptEngineAudioProcessorEditor(*this);
}

bool AlfredPromptEngineAudioProcessor::hasEditor() const {
    return true;
}

const juce::String AlfredPromptEngineAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool AlfredPromptEngineAudioProcessor::acceptsMidi() const {
    return false;
}

bool AlfredPromptEngineAudioProcessor::producesMidi() const {
    return false;
}

bool AlfredPromptEngineAudioProcessor::isMidiEffect() const {
    return false;
}

double AlfredPromptEngineAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int AlfredPromptEngineAudioProcessor::getNumPrograms() {
    return 1;
}

int AlfredPromptEngineAudioProcessor::getCurrentProgram() {
    return 0;
}

void AlfredPromptEngineAudioProcessor::setCurrentProgram(int) {}

const juce::String AlfredPromptEngineAudioProcessor::getProgramName(int) {
    return {};
}

void AlfredPromptEngineAudioProcessor::changeProgramName(int, const juce::String&) {}

void AlfredPromptEngineAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    juce::MemoryOutputStream stream(destData, true);
    stream.writeString("AlfredPromptEngineState:v1");
}

void AlfredPromptEngineAudioProcessor::setStateInformation(const void*, int) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new AlfredPromptEngineAudioProcessor();
}
