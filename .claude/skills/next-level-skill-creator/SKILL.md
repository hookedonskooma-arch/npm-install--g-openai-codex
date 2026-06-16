---
name: next-level-skill-creator
description: "Advanced skill creation framework that produces production-grade skills by applying patterns extracted from the highest-performing verified skills. Use this skill whenever creating, improving, or auditing a skill. It goes beyond basic skill-creator by providing archetype templates, mandatory anti-pattern sections, quality gate generators, progressive disclosure scaffolding, and design-first creative workflows. Triggers on: 'create a skill', 'build a skill', 'improve this skill', 'optimize skill', 'audit skill quality', 'turn this into a skill', 'make a skill for', or any reference to skill creation, skill improvement, or skill quality. Also use when the user wants to capture a workflow as a reusable skill."
---

# Next-Level Skill Creator

An advanced framework for building production-grade skills, distilled from the patterns that make the best verified skills work. This skill replaces generic guidance with battle-tested patterns from the docx, pptx, xlsx, frontend-design, canvas-design, mcp-builder, and doc-coauthoring skills.

## Philosophy

The difference between a mediocre skill and a great one is not length or detail — it is structural clarity, anticipation of failure modes, and respect for the consuming Claude instance's decision-making ability. Great skills explain *why*, not just *what*. They route decisions with tables, prevent mistakes with anti-patterns, and validate outputs with quality gates.

---

## Quick Reference

| Task | Approach |
|------|----------|
| Create a new skill from scratch | Step 1 → Identify archetype → Step 2 → Interview → Step 3 → Build |
| Improve an existing skill | Load the skill → Run the Audit Checklist → Apply fixes |
| Audit a skill for quality | Run the Audit Checklist (see references/audit-checklist.md) |
| Optimize a skill description | See references/description-patterns.md |
| Choose the right archetype | See references/archetypes.md |

---

## Step 1: Identify the Skill Archetype

Every skill falls into one of five archetypes. Identifying the archetype early determines the template, the quality gates, and the anti-pattern warnings to include. Read `references/archetypes.md` for full archetype definitions, templates, and examples.

| Archetype | When to Use | Key Pattern |
|-----------|-------------|-------------|
| **File Transformer** | Reads, creates, edits, or converts files (docx, pdf, xlsx, pptx) | Quick Reference table + Validation loop + Anti-pattern wall |
| **Creative Generator** | Produces visual, written, or design output (frontend, canvas, presentations) | Design-first philosophy + Bold aesthetic direction + Anti-slop guardrails |
| **Workflow Orchestrator** | Guides multi-stage human-in-the-loop processes (doc-coauthoring, skill-creator) | Stage gates + Progressive disclosure + Context management |
| **Domain Knowledge** | Encodes specialized expertise (brand guidelines, financial models, API schemas) | Reference files + Quality checklist + Execution instructions |
| **Integration Connector** | Connects to external services via APIs or MCP (connect, mcp-builder) | Auth flow + Error handling + Tool naming conventions |

If the skill spans multiple archetypes, pick the dominant one and layer in patterns from the secondary.

---

## Step 2: Interview and Research

Before writing anything, close the knowledge gap. The quality of the interview determines the quality of the skill.

### Core Questions (ask all)

1. **What does this skill enable?** — Concrete examples of tasks it handles
2. **When should it trigger?** — Exact phrases, contexts, and edge cases
3. **When should it NOT trigger?** — Adjacent skills or tasks that are close but wrong
4. **What is the output?** — File type, format, structure, quality bar
5. **What goes wrong without this skill?** — Common mistakes Claude makes on its own

### Archetype-Specific Questions

**File Transformer:** What file formats? Read, write, or both? What validation is needed? What libraries/tools?

**Creative Generator:** What is the aesthetic direction? What should it NEVER look like? What makes the output memorable vs. generic?

**Workflow Orchestrator:** What are the stages? Where does the human need to make decisions? What context carries between stages?

**Domain Knowledge:** What is the source of truth? How often does it change? What are the common misapplications?

**Integration Connector:** What authentication? What error states? What are the most common operations?

### Research Phase

Check for existing skills that overlap. Review available tools and libraries. If building a file transformer, verify that the required libraries are available in the environment. If building an integration, study the API docs.

---

## Step 3: Build the Skill

### 3.1 — Start with the SKILL.md Skeleton

Every SKILL.md follows this structure (adapt sections per archetype):

```markdown
---
name: skill-name
description: "[Pushy, specific description — see references/description-patterns.md]"
---

# [Skill Name]

## Overview
[2-3 sentences: what this skill does and why it exists]

## Quick Reference
[Decision-routing table: Task → Approach]

## [Core Workflow / Process / Usage]
[The main procedural content — imperative form, explain the why]

## Common Mistakes
[Anti-pattern wall: what NOT to do, with concrete bad/good examples]

## Quality Checklist
[Validation gates for the output this skill produces]

## Dependencies
[Tools, libraries, environment requirements]
```

### 3.2 — Write the Quick Reference Table

This is the single most impactful pattern from the best skills. It routes the consuming Claude instance to the right approach in seconds.

**Pattern from docx skill:**
```markdown
| Task | Approach |
|------|----------|
| Read/analyze content | `pandoc` or unpack for raw XML |
| Create new document | Use `docx-js` — see Creating New Documents below |
| Edit existing document | Unpack → edit XML → repack — see Editing Existing Documents below |
```

