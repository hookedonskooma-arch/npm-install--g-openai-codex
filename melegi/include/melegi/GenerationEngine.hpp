#pragma once
#include "Common.hpp"
#include "StyleDNA.hpp"
#include "TimingEngine.hpp"
#include "GeoAstralEngine.hpp"
#include <vector>
#include <string>
#include <random>

namespace melegi {

// ── Scale / Chord helpers ──────────────────────────────────────────────────
// Scale intervals in semitones (from root)
static constexpr int IONIAN[]     = {0,2,4,5,7,9,11};
static constexpr int DORIAN[]     = {0,2,3,5,7,9,10};
static constexpr int PHRYGIAN[]   = {0,1,3,5,7,8,10};
static constexpr int LYDIAN[]     = {0,2,4,6,7,9,11};
static constexpr int MIXOLYDIAN[] = {0,2,4,5,7,9,10};
static constexpr int AEOLIAN[]    = {0,2,3,5,7,8,10};
static constexpr int LOCRIAN[]    = {0,1,3,5,6,8,10};

inline const int* getScale(int modeIndex) {
    static const int* modes[] = { IONIAN, DORIAN, PHRYGIAN, LYDIAN,
                                   MIXOLYDIAN, AEOLIAN, LOCRIAN };
    return modes[modeIndex % 7];
}

// ── ChordVoicing ──────────────────────────────────────────────────────────
struct ChordVoicing {
    std::vector<int> midiPitches;   // absolute MIDI pitches in chord
    std::string      name;          // e.g. "Fm7"
    float            tensionScore  {0.f}; // [0=resolved, 1=tense]
};

// ── Arrangement ───────────────────────────────────────────────────────────
struct Arrangement {
    float          bpm        {122.f};
    int            rootNote   {5};        // MIDI note class
    int            modeIndex  {5};        // Aeolian default

    struct Section {
        std::string label;        // "intro", "verse", "hook", "breakdown", "outro"
        float       startBeat;
        float       lengthBeats;
        float       eruptionActive; // 0 or 1
        float       intensity;      // [0,1]
    };
    std::vector<Section> sections;
    float totalBeats{64.f};
};

// ── Generated layers (as MIDI note lists) ─────────────────────────────────
struct GeneratedScore {
    std::vector<MidiNote> chordNotes;
    std::vector<MidiNote> bassNotes;
    std::vector<MidiNote> melodyNotes;
    std::vector<MidiNote> drumHits;    // pitch encodes: 36=kick, 38=snare, 42=hat
    Arrangement           arrangement;
    float                 authenticityEstimate{0.f};
};

// ── GenerationEngine ──────────────────────────────────────────────────────
class GenerationEngine {
public:
    explicit GenerationEngine(uint32_t seed = 0) : rng_(seed) {}

    // High-level: generate a full score from DNA + astral influence
    GeneratedScore generate(const StyleDNA&              dna,
                            const AstralMusicalInfluence& influence,
                            float                         durationSec) const;

    // Component generators
    std::vector<ChordVoicing> generateChordProgression(
        int   rootNote,
        int   modeIndex,
        int   numChords,
        float tensionTarget,
        const StyleDNA& dna) const;

    std::vector<MidiNote> generateBass(
        const std::vector<ChordVoicing>& chords,
        const BeatGrid&                  grid,
        const StyleDNA&                  dna) const;

    std::vector<MidiNote> generateMelody(
        const std::vector<ChordVoicing>& chords,
        const BeatGrid&                  grid,
        const StyleDNA&                  dna) const;

    std::vector<MidiNote> generateDrumPattern(
        const BeatGrid& grid,
        const StyleDNA& dna) const;

    Arrangement generateArrangement(
        float           durationSec,
        float           bpm,
        const StyleDNA& dna) const;

    // Render MIDI notes to AudioBuffer using Karplus-Strong synthesis
    AudioBuffer renderToAudio(const GeneratedScore& score, SampleRate sr) const;

private:
    mutable std::mt19937 rng_;

    // Synthesis helpers
    std::vector<Sample> karplusStrong(SampleRate sr,
                                       float      freqHz,
                                       float      durationSec,
                                       float      amplitude,
                                       float      damping) const;

    std::vector<Sample> synthKick(SampleRate sr,
                                   float      freqHz = 55.f,
                                   float      dur    = 0.8f) const;

    std::vector<Sample> synthSnare(SampleRate sr, float dur = 0.25f) const;

    std::vector<Sample> synthHiHat(SampleRate sr,
                                    bool       open = false) const;

    void mixInto(std::vector<Sample>& dst,
                 const std::vector<Sample>& src,
                 int64_t frameOffset,
                 float   gain) const;

    // Chord quality helpers
    ChordVoicing buildChord(int rootMidi, const int* scale, int degree,
                             int octave, bool add7) const;
};

} // namespace melegi
