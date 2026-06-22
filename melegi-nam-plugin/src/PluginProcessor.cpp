#include "PluginProcessor.h"
#include <cmath>
#include <cstring>

// ── Parameter IDs ─────────────────────────────────────────────────────────────
static constexpr char kInputGain[]   = "inputGain";
static constexpr char kOutputGain[]  = "outputGain";
static constexpr char kShoegazeMix[] = "shoegazeMix";
static constexpr char kModelPath[]   = "modelPath";

// ── Parameter layout ──────────────────────────────────────────────────────────
juce::AudioProcessorValueTreeState::ParameterLayout MelegiNAMProcessor::createParams()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        kInputGain,  "Input Gain",
        juce::NormalisableRange<float>(-24.f, 24.f, 0.1f), 0.f,
        juce::AudioParameterFloatAttributes{}.withLabel("dB")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        kOutputGain, "Output Gain",
        juce::NormalisableRange<float>(-24.f, 24.f, 0.1f), 0.f,
        juce::AudioParameterFloatAttributes{}.withLabel("dB")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        kShoegazeMix, "Shoegaze Mix",
        juce::NormalisableRange<float>(0.f, 1.f, 0.01f), 0.15f));
    layout.add(std::make_unique<juce::AudioParameterBool>(kModelPath, "Model Path", false));
    return layout;
}

// ── Constructor / Destructor ──────────────────────────────────────────────────
MelegiNAMProcessor::MelegiNAMProcessor()
    : AudioProcessor(BusesProperties()
          .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , apvts(*this, nullptr, "STATE", createParams())
    , loadThread(*this)
    , styleDNA(melegi::makeGROUXX())
{
}

MelegiNAMProcessor::~MelegiNAMProcessor()
{
    loadThread.stopThread(2000);
    // Release model — audio thread is stopped by now
    if (auto* m = activeModel.exchange(nullptr))
        delete m;
}

// ── Thread-safe model load ────────────────────────────────────────────────────
void MelegiNAMProcessor::loadModelAsync(const juce::String& path, NeuralModel::Backend backend)
{
    pendingModelPath = path;
    pendingBackend   = backend;
    modelReady.store(false);
    loadThread.startThread();
}

void MelegiNAMProcessor::ModelLoadThread::run()
{
    try {
        auto* newModel = new NeuralModel(
            owner.pendingModelPath.toStdString(), owner.pendingBackend);

        // Validate expected sample rate if the model declares one
        if (newModel->HasExpectedSR()) {
            const double expected = newModel->GetExpectedSampleRate();
            const double actual   = owner.currentSampleRate;
            if (std::abs(expected - actual) > 1.0) {
                juce::Logger::writeToLog(juce::String("MelegiNAM: model expects ")
                    + expected + " Hz but host is at " + actual + " Hz");
            }
        }

        // Reset() allocates buffers and calls prewarm() — matches nam::DSP lifecycle.
        newModel->Reset(owner.currentSampleRate, owner.currentBlockSize);

        if (threadShouldExit()) { delete newModel; return; }

        auto* old = owner.activeModel.exchange(newModel, std::memory_order_release);
        owner.modelReady.store(true, std::memory_order_release);
        delete old;
    }
    catch (const std::exception& e) {
        juce::Logger::writeToLog(juce::String("MelegiNAM model load failed: ") + e.what());
    }
}

// ── Lifecycle ─────────────────────────────────────────────────────────────────
void MelegiNAMProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize  = samplesPerBlock;

    scratchIn.resize(static_cast<size_t>(samplesPerBlock));
    scratchOut.resize(static_cast<size_t>(samplesPerBlock));

    // If a model is already loaded (host changed SR/block size), reset it.
    // Reset() re-allocates model buffers and calls prewarm() per NAM lifecycle.
    // Disable prewarm here — we're on the audio thread and can't afford the
    // cost; prewarm already ran during initial load on the background thread.
    if (auto* m = activeModel.load(std::memory_order_acquire)) {
        m->SetPrewarmOnReset(false);
        m->Reset(sampleRate, samplesPerBlock);
        m->SetPrewarmOnReset(true);
    }

    shoegazeEngine.prepare(sampleRate, samplesPerBlock);

    rmsAccum = 0.f;
    rmsCount = 0;
}

void MelegiNAMProcessor::releaseResources()
{
    shoegazeEngine.reset();
}

double MelegiNAMProcessor::getTailLengthSeconds() const
{
    // FDN reverb decay (~2.4s) + model lookahead (0)
    return 2.4;
}

