import { NextRequest, NextResponse } from "next/server";
import { pollSong } from "@/lib/suno";

export async function GET(
  _request: NextRequest,
  { params }: { params: { id: string } }
) {
  try {
    const song = await pollSong(params.id);
    return NextResponse.json({
      status: song.status,
      audio_url: song.audio_url,
      title: song.title,
      image_url: song.image_url,
    });
  } catch (err) {
    const message = err instanceof Error ? err.message : "Unknown error";
    return NextResponse.json({ error: message }, { status: 500 });
  }
}
