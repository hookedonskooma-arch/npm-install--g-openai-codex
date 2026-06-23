#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <memory>
#include <mutex>
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

    // Model hot-swap.
    // activeModel is the live pointer read on the audio thread.
    // retiredModel holds the previous model until the audio thread is known
    // to have quiesced (drained at end of processBlock via retireModel()).
    std::atomic<NeuralModel*> activeModel { nullptr };
    NeuralModel* retiredModel { nullptr };  // drained by processBlock, never by load thread
    void retireOldModel();                  // called at end of processBlock, not real-time-unsafe

    // Load-thread parameters — protected by loadMutex.
    // The load thread snapshots these under the mutex at thread start,
    // so the UI thread can update them without racing the running thread.
    std::mutex loadMutex;
    std::string pendingModelPath_;
    NeuralModel::Backend pendingBackend_ { NeuralModel::Backend::ONNX };
    std::atomic<double> pendingSampleRate { 44100.0 };
    std::atomic<int>    pendingBlockSize  { 512 };

    // Exposed to editor (read-only)
    juce::String lastModelPath;
    NeuralModel::Backend lastBackend { NeuralModel::Backend::ONNX };

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
    // Pre-allocated mono wrap for ShoegazeEngine — avoids audio-thread allocation
    juce::AudioBuffer<float> monoWrap;

    // RMS accumulator for approximate LUFS display
    float rmsAccum { 0.f };
    int   rmsCount { 0 };

    // Snapshot of SR/blockSize visible to processBlock (written only in prepareToPlay,
    // read only in processBlock — no concurrent access, no atomic needed).
    double currentSampleRate { 44100.0 };
    int    currentBlockSize  { 512 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MelegiNAMProcessor)
};