// ── Audio thread ──────────────────────────────────────────────────────────────
void MelegiNAMProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                       juce::MidiBuffer& /*midi*/)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    if (numSamples < 1) return;

    // Read parameters (atomic — safe on audio thread)
    const float inGainLin  = juce::Decibels::decibelsToGain(
        apvts.getRawParameterValue(kInputGain)->load());
    const float outGainLin = juce::Decibels::decibelsToGain(
        apvts.getRawParameterValue(kOutputGain)->load());
    const float shoegazeMix = apvts.getRawParameterValue(kShoegazeMix)->load();

    // Mix to mono for inference (model expects mono input)
    std::fill(scratchIn.begin(), scratchIn.begin() + numSamples, 0.f);
    const float chanMix = 1.f / static_cast<float>(numChannels);
    for (int ch = 0; ch < numChannels; ++ch) {
        const float* src = buffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i)
            scratchIn[static_cast<size_t>(i)] += src[i] * chanMix * inGainLin;
    }

    // Neural inference — NAM double-pointer channel layout: buf[channel][frame].
    // We're mono (channel 0 only); cap to maxBufferSize so NeuralModel::process
    // never needs to reallocate on the audio thread.
    const int inferSamples = std::min(numSamples, currentBlockSize);
    auto* model = activeModel.load(std::memory_order_acquire);
    if (model && modelReady.load(std::memory_order_acquire)) {
        NAM_SAMPLE* inPtr  = scratchIn.data();
        NAM_SAMPLE* outPtr = scratchOut.data();
        NAM_SAMPLE* inChs[]  = { inPtr  };
        NAM_SAMPLE* outChs[] = { outPtr };
        model->process(inChs, outChs, inferSamples);

        // Apply loudness normalization if model declares it (HALoudness() → true)
        if (model->HasLoudness()) {
            // Target: −18 dBFS headroom in the chain. model->GetLoudness() is in dB.
            const float normGain = juce::Decibels::decibelsToGain(
                static_cast<float>(-18.0 - model->GetLoudness()));
            for (int i = 0; i < inferSamples; ++i)
                scratchOut[static_cast<size_t>(i)] *= normGain;
        }
    } else {
        std::memcpy(scratchOut.data(), scratchIn.data(),
                    static_cast<size_t>(inferSamples) * sizeof(float));
    }

    // Apply Shoegaze CASH chain to the mono processed signal
    // ShoegazeEngine operates on an AudioBuffer — wrap scratchOut
    juce::AudioBuffer<float> monoWrap(1, numSamples);
    monoWrap.copyFrom(0, 0, scratchOut.data(), numSamples);
    juce::MidiBuffer dummyMidi;
    shoegazeEngine.process(monoWrap, static_cast<float>(currentSampleRate),
                            styleDNA, shoegazeMix);

    // Write processed mono back to all output channels with output gain
    for (int ch = 0; ch < numChannels; ++ch) {
        float* dst = buffer.getWritePointer(ch);
        const float* src = monoWrap.getReadPointer(0);
        for (int i = 0; i < numSamples; ++i)
            dst[i] = src[i] * outGainLin;
    }

    // RMS accumulator for live LUFS approximation
    for (int i = 0; i < numSamples; ++i)
        rmsAccum += scratchOut[static_cast<size_t>(i)] * scratchOut[static_cast<size_t>(i)];
    rmsCount += numSamples;

    // Update meter every ~100ms
    const int updateInterval = static_cast<int>(currentSampleRate * 0.1);
    if (rmsCount >= updateInterval) {
        const float rms  = std::sqrt(rmsAccum / static_cast<float>(rmsCount));
        const float lufs = rms > 1e-7f ? 20.f * std::log10(rms) - 0.691f : -70.f;
        liveLufs.store(lufs, std::memory_order_relaxed);
        rmsAccum = 0.f;
        rmsCount = 0;
    }
}

// ── State serialization ───────────────────────────────────────────────────────
void MelegiNAMProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    // Store model path alongside APVTS XML
    state.setProperty("modelPath", pendingModelPath, nullptr);
    state.setProperty("modelBackend",
        static_cast<int>(pendingBackend), nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void MelegiNAMProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes)) {
        auto state = juce::ValueTree::fromXml(*xml);
        apvts.replaceState(state);
        auto path = state.getProperty("modelPath").toString();
        auto backend = static_cast<NeuralModel::Backend>(
            static_cast<int>(state.getProperty("modelBackend", 0)));
        if (path.isNotEmpty())
            loadModelAsync(path, backend);
    }
}

// ── Plugin factory ────────────────────────────────────────────────────────────
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MelegiNAMProcessor();
}

juce::AudioProcessorEditor* MelegiNAMProcessor::createEditor()
{
    // Generic editor until PluginEditor.h/.cpp is implemented
    return new juce::GenericAudioProcessorEditor(*this);
}
