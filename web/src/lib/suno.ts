const SUNO_API_BASE = "https://studio-api.suno.ai/api";

export interface SunoGenerateParams {
  prompt: string;
  tags?: string;
  title?: string;
}

export interface SunoSong {
  id: string;
  title: string;
  status: string;
  audio_url: string | null;
  image_url: string | null;
  tags: string | null;
}

interface SunoGenerateResponse {
  clips: Array<{
    id: string;
    status: string;
  }>;
}

interface SunoFeedItem {
  id: string;
  title: string;
  status: string;
  audio_url: string | null;
  image_url: string | null;
  metadata?: {
    tags?: string;
  };
}

function getHeaders(): HeadersInit {
  const key = process.env.SUNO_API_KEY;
  if (!key) throw new Error("SUNO_API_KEY is not set");
  return {
    Authorization: `Bearer ${key}`,
    "Content-Type": "application/json",
  };
}

export async function generateSong(params: SunoGenerateParams): Promise<string> {
  const body = {
    prompt: params.prompt,
    tags: params.tags ?? "",
    title: params.title ?? "",
    make_instrumental: false,
    mv: "chirp-v3-5",
  };

  const res = await fetch(`${SUNO_API_BASE}/generate/v2/`, {
    method: "POST",
    headers: getHeaders(),
    body: JSON.stringify(body),
  });

  if (!res.ok) {
    const text = await res.text();
    throw new Error(`Suno generate failed: ${res.status} ${text}`);
  }

  const data: SunoGenerateResponse = await res.json();
  const clip = data.clips?.[0];
  if (!clip?.id) throw new Error("Suno returned no clip id");
  return clip.id;
}

export async function pollSong(id: string): Promise<SunoSong> {
  const res = await fetch(`${SUNO_API_BASE}/feed/?ids=${id}`, {
    headers: getHeaders(),
  });

  if (!res.ok) {
    const text = await res.text();
    throw new Error(`Suno feed failed: ${res.status} ${text}`);
  }

  const data: SunoFeedItem[] = await res.json();
  const item = data[0];
  if (!item) throw new Error("Suno feed returned empty");

  return {
    id: item.id,
    title: item.title ?? "",
    status: item.status,
    audio_url: item.audio_url ?? null,
    image_url: item.image_url ?? null,
    tags: item.metadata?.tags ?? null,
  };
}
