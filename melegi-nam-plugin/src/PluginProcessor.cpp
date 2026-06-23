#include "PluginProcessor.h"
#include <cmath>
#include <cstring>

// ── Parameter IDs ─────────────────────────────────────────────────────────────
static constexpr char kInputGain[]   = "inputGain";
static constexpr char kOutputGain[]  = "outputGain";
static constexpr char kShoegazeMix[] = "shoegazeMix";

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
    , monoWrap(1, 512)  // resized in prepareToPlay
{
}

MelegiNAMProcessor::~MelegiNAMProcessor()
{
    loadThread.stopThread(2000);
    if (auto* m = activeModel.exchange(nullptr))
        delete m;
    delete retiredModel;
}

// ── Model hot-swap: deferred deletion ────────────────────────────────────────
// The load thread sets activeModel to the new model. It stores the old pointer
// in retiredModel instead of deleting it immediately. retireOldModel() is called
// at the END of processBlock — by which time the audio thread has certainly
// finished any call to process() on the old model from earlier in this block.
// This is safe because retiredModel is only ever touched from processBlock
// (audio thread) — the load thread writes to activeModel only, never retiredModel.
void MelegiNAMProcessor::retireOldModel()
{
    if (retiredModel) {
        delete retiredModel;
        retiredModel = nullptr;
    }
}

// ── Thread-safe model load ────────────────────────────────────────────────────
void MelegiNAMProcessor::loadModelAsync(const juce::String& path, NeuralModel::Backend backend)
{
    // Snapshot parameters under lock so the load thread reads a consistent view
    // even if loadModelAsync is called again before the thread finishes.
    {
        std::lock_guard<std::mutex> lock(loadMutex);
        pendingModelPath_ = path.toStdString();
        pendingBackend_   = backend;
    }
    pendingSampleRate.store(currentSampleRate, std::memory_order_relaxed);
    pendingBlockSize.store(currentBlockSize,   std::memory_order_relaxed);

    // Stop any in-flight load before starting a new one; prevents the previous
    // thread from loading a stale path after the new path was written.
    loadThread.stopThread(500);
    modelReady.store(false, std::memory_order_release);
    loadThread.startThread();

    lastModelPath = path;
    lastBackend   = backend;
}

