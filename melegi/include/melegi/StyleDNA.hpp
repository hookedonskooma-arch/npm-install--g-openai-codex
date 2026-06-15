#pragma once
#include <string>
#include <algorithm>

namespace melegi {

// ── StyleDNA ───────────────────────────────────────────────────────────────
// Single struct encoding the MELEGI sonic identity.
// Every subsystem reads from this to maintain coherent character.
// All values are normalized [0.0 … 1.0] unless noted.
struct StyleDNA {
    float Humanization      {0.75f}; // Micro-timing / velocity scatter
    float ShoegazeAmount    {0.60f}; // Reverb density / bloom
    float BoomBapAmount     {0.80f}; // Groove pocket weight
    float DustLevel         {0.40f}; // Vinyl crackle + tape hiss
    float Warmth            {0.70f}; // Low-shelf boost + tape saturation
    float Darkness          {0.65f}; // LP cutoff roll-off aggressiveness
    float Imperfection      {0.50f}; // Pitch wobble + timing drift
    float AtmosphericDensity{0.55f}; // Reverb tail length + wet level
    float EmotionalIntensity{0.80f}; // Dynamic range expressiveness
    float VintageCharacter  {0.60f}; // Wow/flutter + harmonic saturation

    // Clamp all values to [0, 1]
    void validate() {
        auto c = [](float& v){ v = std::max(0.f, std::min(1.f, v)); };
        c(Humanization); c(ShoegazeAmount); c(BoomBapAmount);
        c(DustLevel); c(Warmth); c(Darkness); c(Imperfection);
        c(AtmosphericDensity); c(EmotionalIntensity); c(VintageCharacter);
    }

    // Blend two DNAs
    static StyleDNA lerp(const StyleDNA& a, const StyleDNA& b, float t) {
        auto L = [t](float x, float y){ return x + t * (y - x); };
        return StyleDNA{
            L(a.Humanization,       b.Humanization),
            L(a.ShoegazeAmount,     b.ShoegazeAmount),
            L(a.BoomBapAmount,      b.BoomBapAmount),
            L(a.DustLevel,          b.DustLevel),
            L(a.Warmth,             b.Warmth),
            L(a.Darkness,           b.Darkness),
            L(a.Imperfection,       b.Imperfection),
            L(a.AtmosphericDensity, b.AtmosphericDensity),
            L(a.EmotionalIntensity, b.EmotionalIntensity),
            L(a.VintageCharacter,   b.VintageCharacter),
        };
    }

    std::string describe() const {
        return "DNA["
            "H=" + std::to_string(Humanization).substr(0,4) +
            " SG=" + std::to_string(ShoegazeAmount).substr(0,4) +
            " BB=" + std::to_string(BoomBapAmount).substr(0,4) +
            " E=" + std::to_string(EmotionalIntensity).substr(0,4) + "]";
    }
};

// ── Presets ────────────────────────────────────────────────────────────────

// GROUXX — primary identity (measured from GOD_LIGHT.wav)
inline StyleDNA makeGROUXX() {
    return StyleDNA{
        .Humanization       = 0.85f,  // Strong human feel preserved
        .ShoegazeAmount     = 0.30f,  // Subtle atmosphere, not washy
        .BoomBapAmount      = 0.90f,  // Heavy boom-bap pocket
        .DustLevel          = 0.50f,  // Audible vinyl texture
        .Warmth             = 0.80f,  // Warm low-shelf + tape
        .Darkness           = 0.70f,  // LP below 3500 Hz
        .Imperfection       = 0.60f,  // Deliberate pitch wobble
        .AtmosphericDensity = 0.40f,  // Short reverb tail
        .EmotionalIntensity = 0.90f,  // High dynamic expressiveness
        .VintageCharacter   = 0.70f,  // Analog drift
    };
}

// NavyBlue — CASH reference blend
inline StyleDNA makeNavyBlue() {
    return StyleDNA{
        .Humanization       = 0.90f,
        .ShoegazeAmount     = 0.20f,
        .BoomBapAmount      = 0.85f,
        .DustLevel          = 0.45f,
        .Warmth             = 0.75f,
        .Darkness           = 0.65f,
        .Imperfection       = 0.55f,
        .AtmosphericDensity = 0.35f,
        .EmotionalIntensity = 0.85f,
        .VintageCharacter   = 0.60f,
    };
}

// ETownConcrete — ERUPTION reference
inline StyleDNA makeETownConcrete() {
    return StyleDNA{
        .Humanization       = 0.70f,
        .ShoegazeAmount     = 0.25f,
        .BoomBapAmount      = 0.60f,
        .DustLevel          = 0.15f,
        .Warmth             = 0.40f,
        .Darkness           = 0.80f,
        .Imperfection       = 0.50f,
        .AtmosphericDensity = 0.30f,
        .EmotionalIntensity = 0.95f,
        .VintageCharacter   = 0.35f,
    };
}

// Shoegaze ambient — maximum atmosphere
inline StyleDNA makeShoegaze() {
    return StyleDNA{
        .Humanization       = 0.65f,
        .ShoegazeAmount     = 0.95f,
        .BoomBapAmount      = 0.30f,
        .DustLevel          = 0.30f,
        .Warmth             = 0.60f,
        .Darkness           = 0.55f,
        .Imperfection       = 0.70f,
        .AtmosphericDensity = 0.90f,
        .EmotionalIntensity = 0.75f,
        .VintageCharacter   = 0.50f,
    };
}

} // namespace melegi
