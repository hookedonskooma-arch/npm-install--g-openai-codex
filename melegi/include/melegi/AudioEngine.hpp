#pragma once
#include "Common.hpp"
#include <string>
#include <optional>
#include <functional>

namespace melegi {

// ── WavInfo ────────────────────────────────────────────────────────────────
struct WavInfo {
    uint32_t sampleRate   {0};
    uint16_t numChannels  {0};
    uint16_t bitsPerSample{0};
    uint64_t numFrames    {0};
    uint16_t audioFormat  {0};  // 1=PCM, 3=IEEE float
    float    durationSec  {0.f};
};

// ── AudioEngine ────────────────────────────────────────────────────────────
// Handles audio file I/O and in-memory buffer management.
// Supported: WAV (PCM 16/24/32-bit, float 32-bit). RIFF/WAVE only.
class AudioEngine {
public:
    AudioEngine()  = default;
    ~AudioEngine() = default;

    // Load a WAV file into an AudioBuffer. Returns nullopt on failure.
    // err receives the human-readable error message on failure.
    std::optional<AudioBuffer> load(const std::string& path, std::string& err) const;

    // Inspect a WAV without decoding all samples (fast header read)
    std::optional<WavInfo> inspect(const std::string& path, std::string& err) const;

    // Write an AudioBuffer to disk as PCM WAV.
    // bitDepth: 16 or 24
    bool writePCM(const AudioBuffer& buf,
                  const std::string& path,
                  uint16_t           bitDepth,
                  std::string&       err) const;

    // Write as 32-bit IEEE float WAV
    bool writeFloat(const AudioBuffer& buf,
                    const std::string& path,
                    std::string&       err) const;

    // Generate a test tone (sine wave) for pipeline testing
    AudioBuffer generateTestTone(float   frequencyHz,
                                 float   durationSec,
                                 float   amplitude  = 0.5f,
                                 SampleRate sr      = DEFAULT_SAMPLE_RATE) const;

    // Generate silence
    AudioBuffer generateSilence(float durationSec,
                                uint32_t channels = 2,
                                SampleRate sr = DEFAULT_SAMPLE_RATE) const;

private:
    std::optional<AudioBuffer> decodeWav(const std::string& path, std::string& err) const;

    static void writeLE16(uint8_t* dst, uint16_t v);
    static void writeLE32(uint8_t* dst, uint32_t v);
    static uint16_t readLE16(const uint8_t* src);
    static uint32_t readLE32(const uint8_t* src);
};

} // namespace melegi
