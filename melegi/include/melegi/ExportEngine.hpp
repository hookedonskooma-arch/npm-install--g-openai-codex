#pragma once
#include "Common.hpp"
#include "StyleDNA.hpp"
#include "AnalysisEngine.hpp"
#include <string>
#include <optional>
#include <functional>

namespace melegi {

// ── ExportTarget ──────────────────────────────────────────────────────────
enum class ExportTarget {
    BeatStars,   // –10 LUFS, 44100 Hz, 16-bit, TPDF dither
    SunoRef,     // –14 LUFS, 44100 Hz, 16-bit
    LogicImport, // –18 LUFS, 48000 Hz, 24-bit, no limiter
    Archive,     // –14 LUFS, 48000 Hz, 32-bit float
};

// ── ExportConfig ──────────────────────────────────────────────────────────
struct ExportConfig {
    ExportTarget target        {ExportTarget::BeatStars};
    std::string  outputPath;     // including filename
    std::string  projectName    {"MELEGI"};
    std::string  key            {"Fm"};
    float        bpm            {122.f};
    bool         dither         {true};   // TPDF dither for 16-bit
    bool         applyLimiter   {true};
    float        limiterCeiling {-1.f};   // dBFS true peak
    bool         exportMidi     {false};  // export .mid alongside WAV
    bool         exportMetaJson {false};  // export .json sidecar

    // Resolved from target:
    float        targetLUFS     {-10.f};
    uint32_t     sampleRate     {44100};
    uint16_t     bitDepth       {16};
    bool         floatFormat    {false};  // true for Archive target
};

// ── ExportResult ──────────────────────────────────────────────────────────
struct ExportResult {
    bool        success       {false};
    std::string outputPath;
    float       measuredLUFS  {-60.f};
    float       measuredPeak  {-120.f};
    uint64_t    numFrames     {0};
    uint32_t    sampleRate    {0};
    uint16_t    bitDepth      {0};
    std::string error;

    float fileSizeKB{0.f};
    std::string filename() const;
};

// ── ExportEngine ──────────────────────────────────────────────────────────
// Handles the full export pipeline:
//   1. LUFS measurement
//   2. Gain normalization to target LUFS
//   3. True peak limiting (hardcoded –1 dBFS ceiling)
//   4. TPDF dither for 16-bit targets
//   5. Sample rate conversion (simple linear interpolation)
//   6. WAV file write
//   7. JSON metadata sidecar
class ExportEngine {
public:
    ExportEngine() = default;

    // Resolve ExportConfig from target (fills sampleRate, bitDepth, targetLUFS)
    static ExportConfig resolveConfig(ExportTarget target,
                                      const std::string& basePath,
                                      const std::string& projectName,
                                      const std::string& key,
                                      float bpm);

    // Full export pipeline
    ExportResult exportAudio(const AudioBuffer& buf,
                             ExportConfig        cfg,
                             const StyleDNA&     dna,
                             const AnalysisResult* analysis = nullptr) const;

    // Export with progress callback (0.0–1.0)
    ExportResult exportAudioWithProgress(
        const AudioBuffer& buf,
        ExportConfig       cfg,
        const StyleDNA&    dna,
        std::function<void(float)> progressCb) const;

    // Write JSON sidecar alongside WAV
    bool writeMetaJson(const ExportResult&   result,
                       const ExportConfig&   cfg,
                       const StyleDNA&       dna,
                       const AnalysisResult* analysis,
                       std::string&          err) const;

private:
    // LUFS-normalize: compute integrated loudness, apply gain
    AudioBuffer normalizeLUFS(const AudioBuffer& buf,
                               float              targetLUFS) const;

    // True-peak brickwall limiter
    AudioBuffer applyLimiter(const AudioBuffer& buf,
                              float              ceilingDBFS) const;

    // TPDF dither (triangular probability density function)
    void applyTPDFDither(std::vector<Sample>& channel,
                         int                  bitDepth) const;

    // Simple linear interpolation SRC
    AudioBuffer resample(const AudioBuffer& buf, uint32_t targetSR) const;

    // Write RIFF/WAVE to file
    bool writeWav(const AudioBuffer& buf,
                  const std::string& path,
                  uint16_t           bitDepth,
                  bool               floatFormat,
                  bool               dither,
                  std::string&       err) const;
};

} // namespace melegi
