/*
 * MELEGI Audio Engine — First Milestone Pipeline Demo
 *
 * Pipeline:  Audio Input → Analysis → Timing → Authenticity → Shoegaze → Export
 *
 * Usage:
 *   melegi_pipeline [input.wav]          # process an existing WAV
 *   melegi_pipeline                      # generate from scratch and export
 */
#include "melegi/Common.hpp"
#include "melegi/StyleDNA.hpp"
#include "melegi/AudioEngine.hpp"
#include "melegi/AnalysisEngine.hpp"
#include "melegi/TimingEngine.hpp"
#include "melegi/AuthenticityEngine.hpp"
#include "melegi/ShoegazeEngine.hpp"
#include "melegi/GeoAstralEngine.hpp"
#include "melegi/GenerationEngine.hpp"
#include "melegi/AgentFramework.hpp"
#include "melegi/ExportEngine.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <ctime>

using namespace melegi;

// ── Print utilities ────────────────────────────────────────────────────────
static void banner(const char* title) {
    std::cout << "\n=== " << title << " ===\n";
}

static void printAnalysis(const AnalysisResult& r) {
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "  BPM:       " << r.estimatedBPM
              << " (confidence " << static_cast<int>(r.bpmConfidence * 100) << "%)\n";
    std::cout << "  Centroid:  " << r.spectralCentroid << " Hz"
              << "  [target: 757 Hz | drift: "
              << r.centroidDriftPct << "% | " << (r.onTarget ? "ON TARGET" : "OFF TARGET") << "]\n";
    std::cout << "  LUFS:      " << r.estimatedLUFS << "\n";
    std::cout << "  Peak:      " << r.peakDBFS << " dBFS\n";
    std::cout << "  Onsets:    " << r.onsetTimesSeconds.size() << "\n";
    std::cout << "  Bands:     sub=" << r.bandEnergy.sub
              << " lowMid=" << r.bandEnergy.lowMid
              << " mid=" << r.bandEnergy.mid
              << " hiMid=" << r.bandEnergy.hiMid
              << " air=" << r.bandEnergy.air << "\n";
}

static void printDNA(const StyleDNA& dna) {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  " << dna.describe() << "\n";
    std::cout << "  Humanization=" << dna.Humanization
              << " Shoegaze=" << dna.ShoegazeAmount
              << " BoomBap=" << dna.BoomBapAmount << "\n";
    std::cout << "  Warmth=" << dna.Warmth
              << " Darkness=" << dna.Darkness
              << " VintageChar=" << dna.VintageCharacter << "\n";
}

