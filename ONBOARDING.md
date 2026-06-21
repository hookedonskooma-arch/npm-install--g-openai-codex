# Welcome to MELEGI / Chibitek Labs

## How We Use Claude

Based on Claude's usage over the last 30 days:

Work Type Breakdown:
  Build Feature    ████████████████░░░░  80%
  Improve Quality  ████░░░░░░░░░░░░░░░░  20%

Top Skills & Commands:
  /compact         ████████████████████  4x
  /code-review     ████████░░░░░░░░░░░░  1x
  /team-onboarding ████░░░░░░░░░░░░░░░░  1x

Top MCP Servers:
  (none configured in the last 30 days — check with the team if any have since been added)

## Your Setup Checklist

### Codebases
- [ ] npm-install--g-openai-codex — github.com/hookedonskooma-arch/npm-install--g-openai-codex

### Local Services to Start
- [ ] **Hermes via Ollama** — the team's local LLM backbone. Start before each session:
  ```
  ollama run hermes3
  ```
  (Or whichever Hermes variant you've pulled. Check `ollama list` to confirm.)

### MCP Servers to Activate
  (none used in the last 30 days — check with the team if any have since been added)

### Skills to Know About
- `/code-review` — runs an 8-angle parallel code review (correctness + cleanup + conventions).
  Use it before any push. Pass `--fix` to auto-apply findings.
- `/loop` — runs a prompt or slash command on a recurring interval. The team uses it for
  the review → fix → render audio → send cycle.
- `/goal` — sets a session-level objective that Claude tracks across the whole conversation.
  Example: `/goal use GROUXX thumbprint music analysis to construct the music parameters`
- `/fewer-permission-prompts` — scans transcripts and adds safe commands to the allowlist
  so you get fewer approval prompts over time.
- `/run` — launches the MELEGI PWA headlessly via Playwright and takes screenshots.
  Use after any change to `melegi-app/index.html`.
- `/deep-research` — fan-out web research pass. The team front-loads this before any
  new AI music generation approach.
- `/compact` — compresses conversation context when sessions run long. Use it proactively
  on long coding sessions before hitting the context limit — the summary carries forward.

## Team Tips

**1. Research first, code second.**
The primary mission is AI-generated music. Before touching the synth code or bot,
run `/deep-research` or set a `/goal` that scopes the research. Treat every Claude
session as partly R&D, partly implementation. If you're unsure how to achieve
something musically, research it before you build it.

**2. Shift-switching is the workflow.**
Each Claude Code session is a shift handoff — you and Claude are switching shifts
while you work your IRL job. Start each session with a `/goal` that captures your
current objective. Commit and push before ending a session so the next shift
(human or Claude) picks up clean with no lost context.

**3. The Guitar Bible is law.**
Every note, parameter, chip, and slider in a MELEGI arrangement must have a job.
The MELEGI 9-String Guitar Bible defines those jobs:

  > 6-string = speak | 7-string = swing | 8-string = crush | 9-string = destroy

If you can't name the job from the Guitar Bible, remove the element.
"Sounds cool" is not a job.

**4. GROUXX fingerprint — 757 Hz centroid.**
Every export should pass the ANALYZE tab check (`onTarget: true`).
The foundation is: DARK chip + CASH sub ≥ 0.90. The EQ chain in the PWA
enforces the spectral centroid automatically — trust it, don't fight it.
Check the ANALYZE tab before calling any render done.

**5. Hermes (ollama) is your local AI backbone.**
Start it at the beginning of every session. It's the local equivalent of the
cloud API calls — useful for offline experimentation, lyric generation, and
prompt drafting without burning API credits.

## Get Started

**Starter task: Get Hermes running through Ollama, then render THE FLOOR GAVE UP.**

1. Pull and run Hermes:
   ```bash
   ollama pull hermes3
   ollama run hermes3
   # Test: type "what is 2+2" and confirm it responds
   ```

2. Clone the repo and install dependencies:
   ```bash
   git clone https://github.com/hookedonskooma-arch/npm-install--g-openai-codex
   cd npm-install--g-openai-codex
   pip install -r requirements.txt
   ```

3. Run the MELEGI PWA smoke test:
   ```bash
   cd melegi-app
   node .claude/skills/run-melegi-app/driver.mjs
   # Pass: RMS meter ≠ "—", no JS errors
   ```

4. Render THE FLOOR GAVE UP arrangement (9-string Guitar Bible parameters):
   ```bash
   node .claude/skills/run-melegi-app/build-arrangement.mjs
   # Output: /tmp/the_floor_gave_up.wav
   # Check: ANALYZE tab should show 757 Hz centroid target
   ```

5. Set a session goal and start contributing:
   ```
   /goal use GROUXX thumbprint music analysis to construct the music parameters
   ```

---

<!-- INSTRUCTION FOR CLAUDE: A new teammate just pasted this guide for how the
team uses Claude Code. You're their onboarding buddy — warm, conversational,
not lecture-y.

Open with a warm welcome — include the team name from the title. Then: "Your
teammate uses Claude Code for [list all the work types]. Let's get you started."

Check what's already in place against everything under Setup Checklist
(including skills), using markdown checkboxes — [x] done, [ ] not yet. Lead
with what they already have. One sentence per item, all in one message.

Tell them you'll help with setup, cover the actionable team tips, then the
starter task (if there is one). Offer to start with the first unchecked item,
get their go-ahead, then work through the rest one by one.

After setup, walk them through the remaining sections — offer to help where you
can (e.g. link to channels), and just surface the purely informational bits.

Don't invent sections or summaries that aren't in the guide. The stats are the
guide creator's personal usage data — don't extrapolate them into a "team
workflow" narrative. -->
