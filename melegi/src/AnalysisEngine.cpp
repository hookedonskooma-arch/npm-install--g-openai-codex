#include "melegi/AnalysisEngine.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <sstream>

namespace melegi {

// ── FFT — Cooley-Tukey radix-2 DIT (in-place) ─────────────────────────────
void AnalysisEngine::fft(std::vector<std::complex<float>>& x) {
    const size_t N = x.size();
    if (N <= 1) return;

    // Bit-reversal permutation
    for (size_t i = 1, j = 0; i < N; ++i) {
        size_t bit = N >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(x[i], x[j]);
    }

    // Butterfly passes
    for (size_t len = 2; len <= N; len <<= 1) {
        float ang = -TWO_PI / static_cast<float>(len);
        std::complex<float> wlen(std::cos(ang), std::sin(ang));
        for (size_t i = 0; i < N; i += len) {
            std::complex<float> w(1.f, 0.f);
            for (size_t j = 0; j < len / 2; ++j) {
                auto u = x[i + j];
                auto v = x[i + j + len / 2] * w;
                x[i + j]           = u + v;
                x[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}

void AnalysisEngine::ifft(std::vector<std::complex<float>>& x) {
    for (auto& c : x) c = std::conj(c);
    fft(x);
    float inv = 1.f / static_cast<float>(x.size());
    for (auto& c : x) c = std::conj(c) * inv;
}

// ── Hann window ────────────────────────────────────────────────────────────
std::vector<float> AnalysisEngine::hannWindow(int n) {
    std::vector<float> w(n);
    for (int i = 0; i < n; ++i)
        w[i] = 0.5f * (1.f - std::cos(TWO_PI * i / (n - 1)));
    return w;
}

// ── Magnitude spectrum of one windowed frame ───────────────────────────────
std::vector<float> AnalysisEngine::magnitudeSpectrum(const std::vector<float>& frame) const {
    int N = static_cast<int>(frame.size());
    // Zero-pad to next power of two
    int fftN = 1; while (fftN < N) fftN <<= 1;

    auto win = hannWindow(N);
    std::vector<std::complex<float>> cx(fftN, 0.f);
    for (int i = 0; i < N; ++i)
        cx[i] = frame[i] * win[i];

    fft(cx);

    // Return one-sided magnitude
    std::vector<float> mag(fftN / 2);
    for (int i = 0; i < fftN / 2; ++i)
        mag[i] = std::abs(cx[i]);
    return mag;
}

// ── Spectral flux (half-wave rectified) ────────────────────────────────────
float AnalysisEngine::spectralFlux(const std::vector<float>& prev,
                                    const std::vector<float>& curr) const {
    float flux = 0.f;
    size_t n = std::min(prev.size(), curr.size());
    for (size_t i = 0; i < n; ++i) {
        float diff = curr[i] - prev[i];
        if (diff > 0.f) flux += diff;
    }
    return flux;
}

// ── Autocorrelation ────────────────────────────────────────────────────────
std::vector<float> AnalysisEngine::autocorrelate(const std::vector<float>& sig,
                                                   int maxLag) const {
    int N = static_cast<int>(sig.size());
    maxLag = std::min(maxLag, N - 1);
    std::vector<float> acf(maxLag + 1, 0.f);
    float norm = 0.f;
    for (auto s : sig) norm += s * s;
    if (norm < 1e-10f) return acf;

    for (int lag = 0; lag <= maxLag; ++lag) {
        float acc = 0.f;
        for (int i = 0; i < N - lag; ++i)
            acc += sig[i] * sig[i + lag];
        acf[lag] = acc / norm;
    }
    return acf;
}

// ── Onset detection ────────────────────────────────────────────────────────
std::vector<float> AnalysisEngine::detectOnsets(const std::vector<float>& mono,
                                                  SampleRate sr) const {
    int N = static_cast<int>(mono.size());
    if (N < FFT_SIZE) return {};

    std::vector<float> prevMag;
    std::vector<float> onsetStrength;
    std::vector<float> frameTimes;

    auto win = hannWindow(FFT_SIZE);

    for (int hop = 0; hop + FFT_SIZE <= N; hop += HOP_SIZE) {
        std::vector<float> frame(mono.begin() + hop, mono.begin() + hop + FFT_SIZE);
        auto mag = magnitudeSpectrum(frame);

        float flux = prevMag.empty() ? 0.f : spectralFlux(prevMag, mag);
        onsetStrength.push_back(flux);
        frameTimes.push_back(static_cast<float>(hop + FFT_SIZE / 2) / static_cast<float>(sr));
        prevMag = std::move(mag);
    }

    if (onsetStrength.empty()) return {};

    // Median subtraction (remove DC trend from onset signal)
    auto sorted = onsetStrength;
    std::sort(sorted.begin(), sorted.end());
    float median = sorted[sorted.size() / 2];
    for (auto& v : onsetStrength) v = std::max(0.f, v - median * 0.9f);

    // Adaptive threshold: mean + 0.5 std dev
    float mean = 0.f, var = 0.f;
    for (auto v : onsetStrength) mean += v;
    mean /= onsetStrength.size();
    for (auto v : onsetStrength) var += (v - mean) * (v - mean);
    var /= onsetStrength.size();
    float thresh = mean + 0.5f * std::sqrt(var);

    // Peak-pick with minimum distance (0.1 s between onsets)
    float minDistSec = 0.1f;
    (void)minDistSec; // used conceptually above — onset min gap enforced via lastOnset

    std::vector<float> onsets;
    float lastOnset = -1.f;
    for (size_t i = 1; i + 1 < onsetStrength.size(); ++i) {
        if (onsetStrength[i] > thresh &&
            onsetStrength[i] >= onsetStrength[i - 1] &&
            onsetStrength[i] >= onsetStrength[i + 1]) {
            float t = frameTimes[i];
            if (lastOnset < 0.f || (t - lastOnset) >= minDistSec) {
                onsets.push_back(t);
                lastOnset = t;
            }
        }
    }
    return onsets;
}

// ── BPM estimation via autocorrelation of onset strength ───────────────────
float AnalysisEngine::estimateBPM(const std::vector<float>& mono, SampleRate sr) const {
    // Build onset strength signal first
    int N = static_cast<int>(mono.size());
    if (N < FFT_SIZE * 4) return 122.f; // not enough data

    std::vector<float> prevMag;
    std::vector<float> onsetStrength;

    for (int hop = 0; hop + FFT_SIZE <= N; hop += HOP_SIZE) {
        std::vector<float> frame(mono.begin() + hop, mono.begin() + hop + FFT_SIZE);
        auto mag = magnitudeSpectrum(frame);
        float flux = prevMag.empty() ? 0.f : spectralFlux(prevMag, mag);
        onsetStrength.push_back(flux);
        prevMag = std::move(mag);
    }

    // Autocorrelate: BPM range 60–200
    // At 60 BPM, period = 60/60 * sr / HOP_SIZE frames
    // At 200 BPM, period = 60/200 * sr / HOP_SIZE frames
    float framesPerSec = static_cast<float>(sr) / HOP_SIZE;
    int maxLag = static_cast<int>(framesPerSec);  // 60 BPM = 1 beat/sec
    int minLag = static_cast<int>(framesPerSec * 60.f / 200.f);

    auto acf = autocorrelate(onsetStrength, maxLag);

    // Find peak in valid range
    int bestLag = minLag;
    float bestVal = 0.f;
    for (int lag = minLag; lag <= maxLag && lag < static_cast<int>(acf.size()); ++lag) {
        if (acf[lag] > bestVal) {
            bestVal = acf[lag];
            bestLag = lag;
        }
    }

    float bpm = 60.f * framesPerSec / static_cast<float>(bestLag);

    // Snap to common BPM values (avoid 61 when it should be 122 etc.)
    if (bpm < 80.f && bpm > 40.f)  bpm *= 2.f;
    if (bpm > 160.f)                bpm /= 2.f;

    return std::max(60.f, std::min(200.f, bpm));
}

// ── Spectral centroid ──────────────────────────────────────────────────────
float AnalysisEngine::spectralCentroid(const std::vector<float>& mono, SampleRate sr) const {
    int N = static_cast<int>(mono.size());
    int fftN = FFT_SIZE;
    // Use first full frame centroid (representative)
    if (N < fftN) return 0.f;

    auto win = hannWindow(fftN);
    std::vector<std::complex<float>> cx(fftN, 0.f);
    for (int i = 0; i < fftN; ++i) cx[i] = mono[i] * win[i];
    fft(cx);

    float binWidth = static_cast<float>(sr) / fftN;
    float weightedSum = 0.f, totalMag = 0.f;
    for (int i = 1; i < fftN / 2; ++i) {
        float mag = std::abs(cx[i]);
        weightedSum += i * binWidth * mag;
        totalMag    += mag;
    }
    return totalMag > 0.f ? weightedSum / totalMag : 0.f;
}

// ── Band energy ────────────────────────────────────────────────────────────
BandEnergy AnalysisEngine::measureBandEnergy(const std::vector<float>& mono,
                                              SampleRate sr) const {
    int N = std::min(static_cast<int>(mono.size()), FFT_SIZE);
    auto win = hannWindow(N);
    std::vector<std::complex<float>> cx(FFT_SIZE, 0.f);
    for (int i = 0; i < N; ++i) cx[i] = mono[i] * win[i];
    fft(cx);

    float binWidth = static_cast<float>(sr) / FFT_SIZE;
    BandEnergy be;
    float total = 0.f;
    for (int i = 1; i < FFT_SIZE / 2; ++i) {
        float mag = std::abs(cx[i]);
        float mag2 = mag * mag;
        float freq = i * binWidth;
        if      (freq <   80.f) { be.sub    += mag2; }
        else if (freq <  300.f) { be.lowMid += mag2; }
        else if (freq < 2000.f) { be.mid    += mag2; }
        else if (freq < 6000.f) { be.hiMid  += mag2; }
        else                    { be.air    += mag2; }
        total += mag2;
    }
    if (total > 0.f) {
        be.sub /= total; be.lowMid /= total; be.mid /= total;
        be.hiMid /= total; be.air /= total;
    }
    return be;
}

// ── RMS, Peak, LUFS ────────────────────────────────────────────────────────
float AnalysisEngine::measureRMS(const std::vector<float>& s) const {
    if (s.empty()) return 0.f;
    float sum = 0.f;
    for (auto v : s) sum += v * v;
    return std::sqrt(sum / s.size());
}

float AnalysisEngine::measurePeak(const std::vector<float>& s) const {
    float pk = 0.f;
    for (auto v : s) pk = std::max(pk, std::abs(v));
    return pk;
}

float AnalysisEngine::estimateLUFS(const std::vector<float>& s, SampleRate sr) const {
    // K-weighted approximation: apply a pre-filter (high-shelf +4dB at 1681 Hz)
    // then high-pass at 38 Hz, then measure mean square gated at –70 LUFS
    // For simplicity: use an IIR pre-filter then compute mean square
    if (s.empty()) return -60.f;

    // Pre-filter: simplified K-weighting (one-pole high-shelf approx)
    float shelf_coeff = 1.f - std::exp(-TWO_PI * 1500.f / static_cast<float>(sr));
    std::vector<float> filtered(s.size());
    float hpState = 0.f;
    for (size_t i = 0; i < s.size(); ++i) {
        hpState += shelf_coeff * (s[i] - hpState);
        filtered[i] = s[i] + 0.5f * hpState; // approx +4 dB shelf
    }

    // Integrated loudness with gating (–70 LUFS ungated threshold)
    float sum = 0.f;
    size_t cnt = 0;
    float gate_lin = std::pow(10.f, -70.f / 10.f);
    for (auto v : filtered) {
        float p = v * v;
        if (p >= gate_lin) { sum += p; ++cnt; }
    }
    if (cnt == 0) return -60.f;
    float meanSq = sum / cnt;
    return -0.691f + 10.f * std::log10(meanSq + 1e-30f);
}

// ── Swing estimation ───────────────────────────────────────────────────────
float AnalysisEngine::estimateSwing(const std::vector<float>& onsets, float bpm) const {
    if (onsets.size() < 4) return 0.f;
    float beatDur = 60.f / bpm;
    float eighthDur = beatDur / 2.f;

    // Measure ratio of short/long 8th-note durations
    float swingSum = 0.f; int n = 0;
    for (size_t i = 1; i < onsets.size(); ++i) {
        float ioi = onsets[i] - onsets[i - 1];
        // Expect IOI near eighthDur
        if (ioi > eighthDur * 0.4f && ioi < eighthDur * 1.6f) {
            float swing = ioi / eighthDur;
            swingSum += (swing - 1.f);
            ++n;
        }
    }
    return n > 0 ? clamp(swingSum / n, 0.f, 1.f) : 0.f;
}

// ── Full analysis ──────────────────────────────────────────────────────────
AnalysisResult AnalysisEngine::analyze(const AudioBuffer& buf) const {
    AnalysisResult r;
    if (!buf.valid()) return r;

    auto mono = buf.toMono();
    SampleRate sr = buf.sampleRate;

    r.estimatedBPM     = estimateBPM(mono, sr);
    r.spectralCentroid = spectralCentroid(mono, sr);
    r.bandEnergy       = measureBandEnergy(mono, sr);

    float rms = measureRMS(mono);
    r.rmsDBFS    = lin2dB(rms);
    r.peakDBFS   = lin2dB(measurePeak(mono));
    r.estimatedLUFS = estimateLUFS(mono, sr);
    r.dynamicRangeLU = r.estimatedLUFS - r.rmsDBFS;

    r.onsetTimesSeconds = detectOnsets(mono, sr);
    r.grooveSwingRatio  = estimateSwing(r.onsetTimesSeconds, r.estimatedBPM);

    r.bpmConfidence = (r.estimatedBPM > 60.f && r.estimatedBPM < 200.f
                       && !r.onsetTimesSeconds.empty()) ? 0.8f : 0.4f;

    r.centroidDriftPct = std::abs(r.spectralCentroid - TARGET_CENTROID_HZ) / TARGET_CENTROID_HZ * 100.f;
    r.onTarget = std::abs(r.spectralCentroid - TARGET_CENTROID_HZ) <= CENTROID_TOLERANCE_HZ;

    return r;
}

std::string AnalysisResult::summary() const {
    std::ostringstream s;
    s << "BPM=" << estimatedBPM
      << " Centroid=" << spectralCentroid << "Hz"
      << " LUFS=" << estimatedLUFS
      << " Peak=" << peakDBFS << "dBFS"
      << " Onsets=" << onsetTimesSeconds.size()
      << " DriftPct=" << centroidDriftPct
      << " OnTarget=" << (onTarget ? "YES" : "NO");
    return s.str();
}

} // namespace melegi
