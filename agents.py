import os
import httpx
from openai import AsyncOpenAI

SUNO_API_BASE = "https://studio-api.suno.ai/api"


class SunoAgent:
    def __init__(self):
        self.api_key = os.getenv("SUNO_API_KEY", "")
        self.openai = AsyncOpenAI(api_key=os.getenv("OPENAI_API_KEY", ""))

    def _headers(self) -> dict:
        if not self.api_key:
            raise ValueError("SUNO_API_KEY is not set")
        return {
            "Authorization": f"Bearer {self.api_key}",
            "Content-Type": "application/json",
        }

    async def _interpret_cpp(self, code: str) -> str:
        completion = await self.openai.chat.completions.create(
            model="gpt-4o",
            messages=[
                {
                    "role": "system",
                    "content": (
                        "You are a music director. Interpret the following C++ code as musical directives. "
                        "Map function names to song sections, loops to repeated patterns, variable names to "
                        "sonic elements, comments to lyrical themes or production notes. Output a rich, "
                        "detailed music generation prompt for Suno in 2-3 paragraphs."
                    ),
                },
                {"role": "user", "content": code},
            ],
        )
        return completion.choices[0].message.content or code

    async def generate(self, prompt: str, style: str = "", cpp_mode: bool = False) -> str:
        final_prompt = await self._interpret_cpp(prompt) if cpp_mode else prompt

        async with httpx.AsyncClient(timeout=30) as client:
            res = await client.post(
                f"{SUNO_API_BASE}/generate/v2/",
                headers=self._headers(),
                json={
                    "prompt": final_prompt,
                    "tags": style,
                    "title": "",
                    "make_instrumental": False,
                    "mv": "chirp-v3-5",
                },
            )
            res.raise_for_status()
            data = res.json()

        clips = data.get("clips", [])
        if not clips or not clips[0].get("id"):
            raise ValueError("Suno returned no clip id")
        return clips[0]["id"]

    async def poll(self, song_id: str) -> dict:
        async with httpx.AsyncClient(timeout=30) as client:
            res = await client.get(
                f"{SUNO_API_BASE}/feed/?ids={song_id}",
                headers=self._headers(),
            )
            res.raise_for_status()
            data = res.json()

        if not data:
            raise ValueError("Suno feed returned empty")

        item = data[0]
        return {
            "id": item.get("id", ""),
            "title": item.get("title", ""),
            "status": item.get("status", ""),
            "audio_url": item.get("audio_url"),
            "image_url": item.get("image_url"),
        }
