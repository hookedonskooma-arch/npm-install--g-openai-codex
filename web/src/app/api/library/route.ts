import { NextRequest, NextResponse } from "next/server";
import { readSongs, saveSong } from "@/lib/songs";
import type { Song } from "@/lib/songs";

export async function GET() {
  try {
    const songs = readSongs();
    return NextResponse.json({ songs });
  } catch (err) {
    const message = err instanceof Error ? err.message : "Unknown error";
    return NextResponse.json({ error: message }, { status: 500 });
  }
}

export async function POST(request: NextRequest) {
  try {
    const body = await request.json();
    const song: Song = {
      id: body.id,
      title: body.title ?? "Untitled",
      prompt: body.prompt ?? "",
      style: body.style ?? "",
      audio_url: body.audio_url,
      image_url: body.image_url ?? null,
      created_at: new Date().toISOString(),
    };

    if (!song.id || !song.audio_url) {
      return NextResponse.json({ error: "id and audio_url are required" }, { status: 400 });
    }

    saveSong(song);
    return NextResponse.json({ ok: true });
  } catch (err) {
    const message = err instanceof Error ? err.message : "Unknown error";
    return NextResponse.json({ error: message }, { status: 500 });
  }
}
