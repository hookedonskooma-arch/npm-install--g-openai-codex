#pragma once
#include "Common.hpp"
#include "StyleDNA.hpp"
#include <vector>

namespace melegi {

// ── BeatGrid ───────────────────────────────────────────────────────────────
struct BeatGrid {
    float    bpm           {122.f};
    float    halfTimeBPM   {61.f};
    float    beatDurationSec{0.f};
    int      timeSignatureNum{4};
    int      timeSignatureDen{4};
    std::vector<float> beatTimesSeconds;  // absolute beat positions
    std::vector<float> barTimesSeconds;
    float    durationSec   {0.f};
};

// ── GrooveMap ──────────────────────────────────────────────────────────────
// Extracted micro-timing offsets relative to the beat grid.
// offsets[i] is the timing offset of the i-th beat in seconds
// (positive = pushed ahead of the grid, negative = laid back).
struct GrooveMap {
    float              bpm       {122.f};
    float              swingRatio{0.f};     // [0=straight, 1=maxSwing]
    std::vector<float> offsets;             // per-beat micro-timing offsets (sec)
    std::vector<float> velocities;          // per-beat relative velocity [0,1]
    std::vector<bool>  accents;             // accent map (downbeats, etc.)
};

// ── HumanizationMap ────────────────────────────────────────────────────────
// Generated timing + velocity scatter for applying to new content.
struct HumanizationMap {
    std::vector<float> timingOffsetsSec;  // gaussian jitter per note/beat
    std::vector<float> velocityScales;   // [0.85, 1.15] range typically
    std::vector<float> pitchWobbleCents; // subtle pitch offset per note (cents)
    float              swingAmount  {0.f};
    float              humanizationScore{0.f}; // how "human" this map is [0,1]
};

// ── TimingEngine ───────────────────────────────────────────────────────────
class TimingEngine {
public:
    TimingEngine() = default;

    // Build a beat grid for a given BPM and duration
    BeatGrid buildGrid(float bpm,
                       float durationSec,
                       int   timeSigNum = 4,
                       int   timeSigDen = 4) const;

    // Extract groove from detected onset times vs the beat grid
    GrooveMap extractGroove(const std::vector<float>& onsetTimesSeconds,
                            const BeatGrid&            grid) const;

    // Generate a humanization map using the StyleDNA and an optional reference groove
    HumanizationMap generateHumanizationMap(const BeatGrid&      grid,
                                            const StyleDNA&       dna,
                                            const GrooveMap*      refGroove = nullptr,
                                            uint32_t              seed = 0) const;

    // Estimate BPM from onset times (alternative to autocorrelation BPM)
    float bpmFromOnsets(const std::vector<float>& onsetTimes) const;

    // Quantize onset times to a grid (returns quantized copies, doesn't modify input)
    std::vector<float> quantizeToGrid(const std::vector<float>& onsets,
                                      const BeatGrid& grid,
                                      float           amountFactor) const;

private:
    float gaussianRandom(float mean, float stddev, uint32_t& seed) const;
};

} // namespace melegi
