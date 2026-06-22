#include "NeuralModel.h"
#include <stdexcept>
#include <cstring>

NeuralModel::NeuralModel(const std::string& modelPath, Backend backend)
    : path_(modelPath)
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
    }
#ifdef MELEGI_USE_TORCHSCRIPT
    else {
        tsModule_ = torch::jit::load(modelPath);
        tsModule_.eval();
        tsLoaded_ = true;
    }
#else
    else {
        throw std::runtime_error("TorchScript backend not compiled in (rebuild with -DUSE_TORCHSCRIPT=ON)");
    }
#endif
}

NeuralModel::~NeuralModel() = default;

void NeuralModel::warmup(int blockSize)
{
    inputBuf_.resize(static_cast<size_t>(blockSize), 0.f);
    outputBuf_.resize(static_cast<size_t>(blockSize), 0.f);
    lastBlockSize_ = blockSize;

    if (backend_ == Backend::ONNX) {
        std::array<int64_t, 3> shape { 1, static_cast<int64_t>(blockSize), 1 };
        const char* inNames[]  = { inputName_.c_str() };
        const char* outNames[] = { outputName_.c_str() };
        auto inTensor = Ort::Value::CreateTensor<float>(
            memInfo_, inputBuf_.data(), inputBuf_.size(), shape.data(), 3);
        session_->Run(Ort::RunOptions { nullptr }, inNames, &inTensor, 1, outNames, 1);
    }
#ifdef MELEGI_USE_TORCHSCRIPT
    else if (tsLoaded_) {
        auto dummy = torch::zeros({ 1, 1, static_cast<long>(blockSize) });
        tsModule_.forward({ dummy });
    }
#endif
}

void NeuralModel::process(const float* inBuf, float* outBuf, int numSamples)
{
    // Resize scratch only if block size changed (rare — most hosts are constant).
    // Still heap-safe on audio thread: realloc only triggers on size change.
    if (numSamples != lastBlockSize_) {
        inputBuf_.resize(static_cast<size_t>(numSamples));
        outputBuf_.resize(static_cast<size_t>(numSamples));
        lastBlockSize_ = numSamples;
    }

    std::memcpy(inputBuf_.data(), inBuf, static_cast<size_t>(numSamples) * sizeof(float));

    if (backend_ == Backend::ONNX) {
        std::array<int64_t, 3> shape { 1, static_cast<int64_t>(numSamples), 1 };
        const char* inNames[]  = { inputName_.c_str() };
        const char* outNames[] = { outputName_.c_str() };
        auto inTensor = Ort::Value::CreateTensor<float>(
            memInfo_, inputBuf_.data(), inputBuf_.size(), shape.data(), 3);
        auto outputs = session_->Run(
            Ort::RunOptions { nullptr }, inNames, &inTensor, 1, outNames, 1);
        const float* result = outputs[0].GetTensorData<float>();
        std::memcpy(outBuf, result, static_cast<size_t>(numSamples) * sizeof(float));
    }
#ifdef MELEGI_USE_TORCHSCRIPT
    else if (tsLoaded_) {
        auto inTensor = torch::from_blob(inputBuf_.data(), { 1, 1, numSamples });
        auto out = tsModule_.forward({ inTensor }).toTensor().contiguous();
        std::memcpy(outBuf, out.data_ptr<float>(), static_cast<size_t>(numSamples) * sizeof(float));
    }
#endif
}
