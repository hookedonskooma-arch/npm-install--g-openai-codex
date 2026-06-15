#pragma once
#include "Common.hpp"
#include "StyleDNA.hpp"
#include <array>
#include <vector>

namespace melegi {

// ── ShoegazeParams ─────────────────────────────────────────────────────────
struct ShoegazeParams {
    float hazeAmount      {0.6f};  // Reverb wet level [0,1]
    float bloomAmount     {0.4f};  // Pre-reverb high-freq boost amount
    float widthAmount     {0.5f};  // Stereo width [0=mono, 1=wide]
    float saturationAmount{0.3f};  // Tape saturation drive
    float decayAmount     {0.7f};  // FDN reverb decay / RT60 [0=short, 1=long]
    float dampingFreqHz   {4000.f};// High-freq damping cutoff in reverb
    float dryWetMix       {0.35f}; // Overall dry/wet [0=dry, 1=wet]

    // Derive from StyleDNA
    static ShoegazeParams fromDNA(const StyleDNA& dna) {
        return ShoegazeParams{
            .hazeAmount       = dna.ShoegazeAmount * 0.8f,
            .bloomAmount      = dna.ShoegazeAmount * 0.5f,
            .widthAmount      = dna.ShoegazeAmount * 0.6f,
            .saturationAmount = dna.Warmth * 0.5f,
            .decayAmount      = dna.AtmosphericDensity,
            .dampingFreqHz    = 2000.f + (1.f - dna.Darkness) * 4000.f,
            .dryWetMix        = dna.ShoegazeAmount * 0.5f,
        };
    }
};

// ── ShoegazeEngine ─────────────────────────────────────────────────────────
// Responsible for:
//   - FDN (Feedback Delay Network) reverb — 8 delay lines, Hadamard matrix
//   - Tape saturation — tanh waveshaping with harmonic enrichment
//   - Stereo widening — mid-side processing
//   - Spectral bloom — gentle pre-reverb high shelf
//   - Analog drift — wow/flutter modulation
class ShoegazeEngine {
public:
    static constexpr int FDN_SIZE = 8;

    ShoegazeEngine() = default;

    // Full shoegaze processing pass driven by params
    AudioBuffer process(const AudioBuffer& buf, const ShoegazeParams& params) const;

    // Convenience: derive params from DNA
    AudioBuffer processDNA(const AudioBuffer& buf, const StyleDNA& dna) const {
        return process(buf, ShoegazeParams::fromDNA(dna));
    }

    // Individual processors (can be applied in any combination)
    void applyFDNReverb(AudioBuffer& buf,
                        float        decayAmount,    // [0,1]
                        float        wetLevel,       // [0,1]
                        float        dampingHz) const;

    void applyTapeSaturation(AudioBuffer& buf, float drive) const;

    void applyStereoWidening(AudioBuffer& buf, float width) const;

    void applySpectralBloom(AudioBuffer& buf, float amount) const;

    void applyWowFlutter(AudioBuffer& buf,
                         float        wowRate,    // Hz (typ. 0.5–2 Hz)
                         float        wowDepth,   // seconds of delay modulation
                         float        flutterRate,// Hz (typ. 8–14 Hz)
                         float        flutterDepth) const;

    void applyVinylCrackle(AudioBuffer& buf, float amount) const;

    void applyTapeHiss(AudioBuffer& buf, float amount) const;

private:
    // FDN delay lengths (prime numbers at 44100 Hz)
    static constexpr std::array<int, FDN_SIZE> BASE_DELAY_LENGTHS = {
        1237, 1381, 1607, 1777, 1949, 2131, 2311, 2503
    };

    // Fast Walsh-Hadamard transform (in-place, size=8)
    static void hadamard8(std::array<float, 8>& v);

    // One-pole lowpass for damping within FDN
    static float onePoleLP(float in, float& state, float coeff);

    // Linear interpolation for delay modulation
    static float readDelayInterp(const std::vector<float>& buf,
                                 int    writePos,
                                 float  delaySamples);

    // Hann window LFO (bounded smooth oscillator)
    static float lfo(float phase); // phase in [0,1)
};

} // namespace melegi
