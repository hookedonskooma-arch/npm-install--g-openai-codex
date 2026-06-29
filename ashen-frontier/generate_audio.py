#!/usr/bin/env python3
"""
ASHEN FRONTIER — ElevenLabs Audio Generator
Generates: VO lines, ambient music, fire crackle SFX, heartbeat SFX
Requires: ELEVENLABS_API_KEY env var
"""

import os
import sys
import time
from io import BytesIO
from pathlib import Path
from pydub import AudioSegment  # pip install pydub

try:
    from elevenlabs.client import ElevenLabs
    from elevenlabs import VoiceSettings
except ImportError:
    print("ERROR: pip install elevenlabs pydub")
    sys.exit(1)

API_KEY = os.environ.get("ELEVENLABS_API_KEY")
if not API_KEY:
    print("ERROR: ELEVENLABS_API_KEY not set. Export it and re-run.")
    sys.exit(1)

client = ElevenLabs(api_key=API_KEY)
AUDIO_DIR = Path(__file__).parent / "assets" / "audio"
AUDIO_DIR.mkdir(parents=True, exist_ok=True)

# ── Voice settings (calm, hollow, resigned) ───────────────────────────────────
VO_SETTINGS = VoiceSettings(
    stability=0.75,
    similarity_boost=0.80,
    style=0.20,
    use_speaker_boost=False,
)

WHISPER_SETTINGS = VoiceSettings(
    stability=0.50,
    similarity_boost=0.70,
    style=0.10,
    use_speaker_boost=False,
)

# ── VO lines: (filename, text, pause_after_ms, whisper) ──────────────────────
VO_LINES = [
    ("vo_01", "You thought you knew what cold was.", 2000, False),
    ("vo_02", "The forest doesn't want you here.", 2500, False),
    ("vo_03", "Hunger. Cold. The dark.", 800, False),
    ("vo_03b", "One more night.", 2000, False),
    ("vo_04", "You don't win.", 1000, False),
    ("vo_04b", "You just survive a little longer.", 3000, False),
    ("vo_05", "Morning is a long way off.", 0, True),
]

# ── Music prompt ──────────────────────────────────────────────────────────────
MUSIC_PROMPT = (
    "Ambient horror score, cinematic, 65 seconds. "
    "Sparse — mostly silence and texture. "
    "A sustained low drone in C minor throughout. "
    "Cold, hollow synth pad — like wind through metal. "
    "No drums. No melody. "
    "A faint distant choir (wordless) enters at 45 seconds, barely audible. "
    "Fire crackle texture woven in from 28-38 seconds. "
    "Single low piano note at 50 seconds — once, not repeated. "
    "The score should feel like an ending, not a beginning. "
    "Film score quality. No jump scares. Pure atmosphere. "
    "Duration: 65 seconds. Fade out last 5 seconds."
)

# ── SFX prompts ───────────────────────────────────────────────────────────────
SFX_FIRE = "Quiet campfire in a forest at night, no wind, intimate, close mic"
SFX_HEARTBEAT = "Single slow heartbeat, slightly muffled, like heard through a wall"


def save_bytes(data: bytes, path: Path):
    path.write_bytes(data)
    print(f"  SAVE {path.name} ({len(data)//1024} KB)")


def generate_vo_line(name: str, text: str, whisper: bool) -> bytes:
    out = AUDIO_DIR / f"{name}.mp3"
    if out.exists():
        print(f"  SKIP {name} — exists")
        return out.read_bytes()

    print(f"  VO   {name}: \"{text[:50]}...\"" if len(text) > 50 else f"  VO   {name}: \"{text}\"")
    settings = WHISPER_SETTINGS if whisper else VO_SETTINGS
    audio_iter = client.text_to_speech.convert(
        text=text,
        voice_id="pNInz6obpgDQGcFmaJgB",  # Adam — gravelly, hollow
        model_id="eleven_multilingual_v2",
        voice_settings=settings,
        output_format="mp3_44100_128",
    )
    data = b"".join(audio_iter)
    save_bytes(data, out)
    return data


def generate_music() -> None:
    out = AUDIO_DIR / "music_track.mp3"
    if out.exists():
        print("  SKIP music_track — exists")
        return
    print("  MUSIC generating (65s ambient horror score)...")
    result = client.text_to_sound_effects.convert(
        text=MUSIC_PROMPT,
        duration_seconds=65,
        prompt_influence=0.5,
    )
    data = b"".join(result)
    save_bytes(data, out)


def generate_sfx(name: str, prompt: str, duration: int) -> None:
    out = AUDIO_DIR / f"{name}.mp3"
    if out.exists():
        print(f"  SKIP {name} — exists")
        return
    print(f"  SFX  {name}...")
    result = client.text_to_sound_effects.convert(
        text=prompt,
        duration_seconds=duration,
        prompt_influence=0.5,
    )
    data = b"".join(result)
    save_bytes(data, out)


def assemble_vo_track() -> None:
    out = AUDIO_DIR / "vo_track.mp3"
    if out.exists():
        print("  SKIP vo_track — exists")
        return

    print("  ASSEMBLING vo_track.mp3...")
    combined = AudioSegment.silent(duration=10_000)  # 10s lead-in silence

    # Timings (ms from start of trailer) for VO entry
    # Shot 2 VO starts at 10s → ms 10000
    cue_times_ms = {
        "vo_01":  10_000,
        "vo_02":  18_000,
        "vo_03":  28_500,
        "vo_03b": 31_000,
        "vo_04":  38_500,
        "vo_04b": 40_500,
        "vo_05":  57_000,
    }

    max_end = 0
    for name, text, pause_after_ms, whisper in VO_LINES:
        path = AUDIO_DIR / f"{name}.mp3"
        if not path.exists():
            print(f"  WARN {name}.mp3 missing — skipping")
            continue
        seg = AudioSegment.from_mp3(str(path))
        cue = cue_times_ms.get(name, 0)
        needed = cue + len(seg) + pause_after_ms
        if needed > max_end:
            max_end = needed

    # Build combined track long enough
    combined = AudioSegment.silent(duration=max_end + 3000)
    for name, text, pause_after_ms, whisper in VO_LINES:
        path = AUDIO_DIR / f"{name}.mp3"
        if not path.exists():
            continue
        seg = AudioSegment.from_mp3(str(path))
        cue = cue_times_ms.get(name, 0)
        combined = combined.overlay(seg, position=cue)

    combined.export(str(out), format="mp3")
    print(f"  SAVE vo_track.mp3 ({len(combined)//1000}s)")


if __name__ == "__main__":
    print("ASHEN FRONTIER — ElevenLabs Audio Generator")
    print(f"Output: {AUDIO_DIR}")
    print()

    print("[1/4] Generating VO lines...")
    for name, text, pause_after_ms, whisper in VO_LINES:
        generate_vo_line(name, text, whisper)
        time.sleep(0.5)  # rate-limit headroom

    print()
    print("[2/4] Generating ambient music score...")
    generate_music()

    print()
    print("[3/4] Generating SFX...")
    generate_sfx("fire_crackle", SFX_FIRE, 15)
    time.sleep(0.5)
    generate_sfx("heartbeat", SFX_HEARTBEAT, 3)

    print()
    print("[4/4] Assembling VO track...")
    assemble_vo_track()

    print("\nAll audio generated. Run Remotion next.")