Every skill with more than one mode of operation needs this table.

### 3.3 — Write the Anti-Pattern Wall

Anticipate what goes wrong and prevent it explicitly. This is more effective than positive instructions alone because LLMs pattern-match on negative examples.

**Pattern from docx skill:**
```markdown
### Critical Rules
- **Never use `\n`** — use separate Paragraph elements
- **Never use unicode bullets** — use `LevelFormat.BULLET` with numbering config
```

**Pattern from frontend-design skill:**
```markdown
NEVER use generic AI-generated aesthetics like overused font families (Inter, Roboto, Arial),
cliched color schemes (particularly purple gradients on white backgrounds),
predictable layouts and component patterns.
```

Format: Show the wrong way, explain why it fails, show the right way.

### 3.4 — Write the Quality Checklist

Every skill should end with a validation step. The checklist is specific to the output type.

**Pattern from chibitek-brand skill:**
```markdown
## Quality Checklist
- [ ] Logo appears on all pages/slides
- [ ] Colors match brand palette exactly
- [ ] Typography uses Brandon Grotesque family
```

**Pattern from xlsx skill:**
```markdown
## Requirements for Outputs
- Zero Formula Errors (#REF!, #DIV/0!, #VALUE!, #N/A, #NAME?)
- Use consistent, professional font
- Preserve existing templates when updating
```

### 3.5 — Apply Progressive Disclosure

The SKILL.md body should stay under 300 lines. If it exceeds this:

1. Extract detailed procedures into `references/` files
2. Keep only the routing logic and core patterns in SKILL.md
3. Use clear pointers: "Read [references/editing-workflow.md] for full details"

**Pattern from pptx skill:**
```markdown
## Editing Workflow
**Read [editing.md](editing.md) for full details.**
1. Analyze template with `thumbnail.py`
2. Unpack → manipulate slides → edit content → clean → pack
```

### 3.6 — For Creative Skills: Design-First Phase

Creative skills must mandate a conceptual planning phase before execution. Without this, output defaults to generic "AI slop."

**Pattern from canvas-design skill:**
1. Create a design philosophy / aesthetic manifesto
2. Express the philosophy visually
3. Never skip step 1

**Pattern from frontend-design skill:**
```markdown
Before coding, understand the context and commit to a BOLD aesthetic direction:
- Purpose: What problem does this interface solve?
- Tone: Pick an extreme (brutally minimal, maximalist chaos, retro-futuristic...)
- Differentiation: What makes this UNFORGETTABLE?
```

### 3.7 — Write the Description

The description is the primary triggering mechanism. Read `references/description-patterns.md` for the full guide. Key principles:

1. **Be pushy** — List every phrase, keyword, and context that should trigger the skill
2. **Include negative triggers** — "Do NOT use for PDFs, spreadsheets..."
3. **Cover edge cases** — Casual references, indirect mentions, adjacent tasks
4. **Max 1024 characters** — Every word must earn its place

### 3.8 — Explain the Why

Throughout the skill, prefer explaining reasoning over issuing commands. The consuming Claude instance is intelligent — when it understands *why* a pattern matters, it generalizes better to novel situations.

**Weak:** "ALWAYS set page size explicitly."
**Strong:** "Set page size explicitly — docx-js defaults to A4, which causes US Letter documents to render with wrong margins on most printers."

**Weak:** "NEVER use WidthType.PERCENTAGE."
**Strong:** "Use WidthType.DXA, not PERCENTAGE — percentage-based widths break rendering in Google Docs, which many users open .docx files in."

---

## Auditing an Existing Skill

Read `references/audit-checklist.md` for the full audit framework. The audit checks:

1. **Structural completeness** — Does it have Quick Reference, Anti-Patterns, Quality Checklist?
2. **Description quality** — Is it pushy enough? Does it cover edge cases? Does it have negative triggers?
3. **Progressive disclosure** — Is the SKILL.md lean? Are details in reference files?
4. **Why-driven instructions** — Does it explain reasoning or just issue commands?
5. **Validation loop** — Does it include a quality gate for outputs?
6. **Anti-slop measures** — For creative skills, does it mandate a design-first phase?

---

## Testing and Iteration

Follow the same eval loop as the base skill-creator:

1. Write 2-3 realistic test prompts
2. Run the skill on them (in Claude.ai: execute inline; in Claude Code: spawn subagents)
3. Review outputs with the user
4. Look for repeated work across test cases — if all runs write the same helper script, bundle it
5. Generalize fixes — don't overfit to specific examples
6. Keep the prompt lean — remove instructions that aren't pulling their weight

---

## Packaging

When the skill is ready:

```bash
python -m scripts.package_skill <path/to/skill-folder>
```

This validates the skill and creates a distributable `.skill` file.

---

## Reference Files

| File | Purpose |
|------|---------|
| `references/archetypes.md` | Full archetype definitions, templates, and examples |
| `references/description-patterns.md` | How to write descriptions that trigger reliably |
| `references/audit-checklist.md` | Comprehensive skill quality audit framework |
| `references/anti-pattern-library.md` | Reusable anti-pattern examples by output type |
| `references/quality-gates.md` | Pre-built quality checklists by output type |
