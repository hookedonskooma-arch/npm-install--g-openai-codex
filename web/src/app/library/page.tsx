"use client";

import { useEffect, useState } from "react";
import Link from "next/link";

interface Song {
  id: string;
  title: string;
  prompt: string;
  style: string;
  audio_url: string;
  image_url: string | null;
  created_at: string;
}

export default function LibraryPage() {
  const [songs, setSongs] = useState<Song[]>([]);
  const [expanded, setExpanded] = useState<Record<string, boolean>>({});

  useEffect(() => {
    fetch("/api/library")
      .then((r) => r.json())
      .then((data) => setSongs(Array.isArray(data) ? data : (data.songs ?? [])));
  }, []);

  function toggleExpand(id: string) {
    setExpanded((prev) => ({ ...prev, [id]: !prev[id] }));
  }

  return (
    <main className="min-h-screen bg-[#0a0a0a] text-[#e5e5e5]">
      <header className="flex items-center justify-between px-6 py-4 border-b border-[#222]">
        <span className="text-xl font-bold tracking-widest text-white">STUDIO</span>
        <Link href="/" className="text-sm text-[#666] hover:text-[#9b59b6] transition-colors">
          ← Generator
        </Link>
      </header>

      <div className="max-w-5xl mx-auto px-4 py-10">
        <h1 className="text-lg font-bold tracking-widest uppercase mb-8 text-[#666]">Library</h1>

        {songs.length === 0 ? (
          <p className="text-[#444] text-sm">No songs saved yet. Generate something and hit &quot;Save to Library&quot;.</p>
        ) : (
          <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-4">
            {songs.map((song) => (
                <div key={song.id} className="border border-[#222] bg-[#111] flex flex-col">
                  {song.image_url && (
                    // eslint-disable-next-line @next/next/no-img-element
                    <img src={song.image_url} alt={song.title} className="w-full h-40 object-cover" />
                  )}
                  <div className="p-3 flex flex-col gap-2 flex-1">
                    <p className="font-bold text-sm truncate">{song.title || "Untitled"}</p>
                    {song.style && (
                      <p className="text-xs text-[#9b59b6] truncate">{song.style}</p>
                    )}
                    <div className="text-xs text-[#444]">
                      <span
                        className={`block ${expanded[song.id] ? "" : "line-clamp-2"}`}
                      >
                        {song.prompt}
                      </span>
                      {song.prompt.length > 120 && (
                        <button
                          onClick={() => toggleExpand(song.id)}
                          className="text-[#666] hover:text-[#9b59b6] mt-1"
                        >
                          {expanded[song.id] ? "less" : "more"}
                        </button>
                      )}
                    </div>
                    <audio controls src={song.audio_url} className="w-full mt-auto" />
                    <p className="text-xs text-[#333]">
                      {new Date(song.created_at).toLocaleDateString()}
                    </p>
                  </div>
                </div>
              ))}
          </div>
        )}
      </div>
    </main>
  );
}
