#include "melegi/TimingEngine.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>

namespace melegi {

// ── Seeded LCG-based gaussian (Box-Muller) ─────────────────────────────────
float TimingEngine::gaussianRandom(float mean, float stddev, uint32_t& seed) const {
    // LCG: period 2^32
    seed = seed * 1664525u + 1013904223u;
    float u1 = (seed & 0x7FFFFFFF) / static_cast<float>(0x80000000);
    seed = seed * 1664525u + 1013904223u;
    float u2 = (seed & 0x7FFFFFFF) / static_cast<float>(0x80000000);
    if (u1 < 1e-7f) u1 = 1e-7f;
    float gauss = std::sqrt(-2.f * std::log(u1)) * std::cos(TWO_PI * u2);
    return mean + stddev * gauss;
}

// ── Build beat grid ────────────────────────────────────────────────────────
BeatGrid TimingEngine::buildGrid(float bpm, float durationSec,
                                  int timeSigNum, int timeSigDen) const {
    BeatGrid g;
    g.bpm             = bpm;
    g.halfTimeBPM     = bpm / 2.f;
    g.beatDurationSec = 60.f / bpm;
    g.timeSignatureNum = timeSigNum;
    g.timeSignatureDen = timeSigDen;
    g.durationSec     = durationSec;

    int totalBeats = static_cast<int>(durationSec / g.beatDurationSec) + 1;
    g.beatTimesSeconds.reserve(totalBeats);
    g.barTimesSeconds.reserve(totalBeats / timeSigNum + 1);

    for (int b = 0; b < totalBeats; ++b) {
        float t = b * g.beatDurationSec;
        if (t > durationSec + g.beatDurationSec) break;
        g.beatTimesSeconds.push_back(t);
        if (b % timeSigNum == 0)
            g.barTimesSeconds.push_back(t);
    }
    return g;
}

// ── Extract groove from onsets vs grid ────────────────────────────────────
GrooveMap TimingEngine::extractGroove(const std::vector<float>& onsets,
                                       const BeatGrid& grid) const {
    GrooveMap gm;
    gm.bpm = grid.bpm;
    if (onsets.empty() || grid.beatTimesSeconds.empty()) return gm;

    float beatDur = grid.beatDurationSec;
    std::vector<float> timingOffsets;
    std::vector<float> velocities;

    // For each onset, find nearest beat and measure offset
    for (float t : onsets) {
        float minDist = 1e9f;
        float nearestBeat = 0.f;
        for (float bt : grid.beatTimesSeconds) {
            float d = std::abs(t - bt);
            if (d < minDist) { minDist = d; nearestBeat = bt; }
        }
        float offset = t - nearestBeat;
        // Only include if within half a beat (likely a real correspondence)
        if (std::abs(offset) < beatDur * 0.5f) {
            timingOffsets.push_back(offset);
            velocities.push_back(1.f); // we don't have velocity from audio easily
        }
    }

    gm.offsets    = timingOffsets;
    gm.velocities = velocities;

    // Detect swing: compare even/odd 16th-note IOIs
    if (onsets.size() >= 4) {
        float s16 = beatDur / 4.f;
        float longSum = 0.f, shortSum = 0.f;
        int n = 0;
        for (size_t i = 1; i < onsets.size(); ++i) {
            float ioi = onsets[i] - onsets[i - 1];
            if (ioi > s16 * 0.5f && ioi < s16 * 2.f) {
                if (n % 2 == 0) longSum  += ioi;
                else            shortSum += ioi;
                ++n;
            }
        }
        if (n > 1 && shortSum > 0.f) {
            float ratio = longSum / shortSum;
            // Swing: 0 = straight (ratio 1:1), 1 = max swing (ratio 2:1)
            gm.swingRatio = clamp((ratio - 1.f) / 1.f, 0.f, 1.f);
        }
    }

    // Accent map: downbeats (every timeSigNum beats)
    int sig = grid.timeSignatureNum;
    gm.accents.resize(timingOffsets.size(), false);
    for (size_t i = 0; i < timingOffsets.size(); i += sig)
        gm.accents[i] = true;

    return gm;
}

// ── Generate humanization map ──────────────────────────────────────────────
HumanizationMap TimingEngine::generateHumanizationMap(const BeatGrid& grid,
                                                        const StyleDNA& dna,
                                                        const GrooveMap* ref,
                                                        uint32_t seed) const {
    HumanizationMap hm;
    size_t n = grid.beatTimesSeconds.size();
    if (n == 0) return hm;

    hm.timingOffsetsSec.resize(n);
    hm.velocityScales.resize(n);
    hm.pitchWobbleCents.resize(n);

    // Timing jitter: gaussian with std dev proportional to Humanization
    // Typical feel: Humanization=0.8 → stddev ~10 ms
    float timingStddev = dna.Humanization * 0.015f; // max ~15 ms at H=1

    // Velocity scatter
    float velStddev = dna.Humanization * 0.12f;

    // Pitch wobble: Imperfection drives range (max ±25 cents at 1.0)
    float pitchRange = dna.Imperfection * 25.f;

    // If we have a reference groove, bias offsets toward it
    for (size_t i = 0; i < n; ++i) {
        float refOffset = 0.f;
        if (ref && i < ref->offsets.size())
            refOffset = ref->offsets[i];

        float jitter = gaussianRandom(refOffset, timingStddev, seed);
        // Boom-bap lay-back: negative offset (slightly behind grid)
        jitter -= dna.BoomBapAmount * 0.004f; // up to 4 ms lay-back
        hm.timingOffsetsSec[i] = jitter;

        float vel = gaussianRandom(1.f, velStddev, seed);
        hm.velocityScales[i] = clamp(vel, 0.65f, 1.35f);

        float wobble = gaussianRandom(0.f, pitchRange * 0.33f, seed);
        hm.pitchWobbleCents[i] = clamp(wobble, -pitchRange, pitchRange);
    }

    // Swing: inject half-time feel if BoomBap is strong
    hm.swingAmount = (ref ? ref->swingRatio : 0.f) * 0.5f
                   + dna.BoomBapAmount * 0.3f;

    // Score: standard deviation of offsets relative to timing std dev
    float offsetVar = 0.f;
    for (auto v : hm.timingOffsetsSec) offsetVar += v * v;
    offsetVar /= n;
    hm.humanizationScore = clamp(std::sqrt(offsetVar) / (timingStddev + 1e-6f) * 0.5f,
                                  0.f, 1.f);

    return hm;
}

// ── BPM from inter-onset intervals ────────────────────────────────────────
float TimingEngine::bpmFromOnsets(const std::vector<float>& onsets) const {
    if (onsets.size() < 2) return 122.f;
    float sumIOI = 0.f;
    int n = 0;
    for (size_t i = 1; i < onsets.size(); ++i) {
        float ioi = onsets[i] - onsets[i - 1];
        if (ioi > 0.1f && ioi < 2.5f) { sumIOI += ioi; ++n; }
    }
    if (n == 0) return 122.f;
    float avgIOI = sumIOI / n;
    float bpm = 60.f / avgIOI;
    if (bpm < 80.f) bpm *= 2.f;
    if (bpm > 160.f) bpm /= 2.f;
    return clamp(bpm, 60.f, 200.f);
}

// ── Quantize to grid ───────────────────────────────────────────────────────
std::vector<float> TimingEngine::quantizeToGrid(const std::vector<float>& onsets,
                                                  const BeatGrid& grid,
                                                  float amountFactor) const {
    std::vector<float> q = onsets;
    for (auto& t : q) {
        // Find nearest beat
        float minDist = 1e9f;
        float snap = t;
        for (float bt : grid.beatTimesSeconds) {
            float d = std::abs(t - bt);
            if (d < minDist) { minDist = d; snap = bt; }
        }
        t = t + amountFactor * (snap - t);
    }
    return q;
}

} // namespace melegi
