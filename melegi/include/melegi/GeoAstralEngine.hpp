#pragma once
#include "Common.hpp"
#include "StyleDNA.hpp"
#include <ctime>
#include <string>

namespace melegi {

// ── GeoAstralData ──────────────────────────────────────────────────────────
struct GeoAstralData {
    double    latitude    {40.7128};   // degrees N
    double    longitude   {-74.006};  // degrees E
    double    elevationM  {10.0};     // meters above sea level
    float     temperatureC{20.f};
    float     humidityPct {50.f};
    std::time_t utcTime   {0};        // Unix timestamp

    // Derived (computed by GeoAstralEngine::compute())
    float moonPhase       {0.f};  // [0=new, 0.5=full, 1.0=new]
    float solarElevation  {0.f};  // degrees above horizon
    float solarAzimuth    {0.f};  // degrees clockwise from N
    int   dayOfYear       {0};
    float seasonFactor    {0.f};  // [0=winter, 0.5=spring/autumn, 1=summer]
    std::string dominantConstellation;
};

// ── AstralMusicalInfluence ─────────────────────────────────────────────────
// Translation of celestial/environmental data into musical parameters.
struct AstralMusicalInfluence {
    int   rootNote       {5};   // MIDI note class 0–11 (0=C)
    int   modeIndex      {0};   // 0=Ionian, 1=Dorian, 2=Phrygian, 3=Lydian,
                                // 4=Mixolydian, 5=Aeolian, 6=Locrian
    float tempoModifier  {0.f}; // ±BPM offset from base (e.g. -3 to +3)
    float rhythmDensity  {0.5f};// note density [0=sparse, 1=dense]
    float modulationRate {0.2f};// harmonic movement rate [0,1]
    float textureEvolution{0.3f};// how much texture changes over time [0,1]
    float dynamicContour {0.6f};// energy arc [0=flat, 1=dynamic]
    float darknessBias   {0.f}; // ±darkness adjustment on StyleDNA
    std::string description;    // human-readable summary
};

// ── GeoAstralEngine ────────────────────────────────────────────────────────
// Converts location, time, and environmental data into musical influence.
// Every output is deterministic for a given GeoAstralData input.
class GeoAstralEngine {
public:
    GeoAstralEngine() = default;

    // Compute derived celestial quantities from raw location + time
    GeoAstralData compute(GeoAstralData data) const;

    // Translate computed GeoAstralData → musical influence
    AstralMusicalInfluence influence(const GeoAstralData& data) const;

    // Convenience: run compute then influence in one call
    AstralMusicalInfluence computeAndInfluence(GeoAstralData data) const {
        return influence(compute(data));
    }

    // Modulate a StyleDNA using astral influence
    StyleDNA modulateDNA(const StyleDNA&          base,
                         const AstralMusicalInfluence& infl) const;

    // Build a GeoAstralData for "right now" at a given location
    static GeoAstralData now(double latDeg, double lonDeg, double elevM = 0.0);

private:
    // Moon phase: 0=new, 0.25=first quarter, 0.5=full, 0.75=last quarter
    static float moonPhase(std::time_t utc);

    // Solar position (elevation + azimuth) using simplified NOAA algorithm
    static void solarPosition(double lat, double lon, std::time_t utc,
                               float& elevDeg, float& azDeg);

    // Map moon phase to mode index
    static int moonToMode(float phase);

    // Map solar elevation + season → root note class
    static int solarToRoot(float elevDeg, float seasonFactor);

    // Map humidity + temperature to darkness bias
    static float envToDarkness(float tempC, float humPct);

    static const char* dominantConstellationForDOY(int doy);
};

} // namespace melegi
