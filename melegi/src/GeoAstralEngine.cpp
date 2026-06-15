#include "melegi/GeoAstralEngine.hpp"
#include <cmath>
#include <ctime>
#include <sstream>
#include <algorithm>

namespace melegi {

// ── Moon phase (Julian date based, accurate within ±1 day) ─────────────────
float GeoAstralEngine::moonPhase(std::time_t utc) {
    // Julian date from Unix timestamp
    double jd = utc / 86400.0 + 2440587.5;
    // Reference new moon: Jan 6, 2000 (JD 2451549.5) cycle 29.53059 days
    double cycle = 29.53059;
    double phase = std::fmod(jd - 2451549.5, cycle) / cycle;
    if (phase < 0.0) phase += 1.0;
    return static_cast<float>(phase);
}

// ── Solar position (simplified NOAA algorithm, ±1° accuracy) ─────────────
void GeoAstralEngine::solarPosition(double lat, double lon, std::time_t utc,
                                     float& elevDeg, float& azDeg) {
    const double DEG = M_PI / 180.0;
    // Julian date
    double jd  = utc / 86400.0 + 2440587.5;
    double n   = jd - 2451545.0;           // J2000.0 epoch
    double L   = std::fmod(280.460 + 0.9856474 * n, 360.0);
    double g   = std::fmod(357.528 + 0.9856003 * n, 360.0) * DEG;
    double lam = (L + 1.915 * std::sin(g) + 0.020 * std::sin(2 * g)) * DEG;
    double eps = 23.439 * DEG;

    double sinDec = std::sin(eps) * std::sin(lam);
    double dec = std::asin(sinDec);
    double cosDec = std::cos(dec);

    // Greenwich Mean Sidereal Time
    double gst  = std::fmod(6.697375 + 0.0657098242 * n, 24.0);
    double lst  = gst + lon / 15.0;
    double ha   = (lst * 15.0 - 180.0) * DEG;

    double latR = lat * DEG;
    double sinEl = std::sin(latR) * sinDec + std::cos(latR) * cosDec * std::cos(ha);
    elevDeg = static_cast<float>(std::asin(clamp(static_cast<float>(sinEl), -1.f, 1.f)) / DEG);

    double cosAz = (sinDec - std::sin(latR) * sinEl)
                  / (std::cos(latR) * std::cos(std::asin(sinEl)) + 1e-10);
    azDeg = static_cast<float>(std::acos(clamp(static_cast<float>(cosAz), -1.f, 1.f)) / DEG);
    if (std::sin(ha) > 0.0) azDeg = 360.f - azDeg;
}

// ── Constellation for day of year ─────────────────────────────────────────
const char* GeoAstralEngine::dominantConstellationForDOY(int doy) {
    // 12 roughly equal segments
    static const char* names[] = {
        "Aries","Taurus","Gemini","Cancer","Leo","Virgo",
        "Libra","Scorpio","Ophiuchus","Sagittarius","Capricorn","Aquarius"
    };
    return names[(doy / 30) % 12];
}

// ── Celestial → mode ──────────────────────────────────────────────────────
int GeoAstralEngine::moonToMode(float phase) {
    // 8 phases → 7 modes (skip Locrian at full moon — too unstable for MELEGI)
    if (phase < 0.125f)       return 5; // New: Aeolian (dark foundation)
    else if (phase < 0.25f)   return 4; // Waxing crescent: Mixolydian
    else if (phase < 0.375f)  return 1; // First quarter: Dorian
    else if (phase < 0.5f)    return 3; // Waxing gibbous: Lydian (lift)
    else if (phase < 0.625f)  return 0; // Full: Ionian (resolved)
    else if (phase < 0.75f)   return 2; // Waning gibbous: Phrygian
    else if (phase < 0.875f)  return 4; // Last quarter: Mixolydian
    else                      return 5; // Waning crescent: Aeolian (returns to dark)
}

int GeoAstralEngine::solarToRoot(float elevDeg, float seasonFactor) {
    // Solar elevation → root note class (0=C … 11=B)
    // Low sun (winter, night) → flat keys (Bb, Eb, Ab = 10,3,8)
    // High sun (summer, noon) → bright keys (G,D,A = 7,2,9)
    float normalized = (elevDeg + 90.f) / 180.f; // [0,1]
    float blend = normalized * 0.6f + seasonFactor * 0.4f;
    // Map [0,1] → note classes (dark→bright arc)
    static const int roots[] = {10, 8, 3, 5, 0, 7, 2, 9, 4, 11, 6, 1};
    int idx = static_cast<int>(blend * 11.999f);
    return roots[std::max(0, std::min(11, idx))];
}

float GeoAstralEngine::envToDarkness(float tempC, float humPct) {
    // Cold + humid → darker; hot + dry → brighter
    float coldness = std::max(0.f, (20.f - tempC) / 40.f);  // 0=warm, 1=very cold
    float moisture = humPct / 100.f;
    return clamp((coldness * 0.6f + moisture * 0.4f) - 0.2f, -0.3f, 0.3f);
}

// ── Compute derived quantities ────────────────────────────────────────────
GeoAstralData GeoAstralEngine::compute(GeoAstralData d) const {
    d.moonPhase = moonPhase(d.utcTime);
    solarPosition(d.latitude, d.longitude, d.utcTime,
                  d.solarElevation, d.solarAzimuth);

    // Day of year
    std::tm* lt = std::gmtime(&d.utcTime);
    d.dayOfYear = lt ? lt->tm_yday : 180;

    // Season: 0=winter, 0.5=equinox, 1=summer (northern hemisphere)
    float solsticeOffset = static_cast<float>((d.dayOfYear - 172 + 365) % 365) / 365.f;
    d.seasonFactor = 0.5f + 0.5f * std::cos(TWO_PI * solsticeOffset);
    // Flip for southern hemisphere
    if (d.latitude < 0.f) d.seasonFactor = 1.f - d.seasonFactor;

    d.dominantConstellation = dominantConstellationForDOY(d.dayOfYear);
    return d;
}

// ── Translate to musical influence ───────────────────────────────────────
AstralMusicalInfluence GeoAstralEngine::influence(const GeoAstralData& d) const {
    AstralMusicalInfluence inf;

    inf.rootNote   = solarToRoot(d.solarElevation, d.seasonFactor);
    inf.modeIndex  = moonToMode(d.moonPhase);

    // Elevation at night → slow tempo, daytime → slightly faster
    float solarNorm = (d.solarElevation + 90.f) / 180.f;
    inf.tempoModifier = (solarNorm - 0.5f) * 6.f;  // ±3 BPM

    // Humidity drives density (denser when humid)
    inf.rhythmDensity = 0.3f + d.humidityPct / 100.f * 0.4f;

    // Moon waxing (phase < 0.5) → more modulation
    inf.modulationRate = (d.moonPhase < 0.5f)
        ? 0.1f + d.moonPhase * 0.6f
        : 0.4f - (d.moonPhase - 0.5f) * 0.4f;

    // Seasonal texture evolution
    inf.textureEvolution = d.seasonFactor * 0.5f;

    // Dynamic contour follows solar arc
    inf.dynamicContour = solarNorm;

    // Environmental darkness bias
    inf.darknessBias = envToDarkness(d.temperatureC, d.humidityPct);

    // Description
    std::ostringstream ss;
    static const char* noteNames[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    static const char* modeNames[] = {"Ionian","Dorian","Phrygian","Lydian","Mixolydian","Aeolian","Locrian"};
    ss << noteNames[inf.rootNote] << " " << modeNames[inf.modeIndex % 7]
       << " | Moon=" << static_cast<int>(d.moonPhase * 100.f) << "%"
       << " | Sun=" << static_cast<int>(d.solarElevation) << "°"
       << " | " << d.dominantConstellation;
    inf.description = ss.str();

    return inf;
}

StyleDNA GeoAstralEngine::modulateDNA(const StyleDNA& base,
                                       const AstralMusicalInfluence& inf) const {
    StyleDNA dna = base;
    // Dark modes → more darkness
    float modeOffset = (inf.modeIndex == 5 || inf.modeIndex == 2) ? 0.1f : 0.f;
    dna.Darkness    = clamp(dna.Darkness + modeOffset + inf.darknessBias, 0.f, 1.f);

    // Rhythm density → humanization
    dna.Humanization = clamp(dna.Humanization + (inf.rhythmDensity - 0.5f) * 0.1f, 0.f, 1.f);

    // More texture evolution → more atmospheric density
    dna.AtmosphericDensity = clamp(dna.AtmosphericDensity + inf.textureEvolution * 0.15f, 0.f, 1.f);

    // High dynamic contour → more emotional intensity
    dna.EmotionalIntensity = clamp(dna.EmotionalIntensity + (inf.dynamicContour - 0.5f) * 0.1f, 0.f, 1.f);

    return dna;
}

GeoAstralData GeoAstralEngine::now(double latDeg, double lonDeg, double elevM) {
    GeoAstralData d;
    d.latitude    = latDeg;
    d.longitude   = lonDeg;
    d.elevationM  = elevM;
    d.utcTime     = std::time(nullptr);
    return d;
}

} // namespace melegi
