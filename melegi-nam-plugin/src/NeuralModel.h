#pragma once
#include <string>
#include <vector>
#include <memory>
#include <onnxruntime/core/session/onnxruntime_cxx_api.h>

#ifdef MELEGI_USE_TORCHSCRIPT
#include <torch/script.h>
#endif

// Wraps either an ONNX Runtime session or a TorchScript module.
// All public methods are safe to call from any thread EXCEPT process(),
// which must only be called on the audio thread after warmup completes.
class NeuralModel {
public:
    enum class Backend { ONNX, TorchScript };

    explicit NeuralModel(const std::string& modelPath, Backend backend = Backend::ONNX);
    ~NeuralModel();

    // Non-copyable — owns Ort::Session
    NeuralModel(const NeuralModel&) = delete;
    NeuralModel& operator=(const NeuralModel&) = delete;

    // Run inference. Input: mono float samples [numSamples].
    // Output: written into outBuf (pre-allocated, same length).
    // MUST be called on audio thread only, after warmup().
    void process(const float* inBuf, float* outBuf, int numSamples);

    // Warm up the model (run a dummy forward pass to trigger JIT compilation).
    // Call on background thread before marking the model ready.
    void warmup(int blockSize);

    Backend backend() const { return backend_; }
    std::string path() const { return path_; }

private:
    std::string path_;
    Backend backend_;

    // ONNX Runtime
    Ort::Env env_;
    std::unique_ptr<Ort::Session> session_;
    std::string inputName_;
    std::string outputName_;
    Ort::MemoryInfo memInfo_;

    // Pre-allocated tensor buffers (sized in warmup)
    std::vector<float> inputBuf_;
    std::vector<float> outputBuf_;
    int lastBlockSize_ { 0 };

#ifdef MELEGI_USE_TORCHSCRIPT
    torch::jit::script::Module tsModule_;
    bool tsLoaded_ { false };
#endif
};
