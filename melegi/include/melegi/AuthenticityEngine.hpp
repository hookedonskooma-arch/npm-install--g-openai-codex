#pragma once
#include "Common.hpp"
#include "StyleDNA.hpp"
#include "TimingEngine.hpp"
#include <vector>

namespace melegi {

// ── AuthenticityReport ─────────────────────────────────────────────────────
struct AuthenticityReport {
    float overallScore    {0.f};   // [0,100]

    // Sub-scores [0,100]
    float timingVariance  {0.f};   // micro-timing irregularity (higher=more human)
    float dynamicVariance {0.f};   // amplitude fluctuation
    float spectralVariance{0.f};   // spectral variation over time
    float transientClarity{0.f};   // attack sharpness preserved
    float pitchVariance   {0.f};   // subtle intonation movement

    bool  passesAuthenticity{false};  // score >= 50
    std::string diagnosis;
};

// ── AuthenticityEngine ─────────────────────────────────────────────────────
// Preserves humanity.
// NEVER over-quantize. NEVER remove natural breaths.
// NEVER normalize emotional dynamics to a flat line.
// Authenticity wins over technical perfection — always.
class AuthenticityEngine {
public:
    AuthenticityEngine() = default;

    // Score an existing audio buffer for human authenticity
    AuthenticityReport score(const AudioBuffer& buf) const;

    // Apply humanization to an AudioBuffer:
    //   - Pitch wobble (subtle vibrato via interpolated delay)
    //   - Volume tremolo (LFO-driven amplitude variation)
    //   - Transient preservation (soft-knee compression that spares attacks)
    // Returns the processed buffer. Preserves all natural imperfections already present.
    AudioBuffer process(const AudioBuffer&      buf,
                        const HumanizationMap&  map,
                        const StyleDNA&         dna) const;

    // Apply pitch wobble to a buffer in-place.
    // rate: LFO rate in Hz (e.g. 0.3–6 Hz)
    // depth: maximum pitch deviation in cents
    void applyPitchWobble(AudioBuffer& buf,
                          float        rateHz,
                          float        depthCents) const;

    // Apply volume tremolo (LFO-modulated gain) in-place.
    void applyTremolo(AudioBuffer& buf,
                      float        rateHz,
                      float        depthNormalized) const;

    // Soft-knee transient preservation compressor.
    // attack > release: spares the transient, compresses the tail.
    void preserveTransients(AudioBuffer& buf,
                            float        thresholdDBFS = -18.f,
                            float        ratioPostAttack = 2.5f) const;

private:
    // Vibrato using an interpolated delay line
    void vibratolDelay(std::vector<Sample>& channel,
                       SampleRate           sr,
                       float                rateHz,
                       float                depthCents,
                       float                phase) const;

    float computeTimingVariance(const AudioBuffer& buf) const;
    float computeDynamicVariance(const AudioBuffer& buf) const;
    float computeSpectralVariance(const AudioBuffer& buf) const;
    float computeTransientClarity(const AudioBuffer& buf) const;
};

} // namespace melegi
