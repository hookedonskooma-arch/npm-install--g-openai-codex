#include "melegi/ShoegazeEngine.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <random>

namespace melegi {

// ── Hadamard transform (size=8, in-place) ─────────────────────────────────
void ShoegazeEngine::hadamard8(std::array<float, 8>& v) {
    // 3 passes of butterfly
    for (int step = 1; step < 8; step <<= 1) {
        for (int i = 0; i < 8; i += step << 1) {
            for (int j = i; j < i + step; ++j) {
                float a = v[j], b = v[j + step];
                v[j]        = a + b;
                v[j + step] = a - b;
            }
        }
    }
    constexpr float norm = 1.f / 2.8284271f; // 1/sqrt(8)
    for (auto& x : v) x *= norm;
}

float ShoegazeEngine::onePoleLP(float in, float& state, float coeff) {
    state += coeff * (in - state);
    return state;
}

float ShoegazeEngine::readDelayInterp(const std::vector<float>& buf,
                                       int writePos, float delaySamples) {
    int N = static_cast<int>(buf.size());
    int di = static_cast<int>(delaySamples);
    float frac = delaySamples - di;
    int r0 = ((writePos - di)     % N + N) % N;
    int r1 = ((writePos - di - 1) % N + N) % N;
    return buf[r0] * (1.f - frac) + buf[r1] * frac;
}

float ShoegazeEngine::lfo(float phase) {
    return 0.5f + 0.5f * std::sin(phase * TWO_PI);
}

// ── FDN Reverb ────────────────────────────────────────────────────────────
void ShoegazeEngine::applyFDNReverb(AudioBuffer& buf,
                                     float decayAmount,
                                     float wetLevel,
                                     float dampingHz) const {
    if (buf.numFrames == 0 || wetLevel < 0.001f) return;

    SampleRate sr = buf.sampleRate;
    float scaleHz = static_cast<float>(sr) / 44100.f;

    // Scale delay lengths for sample rate
    std::array<int, FDN_SIZE> delays;
    for (int i = 0; i < FDN_SIZE; ++i)
        delays[i] = static_cast<int>(BASE_DELAY_LENGTHS[i] * scaleHz);

    // Compute feedback gain from RT60
    // RT60 in seconds: map decayAmount [0,1] → [0.15, 4.0] s
    float rt60 = 0.15f + decayAmount * 3.85f;

    // Damping one-pole LP coefficient
    float dampCoeff = 1.f - std::exp(-TWO_PI * dampingHz / static_cast<float>(sr));

    std::array<std::vector<float>, FDN_SIZE> delayBufs;
    for (int i = 0; i < FDN_SIZE; ++i)
        delayBufs[i].assign(delays[i], 0.f);

    std::array<int, FDN_SIZE> writePos = {};
    std::array<float, FDN_SIZE> fbGains, dampStates = {};

    for (int i = 0; i < FDN_SIZE; ++i) {
        // Per-delay gain from RT60: g = 10^(-3 * delayTime / RT60)
        float delayTime = delays[i] / static_cast<float>(sr);
        fbGains[i] = std::pow(10.f, -3.f * delayTime / (rt60 + 1e-6f));
        fbGains[i] = clamp(fbGains[i], 0.f, 0.9999f);
    }

    // Process each channel
    for (uint32_t c = 0; c < buf.numChannels; ++c) {
        // Reset delay buffers per channel
        for (auto& db : delayBufs) std::fill(db.begin(), db.end(), 0.f);
        std::fill(writePos.begin(), writePos.end(), 0);
        std::fill(dampStates.begin(), dampStates.end(), 0.f);

        for (uint64_t f = 0; f < buf.numFrames; ++f) {
            float in = buf.at(c, f);

            // Read FDN outputs
            std::array<float, FDN_SIZE> outputs;
            for (int i = 0; i < FDN_SIZE; ++i)
                outputs[i] = delayBufs[i][writePos[i]];

            // Apply Hadamard mixing matrix
            auto mixed = outputs;
            hadamard8(mixed);

            // Feedback + input, write to delay lines with LP damping
            for (int i = 0; i < FDN_SIZE; ++i) {
                float fb = onePoleLP(mixed[i] * fbGains[i], dampStates[i], dampCoeff);
                float toWrite = in * 0.25f + fb;
                delayBufs[i][writePos[i]] = toWrite;
                writePos[i] = (writePos[i] + 1) % delays[i];
            }

            // Sum reverb output
            float wet = 0.f;
            for (auto o : outputs) wet += o;
            wet /= FDN_SIZE;

            // Dry/wet blend
            buf.at(c, f) = in * (1.f - wetLevel) + wet * wetLevel;
        }
    }
}

// ── Tape saturation ────────────────────────────────────────────────────────
void ShoegazeEngine::applyTapeSaturation(AudioBuffer& buf, float drive) const {
    if (drive < 0.01f) return;
    float d = 1.f + drive * 4.f;
    float norm = std::tanh(d);

    for (uint32_t c = 0; c < buf.numChannels; ++c)
        for (uint64_t f = 0; f < buf.numFrames; ++f) {
            Sample& s = buf.at(c, f);
            s = std::tanh(s * d) / norm;
        }
}

// ── Stereo widening (mid-side) ─────────────────────────────────────────────
void ShoegazeEngine::applyStereoWidening(AudioBuffer& buf, float width) const {
    if (buf.numChannels < 2 || width < 0.01f) return;

    float midGain  = 1.f - width * 0.3f;
    float sideGain = 1.f + width;

    for (uint64_t f = 0; f < buf.numFrames; ++f) {
        float L = buf.at(0, f), R = buf.at(1, f);
        float mid  = (L + R) * 0.5f * midGain;
        float side = (L - R) * 0.5f * sideGain;
        buf.at(0, f) = clamp(mid + side, -1.f, 1.f);
        buf.at(1, f) = clamp(mid - side, -1.f, 1.f);
    }
}

// ── Spectral bloom (gentle high-shelf boost before reverb) ────────────────
void ShoegazeEngine::applySpectralBloom(AudioBuffer& buf, float amount) const {
    if (amount < 0.01f) return;
    float sr = static_cast<float>(buf.sampleRate);
    // High-shelf IIR boost at 3 kHz
    float fc     = 3000.f / sr;
    float gainDB = amount * 6.f;  // 0–6 dB bloom
    float A      = std::pow(10.f, gainDB / 40.f);
    float w0     = TWO_PI * fc;
    float cosW   = std::cos(w0);
    float sinW   = std::sin(w0);
    float alpha  = sinW / (2.f * 0.707f);
    float sqA    = std::sqrt(A);

    float b0 =  A * ((A + 1.f) + (A - 1.f) * cosW + 2.f * sqA * alpha);
    float b1 = -2.f * A * ((A - 1.f) + (A + 1.f) * cosW);
    float b2 =  A * ((A + 1.f) + (A - 1.f) * cosW - 2.f * sqA * alpha);
    float a0 =        (A + 1.f) - (A - 1.f) * cosW + 2.f * sqA * alpha;
    float a1 =  2.f * ((A - 1.f) - (A + 1.f) * cosW);
    float a2 =        (A + 1.f) - (A - 1.f) * cosW - 2.f * sqA * alpha;

    b0 /= a0; b1 /= a0; b2 /= a0; a1 /= a0; a2 /= a0;

    for (uint32_t c = 0; c < buf.numChannels; ++c) {
        float x1 = 0.f, x2 = 0.f, y1 = 0.f, y2 = 0.f;
        for (uint64_t f = 0; f < buf.numFrames; ++f) {
            float x = buf.at(c, f);
            float y = b0*x + b1*x1 + b2*x2 - a1*y1 - a2*y2;
            x2 = x1; x1 = x; y2 = y1; y1 = y;
            buf.at(c, f) = y;
        }
    }
}

// ── Wow/flutter ────────────────────────────────────────────────────────────
void ShoegazeEngine::applyWowFlutter(AudioBuffer& buf,
                                      float wowRate,
                                      float wowDepth,
                                      float flutterRate,
                                      float flutterDepth) const {
    SampleRate sr = buf.sampleRate;
    float maxDelay = std::max(wowDepth, flutterDepth) * sr + 4.f;
    int maxD = static_cast<int>(maxDelay) + 4;

    for (uint32_t c = 0; c < buf.numChannels; ++c) {
        std::vector<float> delayBuf(maxD, 0.f);
        int wp = 0;
        float wowPh = c * 0.15f, flutterPh = c * 0.27f;
        float wowInc     = wowRate     / sr;
        float flutterInc = flutterRate / sr;

        for (uint64_t f = 0; f < buf.numFrames; ++f) {
            delayBuf[wp] = buf.at(c, f);

            float wow     = lfo(wowPh) * wowDepth * sr;
            float flutter = lfo(flutterPh) * flutterDepth * sr;
            float delaySamp = wow + flutter + 1.f;

            buf.at(c, f) = readDelayInterp(delayBuf, wp, delaySamp);

            wp = (wp + 1) % maxD;
            wowPh += wowInc; if (wowPh >= 1.f) wowPh -= 1.f;
            flutterPh += flutterInc; if (flutterPh >= 1.f) flutterPh -= 1.f;
        }
    }
}

// ── Vinyl crackle ──────────────────────────────────────────────────────────
void ShoegazeEngine::applyVinylCrackle(AudioBuffer& buf, float amount) const {
    if (amount < 0.001f) return;
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(0.f, 1.f);

    for (uint32_t c = 0; c < buf.numChannels; ++c) {
        for (uint64_t f = 0; f < buf.numFrames; ++f) {
            if (dist(rng) < 0.0008f * amount * 10.f) {
                // Short click burst
                int burstLen = static_cast<int>(dist(rng) * 20.f + 3.f);
                for (int b = 0; b < burstLen && f + b < buf.numFrames; ++b) {
                    float click = (dist(rng) * 2.f - 1.f)
                                * std::exp(-b * 0.4f) * amount * 0.6f;
                    buf.at(c, f + b) += click;
                }
            }
            // Continuous grain noise
            buf.at(c, f) += (dist(rng) * 2.f - 1.f) * amount * 0.006f;
        }
    }
}

// ── Tape hiss ──────────────────────────────────────────────────────────────
void ShoegazeEngine::applyTapeHiss(AudioBuffer& buf, float amount) const {
    if (amount < 0.001f) return;
    std::mt19937 rng(137);
    std::uniform_real_distribution<float> dist(-1.f, 1.f);

    // Filtered white noise (one-pole LP to give 'hiss' colour, not white)
    float lpCoeff = 1.f - std::exp(-TWO_PI * 6000.f / static_cast<float>(buf.sampleRate));

    for (uint32_t c = 0; c < buf.numChannels; ++c) {
        float lpState = 0.f;
        for (uint64_t f = 0; f < buf.numFrames; ++f) {
            float noise = dist(rng) * amount * 0.02f;
            lpState += lpCoeff * (noise - lpState);
            buf.at(c, f) += lpState;
        }
    }
}

// ── Full shoegaze processing pass ──────────────────────────────────────────
AudioBuffer ShoegazeEngine::process(const AudioBuffer& in,
                                     const ShoegazeParams& p) const {
    AudioBuffer out = in;

    // 1. Tape saturation (warmth first, before spectral shaping)
    applyTapeSaturation(out, p.saturationAmount);

    // 2. Spectral bloom (add shimmer before reverb)
    applySpectralBloom(out, p.bloomAmount);

    // 3. FDN reverb (core shoegaze texture)
    applyFDNReverb(out, p.decayAmount, p.hazeAmount, p.dampingFreqHz);

    // 4. Stereo widening
    applyStereoWidening(out, p.widthAmount);

    // 5. Final dry/wet blend
    for (uint32_t c = 0; c < out.numChannels; ++c)
        for (uint64_t f = 0; f < out.numFrames; ++f)
            out.at(c, f) = in.at(c, f) * (1.f - p.dryWetMix)
                         + out.at(c, f) * p.dryWetMix;

    // Prevent clipping
    float pk = out.peakAmplitude();
    if (pk > 0.98f) out.normalize(0.95f);

    return out;
}

} // namespace melegi
