import os
import asyncio
import discord
from discord import app_commands
from dotenv import load_dotenv
from agents import SunoAgent

load_dotenv()

DISCORD_TOKEN = os.getenv("DISCORD_TOKEN")

intents = discord.Intents.default()
client = discord.Client(intents=intents)
tree = app_commands.CommandTree(client)
suno = SunoAgent()


@client.event
async def on_ready():
    await tree.sync()
    print(f"Logged in as {client.user.name}")


@tree.command(name="generate", description="Generate a song with Suno AI")
@app_commands.describe(
    prompt="Song description or C++ directive",
    style="Style tags (e.g. 'boom bap, lo-fi, 90s NYC')",
    cpp_mode="Interpret prompt as C++ musical directive",
)
async def generate(
    interaction: discord.Interaction,
    prompt: str,
    style: str = "",
    cpp_mode: bool = False,
):
    await interaction.response.defer(ephemeral=True)

    try:
        await interaction.followup.send("Submitting to Suno...", ephemeral=True)
        song_id = await suno.generate(prompt=prompt, style=style, cpp_mode=cpp_mode)

        await interaction.followup.send("Generating... polling for result (up to 2 min)", ephemeral=True)

        elapsed = 0
        while elapsed < 120:
            await asyncio.sleep(5)
            elapsed += 5
            song = await suno.poll(song_id)
            if song["status"] == "complete" and song.get("audio_url"):
                embed = discord.Embed(
                    title=song.get("title") or "Untitled",
                    description=f"**Style:** {style or 'none'}\n**Prompt:** {prompt[:200]}{'...' if len(prompt) > 200 else ''}",
                    color=0x9B59B6,
                )
                embed.add_field(name="Audio", value=song["audio_url"])
                if song.get("image_url"):
                    embed.set_thumbnail(url=song["image_url"])
                await interaction.followup.send(embed=embed, ephemeral=True)
                return
            elif song["status"] == "error":
                await interaction.followup.send("Suno returned an error. Try again.", ephemeral=True)
                return

        await interaction.followup.send("Timed out waiting for Suno.", ephemeral=True)

    except Exception as e:
        await interaction.followup.send(f"Error: {e}", ephemeral=True)


client.run(DISCORD_TOKEN)
