import { useState, useRef } from "react";

const MOODS = ["grimey", "melancholic", "eerie", "nostalgic", "tense", "murky", "cold"];
const TEXTURES = ["lo-fi tape", "vinyl grain", "chopped soul", "jazz loop", "dusty drums", "church organ", "dark ambient"];
const KEYS = ["C min", "D min", "E♭ min", "F# min", "G min", "A min", "B♭ min"];
const STRUCTURES = ["loop", "verse beat", "hook build", "intro texture", "outro decay"];

const STYLE_NOTE = `You are a beat/loop concept generator inspired by underground East Coast hip-hop producers like Grubby Pawz.
Your style: dense sample chops, lo-fi textures, jazz/soul roots twisted dark, unconventional drum placement,
cinematic atmosphere. Think East Boston grit meets NYC underground. Avoid mainstream trap clichés.

Generate a detailed beat/loop concept. Return ONLY valid JSON (no markdown, no backticks, no explanation):
{
  "title": "short evocative track title",
  "sampleSource": "what to sample or interpolate (specific era, genre, instrument)",
  "chopTechnique": "how to process/chop/layer the sample",
  "drumPattern": "drum pattern description with placement and feel",
  "bassline": "bassline character and movement",
  "atmosphere": "additional layers, fx, ambience",
  "productionNotes": "2-3 sentences on the overall feel and intent",
  "referenceTracks": ["3 underground track references with artists that match this vibe"]
}`;

