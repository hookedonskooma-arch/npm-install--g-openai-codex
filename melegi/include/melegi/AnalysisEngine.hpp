#pragma once
#include "Common.hpp"
#include <vector>
#include <complex>
#include <string>

namespace melegi {

// ── AnalysisResult ─────────────────────────────────────────────────────────
struct BandEnergy {
    float sub    {0.f};  // 20–80 Hz
    float lowMid {0.f};  // 80–300 Hz
    float mid    {0.f};  // 300–2000 Hz
    float hiMid  {0.f};  // 2000–6000 Hz
    float air    {0.f};  // 6000–20000 Hz
};

struct AnalysisResult {
    // Temporal
    float estimatedBPM    {0.f};
    float bpmConfidence   {0.f};  // [0,1]

    // Spectral
    float spectralCentroid{0.f};  // Hz — weighted mean frequency
    BandEnergy bandEnergy;
    float dominantKeyHz   {0.f};  // Hz of detected fundamental

    // Loudness / dynamics
    float rmsDBFS         {-60.f};
    float peakDBFS        {-60.f};
    float estimatedLUFS   {-60.f};
    float dynamicRangeLU  {0.f};

    // Performance
    std::vector<float> onsetTimesSeconds; // detected onset positions
    float grooveSwingRatio{0.f};          // [0=straight, 1=full swing]

    // Drift from GROUXX fingerprint
    float centroidDriftPct{0.f};  // % deviation from 757 Hz
    bool  onTarget        {false};// centroid within ±50 Hz of 757 Hz

    std::string summary() const;
};

// ── AnalysisEngine ─────────────────────────────────────────────────────────
class AnalysisEngine {
public:
    // Frame size and hop for STFT analysis
    static constexpr int FFT_SIZE  = 4096;
    static constexpr int HOP_SIZE  = 512;
    static constexpr float TARGET_CENTROID_HZ = 757.f;
    static constexpr float CENTROID_TOLERANCE_HZ = 50.f;

    AnalysisEngine() = default;

    // Full analysis pass — runs everything
    AnalysisResult analyze(const AudioBuffer& buf) const;

    // Component-level access
    float         estimateBPM(const std::vector<float>& mono, SampleRate sr) const;
    float         spectralCentroid(const std::vector<float>& mono, SampleRate sr) const;
    BandEnergy    measureBandEnergy(const std::vector<float>& mono, SampleRate sr) const;
    float         measureRMS(const std::vector<float>& samples) const;
    float         measurePeak(const std::vector<float>& samples) const;
    float         estimateLUFS(const std::vector<float>& samples, SampleRate sr) const;
    std::vector<float> detectOnsets(const std::vector<float>& mono, SampleRate sr) const;
    float         estimateSwing(const std::vector<float>& onsetTimes, float bpm) const;

    // FFT (in-place Cooley-Tukey radix-2 DIT)
    static void fft(std::vector<std::complex<float>>& x);
    static void ifft(std::vector<std::complex<float>>& x);

private:
    // Hann window of length n
    static std::vector<float> hannWindow(int n);

    // Compute magnitude spectrum of one frame
    std::vector<float> magnitudeSpectrum(const std::vector<float>& frame) const;

    // Spectral flux (positive only) between consecutive frames
    float spectralFlux(const std::vector<float>& prev,
                       const std::vector<float>& curr) const;

    // Autocorrelation of a 1-D signal
    std::vector<float> autocorrelate(const std::vector<float>& signal,
                                     int maxLag) const;
};

} // namespace melegi
