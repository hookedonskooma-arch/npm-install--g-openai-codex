"use client";

import { useState, useRef } from "react";
import Link from "next/link";

const STYLE_PRESETS = {
  "East Coast Underground": "east coast underground hip hop, boom bap, gritty, lo-fi, raw bars, jazz samples, 90s NYC",
  "Post-Hardcore Shoegaze": "post-hardcore, shoegaze, distorted guitars, ethereal vocals, reverb-soaked, dreamy, heavy, My Bloody Valentine",
};

interface GeneratedSong {
  songId: string;
  audio_url: string;
  title: string;
  image_url: string | null;
}

export default function HomePage() {
  const [prompt, setPrompt] = useState("");
  const [style, setStyle] = useState("");
  const [title, setTitle] = useState("");
  const [mode, setMode] = useState<"text" | "cpp">("text");
  const [loading, setLoading] = useState(false);
  const [status, setStatus] = useState("");
  const [error, setError] = useState("");
  const [song, setSong] = useState<GeneratedSong | null>(null);
  const [saved, setSaved] = useState(false);
  const pollRef = useRef<ReturnType<typeof setInterval> | null>(null);

  function setPreset(presetName: keyof typeof STYLE_PRESETS) {
    setStyle(STYLE_PRESETS[presetName]);
  }

  async function handleGenerate() {
    if (!prompt.trim()) return;
    setLoading(true);
    setError("");
    setStatus("Submitting to Suno...");
    setSong(null);
    setSaved(false);

    try {
      const res = await fetch("/api/generate", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ prompt, style, title, mode }),
      });
      const data = await res.json();
      if (!res.ok) throw new Error(data.error ?? "Generation failed");

      const { songId } = data;
      setStatus("Generating... (this can take up to 2 minutes)");

      let elapsed = 0;
      pollRef.current = setInterval(async () => {
        elapsed += 3;
        if (elapsed > 120) {
          clearInterval(pollRef.current!);
          setLoading(false);
          setError("Timed out waiting for Suno. Check your library later.");
          return;
        }

        const poll = await fetch(`/api/status/${songId}`);
        const pollData = await poll.json();

        if (pollData.status === "complete" && pollData.audio_url) {
          clearInterval(pollRef.current!);
          setSong({
            songId,
            audio_url: pollData.audio_url,
            title: pollData.title ?? title ?? "Untitled",
            image_url: pollData.image_url ?? null,
          });
          setStatus("");
          setLoading(false);
        } else if (pollData.status === "error") {
          clearInterval(pollRef.current!);
          setError("Suno returned an error. Try again.");
          setLoading(false);
        }
      }, 3000);
    } catch (e) {
      setError(e instanceof Error ? e.message : "Unknown error");
      setLoading(false);
      setStatus("");
    }
  }

  async function handleSave() {
    if (!song) return;
    await fetch("/api/library", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({
        id: song.songId,
        title: song.title,
        prompt,
        style,
        audio_url: song.audio_url,
        image_url: song.image_url,
      }),
    });
    setSaved(true);
  }

  return (
    <main className="min-h-screen bg-[#0a0a0a] text-[#e5e5e5] flex flex-col">
      <header className="flex items-center justify-between px-6 py-4 border-b border-[#222]">
        <span className="text-xl font-bold tracking-widest text-white">STUDIO</span>
        <Link href="/library" className="text-sm text-[#666] hover:text-[#9b59b6] transition-colors">
          Library →
        </Link>
      </header>

      <div className="flex-1 max-w-3xl mx-auto w-full px-4 py-10 flex flex-col gap-6">
        {/* Mode toggle */}
        <div className="flex gap-1 bg-[#111] border border-[#222] p-1 w-fit">
          <button
            onClick={() => setMode("text")}
            className={`px-4 py-1.5 text-sm font-medium transition-colors ${
              mode === "text" ? "bg-[#9b59b6] text-white" : "text-[#666] hover:text-[#e5e5e5]"
            }`}
          >
            Text Prompt
          </button>
          <button
            onClick={() => setMode("cpp")}
            className={`px-4 py-1.5 text-sm font-medium transition-colors ${
              mode === "cpp" ? "bg-[#9b59b6] text-white" : "text-[#666] hover:text-[#e5e5e5]"
            }`}
          >
            C++ Directive
          </button>
        </div>

        {/* Prompt area */}
        <div className="flex flex-col gap-2">
          <label className="text-xs text-[#666] uppercase tracking-widest">
            {mode === "cpp" ? "C++ Directive (interpreted as music)" : "Prompt"}
          </label>
          <textarea
            value={prompt}
            onChange={(e) => setPrompt(e.target.value)}
            placeholder={
              mode === "cpp"
                ? `// Define your track\nstruct Track {\n  int bpm = 90;\n  // dark, introspective verse\n  void verse() {\n    for (int bar = 0; bar < 8; bar++) {\n      flow(aggressive, internal_rhyme);\n    }\n  }\n};`
                : "Describe your song — no character limit. Be as detailed as you want about mood, lyrics, structure, influences..."
            }
            rows={mode === "cpp" ? 12 : 6}
            className={`w-full bg-[#111] border border-[#222] p-3 text-sm resize-y focus:outline-none focus:border-[#9b59b6] transition-colors ${
              mode === "cpp" ? "font-mono text-[#9b59b6]" : "text-[#e5e5e5]"
            }`}
          />
          {mode === "cpp" && (
            <p className="text-xs text-[#444]">
              OpenAI will interpret your C++ as musical directives before sending to Suno.
            </p>
          )}
        </div>

        {/* Style presets */}
        <div className="flex flex-col gap-2">
          <label className="text-xs text-[#666] uppercase tracking-widest">Style Presets</label>
          <div className="flex gap-2 flex-wrap">
            {(Object.keys(STYLE_PRESETS) as (keyof typeof STYLE_PRESETS)[]).map((preset) => (
              <button
                key={preset}
                onClick={() => setPreset(preset)}
                className="px-3 py-1.5 text-sm border border-[#333] hover:border-[#9b59b6] hover:text-[#9b59b6] transition-colors"
              >
                {preset}
              </button>
            ))}
          </div>
        </div>

        {/* Style/tags input */}
        <div className="flex flex-col gap-2">
          <label className="text-xs text-[#666] uppercase tracking-widest">Style / Tags</label>
          <input
            type="text"
            value={style}
            onChange={(e) => setStyle(e.target.value)}
            placeholder="e.g. boom bap, dark, lo-fi, 90s..."
            className="w-full bg-[#111] border border-[#222] p-3 text-sm focus:outline-none focus:border-[#9b59b6] transition-colors"
          />
        </div>

        {/* Title */}
        <div className="flex flex-col gap-2">
          <label className="text-xs text-[#666] uppercase tracking-widest">Title (optional)</label>
          <input
            type="text"
            value={title}
            onChange={(e) => setTitle(e.target.value)}
            placeholder="Leave blank for auto-generated title"
            className="w-full bg-[#111] border border-[#222] p-3 text-sm focus:outline-none focus:border-[#9b59b6] transition-colors"
          />
        </div>

        {/* Generate */}
        <button
          onClick={handleGenerate}
          disabled={loading || !prompt.trim()}
          className={`px-6 py-3 font-bold tracking-widest text-sm uppercase transition-all border ${
            loading
              ? "border-[#9b59b6] text-[#9b59b6] animate-pulse cursor-not-allowed"
              : "border-[#9b59b6] bg-[#9b59b6] text-white hover:bg-transparent hover:text-[#9b59b6]"
          } disabled:opacity-50`}
        >
          {loading ? "Generating..." : "Generate"}
        </button>

        {/* Status */}
        {status && <p className="text-xs text-[#666]">{status}</p>}
        {error && <p className="text-xs text-red-400">{error}</p>}

        {/* Result */}
        {song && (
          <div className="border border-[#222] bg-[#111] p-4 flex flex-col gap-3">
            {song.image_url && (
              // eslint-disable-next-line @next/next/no-img-element
              <img src={song.image_url} alt={song.title} className="w-full max-h-48 object-cover" />
            )}
            <p className="font-bold">{song.title}</p>
            <audio controls src={song.audio_url} className="w-full" />
            <button
              onClick={handleSave}
              disabled={saved}
              className="text-sm border border-[#333] px-3 py-1.5 w-fit hover:border-[#9b59b6] hover:text-[#9b59b6] transition-colors disabled:opacity-50"
            >
              {saved ? "Saved to Library" : "Save to Library"}
            </button>
          </div>
        )}
      </div>
    </main>
  );
}
