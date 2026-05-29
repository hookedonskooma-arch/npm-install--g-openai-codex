# Alfred Studio

Alfred Studio is a production-ready SaaS-style Discord bot built with `discord.py`, OpenAI API integration, modular agents, and persistent memory. This bot is designed for ease of use and prepared for future subscription models.

## Features
- Python based and powered by `discord.py`.
- Integration with OpenAI's API for intelligent responses.
- Modular agent system for scalable and reusable functionality.
- Slash command support for a modern Discord experience.
- Persistent memory for enhanced bot utility.
- Admin-only commands to monitor and enforce permissions.
- Subscription-ready foundation for SaaS bot integration.

## Project Structure
```
alfred-studio/
|-- requirements.txt  # Python dependencies
|-- .env.example      # Example environment variable template
|-- bot.py            # Main entry point for the bot
|-- agents.py         # Modular bot agents and tasks
|-- memory.py         # Persistent memory layer
|-- README.md         # Project documentation
```

## Setup

Make sure you have Python installed (>=3.8 recommended). Clone this repository and follow the instructions below:

1. Install dependencies:
    ```bash
    pip install -r requirements.txt
    ```

2. Rename `.env.example` to `.env` and populate the required secrets:
    ```dotenv
    DISCORD_TOKEN=<Your Discord Bot Token>
    OPENAI_API_KEY=<Your OpenAI API Key>
    ```

3. Run the bot:
    ```bash
    python bot.py
    ```