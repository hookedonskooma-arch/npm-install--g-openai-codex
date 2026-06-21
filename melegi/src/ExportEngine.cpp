#include "melegi/ExportEngine.hpp"
#include "melegi/AudioEngine.hpp"
#include <fstream>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <random>

namespace melegi {

// ── Resolve config from target ─────────────────────────────────────────────
ExportConfig ExportEngine::resolveConfig(ExportTarget target,
                                          const std::string& basePath,
                                          const std::string& projectName,
                                          const std::string& key,
                                          float bpm) {
    ExportConfig cfg;
    cfg.target      = target;
    cfg.projectName = projectName;
    cfg.key         = key;
    cfg.bpm         = bpm;

    // Generate filename
    std::time_t now = std::time(nullptr);
    char ts[20]; std::strftime(ts, sizeof(ts), "%Y%m%d_%H%M%S", std::gmtime(&now));

    const char* targetStr = "";
    switch (target) {
        case ExportTarget::BeatStars:
            cfg.targetLUFS = -10.f; cfg.sampleRate = 44100; cfg.bitDepth = 16;
            cfg.dither = true; cfg.applyLimiter = true;
            cfg.floatFormat = false; targetStr = "BeatStars"; break;
        case ExportTarget::SunoRef:
            cfg.targetLUFS = -14.f; cfg.sampleRate = 44100; cfg.bitDepth = 16;
            cfg.dither = true; cfg.applyLimiter = true;
            cfg.floatFormat = false; targetStr = "SunoRef"; break;
        case ExportTarget::LogicImport:
            cfg.targetLUFS = -18.f; cfg.sampleRate = 48000; cfg.bitDepth = 24;
            cfg.dither = false; cfg.applyLimiter = false;
            cfg.floatFormat = false; targetStr = "Logic"; break;
        case ExportTarget::Archive:
            cfg.targetLUFS = -14.f; cfg.sampleRate = 48000; cfg.bitDepth = 32;
            cfg.dither = false; cfg.applyLimiter = true;
            cfg.floatFormat = true; targetStr = "Archive"; break;
    }

    std::ostringstream fn;
    fn << basePath << "/" << projectName << "_" << targetStr
       << "_" << static_cast<int>(bpm) << "bpm_" << key << "_" << ts << ".wav";
    cfg.outputPath = fn.str();
    return cfg;
}

// ── LUFS normalization ──────────────────────────────────────────────────────
AudioBuffer ExportEngine::normalizeLUFS(const AudioBuffer& buf, float targetLUFS) const {
    AnalysisEngine ae;
    auto mono = buf.toMono();
    float measuredLUFS = ae.estimateLUFS(mono, buf.sampleRate);
    if (!std::isfinite(measuredLUFS) || measuredLUFS < -80.f) return buf;

    float gainDB = targetLUFS - measuredLUFS;
    float gainLin = dB2lin(gainDB);
    gainLin = clamp(gainLin, 0.05f, 10.f); // Clamp extreme gains

    AudioBuffer out = buf;
    for (auto& ch : out.channels)
        for (auto& s : ch)
            s *= gainLin;
    return out;
}

// ── True-peak brickwall limiter ─────────────────────────────────────────────
AudioBuffer ExportEngine::applyLimiter(const AudioBuffer& buf, float ceilingDBFS) const {
    float ceil = dB2lin(ceilingDBFS);
    AudioBuffer out = buf;

    // Simple inter-sample peak limiting:
    // 4x oversampling check using linear interpolation
    for (uint32_t c = 0; c < out.numChannels; ++c) {
        float prev = 0.f;
        float attack  = std::exp(-1.f / (0.0001f * out.sampleRate));
        float release = std::exp(-1.f / (0.0500f * out.sampleRate));
        float env = 0.f;

        for (uint64_t f = 0; f < out.numFrames; ++f) {
            float s = out.at(c, f);
            // Check inter-sample peak (between prev and s)
            float interPeak = std::max(std::abs(s), std::abs((s + prev) * 0.5f));
            if (interPeak > env) env = env + (1.f - attack)  * (interPeak - env);
            else                 env = env + (1.f - release) * (interPeak - env);

            if (env > ceil) {
                float gain = ceil / (env + 1e-9f);
                out.at(c, f) *= gain;
            }
            prev = s;
        }
    }
    return out;
}

// ── TPDF dither ────────────────────────────────────────────────────────────
void ExportEngine::applyTPDFDither(std::vector<Sample>& ch, int bitDepth) const {
    float lsb = 1.f / std::pow(2.f, bitDepth - 1);
    std::mt19937 rng(0xD17437);
    std::uniform_real_distribution<float> dist(-1.f, 1.f);
    for (auto& s : ch)
        s += (dist(rng) + dist(rng)) * 0.5f * lsb;
}

// ── Simple linear SRC ──────────────────────────────────────────────────────
AudioBuffer ExportEngine::resample(const AudioBuffer& buf, uint32_t targetSR) const {
    if (buf.sampleRate == targetSR) return buf;
    double ratio = static_cast<double>(targetSR) / buf.sampleRate;
    uint64_t newFrames = static_cast<uint64_t>(buf.numFrames * ratio);
    AudioBuffer out(buf.numChannels, newFrames, targetSR);
    for (uint32_t c = 0; c < buf.numChannels; ++c) {
        for (uint64_t f = 0; f < newFrames; ++f) {
            double srcPos = f / ratio;
            uint64_t i0 = static_cast<uint64_t>(srcPos);
            float frac = static_cast<float>(srcPos - i0);
            uint64_t i1 = std::min(i0 + 1, buf.numFrames - 1);
            out.at(c, f) = buf.at(c, i0) * (1.f - frac) + buf.at(c, i1) * frac;
        }
    }
    return out;
}

// ── WAV write ──────────────────────────────────────────────────────────────
bool ExportEngine::writeWav(const AudioBuffer& buf, const std::string& path,
                             uint16_t bitDepth, bool floatFormat, bool dither,
                             std::string& err) const {
    AudioEngine ae;
    if (floatFormat)
        return ae.writeFloat(buf, path, err);

    // Apply dither before quantization
    AudioBuffer dithered = buf;
    if (dither && bitDepth <= 16) {
        for (uint32_t c = 0; c < dithered.numChannels; ++c)
            applyTPDFDither(dithered.channels[c], bitDepth);
    }
    return ae.writePCM(dithered, path, bitDepth, err);
}

// ── JSON metadata ──────────────────────────────────────────────────────────
bool ExportEngine::writeMetaJson(const ExportResult& result, const ExportConfig& cfg,
                                  const StyleDNA& dna, const AnalysisResult* analysis,
                                  std::string& err) const {
    std::string jsonPath = cfg.outputPath + ".json";
    std::ofstream f(jsonPath);
    if (!f.is_open()) { err = "Cannot write JSON: " + jsonPath; return false; }

    f << "{\n"
      << "  \"project\": \"" << cfg.projectName << "\",\n"
      << "  \"key\": \"" << cfg.key << "\",\n"
      << "  \"bpm\": " << cfg.bpm << ",\n"
      << "  \"target\": " << static_cast<int>(cfg.target) << ",\n"
      << "  \"sampleRate\": " << result.sampleRate << ",\n"
      << "  \"bitDepth\": " << result.bitDepth << ",\n"
      << "  \"measuredLUFS\": " << std::fixed << std::setprecision(2) << result.measuredLUFS << ",\n"
      << "  \"measuredPeakDBFS\": " << result.measuredPeak << ",\n"
      << "  \"numFrames\": " << result.numFrames << ",\n"
      << "  \"styleDNA\": {\n"
      << "    \"Humanization\": "       << dna.Humanization       << ",\n"
      << "    \"ShoegazeAmount\": "     << dna.ShoegazeAmount     << ",\n"
      << "    \"BoomBapAmount\": "      << dna.BoomBapAmount      << ",\n"
      << "    \"DustLevel\": "          << dna.DustLevel          << ",\n"
      << "    \"Warmth\": "             << dna.Warmth             << ",\n"
      << "    \"Darkness\": "           << dna.Darkness           << ",\n"
      << "    \"Imperfection\": "       << dna.Imperfection       << ",\n"
      << "    \"AtmosphericDensity\": " << dna.AtmosphericDensity << ",\n"
      << "    \"EmotionalIntensity\": " << dna.EmotionalIntensity << ",\n"
      << "    \"VintageCharacter\": "   << dna.VintageCharacter   << "\n"
      << "  }";

    if (analysis) {
        f << ",\n  \"analysis\": {\n"
          << "    \"estimatedBPM\": "      << analysis->estimatedBPM      << ",\n"
          << "    \"spectralCentroid\": "  << analysis->spectralCentroid  << ",\n"
          << "    \"centroidDriftPct\": "  << analysis->centroidDriftPct  << ",\n"
          << "    \"onTarget\": "          << (analysis->onTarget ? "true" : "false") << ",\n"
          << "    \"estimatedLUFS\": "     << analysis->estimatedLUFS     << ",\n"
          << "    \"onsetCount\": "        << analysis->onsetTimesSeconds.size() << "\n"
          << "  }";
    }
    f << "\n}\n";
    return f.good();
}

// ── Main export pipeline ───────────────────────────────────────────────────
ExportResult ExportEngine::exportAudio(const AudioBuffer& buf, ExportConfig cfg,
                                        const StyleDNA& dna,
                                        const AnalysisResult* analysis) const {
    return exportAudioWithProgress(buf, cfg, dna, [](float){});
}

ExportResult ExportEngine::exportAudioWithProgress(
    const AudioBuffer& buf, ExportConfig cfg, const StyleDNA& dna,
    std::function<void(float)> progressCb) const {

    ExportResult result;
    if (!buf.valid()) { result.error = "Empty input buffer"; return result; }

    progressCb(0.05f);

    // 1. Sample rate conversion
    AudioBuffer working = (buf.sampleRate != cfg.sampleRate)
        ? resample(buf, cfg.sampleRate)
        : buf;
    progressCb(0.20f);

    // 2. LUFS normalization
    working = normalizeLUFS(working, cfg.targetLUFS);
    progressCb(0.50f);

    // 3. Limiter (except Logic Import which wants headroom)
    if (cfg.applyLimiter)
        working = applyLimiter(working, cfg.limiterCeiling);
    progressCb(0.70f);

    // 4. Measure final stats
    AnalysisEngine ae;
    auto mono = working.toMono();
    result.measuredLUFS  = ae.estimateLUFS(mono, working.sampleRate);
    result.measuredPeak  = lin2dB(ae.measurePeak(mono));
    result.numFrames     = working.numFrames;
    result.sampleRate    = working.sampleRate;
    result.bitDepth      = cfg.bitDepth;
    progressCb(0.80f);

    // 5. Write WAV
    std::string err;
    bool ok = writeWav(working, cfg.outputPath, cfg.bitDepth,
                       cfg.floatFormat, cfg.dither, err);
    progressCb(0.95f);

    if (!ok) { result.error = err; return result; }

    // 6. JSON sidecar
    if (cfg.exportMetaJson) {
        std::string jerr;
        writeMetaJson(result, cfg, dna, nullptr, jerr);
    }

    result.success    = true;
    result.outputPath = cfg.outputPath;
    result.fileSizeKB = static_cast<float>(
        working.numFrames * working.numChannels * (cfg.bitDepth / 8)) / 1024.f;

    progressCb(1.0f);
    return result;
}

std::string ExportResult::filename() const {
    size_t pos = outputPath.find_last_of("/\\");
    return pos != std::string::npos ? outputPath.substr(pos + 1) : outputPath;
}

} // namespace melegi