void MelegiNAMProcessor::ModelLoadThread::run()
{
    // Snapshot parameters under lock — UI thread may call loadModelAsync again.
    std::string modelPath;
    NeuralModel::Backend backend;
    {
        std::lock_guard<std::mutex> lock(owner.loadMutex);
        modelPath = owner.pendingModelPath_;
        backend   = owner.pendingBackend_;
    }
    const double sr       = owner.pendingSampleRate.load(std::memory_order_relaxed);
    const int    maxBlock = owner.pendingBlockSize.load(std::memory_order_relaxed);

    try {
        auto* newModel = new NeuralModel(modelPath, backend);

        if (newModel->HasExpectedSR()) {
            const double expected = newModel->GetExpectedSampleRate();
            if (std::abs(expected - sr) > 1.0)
                juce::Logger::writeToLog(juce::String("MelegiNAM: model expects ")
                    + expected + " Hz but host is at " + sr + " Hz");
        }

        newModel->Reset(sr, maxBlock);

        if (threadShouldExit()) { delete newModel; return; }

        // Exchange: store old in retiredModel for deferred deletion by audio thread.
        // The audio thread drains retiredModel at end of processBlock, after
        // it is guaranteed to be done with any prior process() call.
        auto* old = owner.activeModel.exchange(newModel, std::memory_order_release);
        owner.retiredModel = old;   // written by load thread, read/deleted by audio thread
        owner.modelReady.store(true, std::memory_order_release);
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

    scratchIn.assign(static_cast<size_t>(samplesPerBlock), 0.f);
    scratchOut.assign(static_cast<size_t>(samplesPerBlock), 0.f);

    // Pre-allocate mono wrap — avoids heap alloc in processBlock.
    monoWrap.setSize(1, samplesPerBlock, false, true, false);

    // If a model is live, resize its buffers for the new SR/blockSize.
    // Disable prewarm — we're on the audio thread and can't afford it;
    // prewarm already ran during initial load on the background thread.
    if (auto* m = activeModel.load(std::memory_order_acquire)) {
        m->SetPrewarmOnReset(false);
        m->Reset(sampleRate, samplesPerBlock);
        m->SetPrewarmOnReset(true);
    }

    // Propagate new SR/blockSize to the pending-load atomics so the next
    // loadModelAsync() picks them up correctly.
    pendingSampleRate.store(sampleRate,      std::memory_order_relaxed);
    pendingBlockSize.store(samplesPerBlock, std::memory_order_relaxed);

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
    return 2.4;  // FDN reverb decay
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
    const float inGainLin   = juce::Decibels::decibelsToGain(
        apvts.getRawParameterValue(kInputGain)->load());
    const float outGainLin  = juce::Decibels::decibelsToGain(
        apvts.getRawParameterValue(kOutputGain)->load());
    const float shoegazeMix = apvts.getRawParameterValue(kShoegazeMix)->load();

    // Mix to mono for inference
    std::fill(scratchIn.begin(), scratchIn.begin() + numSamples, 0.f);
    const float chanMix = 1.f / static_cast<float>(numChannels);
    for (int ch = 0; ch < numChannels; ++ch) {
        const float* src = buffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i)
            scratchIn[static_cast<size_t>(i)] += src[i] * chanMix * inGainLin;
    }

    // Neural inference — cap to currentBlockSize so NeuralModel::process never
    // reallocates on the audio thread. If host delivers fewer samples than
    // currentBlockSize (the normal case), inferSamples == numSamples.
    // If it somehow delivers more (should not happen with JUCE), we cap it and
    // fill the remainder with zeros to avoid stale data propagating.
    const int inferSamples = std::min(numSamples, currentBlockSize);
    if (inferSamples < numSamples)
        std::fill(scratchOut.begin() + inferSamples,
                  scratchOut.begin() + numSamples, 0.f);

    auto* model = activeModel.load(std::memory_order_acquire);
    if (model && modelReady.load(std::memory_order_acquire)) {
        NAM_SAMPLE* inChs[]  = { scratchIn.data() };
        NAM_SAMPLE* outChs[] = { scratchOut.data() };
        model->process(inChs, outChs, inferSamples);

        if (model->HasLoudness()) {
            const float normGain = juce::Decibels::decibelsToGain(
                static_cast<float>(-18.0 - model->GetLoudness()));
            for (int i = 0; i < inferSamples; ++i)
                scratchOut[static_cast<size_t>(i)] *= normGain;
        }
    } else {
        std::memcpy(scratchOut.data(), scratchIn.data(),
                    static_cast<size_t>(numSamples) * sizeof(float));
    }

    // ShoegazeEngine CASH chain — use pre-allocated monoWrap (no heap alloc)
    monoWrap.copyFrom(0, 0, scratchOut.data(), numSamples);
    shoegazeEngine.process(monoWrap, static_cast<float>(currentSampleRate),
                            styleDNA, shoegazeMix);

    // Write processed mono to all output channels with output gain
    for (int ch = 0; ch < numChannels; ++ch) {
        float* dst = buffer.getWritePointer(ch);
        const float* src = monoWrap.getReadPointer(0);
        for (int i = 0; i < numSamples; ++i)
            dst[i] = src[i] * outGainLin;
    }

    // RMS accumulator — reads scratchOut which is fully written (zeros beyond inferSamples)
    for (int i = 0; i < numSamples; ++i)
        rmsAccum += scratchOut[static_cast<size_t>(i)] * scratchOut[static_cast<size_t>(i)];
    rmsCount += numSamples;

    const int updateInterval = static_cast<int>(currentSampleRate * 0.1);
    if (rmsCount >= updateInterval) {
        const float rms  = std::sqrt(rmsAccum / static_cast<float>(rmsCount));
        const float lufs = rms > 1e-7f ? 20.f * std::log10(rms) - 0.691f : -70.f;
        liveLufs.store(lufs, std::memory_order_relaxed);
        rmsAccum = 0.f;
        rmsCount = 0;
    }

    // Deferred deletion of retired model — safe here because we've completed
    // any process() call on it that may have started this block.
    retireOldModel();
}

// ── State serialization ───────────────────────────────────────────────────────
void MelegiNAMProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty("modelPath",    lastModelPath,                   nullptr);
    state.setProperty("modelBackend", static_cast<int>(lastBackend),   nullptr);
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
    return new juce::GenericAudioProcessorEditor(*this);
}
