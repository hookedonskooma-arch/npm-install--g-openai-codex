#include "melegi/AuthenticityEngine.hpp"
#include "melegi/AnalysisEngine.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <sstream>

namespace melegi {

// ── Pitch wobble (vibrato) via modulated fractional delay ──────────────────
void AuthenticityEngine::vibratolDelay(std::vector<Sample>& ch,
                                        SampleRate sr,
                                        float rateHz,
                                        float depthCents,
                                        float phase) const {
    // Convert depth from cents to seconds of delay
    // 1 cent = 1/1200 of an octave; pitch shift via delay: Δd = Δf * d / f₀
    // Maximum delay modulation for depthCents pitch shift at any f0:
    //   use depthCents / 1200 semitone fraction → max delay ~ 3 ms at 25 cents
    float maxDelaySec = depthCents / 1200.f * 0.003f + 0.001f;
    int   maxDelaySamp = static_cast<int>(maxDelaySec * sr) + 2;

    // Delay line
    std::vector<float> delayLine(maxDelaySamp + 1, 0.f);
    int writePos = 0;

    float phaseInc = rateHz / static_cast<float>(sr);

    for (size_t i = 0; i < ch.size(); ++i) {
        // Write current sample
        delayLine[writePos] = ch[i];

        // LFO: triangle wave for smooth vibrato feel
        float lfoVal = 2.f * std::abs(phase - 0.5f); // [0,1]
        float delaySamp = lfoVal * maxDelaySamp;

        // Fractional delay read with linear interpolation
        int di = static_cast<int>(delaySamp);
        float frac = delaySamp - di;
        int r1 = (writePos - di + maxDelaySamp + 1) % (maxDelaySamp + 1);
        int r2 = (writePos - di - 1 + maxDelaySamp + 1) % (maxDelaySamp + 1);
        ch[i] = delayLine[r1] * (1.f - frac) + delayLine[r2] * frac;

        writePos = (writePos + 1) % (maxDelaySamp + 1);
        phase += phaseInc;
        if (phase >= 1.f) phase -= 1.f;
    }
}

void AuthenticityEngine::applyPitchWobble(AudioBuffer& buf,
                                           float rateHz,
                                           float depthCents) const {
    if (depthCents < 0.5f) return;
    float startPhase = 0.f;
    for (uint32_t c = 0; c < buf.numChannels; ++c) {
        // Offset phase slightly per channel for natural feel
        float ph = startPhase + c * 0.07f;
        vibratolDelay(buf.channels[c], buf.sampleRate, rateHz, depthCents, ph);
    }
}

void AuthenticityEngine::applyTremolo(AudioBuffer& buf,
                                       float rateHz,
                                       float depthNorm) const {
    if (depthNorm < 0.01f) return;
    float phaseInc = rateHz / static_cast<float>(buf.sampleRate);
    float phase = 0.f;
    for (uint64_t f = 0; f < buf.numFrames; ++f) {
        float lfo = 0.5f * (1.f + std::sin(phase * TWO_PI));
        float gain = 1.f - depthNorm * lfo;
        for (uint32_t c = 0; c < buf.numChannels; ++c)
            buf.at(c, f) *= gain;
        phase += phaseInc;
        if (phase >= 1.f) phase -= 1.f;
    }
}

void AuthenticityEngine::preserveTransients(AudioBuffer& buf,
                                             float threshDBFS,
                                             float ratio) const {
    // Envelope follower compressor: fast attack (1 ms), slow release (80 ms)
    float threshLin = dB2lin(threshDBFS);
    float sr = static_cast<float>(buf.sampleRate);
    float attackCoef  = std::exp(-1.f / (0.001f * sr));
    float releaseCoef = std::exp(-1.f / (0.080f * sr));

    for (uint32_t c = 0; c < buf.numChannels; ++c) {
        float env = 0.f;
        for (uint64_t f = 0; f < buf.numFrames; ++f) {
            float absS = std::abs(buf.at(c, f));
            env = absS > env
                ? env + (1.f - attackCoef) * (absS - env)
                : env + (1.f - releaseCoef) * (absS - env);

            if (env > threshLin) {
                float gainRed = threshLin + (env - threshLin) / ratio;
                buf.at(c, f) *= gainRed / (env + 1e-9f);
            }
        }
    }
}

// ── Scoring sub-components ─────────────────────────────────────────────────
float AuthenticityEngine::computeTimingVariance(const AudioBuffer& buf) const {
    // Estimate from amplitude envelope zero-crossing rate variation
    if (!buf.valid()) return 0.f;
    auto mono = buf.toMono();
    // Compute short-term RMS in 20ms windows and measure its variance
    int winSize = static_cast<int>(buf.sampleRate * 0.02f);
    if (winSize < 2) return 0.f;
    std::vector<float> rmsVals;
    for (size_t i = 0; i + winSize < mono.size(); i += winSize) {
        float s = 0.f;
        for (int j = 0; j < winSize; ++j) s += mono[i + j] * mono[i + j];
        rmsVals.push_back(std::sqrt(s / winSize));
    }
    if (rmsVals.empty()) return 0.f;
    float mean = 0.f; for (auto v : rmsVals) mean += v; mean /= rmsVals.size();
    float var  = 0.f; for (auto v : rmsVals) var += (v - mean) * (v - mean);
    var /= rmsVals.size();
    // High variance in RMS envelope suggests dynamic, human performance
    return clamp(std::sqrt(var) / (mean + 1e-6f) * 200.f, 0.f, 100.f);
}

float AuthenticityEngine::computeDynamicVariance(const AudioBuffer& buf) const {
    if (!buf.valid()) return 0.f;
    auto mono = buf.toMono();
    // Inter-frame RMS differences
    int hop = static_cast<int>(buf.sampleRate * 0.05f); // 50ms
    int frame = hop * 2;
    std::vector<float> rms;
    for (size_t i = 0; i + frame < mono.size(); i += hop) {
        float s = 0.f;
        for (int j = 0; j < frame; ++j) s += mono[i+j]*mono[i+j];
        rms.push_back(std::sqrt(s / frame));
    }
    if (rms.size() < 2) return 50.f;
    float diffSum = 0.f;
    for (size_t i = 1; i < rms.size(); ++i)
        diffSum += std::abs(rms[i] - rms[i-1]);
    float avgDiff = diffSum / rms.size();
    float meanRMS = 0.f; for (auto v : rms) meanRMS += v; meanRMS /= rms.size();
    return clamp(avgDiff / (meanRMS + 1e-6f) * 150.f, 0.f, 100.f);
}

float AuthenticityEngine::computeSpectralVariance(const AudioBuffer& buf) const {
    if (!buf.valid() || buf.numFrames < 4096) return 50.f;
    auto mono = buf.toMono();
    AnalysisEngine ae;
    // Measure centroid in two halves, difference = spectral movement
    size_t half = mono.size() / 2;
    std::vector<float> f1(mono.begin(), mono.begin() + half);
    std::vector<float> f2(mono.begin() + half, mono.end());
    float c1 = ae.spectralCentroid(f1, buf.sampleRate);
    float c2 = ae.spectralCentroid(f2, buf.sampleRate);
    float diff = std::abs(c2 - c1) / 1000.f * 100.f;
    return clamp(diff, 0.f, 100.f);
}

float AuthenticityEngine::computeTransientClarity(const AudioBuffer& buf) const {
    if (!buf.valid()) return 0.f;
    auto mono = buf.toMono();
    // Transient clarity: peak / RMS ratio in 5ms attack windows
    int attackWin = static_cast<int>(buf.sampleRate * 0.005f);
    if (attackWin < 2) return 50.f;
    float maxCrest = 0.f;
    for (size_t i = attackWin; i + attackWin < mono.size(); i += attackWin) {
        float pk = 0.f, sq = 0.f;
        for (int j = 0; j < attackWin; ++j) {
            float v = std::abs(mono[i + j]);
            pk = std::max(pk, v);
            sq += v * v;
        }
        float rms = std::sqrt(sq / attackWin);
        if (rms > 0.f) maxCrest = std::max(maxCrest, pk / rms);
    }
    return clamp((maxCrest - 1.f) / 5.f * 100.f, 0.f, 100.f);
}

// ── Score ──────────────────────────────────────────────────────────────────
AuthenticityReport AuthenticityEngine::score(const AudioBuffer& buf) const {
    AuthenticityReport r;
    if (!buf.valid()) {
        r.diagnosis = "Empty buffer — cannot score.";
        return r;
    }

    r.timingVariance   = computeTimingVariance(buf);
    r.dynamicVariance  = computeDynamicVariance(buf);
    r.spectralVariance = computeSpectralVariance(buf);
    r.transientClarity = computeTransientClarity(buf);
    r.pitchVariance    = 50.f; // requires pitch tracking; default neutral

    r.overallScore = 0.25f * r.timingVariance
                   + 0.25f * r.dynamicVariance
                   + 0.20f * r.spectralVariance
                   + 0.20f * r.transientClarity
                   + 0.10f * r.pitchVariance;

    r.passesAuthenticity = r.overallScore >= 50.f;

    std::ostringstream os;
    os << "Authenticity=" << static_cast<int>(r.overallScore) << "/100 | "
       << "Timing=" << static_cast<int>(r.timingVariance) << " "
       << "Dynamic=" << static_cast<int>(r.dynamicVariance) << " "
       << "Spectral=" << static_cast<int>(r.spectralVariance) << " "
       << "Transient=" << static_cast<int>(r.transientClarity);
    if (!r.passesAuthenticity)
        os << " | FAILED AUTHENTICITY CHECK — needs more humanity";
    r.diagnosis = os.str();

    return r;
}

// ── Full process pass ──────────────────────────────────────────────────────
AudioBuffer AuthenticityEngine::process(const AudioBuffer& buf,
                                         const HumanizationMap& /*map*/,
                                         const StyleDNA& dna) const {
    // Copy — we process the copy
    AudioBuffer out = buf;

    // 1. Pitch wobble — rate driven by imperfection, depth by imperfection
    float wobbleRate  = 0.3f + dna.Imperfection * 3.f;    // 0.3–3.3 Hz
    float wobbleDepth = dna.Imperfection * 18.f;           // 0–18 cents
    if (wobbleDepth > 0.5f)
        applyPitchWobble(out, wobbleRate, wobbleDepth);

    // 2. Tremolo — subtle amplitude modulation
    float tremoloRate  = 0.2f + dna.Humanization * 2.f;   // 0.2–2.2 Hz
    float tremoloDepth = dna.Humanization * 0.06f;         // 0–6%
    if (tremoloDepth > 0.005f)
        applyTremolo(out, tremoloRate, tremoloDepth);

    // 3. Transient preservation — protect attack, compress tail lightly
    // Only apply if we have enough emotional intensity
    if (dna.EmotionalIntensity > 0.4f)
        preserveTransients(out, -18.f, 2.5f);

    return out;
}

} // namespace melegi
