#include "melegi/GenerationEngine.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>

namespace melegi {

// ── Karplus-Strong string synthesis ───────────────────────────────────────
std::vector<Sample> GenerationEngine::karplusStrong(SampleRate sr, float freqHz,
                                                      float durSec, float amp,
                                                      float damping) const {
    int n      = static_cast<int>(durSec * sr);
    int bufLen = std::max(2, static_cast<int>(sr / freqHz));
    std::vector<float> excBuf(bufLen);
    for (auto& s : excBuf) {
        // Uniform random excitation
        uint32_t seed = rng_();
        excBuf[&s - excBuf.data()] = (seed & 0x7FFF) / 16384.f - 1.f;
    }

    std::vector<Sample> out(n);
    int idx = 0;
    for (int i = 0; i < n; ++i) {
        float s = excBuf[idx];
        out[i] = s * amp;
        // Average filter (low-pass) with damping
        int nx = (idx + 1) % bufLen;
        excBuf[idx] = (s + excBuf[nx]) * 0.5f * damping;
        idx = nx;

        // Envelope
        float t = static_cast<float>(i) / n;
        float env = t < 0.005f ? t / 0.005f : std::exp(-2.5f * t);
        out[i] *= env;
    }
    return out;
}

// ── 808-style kick drum ───────────────────────────────────────────────────
std::vector<Sample> GenerationEngine::synthKick(SampleRate sr, float freqHz,
                                                  float dur) const {
    int n = static_cast<int>(dur * sr);
    std::vector<Sample> out(n);
    for (int i = 0; i < n; ++i) {
        float t = static_cast<float>(i) / sr;
        float sweep = freqHz + 200.f * std::exp(-t * 18.f);
        float click = ((rng_() & 0xFFFF) / 32768.f - 1.f) * std::exp(-t * 80.f) * 0.35f;
        float env   = std::exp(-t * 3.5f);
        out[i] = std::tanh((std::sin(TWO_PI * sweep * t) * env + click) * 1.2f) / 1.2f * 0.85f;
    }
    return out;
}

// ── Snare ─────────────────────────────────────────────────────────────────
std::vector<Sample> GenerationEngine::synthSnare(SampleRate sr, float dur) const {
    int n = static_cast<int>(dur * sr);
    std::vector<Sample> out(n);
    for (int i = 0; i < n; ++i) {
        float t = static_cast<float>(i) / sr;
        float noise = ((rng_() & 0xFFFF) / 32768.f - 1.f) * std::exp(-t * 14.f) * 0.55f;
        float tone  = std::sin(TWO_PI * 200.f * t) * std::exp(-t * 22.f) * 0.25f
                    + std::sin(TWO_PI * 180.f * t) * std::exp(-t * 8.f)  * 0.20f;
        out[i] = std::tanh((noise + tone) * 1.1f) / 1.1f * 0.75f;
    }
    return out;
}

// ── Hi-hat ────────────────────────────────────────────────────────────────
std::vector<Sample> GenerationEngine::synthHiHat(SampleRate sr, bool open) const {
    float dur  = open ? 0.25f : 0.06f;
    float decay= open ? 12.f  : 35.f;
    int   n    = static_cast<int>(dur * sr);
    std::vector<Sample> out(n);
    static const float ratios[] = {1.f, 1.483f, 1.932f, 2.546f, 3.175f, 4.023f};
    for (int i = 0; i < n; ++i) {
        float t = static_cast<float>(i) / sr;
        float env = std::exp(-decay * t);
        float s = 0.f;
        for (float r : ratios) s += std::sin(TWO_PI * 8000.f * r * t);
        s /= 6.f;
        float noise = ((rng_() & 0xFFFF) / 32768.f - 1.f) * 0.3f;
        out[i] = (s + noise) * env * 0.4f;
    }
    return out;
}

// ── Mix a short buffer into a long one at offset ─────────────────────────
void GenerationEngine::mixInto(std::vector<Sample>& dst,
                                 const std::vector<Sample>& src,
                                 int64_t frameOffset, float gain) const {
    for (size_t i = 0; i < src.size(); ++i) {
        int64_t idx = frameOffset + i;
        if (idx >= 0 && idx < static_cast<int64_t>(dst.size()))
            dst[idx] += src[i] * gain;
    }
}

// ── Build chord voicing from scale degree ─────────────────────────────────
ChordVoicing GenerationEngine::buildChord(int rootMidi, const int* scale,
                                           int degree, int octave, bool add7) const {
    ChordVoicing cv;
    // Triad: root, third, fifth
    int root   = rootMidi + 12 * octave + scale[degree % 7];
    int third  = rootMidi + 12 * octave + scale[(degree + 2) % 7];
    int fifth  = rootMidi + 12 * octave + scale[(degree + 4) % 7];
    if (third < root) third += 12;
    if (fifth < root) fifth += 12;
    cv.midiPitches = {root, third, fifth};
    if (add7) {
        int seventh = rootMidi + 12 * octave + scale[(degree + 6) % 7];
        while (seventh < fifth) seventh += 12;
        cv.midiPitches.push_back(seventh);
    }
    // Tension: diminished/augmented are more tense
    int fifth_interval = fifth - root;
    cv.tensionScore = (fifth_interval == 6) ? 0.9f : (fifth_interval == 8) ? 0.5f : 0.1f;

    static const char* degreeNames[] = {"I","II","III","IV","V","VI","VII"};
    cv.name = degreeNames[degree % 7];
    return cv;
}

// ── Chord progression ────────────────────────────────────────────────────
std::vector<ChordVoicing> GenerationEngine::generateChordProgression(
    int rootNote, int modeIndex, int numChords,
    float tensionTarget, const StyleDNA& dna) const {

    const int* scale = getScale(modeIndex);
    int rootMidi = 48 + rootNote; // C3 = 48 base

    // Classic MELEGI progressions: favors ii-V-i and IV-VII-III-VI movements
    std::vector<int> degrees;

    // Aeolian/Dorian (CASH lane) preferred progressions
    if (modeIndex == 5 || modeIndex == 1) { // Aeolian or Dorian
        degrees = {5, 6, 3, 0}; // vi VII III I
    } else if (modeIndex == 4) { // Mixolydian
        degrees = {0, 6, 3, 4}; // I VII IV V
    } else if (modeIndex == 0) { // Ionian
        degrees = {0, 3, 4, 0}; // I IV V I
    } else {
        degrees = {0, 5, 3, 4}; // I vi IV V (generic)
    }

    // Extend/repeat to fill numChords
    std::vector<ChordVoicing> chords;
    bool add7 = dna.Darkness > 0.5f; // 7ths for darker textures
    for (int i = 0; i < numChords; ++i) {
        int deg = degrees[i % degrees.size()];
        chords.push_back(buildChord(rootMidi, scale, deg, 1, add7));
    }

    // Tension injection: if tensionTarget > 0.5, replace one chord with dim/sus
    if (tensionTarget > 0.5f && chords.size() >= 3) {
        // Replace 3rd chord with bVII (borrowed chord — always sounds heavy)
        int bVII = rootMidi + scale[6] - 12;
        ChordVoicing tension;
        tension.midiPitches = {bVII, bVII + 4, bVII + 7};
        tension.name = "bVII";
        tension.tensionScore = 0.7f;
        chords[2] = tension;
    }

    return chords;
}

// ── Bass line ─────────────────────────────────────────────────────────────
std::vector<MidiNote> GenerationEngine::generateBass(
    const std::vector<ChordVoicing>& chords,
    const BeatGrid& grid, const StyleDNA& dna) const {

    std::vector<MidiNote> notes;
    if (chords.empty() || grid.beatTimesSeconds.empty()) return notes;

    float beatsPerChord = 4.f; // one chord per bar
    float beatDur = grid.beatDurationSec;
    float halfTimeMult = dna.BoomBapAmount > 0.5f ? 2.f : 1.f; // half-time feel

    for (size_t ci = 0; ci < chords.size(); ++ci) {
        int rootPitch = chords[ci].midiPitches[0] - 12; // one octave down
        float chordStart = ci * beatsPerChord * beatDur;

        // Root note on beat 1
        MidiNote n1;
        n1.pitch     = static_cast<uint8_t>(rootPitch);
        n1.velocity  = 85 + static_cast<uint8_t>(dna.BoomBapAmount * 15.f);
        n1.startBeat = chordStart / beatDur;
        n1.durBeats  = halfTimeMult;
        notes.push_back(n1);

        // Ghost note on beat 3 (if boom-bap density allows)
        if (dna.BoomBapAmount > 0.4f) {
            MidiNote n2 = n1;
            n2.startBeat = (chordStart + beatDur * 2.f * halfTimeMult) / beatDur;
            n2.velocity  = static_cast<uint8_t>(n1.velocity * 0.7f);
            n2.pitch     = static_cast<uint8_t>(rootPitch + 7); // fifth
            n2.durBeats  = halfTimeMult * 0.75f;
            notes.push_back(n2);
        }
    }
    return notes;
}

// ── Melody ────────────────────────────────────────────────────────────────
std::vector<MidiNote> GenerationEngine::generateMelody(
    const std::vector<ChordVoicing>& chords,
    const BeatGrid& grid, const StyleDNA& dna) const {

    std::vector<MidiNote> notes;
    if (chords.empty()) return notes;

    const int* scale = getScale(5); // Aeolian default
    int rootMidi = chords[0].midiPitches[0];
    float beatDur = grid.beatDurationSec;
    float density = dna.EmotionalIntensity;

    // Simple motivic: 4-note phrase over 2 bars
    std::vector<int> motif = {0, 2, 5, 4}; // scale degrees (MELEGI feel: descending arc)

    for (size_t ci = 0; ci < chords.size(); ++ci) {
        float barStart = ci * 4.f * beatDur;
        for (size_t mi = 0; mi < motif.size(); ++mi) {
            if ((rng_() & 0xFF) / 255.f > density) continue; // sparse when low intensity
            MidiNote n;
            n.pitch    = static_cast<uint8_t>(rootMidi + 12 + scale[motif[mi]]);
            n.velocity = 55 + static_cast<uint8_t>(dna.EmotionalIntensity * 30.f)
                            + static_cast<uint8_t>((rng_() & 0xF));
            n.startBeat = (barStart + mi * beatDur) / beatDur;
            n.durBeats  = 0.75f;
            notes.push_back(n);
        }
    }
    return notes;
}

// ── Drum pattern (humanized boom-bap) ─────────────────────────────────────
std::vector<MidiNote> GenerationEngine::generateDrumPattern(
    const BeatGrid& grid, const StyleDNA& dna) const {

    std::vector<MidiNote> hits;
    float beatDur = grid.beatDurationSec;
    float numBars = grid.durationSec / (4.f * beatDur);

    for (int bar = 0; bar < static_cast<int>(numBars); ++bar) {
        float barStart = bar * 4.f * beatDur;

        // Kick: beats 1 and 3 (half-time feel if BoomBap > 0.6)
        bool halfTime = dna.BoomBapAmount > 0.6f;
        std::vector<float> kickBeats = halfTime
            ? std::vector<float>{0.f, 2.f * beatDur}
            : std::vector<float>{0.f, 1.f * beatDur, 2.f * beatDur, 3.f * beatDur};

        for (float kb : kickBeats) {
            // Humanize: ±15 ms
            float hum = ((rng_() & 0x3FF) / 512.f - 1.f) * dna.Humanization * 0.015f;
            MidiNote k;
            k.pitch    = 36; // kick
            k.velocity = static_cast<uint8_t>(80 + dna.BoomBapAmount * 20.f
                            + (rng_() & 0xF) - 8);
            k.startBeat = (barStart + kb + hum) / beatDur;
            k.durBeats  = 0.5f;
            hits.push_back(k);
        }

        // Snare: beat 3 (half-time) or 2+4
        std::vector<float> snareBeats = halfTime
            ? std::vector<float>{2.f * beatDur}
            : std::vector<float>{beatDur, 3.f * beatDur};

        for (float sb : snareBeats) {
            float hum = ((rng_() & 0x3FF) / 512.f - 1.f) * dna.Humanization * 0.02f;
            MidiNote s;
            s.pitch    = 38;
            s.velocity = static_cast<uint8_t>(75 + (rng_() & 0xF));
            s.startBeat = (barStart + sb + hum) / beatDur;
            s.durBeats  = 0.25f;
            hits.push_back(s);

            // Ghost snare
            if (dna.Humanization > 0.5f && (rng_() & 3) == 0) {
                MidiNote g = s;
                g.startBeat += 0.375f;
                g.velocity  = static_cast<uint8_t>(s.velocity * 0.3f);
                hits.push_back(g);
            }
        }

        // Hi-hats: swung 8ths
        for (int h = 0; h < 8; ++h) {
            float swingOff = (h % 2 == 1) ? dna.BoomBapAmount * 0.04f : 0.f;
            float hum = ((rng_() & 0xFF) / 128.f - 1.f) * dna.Humanization * 0.005f;
            bool openHat = (h == 3 || h == 7) && dna.BoomBapAmount > 0.5f;
            MidiNote hat;
            hat.pitch   = openHat ? 46 : 42;
            hat.velocity = static_cast<uint8_t>(40 + (rng_() & 0x1F));
            hat.startBeat = (barStart + h * beatDur * 0.5f + swingOff + hum) / beatDur;
            hat.durBeats  = openHat ? 0.25f : 0.1f;
            hits.push_back(hat);
        }
    }

    // Sort by time
    std::sort(hits.begin(), hits.end(),
              [](const MidiNote& a, const MidiNote& b){ return a.startBeat < b.startBeat; });
    return hits;
}

// ── Arrangement ───────────────────────────────────────────────────────────
Arrangement GenerationEngine::generateArrangement(float durSec, float bpm,
                                                   const StyleDNA& dna) const {
    Arrangement arr;
    arr.bpm = bpm;
    arr.totalBeats = durSec * bpm / 60.f;

    float beats = arr.totalBeats;

    // MELEGI structural template (Four Laws: Foundation before decoration)
    //  Intro (8b) → Verse (16b) → Hook (8b) → Verse (16b) → ERUPTION Breakdown (8b) → Outro (8b)
    float pos = 0.f;
    auto addSection = [&](const char* label, float len, float eruption, float intensity){
        Arrangement::Section s;
        s.label         = label;
        s.startBeat     = pos;
        s.lengthBeats   = std::min(len, beats - pos);
        s.eruptionActive = eruption;
        s.intensity     = intensity;
        arr.sections.push_back(s);
        pos += s.lengthBeats;
    };

    if (beats >= 64.f) {
        addSection("intro",     8.f,  0.f, 0.4f);
        addSection("verse1",    16.f, 0.f, 0.6f);
        addSection("hook",      8.f,  0.f, 0.8f);
        addSection("verse2",    16.f, 0.f, 0.65f);
        if (dna.ShoegazeAmount + dna.BoomBapAmount > 1.0f && beats >= 56.f)
            addSection("breakdown", 8.f, 1.f, 1.0f); // ERUPTION detonates
        if (pos < beats)
            addSection("outro",  beats - pos, 0.f, 0.3f);
    } else {
        // Short form
        addSection("intro",     beats * 0.15f, 0.f, 0.4f);
        addSection("main",      beats * 0.70f, 0.f, 0.8f);
        addSection("outro",     beats * 0.15f, 0.f, 0.3f);
    }

    return arr;
}

// ── Render MIDI to audio ──────────────────────────────────────────────────
AudioBuffer GenerationEngine::renderToAudio(const GeneratedScore& score, SampleRate sr) const {
    float durSec = score.arrangement.totalBeats / (score.arrangement.bpm / 60.f);
    if (durSec <= 0.f) return AudioBuffer();

    uint64_t totalFrames = static_cast<uint64_t>(durSec * sr);
    // Mono sum first, then duplicate to stereo
    std::vector<Sample> mono(totalFrames, 0.f);

    auto noteToOffset = [&](const MidiNote& n) -> int64_t {
        return static_cast<int64_t>(n.startBeat / score.arrangement.bpm * 60.f * sr);
    };

    // Chords
    for (const auto& n : score.chordNotes) {
        float durS = n.durBeats / score.arrangement.bpm * 60.f;
        float vel  = n.velocity / 127.f;
        auto data  = karplusStrong(sr, n.frequencyHz(), durS, vel * 0.3f, 0.998f);
        mixInto(mono, data, noteToOffset(n), 0.45f);
    }

    // Bass
    for (const auto& n : score.bassNotes) {
        float durS = n.durBeats / score.arrangement.bpm * 60.f;
        float vel  = n.velocity / 127.f;
        auto data  = karplusStrong(sr, n.frequencyHz(), durS, vel * 0.4f, 0.999f);
        mixInto(mono, data, noteToOffset(n), 0.35f);
    }

    // Melody
    for (const auto& n : score.melodyNotes) {
        float durS = n.durBeats / score.arrangement.bpm * 60.f;
        float vel  = n.velocity / 127.f;
        auto data  = karplusStrong(sr, n.frequencyHz(), durS, vel * 0.25f, 0.996f);
        mixInto(mono, data, noteToOffset(n), 0.25f);
    }

    // Drums
    for (const auto& n : score.drumHits) {
        std::vector<Sample> data;
        if      (n.pitch == 36) data = synthKick(sr);
        else if (n.pitch == 38) data = synthSnare(sr);
        else if (n.pitch == 42 || n.pitch == 46) data = synthHiHat(sr, n.pitch == 46);
        else continue;
        float vel = n.velocity / 127.f;
        mixInto(mono, data, noteToOffset(n), vel);
    }

    // Normalize
    float pk = 0.f; for (auto s : mono) pk = std::max(pk, std::abs(s));
    if (pk > 0.f) { float g = 0.9f / pk; for (auto& s : mono) s *= g; }

    // Stereo spread (slight delay on right channel for width)
    AudioBuffer out(2, totalFrames, sr);
    int rightDelay = static_cast<int>(0.0001f * sr); // 0.1 ms
    for (uint64_t f = 0; f < totalFrames; ++f) {
        out.at(0, f) = mono[f];
        out.at(1, f) = f >= static_cast<uint64_t>(rightDelay) ? mono[f - rightDelay] : 0.f;
    }
    return out;
}

// ── Full generate ─────────────────────────────────────────────────────────
GeneratedScore GenerationEngine::generate(const StyleDNA& dna,
                                           const AstralMusicalInfluence& inf,
                                           float durationSec) const {
    GeneratedScore gs;
    float bpm = 122.f + inf.tempoModifier;

    TimingEngine te;
    gs.arrangement = generateArrangement(durationSec, bpm, dna);
    gs.arrangement.rootNote  = inf.rootNote;
    gs.arrangement.modeIndex = inf.modeIndex;
    gs.arrangement.bpm       = bpm;

    BeatGrid grid = te.buildGrid(bpm, durationSec);

    int numChords = static_cast<int>(gs.arrangement.totalBeats / 4.f);
    numChords     = std::max(1, numChords);

    gs.chordNotes  = {};
    auto chords = generateChordProgression(
        inf.rootNote, inf.modeIndex, numChords,
        dna.Darkness, dna);

    // Expand chords into MIDI notes (one hit per chord, sustained)
    for (size_t ci = 0; ci < chords.size(); ++ci) {
        float t = ci * 4.f; // 4 beats per chord
        for (int pitch : chords[ci].midiPitches) {
            MidiNote n;
            n.pitch     = static_cast<uint8_t>(pitch);
            n.velocity  = 65;
            n.startBeat = t;
            n.durBeats  = 3.8f;
            gs.chordNotes.push_back(n);
        }
    }

    gs.bassNotes   = generateBass(chords, grid, dna);
    gs.melodyNotes = generateMelody(chords, grid, dna);
    gs.drumHits    = generateDrumPattern(grid, dna);

    gs.authenticityEstimate = dna.Humanization * 0.8f + 0.2f;
    return gs;
}

} // namespace melegi