export default function App() {
  const [bpm, setBpm] = useState(87);
  const [key, setKey] = useState("D min");
  const [mood, setMood] = useState("grimey");
  const [texture, setTexture] = useState("chopped soul");
  const [structure, setStructure] = useState("loop");
  const [intensity, setIntensity] = useState(60);
  const [result, setResult] = useState(null);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState(null);
  const abortRef = useRef(null);

  const generate = async () => {
    setLoading(true);
    setResult(null);
    setError(null);

    const prompt = `Generate a beat concept with these parameters:
- BPM: ${bpm}
- Key: ${key}
- Mood: ${mood}
- Primary texture: ${texture}
- Structure type: ${structure}
- Intensity (0-100): ${intensity} — ${intensity < 40 ? "sparse, ghostly" : intensity < 70 ? "mid-density, deliberate" : "dense, layered"}

Make it sound like something that would fit in the Grubby Pawz catalog — underground East Coast, textured, not commercial.`;

    try {
      const apiKey = import.meta.env.VITE_ANTHROPIC_API_KEY;
      if (!apiKey) throw new Error("VITE_ANTHROPIC_API_KEY not set");

      const res = await fetch("https://api.anthropic.com/v1/messages", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
          "x-api-key": apiKey,
          "anthropic-version": "2023-06-01",
          "anthropic-dangerous-direct-browser-access": "true",
        },
        body: JSON.stringify({
          model: "claude-sonnet-4-20250514",
          max_tokens: 1000,
          system: STYLE_NOTE,
          messages: [{ role: "user", content: prompt }],
        }),
      });
      const data = await res.json();
      const raw = data.content?.map(b => b.text || "").join("") || "";
      const clean = raw.replace(/```json|```/g, "").trim();
      const parsed = JSON.parse(clean);
      setResult(parsed);
    } catch (e) {
      setError("Generation failed. Check your API key and connection.");
    } finally {
      setLoading(false);
    }
  };

  return (
    <div style={{
      minHeight: "100vh",
      background: "#0a0a0a",
      color: "#e8e0d0",
      fontFamily: "'Courier New', monospace",
      padding: "0",
      position: "relative",
      overflow: "hidden",
    }}>
      {/* grain overlay */}
      <div style={{
        position: "fixed", inset: 0, pointerEvents: "none", zIndex: 0,
        backgroundImage: `url("data:image/svg+xml,%3Csvg viewBox='0 0 256 256' xmlns='http://www.w3.org/2000/svg'%3E%3Cfilter id='noise'%3E%3CfeTurbulence type='fractalNoise' baseFrequency='0.9' numOctaves='4' stitchTiles='stitch'/%3E%3C/filter%3E%3Crect width='100%25' height='100%25' filter='url(%23noise)' opacity='0.04'/%3E%3C/svg%3E")`,
        opacity: 0.6,
      }} />

      <div style={{ position: "relative", zIndex: 1, maxWidth: 700, margin: "0 auto", padding: "40px 20px 60px" }}>

        {/* header */}
        <div style={{ marginBottom: 40 }}>
          <div style={{ fontSize: 10, letterSpacing: 6, color: "#7a6f5e", marginBottom: 8, textTransform: "uppercase" }}>
            East Boston / Underground
          </div>
          <h1 style={{
            fontSize: "clamp(28px, 6vw, 52px)",
            fontFamily: "'Georgia', serif",
            fontWeight: 400,
            letterSpacing: -1,
            color: "#e8e0d0",
            margin: 0,
            lineHeight: 1,
          }}>
            CURRY<span style={{ color: "#c8a96e" }}>GOAT</span>
          </h1>
          <div style={{ fontSize: 10, letterSpacing: 4, color: "#7a6f5e", marginTop: 6, textTransform: "uppercase" }}>
            beat concept generator — grubby pawz style
          </div>
        </div>

        {/* controls grid */}
        <div style={{ display: "grid", gap: 20, marginBottom: 32 }}>

          {/* BPM */}
          <div>
            <Label>BPM <span style={{ color: "#c8a96e", fontWeight: 700 }}>{bpm}</span></Label>
            <input type="range" min={60} max={120} value={bpm}
              onChange={e => setBpm(Number(e.target.value))}
              style={sliderStyle} />
            <div style={{ display: "flex", justifyContent: "space-between", fontSize: 9, color: "#5a5040", marginTop: 4 }}>
              <span>60 slow drag</span><span>120 uptempo</span>
            </div>
          </div>

          {/* Intensity */}
          <div>
            <Label>DENSITY <span style={{ color: "#c8a96e", fontWeight: 700 }}>{intensity}%</span></Label>
            <input type="range" min={0} max={100} value={intensity}
              onChange={e => setIntensity(Number(e.target.value))}
              style={sliderStyle} />
            <div style={{ display: "flex", justifyContent: "space-between", fontSize: 9, color: "#5a5040", marginTop: 4 }}>
              <span>ghostly / sparse</span><span>dense / layered</span>
            </div>
          </div>

          {/* row: key + structure */}
          <div style={{ display: "grid", gridTemplateColumns: "1fr 1fr", gap: 16 }}>
            <div>
              <Label>KEY</Label>
              <Select value={key} onChange={e => setKey(e.target.value)} options={KEYS} />
            </div>
            <div>
              <Label>STRUCTURE</Label>
              <Select value={structure} onChange={e => setStructure(e.target.value)} options={STRUCTURES} />
            </div>
          </div>

          {/* row: mood + texture */}
          <div style={{ display: "grid", gridTemplateColumns: "1fr 1fr", gap: 16 }}>
            <div>
              <Label>MOOD</Label>
              <Select value={mood} onChange={e => setMood(e.target.value)} options={MOODS} />
            </div>
            <div>
              <Label>TEXTURE</Label>
              <Select value={texture} onChange={e => setTexture(e.target.value)} options={TEXTURES} />
            </div>
          </div>
        </div>

        {/* generate button */}
        <button onClick={generate} disabled={loading} style={{
          width: "100%",
          padding: "16px 0",
          background: loading ? "#1a1710" : "#c8a96e",
          color: loading ? "#5a5040" : "#0a0a0a",
          border: "none",
          fontFamily: "'Courier New', monospace",
          fontSize: 12,
          letterSpacing: 5,
          textTransform: "uppercase",
          cursor: loading ? "not-allowed" : "pointer",
          transition: "all 0.2s",
          marginBottom: 32,
        }}>
          {loading ? "cooking..." : "generate beat concept"}
        </button>

        {error && (
          <div style={{ color: "#8b4040", fontSize: 12, marginBottom: 24, letterSpacing: 1 }}>{error}</div>
        )}

        {/* result */}
        {result && (
          <div style={{
            background: "#0f0e0c",
            border: "1px solid #2a2418",
            padding: "28px 24px",
            animation: "fadeIn 0.4s ease",
          }}>
            <style>{`@keyframes fadeIn { from { opacity:0; transform:translateY(8px) } to { opacity:1; transform:translateY(0) } }`}</style>

            <div style={{ fontSize: 9, letterSpacing: 4, color: "#7a6f5e", marginBottom: 4, textTransform: "uppercase" }}>
              {bpm} BPM · {key} · {mood}
            </div>
            <h2 style={{ fontFamily: "'Georgia', serif", fontWeight: 400, fontSize: 26, color: "#c8a96e", margin: "0 0 24px", letterSpacing: -0.5 }}>
              {result.title}
            </h2>

            <Row label="SAMPLE SOURCE" value={result.sampleSource} />
            <Row label="CHOP TECHNIQUE" value={result.chopTechnique} />
            <Row label="DRUM PATTERN" value={result.drumPattern} />
            <Row label="BASSLINE" value={result.bassline} />
            <Row label="ATMOSPHERE" value={result.atmosphere} />

            <div style={{ margin: "20px 0", height: 1, background: "#1e1c18" }} />

            <div style={{ marginBottom: 20 }}>
              <FieldLabel>PRODUCTION NOTES</FieldLabel>
              <p style={{ fontSize: 13, color: "#b8a888", lineHeight: 1.7, margin: 0 }}>{result.productionNotes}</p>
            </div>

            {result.referenceTracks?.length > 0 && (
              <div>
                <FieldLabel>REFERENCE TRACKS</FieldLabel>
                {result.referenceTracks.map((t, i) => (
                  <div key={i} style={{ fontSize: 12, color: "#7a6f5e", marginBottom: 4 }}>— {t}</div>
                ))}
              </div>
            )}
          </div>
        )}
      </div>
    </div>
  );
}

function Label({ children }) {
  return <div style={{ fontSize: 9, letterSpacing: 4, color: "#7a6f5e", textTransform: "uppercase", marginBottom: 8 }}>{children}</div>;
}

function FieldLabel({ children }) {
  return <div style={{ fontSize: 9, letterSpacing: 4, color: "#5a5040", textTransform: "uppercase", marginBottom: 6 }}>{children}</div>;
}

function Row({ label, value }) {
  return (
    <div style={{ marginBottom: 16 }}>
      <FieldLabel>{label}</FieldLabel>
      <div style={{ fontSize: 13, color: "#c8bea0", lineHeight: 1.6 }}>{value}</div>
    </div>
  );
}

function Select({ value, onChange, options }) {
  return (
    <select value={value} onChange={onChange} style={{
      width: "100%",
      background: "#111009",
      color: "#c8a96e",
      border: "1px solid #2a2418",
      padding: "10px 12px",
      fontFamily: "'Courier New', monospace",
      fontSize: 12,
      letterSpacing: 1,
      cursor: "pointer",
      appearance: "none",
    }}>
      {options.map(o => <option key={o} value={o}>{o}</option>)}
    </select>
  );
}

const sliderStyle = {
  width: "100%",
  WebkitAppearance: "none",
  appearance: "none",
  height: 2,
  background: "#2a2418",
  outline: "none",
  cursor: "pointer",
  accentColor: "#c8a96e",
};
