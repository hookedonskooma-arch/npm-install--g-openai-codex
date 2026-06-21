#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <cassert>

namespace melegi {

using Sample    = float;
using SampleRate = uint32_t;

static constexpr SampleRate DEFAULT_SAMPLE_RATE = 44100;
static constexpr float      TWO_PI = 6.28318530718f;

// ── AudioBuffer ────────────────────────────────────────────────────────────
// Interleaved-free: channels[c][frame]
struct AudioBuffer {
    std::vector<std::vector<Sample>> channels;
    SampleRate  sampleRate  {DEFAULT_SAMPLE_RATE};
    uint32_t    numChannels {0};
    uint64_t    numFrames   {0};

    AudioBuffer() = default;
    AudioBuffer(uint32_t ch, uint64_t frames, SampleRate sr = DEFAULT_SAMPLE_RATE)
        : channels(ch, std::vector<Sample>(frames, 0.f))
        , sampleRate(sr), numChannels(ch), numFrames(frames) {}

    Sample&       at(uint32_t ch, uint64_t frame)       { return channels[ch][frame]; }
    const Sample& at(uint32_t ch, uint64_t frame) const { return channels[ch][frame]; }

    bool  valid()           const { return numChannels > 0 && numFrames > 0; }
    float durationSeconds() const { return static_cast<float>(numFrames) / static_cast<float>(sampleRate); }

    void resize(uint32_t ch, uint64_t frames, SampleRate sr = DEFAULT_SAMPLE_RATE) {
        numChannels = ch;
        numFrames   = frames;
        sampleRate  = sr;
        channels.assign(ch, std::vector<Sample>(frames, 0.f));
    }

    // Mix src into this buffer at frameOffset, respecting channel count
    void mixIn(const AudioBuffer& src, uint64_t frameOffset = 0, float gain = 1.f) {
        uint32_t ch = std::min(numChannels, src.numChannels);
        for (uint32_t c = 0; c < ch; ++c) {
            for (uint64_t f = 0; f < src.numFrames; ++f) {
                uint64_t dst = frameOffset + f;
                if (dst < numFrames)
                    channels[c][dst] += src.channels[c][f] * gain;
            }
        }
    }

    // Return mono mix of all channels
    std::vector<Sample> toMono() const {
        if (numChannels == 0) return {};
        std::vector<Sample> mono(numFrames, 0.f);
        float inv = 1.f / static_cast<float>(numChannels);
        for (uint32_t c = 0; c < numChannels; ++c)
            for (uint64_t f = 0; f < numFrames; ++f)
                mono[f] += channels[c][f] * inv;
        return mono;
    }

    // Peak amplitude across all channels
    float peakAmplitude() const {
        float peak = 0.f;
        for (auto& ch : channels)
            for (auto s : ch)
                peak = std::max(peak, std::abs(s));
        return peak;
    }

    // Normalize to target peak
    void normalize(float targetPeak = 0.9f) {
        float peak = peakAmplitude();
        if (peak < 1e-9f) return;
        float gain = targetPeak / peak;
        for (auto& ch : channels)
            for (auto& s : ch)
                s *= gain;
    }
};

// ── MidiNote ───────────────────────────────────────────────────────────────
struct MidiNote {
    uint8_t  pitch     {60};   // C4
    uint8_t  velocity  {80};
    double   startBeat {0.0};
    double   durBeats  {1.0};

    float frequencyHz() const {
        return 440.f * std::pow(2.f, (pitch - 69) / 12.f);
    }
};

// ── Simple fixed-size ring buffer ──────────────────────────────────────────
template<typename T>
class RingBuffer {
public:
    explicit RingBuffer(size_t cap) : buf_(cap), cap_(cap) {}

    void push(T val) {
        buf_[writePos_ % cap_] = val;
        ++writePos_;
        if (size_ < cap_) ++size_;
    }

    T operator[](size_t i) const { // 0 = oldest
        size_t start = (writePos_ - size_) % cap_;
        return buf_[(start + i) % cap_];
    }

    size_t size() const { return size_; }
    bool   full()  const { return size_ == cap_; }
    void   clear() { size_ = 0; writePos_ = 0; }

private:
    std::vector<T> buf_;
    size_t         cap_      {0};
    size_t         writePos_ {0};
    size_t         size_     {0};
};

// ── Clamp / dB helpers ─────────────────────────────────────────────────────
inline float clamp(float v, float lo, float hi) { return std::max(lo, std::min(hi, v)); }
inline float lin2dB(float lin) { return lin > 0.f ? 20.f * std::log10(lin) : -120.f; }
inline float dB2lin(float dB)  { return std::pow(10.f, dB / 20.f); }

} // namespace melegi
