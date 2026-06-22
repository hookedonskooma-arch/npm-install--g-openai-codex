#include "NeuralModel.h"
#include <stdexcept>
#include <cstring>

static constexpr int NAM_DEFAULT_MAX_BUFFER_SIZE = 4096;

// ── Construction ──────────────────────────────────────────────────────────────

NeuralModel::NeuralModel(const std::string& modelPath, Backend backend,
                         double expected_sample_rate)
    : mExpectedSampleRate(expected_sample_rate)
    , path_(modelPath)
    , backend_(backend)
    , env_(ORT_LOGGING_LEVEL_WARNING, "MelegiNAM")
    , memInfo_(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault))
{
    if (backend_ == Backend::ONNX) {
        Ort::SessionOptions opts;
        opts.SetIntraOpNumThreads(1);
        opts.SetGraphOptimizationLevel(ORT_ENABLE_ALL);
        session_ = std::make_unique<Ort::Session>(env_, modelPath.c_str(), opts);

        Ort::AllocatorWithDefaultOptions alloc;
        inputName_  = session_->GetInputNameAllocated(0, alloc).get();
        outputName_ = session_->GetOutputNameAllocated(0, alloc).get();

        // If the ONNX model embeds an expected sample rate as metadata, read it.
        // Convention: key "expected_sample_rate" in model's custom metadata.
        try {
            auto keys = session_->GetModelMetadata().GetCustomMetadataMapKeysAllocated(alloc);
            for (const auto& k : keys) {
                if (std::string(k.get()) == "expected_sample_rate") {
                    auto val = session_->GetModelMetadata()
                                        .LookupCustomMetadataMapAllocated(k.get(), alloc);
                    mExpectedSampleRate = std::stod(val.get());
                }
                if (std::string(k.get()) == "loudness_db") {
                    auto val = session_->GetModelMetadata()
                                        .LookupCustomMetadataMapAllocated(k.get(), alloc);
                    SetLoudness(std::stod(val.get()));
                }
            }
        } catch (...) {} // metadata is optional
    }
#ifdef MELEGI_USE_TORCHSCRIPT
    else {
        tsModule_ = torch::jit::load(modelPath);
        tsModule_.eval();
        tsLoaded_ = true;
    }
#else
    else {
        throw std::runtime_error(
            "TorchScript backend not compiled in — rebuild with -DUSE_TORCHSCRIPT=ON");
    }
#endif
}

NeuralModel::~NeuralModel() = default;

// ── NAM DSP lifecycle ─────────────────────────────────────────────────────────

void NeuralModel::SetMaxBufferSize(int maxBufferSize)
{
    mMaxBufferSize = maxBufferSize;
    scratchIn_.resize(static_cast<size_t>(maxBufferSize), 0.f);
    scratchOut_.resize(static_cast<size_t>(maxBufferSize), 0.f);
}

void NeuralModel::prewarm()
{
    // Run NAM_DEFAULT_MAX_BUFFER_SIZE zeros through the model to settle
    // recurrent state (LSTM hidden vectors, WaveNet dilated conv buffers).
    const int N = std::max(mMaxBufferSize, NAM_DEFAULT_MAX_BUFFER_SIZE);
    std::vector<float> zeros(static_cast<size_t>(N), 0.f);
    std::vector<float> sink (static_cast<size_t>(N), 0.f);
    if (backend_ == Backend::ONNX)
        runONNX(zeros.data(), sink.data(), N);
#ifdef MELEGI_USE_TORCHSCRIPT
    else if (tsLoaded_)
        runTorch(zeros.data(), sink.data(), N);
#endif
}

void NeuralModel::Reset(double sampleRate, int maxBufferSize)
{
    mExternalSampleRate = sampleRate;
    SetMaxBufferSize(maxBufferSize);
    if (mPrewarmOnReset.load(std::memory_order_acquire))
        prewarm();
}

// ── Audio thread: process ─────────────────────────────────────────────────────

void NeuralModel::process(NAM_SAMPLE** input, NAM_SAMPLE** output, int num_frames)
{
    // Resize scratch only if block grew beyond pre-allocated size (rare).
    // This does allocate on the heap — guard in PluginProcessor by capping
    // num_frames to maxBufferSize from Reset(), making this branch dead.
    if (num_frames > mMaxBufferSize) {
        scratchIn_.resize(static_cast<size_t>(num_frames));
        scratchOut_.resize(static_cast<size_t>(num_frames));
        mMaxBufferSize = num_frames;
    }

    // Copy channel 0 of input (mono model) into flat scratch buffer
    std::memcpy(scratchIn_.data(), input[0],
                static_cast<size_t>(num_frames) * sizeof(float));

    if (backend_ == Backend::ONNX)
        runONNX(scratchIn_.data(), scratchOut_.data(), num_frames);
#ifdef MELEGI_USE_TORCHSCRIPT
    else if (tsLoaded_)
        runTorch(scratchIn_.data(), scratchOut_.data(), num_frames);
#endif

    // Write result into channel 0 of output
    std::memcpy(output[0], scratchOut_.data(),
                static_cast<size_t>(num_frames) * sizeof(float));
}

// ── Backend inference ─────────────────────────────────────────────────────────

void NeuralModel::runONNX(const float* in, float* out, int numSamples)
{
    // Tensor shape: [1 (batch), numSamples (seq), 1 (channel)]
    // Adjust if your model's contract differs — see SHARED.md "Model contract".
    std::array<int64_t, 3> shape { 1, static_cast<int64_t>(numSamples), 1 };
    const char* inNames[]  = { inputName_.c_str() };
    const char* outNames[] = { outputName_.c_str() };

    auto inTensor = Ort::Value::CreateTensor<float>(
        memInfo_,
        const_cast<float*>(in),   // CreateTensor takes non-const; we won't mutate
        static_cast<size_t>(numSamples),
        shape.data(), 3);

    auto outputs = session_->Run(
        Ort::RunOptions { nullptr }, inNames, &inTensor, 1, outNames, 1);
    const float* result = outputs[0].GetTensorData<float>();
    std::memcpy(out, result, static_cast<size_t>(numSamples) * sizeof(float));
}

void NeuralModel::runTorch(const float* in, float* out, int numSamples)
{
#ifdef MELEGI_USE_TORCHSCRIPT
    if (!tsLoaded_) return;
    // Wrap input without copy (channels-first: [1, 1, N])
    auto inTensor = torch::from_blob(
        const_cast<float*>(in), { 1, 1, numSamples });
    auto result = tsModule_.forward({ inTensor }).toTensor().contiguous();
    std::memcpy(out, result.data_ptr<float>(),
                static_cast<size_t>(numSamples) * sizeof(float));
#else
    (void)in; (void)out; (void)numSamples;
#endif
}
