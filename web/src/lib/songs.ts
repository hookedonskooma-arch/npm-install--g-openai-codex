import fs from "fs";
import path from "path";

const DATA_DIR = path.join(process.cwd(), "data");
const SONGS_FILE = path.join(DATA_DIR, "songs.json");

export interface Song {
  id: string;
  title: string;
  prompt: string;
  style: string;
  audio_url: string;
  image_url: string | null;
  created_at: string;
}

function ensureFile(): void {
  if (!fs.existsSync(DATA_DIR)) {
    fs.mkdirSync(DATA_DIR, { recursive: true });
  }
  if (!fs.existsSync(SONGS_FILE)) {
    fs.writeFileSync(SONGS_FILE, "[]", "utf-8");
  }
}

export function readSongs(): Song[] {
  ensureFile();
  const raw = fs.readFileSync(SONGS_FILE, "utf-8");
  try {
    return JSON.parse(raw) as Song[];
  } catch {
    return [];
  }
}

export function saveSong(song: Song): void {
  ensureFile();
  const songs = readSongs();
  const exists = songs.find((s) => s.id === song.id);
  if (!exists) {
    songs.unshift(song);
    fs.writeFileSync(SONGS_FILE, JSON.stringify(songs, null, 2), "utf-8");
  }
}