// ── Geo-astral influence report ───────────────────────────────────────────
static void printAstralInfluence(const AstralMusicalInfluence& inf) {
    static const char* noteNames[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    static const char* modeNames[] = {"Ionian","Dorian","Phrygian","Lydian","Mixolydian","Aeolian","Locrian"};
    std::cout << "  " << inf.description << "\n";
    std::cout << "  Root: " << noteNames[inf.rootNote % 12]
              << " | Mode: " << modeNames[inf.modeIndex % 7]
              << " | TempoMod: " << (inf.tempoModifier >= 0 ? "+" : "") << inf.tempoModifier << " BPM\n";
}

// ── Pipeline step header ──────────────────────────────────────────────────
static void step(int n, const char* label) {
    std::cout << "\n[" << n << "] " << label << "\n";
}

// ═════════════════════════════════════════════════════════════════════════════
int main(int argc, char* argv[]) {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "  MELEGI AUDIO ENGINE — FIRST MILESTONE PIPELINE\n";
    std::cout << "  CASH/ERUPTION Architecture | GROUXX Fingerprint\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";

    // ── StyleDNA: GROUXX preset ───────────────────────────────────────────
    banner("STYLE DNA");
    StyleDNA dna = makeGROUXX();
    printDNA(dna);

    // ── Geo-Astral influence ──────────────────────────────────────────────
    banner("GEO-ASTRAL ENGINE");
    std::cout << "  Location: New York City (40.71°N, 74.00°W)\n";
    GeoAstralEngine gae;
    auto astralData = GeoAstralEngine::now(40.71, -74.00);
    astralData.temperatureC = 18.f;
    astralData.humidityPct  = 60.f;
    astralData = gae.compute(astralData);
    auto influence = gae.influence(astralData);
    printAstralInfluence(influence);

    // Modulate DNA with astral influence
    dna = gae.modulateDNA(dna, influence);
    dna.validate();

    // ── Audio source ──────────────────────────────────────────────────────
    AudioEngine audioEngine;
    AudioBuffer workingBuf;
    std::string inputPath = (argc > 1) ? argv[1] : "";

    step(1, "AUDIO INPUT");

    if (!inputPath.empty()) {
        std::string err;
        auto loaded = audioEngine.load(inputPath, err);
        if (!loaded.has_value()) {
            std::cerr << "  ERROR loading " << inputPath << ": " << err << "\n";
            std::cerr << "  Falling back to synthesized source.\n";
            inputPath = "";
        } else {
            workingBuf = std::move(loaded.value());
            std::cout << "  Loaded: " << inputPath << "\n";
            std::cout << "  Channels: " << workingBuf.numChannels
                      << "  SR: " << workingBuf.sampleRate
                      << "  Duration: " << workingBuf.durationSeconds() << "s\n";
        }
    }

    if (inputPath.empty()) {
        // Generate from scratch using the full generation pipeline
        std::cout << "  Generating from scratch (8s @ 44100 Hz)\n";
        std::cout << "  Mode: " << influence.modeIndex << " | Root: " << influence.rootNote << "\n";

        GenerationEngine gen(42);
        auto score = gen.generate(dna, influence, 8.0f);
        workingBuf = gen.renderToAudio(score, DEFAULT_SAMPLE_RATE);

        std::cout << "  Generated: " << score.chordNotes.size() << " chord notes | "
                  << score.drumHits.size() << " drum hits | "
                  << score.bassNotes.size() << " bass notes\n";
        std::cout << "  Arrangement sections: " << score.arrangement.sections.size() << "\n";
        for (auto& s : score.arrangement.sections) {
            std::cout << "    [" << s.label << "] beat " << s.startBeat
                      << " len=" << s.lengthBeats
                      << (s.eruptionActive > 0.5f ? " <<ERUPTION>>" : "") << "\n";
        }
    }

    if (!workingBuf.valid()) {
        std::cerr << "  FATAL: No audio to process.\n"; return 1;
    }
    std::cout << "  Buffer ready: " << workingBuf.numFrames << " frames | "
              << workingBuf.durationSeconds() << "s\n";

    // ── Analysis ──────────────────────────────────────────────────────────
    step(2, "ANALYSIS ENGINE");
    AnalysisEngine ae;
    auto analysis = ae.analyze(workingBuf);
    printAnalysis(analysis);

    // ── Timing ────────────────────────────────────────────────────────────
    step(3, "TIMING ENGINE");
    TimingEngine te;
    float bpm = analysis.estimatedBPM > 0.f ? analysis.estimatedBPM : 122.f;
    BeatGrid grid = te.buildGrid(bpm, workingBuf.durationSeconds());
    std::cout << "  BPM: " << bpm << " | Beats: " << grid.beatTimesSeconds.size()
              << " | Bars: " << grid.barTimesSeconds.size() << "\n";
    std::cout << "  Half-time feel at " << grid.halfTimeBPM << " BPM\n";

    GrooveMap groove;
    if (!analysis.onsetTimesSeconds.empty()) {
        groove = te.extractGroove(analysis.onsetTimesSeconds, grid);
        std::cout << "  Groove: " << groove.offsets.size() << " offset points"
                  << " | Swing: " << std::fixed << std::setprecision(2)
                  << groove.swingRatio << "\n";
    } else {
        std::cout << "  No onsets detected — using clean grid\n";
    }

    HumanizationMap humMap = te.generateHumanizationMap(
        grid, dna,
        groove.offsets.empty() ? nullptr : &groove,
        2025);
    std::cout << "  Humanization score: "
              << std::setprecision(2) << humMap.humanizationScore
              << " | Swing: " << humMap.swingAmount << "\n";

    // ── Authenticity ──────────────────────────────────────────────────────
    step(4, "AUTHENTICITY ENGINE");
    AuthenticityEngine authEng;

    // Score before humanization
    auto reportBefore = authEng.score(workingBuf);
    std::cout << "  Before: " << reportBefore.diagnosis << "\n";

    // Apply humanization
    workingBuf = authEng.process(workingBuf, humMap, dna);

    auto reportAfter = authEng.score(workingBuf);
    std::cout << "  After:  " << reportAfter.diagnosis << "\n";
    std::cout << "  Score: " << static_cast<int>(reportAfter.overallScore) << "/100"
              << (reportAfter.passesAuthenticity ? " [PASS]" : " [WARN]") << "\n";

    // ── Shoegaze ──────────────────────────────────────────────────────────
    step(5, "SHOEGAZE ENGINE (CASH processing chain)");
    ShoegazeEngine sge;
    auto shoegazeParams = ShoegazeParams::fromDNA(dna);

    std::cout << "  Haze: " << shoegazeParams.hazeAmount
              << " | Decay: " << shoegazeParams.decayAmount
              << " | Width: " << shoegazeParams.widthAmount
              << " | Sat: " << shoegazeParams.saturationAmount << "\n";
    std::cout << "  Damping: " << shoegazeParams.dampingFreqHz << " Hz\n";

    // Apply lo-fi texture first (before reverb — correct order)
    sge.applyVinylCrackle(workingBuf, dna.DustLevel * 0.6f);
    sge.applyTapeHiss(workingBuf, dna.DustLevel * 0.3f);
    sge.applyWowFlutter(workingBuf, 1.0f, dna.VintageCharacter * 0.0015f,
                         10.f, dna.VintageCharacter * 0.0005f);

    // Full shoegaze pass
    workingBuf = sge.process(workingBuf, shoegazeParams);
    std::cout << "  Applied: FDN reverb, tape saturation, stereo widening, vinyl crackle\n";

    // StyleDNA final processing
    banner("STYLE DNA PROCESSING");
    auto finalAnalysis = ae.analyze(workingBuf);
    std::cout << "  Final centroid: " << finalAnalysis.spectralCentroid << " Hz"
              << " (drift: " << finalAnalysis.centroidDriftPct << "%"
              << " | " << (finalAnalysis.onTarget ? "ON TARGET" : "OFF TARGET") << ")\n";
    std::cout << "  Final LUFS: " << finalAnalysis.estimatedLUFS << "\n";

    // ── Multi-agent framework (optional — for generated content) ──────────
    // Skip for loaded WAV (already processed); run for generated content
    if (inputPath.empty()) {
        banner("AGENT FRAMEWORK");
        std::cout << "  Running 5-agent collaborative pass...\n";
        GeneratedScore agentScore;
        agentScore.arrangement.totalBeats = 8.f * 8.f;
        agentScore.arrangement.bpm = bpm;
        AgentContext ctx;
        ctx.dna      = dna;
        ctx.score    = &agentScore;
        ctx.audioOut = &workingBuf;
        ctx.creativityBudget = 0.5f;

        AgentFramework fw;
        bool accepted = fw.run(ctx, 6);
        std::cout << "  Outcome: " << (accepted ? "ACCEPTED" : "TIMEOUT — using last output") << "\n";
        std::cout << "  Critic score: " << static_cast<int>(fw.critic()->lastScore()) << "/100\n";
    }

    // ── Export ────────────────────────────────────────────────────────────
    step(6, "EXPORT ENGINE");

    auto exportWithTarget = [&](ExportTarget target, const char* label) {
        auto cfg = ExportEngine::resolveConfig(target, ".", "MELEGI", "Fm", bpm);
        cfg.exportMetaJson = true;

        ExportEngine ee;
        std::cout << "  [" << label << "] " << cfg.outputPath << "\n";

        auto result = ee.exportAudioWithProgress(
            workingBuf, cfg, dna,
            [&label](float p) {
                if (static_cast<int>(p * 10) % 2 == 0)
                    std::cout << "\r    " << label << ": "
                              << static_cast<int>(p * 100) << "%" << std::flush;
            });
        std::cout << "\n";

        if (result.success) {
            std::cout << "    OK: " << result.measuredLUFS << " LUFS | "
                      << result.measuredPeak << " dBFS peak | "
                      << result.fileSizeKB << " KB\n";
        } else {
            std::cout << "    ERROR: " << result.error << "\n";
        }
        return result.success;
    };

    bool ok = true;
    ok &= exportWithTarget(ExportTarget::BeatStars,   "BeatStars  –10 LUFS 44.1k/16b");
    ok &= exportWithTarget(ExportTarget::SunoRef,     "SunoRef    –14 LUFS 44.1k/16b");
    ok &= exportWithTarget(ExportTarget::LogicImport, "Logic      –18 LUFS 48k/24b  ");

    // ── Summary ───────────────────────────────────────────────────────────
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "  PIPELINE COMPLETE\n";
    std::cout << "  Priority: Emotion → Authenticity → Groove → Atmosphere\n";
    std::cout << "  Authenticity: " << static_cast<int>(reportAfter.overallScore) << "/100\n";
    std::cout << "  Fingerprint: centroid=" << finalAnalysis.spectralCentroid
              << "Hz | " << (finalAnalysis.onTarget ? "ON TARGET ✓" : "OFF TARGET") << "\n";
    std::cout << "  Exports: " << (ok ? "ALL SUCCESSFUL" : "SOME FAILED") << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";

    return ok ? 0 : 1;
}
