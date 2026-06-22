#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <memory>
#include <string>
#include "NeuralModel.h"  // also defines NAM_SAMPLE
#include <melegi/StyleDNA.hpp>
#include <melegi/ShoegazeEngine.hpp>
#include <melegi/AnalysisEngine.hpp>

class MelegiNAMProcessor : public juce::AudioProcessor {
public:
    MelegiNAMProcessor();
    ~MelegiNAMProcessor() override;

    // AudioProcessor interface
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "MelegiNAM"; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override;

    int getNumPrograms() override     { return 1; }
    int getCurrentProgram() override  { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Called from editor to trigger model load on background thread
    void loadModelAsync(const juce::String& path, NeuralModel::Backend backend);

    // APVTS — public so editor can create attachments
    juce::AudioProcessorValueTreeState apvts;

    // Read-only state for editor polling
    std::atomic<float> liveLufs  { -70.f };
    std::atomic<float> liveCentroid { 0.f };
    std::atomic<bool>  modelReady { false };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParams();

    // Model hot-swap
    std::atomic<NeuralModel*> activeModel { nullptr };
    juce::String pendingModelPath;
    NeuralModel::Backend pendingBackend { NeuralModel::Backend::ONNX };

    class ModelLoadThread : public juce::Thread {
    public:
        explicit ModelLoadThread(MelegiNAMProcessor& p) : juce::Thread("ModelLoader"), owner(p) {}
        void run() override;
        MelegiNAMProcessor& owner;
    };
    ModelLoadThread loadThread;

    // MELEGI engine objects
    melegi::StyleDNA styleDNA;
    melegi::ShoegazeEngine shoegazeEngine;
    melegi::AnalysisEngine analysisEngine;

    // Pre-allocated scratch buffers (sized in prepareToPlay)
    std::vector<float> scratchIn;
    std::vector<float> scratchOut;

    // RMS accumulator for approximate LUFS display
    float rmsAccum { 0.f };
    int   rmsCount { 0 };

    double currentSampleRate { 44100.0 };
    int    currentBlockSize  { 512 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MelegiNAMProcessor)
};
