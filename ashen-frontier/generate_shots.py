#!/usr/bin/env python3
"""
ASHEN FRONTIER — DALL-E 3 Shot Generator
Generates 5 cinematic shots and saves them to assets/images/
Requires: OPENAI_API_KEY env var
"""

import os
import sys
import requests
from pathlib import Path
import openai

API_KEY = os.environ.get("OPENAI_API_KEY")
if not API_KEY:
    print("ERROR: OPENAI_API_KEY not set. Export it and re-run.")
    sys.exit(1)

client = openai.OpenAI(api_key=API_KEY)

OUT_DIR = Path(__file__).parent / "assets" / "images"
OUT_DIR.mkdir(parents=True, exist_ok=True)

SHOTS = {
    "shot_01": (
        "Cinematic widescreen still, 16:9. A forest clearing at dawn — but wrong. "
        "Dead white birch trees ring the edge, bark stripped. The clearing is bare earth, "
        "no grass. Ash on the ground like snow. Pale grey sky bleeding into pale orange "
        "at the horizon. Fog at the treeline. One set of footprints leading into frame "
        "from the bottom edge. No human visible. "
        "Desaturated color grade, heavy vignette. Film grain. Ultra-realistic. "
        "Moody, dread-inducing. No text. No UI."
    ),
    "shot_02": (
        "Cinematic 16:9. Close shot — survival equipment laid out on dead earth. "
        "A cracked leather canteen, empty. A small knife with a worn handle. "
        "One match, spent. Ash dusted over everything. "
        "Cold blue-grey light from above. No warmth. "
        "The objects feel like evidence of someone already gone. "
        "Film grain. Extreme detail. No text. No UI."
    ),
    "shot_03": (
        "Cinematic 16:9. The treeline at dusk. Dead birch and pine silhouettes against "
        "a bruised purple-grey sky. Deep shadow at the forest floor — you cannot see "
        "what is in there. One branch broken at eye level, recently. "
        "No visible creature, but the composition implies something just moved. "
        "Heavy vignette. Extremely desaturated. One faint warm glow far to the left — "
        "a campfire barely visible, 15% of the frame. Film grain."
    ),
    "shot_04": (
        "Cinematic 16:9. A small campfire in a forest clearing at night. "
        "The fire is the only warm color in the entire frame — deep amber, "
        "against absolute darkness behind it. The light barely reaches 2 feet. "
        "Beyond the firelight: nothing. The treeline is invisible. "
        "A pair of hands — chapped, dirty — held toward the flame from the bottom edge. "
        "The fire is visibly small. Struggling. "
        "Extreme contrast. Film grain. Ultra-realistic."
    ),
    "shot_07": (
        "Cinematic 16:9. A forest clearing at night — same clearing, different moment. "
        "No fire visible. Ash on the ground. Footprints still there. "
        "Absolute darkness at the treeline. Black sky above. "
        "One thin horizontal band of deep red at the horizon, barely visible. "
        "Not dawn. The last of something dying. "
        "Film grain. Maximum vignette. Near silhouette. No text."
    ),
}


def generate_shot(name: str, prompt: str) -> Path:
    out_path = OUT_DIR / f"{name}.jpg"
    if out_path.exists():
        print(f"  SKIP {name} — already exists")
        return out_path

    print(f"  GEN  {name}...")
    response = client.images.generate(
        model="dall-e-3",
        prompt=prompt,
        size="1792x1024",
        quality="hd",
        n=1,
    )

    image_url = response.data[0].url
    img_data = requests.get(image_url, timeout=60).content
    out_path.write_bytes(img_data)
    print(f"  SAVE {out_path} ({len(img_data)//1024} KB)")
    return out_path


if __name__ == "__main__":
    print("ASHEN FRONTIER — DALL-E 3 Shot Generator")
    print(f"Output: {OUT_DIR}")
    print()
    for name, prompt in SHOTS.items():
        generate_shot(name, prompt)
    print("\nAll shots generated.")
