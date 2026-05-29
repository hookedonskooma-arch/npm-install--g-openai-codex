import os
from dotenv import load_dotenv
from discord.ext import commands

load_dotenv()

DISCORD_TOKEN = os.getenv("DISCORD_TOKEN")

bot = commands.Bot(command_prefix="!", intents=commands.Intents.all())

@bot.event
dasync def on_ready():
    print(f"Logged in as {bot.user.name}")

# Load slash command and agent extensions here in future
bot.run(DISCORD_TOKEN)