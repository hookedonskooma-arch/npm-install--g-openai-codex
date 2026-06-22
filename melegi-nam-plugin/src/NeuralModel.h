#pragma once
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <onnxruntime/core/session/onnxruntime_cxx_api.h>

#ifdef MELEGI_USE_TORCHSCRIPT
#include <torch/script.h>
#endif

// NAM_SAMPLE convention: double precision matches NAM's default, but our ONNX
// models are float32. Keep as float — caller is responsible for the mismatch.
using NAM_SAMPLE = float;

// Mirrors nam::DSP's interface so MelegiNAM can be a drop-in host for either
// our ONNX/TorchScript models or a real nam::DSP subclass in the future.
//
// Lifecycle (matches nam::DSP exactly):
//   1. Construct on background thread.
//   2. Call Reset(sampleRate, maxBufferSize) — calls prewarm() internally if
//      mPrewarmOnReset is true (default).
//   3. Store in atomic<NeuralModel*>, set modelReady = true.
//   4. Audio thread calls process() with channel-first double-pointer buffers.
//   5. On model swap: exchange atomic ptr, delete old on background thread.
class NeuralModel {
public:
    enum class Backend { ONNX, TorchScript };

    // expected_sample_rate: pass -1.0 if unknown (NAM_UNKNOWN_EXPECTED_SAMPLE_RATE)
    NeuralModel(const std::string& modelPath,
                Backend backend = Backend::ONNX,
                double expected_sample_rate = -1.0);
    virtual ~NeuralModel();

    NeuralModel(const NeuralModel&)            = delete;
    NeuralModel& operator=(const NeuralModel&) = delete;

    // ── NAM DSP interface ────────────────────────────────────────────────────

    // Settle initial state. Expensive — only call off the audio thread.
    // Default: processes NAM_DEFAULT_MAX_BUFFER_SIZE zeros through the model.
    virtual void prewarm();

    // Unified lifecycle entry point. Updates sample rate and max buffer size,
    // then calls prewarm() if mPrewarmOnReset is true.
    // Call in prepareToPlay(); safe to call repeatedly on background thread.
    virtual void Reset(double sampleRate, int maxBufferSize);

    // process() — NAM double-pointer channel layout:
    //   input[channel][frame]   (in_channels × num_frames)
    //   output[channel][frame]  (out_channels × num_frames)
    // For mono ONNX models: in_channels=1, out_channels=1.
    // MUST be called on the audio thread only, after Reset() completes.
    virtual void process(NAM_SAMPLE** input, NAM_SAMPLE** output, int num_frames);

    // ── Level / loudness (mirrors nam::DSP) ──────────────────────────────────
    bool   HasLoudness()     const { return mHasLoudness; }
    double GetLoudness()     const { return mLoudness; }  // dB, throws if !HasLoudness
    bool   HasExpectedSR()   const { return mExpectedSampleRate > 0.0; }
    double GetExpectedSampleRate() const { return mExpectedSampleRate; }

    void SetLoudness(double dB)  { mLoudness = dB; mHasLoudness = true; }
    void SetPrewarmOnReset(bool v) { mPrewarmOnReset.store(v); }
    bool GetPrewarmOnReset()     const { return mPrewarmOnReset.load(); }

    int    GetMaxBufferSize()    const { return mMaxBufferSize; }
    Backend backend()            const { return backend_; }
    const std::string& path()   const { return path_; }

protected:
    bool               mHasLoudness       { false };
    double             mLoudness          { 0.0 };
    double             mExpectedSampleRate;
    double             mExternalSampleRate { -1.0 };
    int                mMaxBufferSize      { 0 };
    std::atomic<bool>  mPrewarmOnReset    { true };

    virtual void SetMaxBufferSize(int maxBufferSize);

private:
    std::string  path_;
    Backend      backend_;

    // ONNX Runtime
    Ort::Env env_;
    std::unique_ptr<Ort::Session> session_;
    std::string inputName_;
    std::string outputName_;
    Ort::MemoryInfo memInfo_;

    // Pre-allocated inference buffers
    std::vector<float> scratchIn_;
    std::vector<float> scratchOut_;

#ifdef MELEGI_USE_TORCHSCRIPT
    torch::jit::script::Module tsModule_;
    bool tsLoaded_ { false };
#endif

    void runONNX(const float* in, float* out, int numSamples);
    void runTorch(const float* in, float* out, int numSamples);
};
