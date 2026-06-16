# Skill Archetypes

Five foundational patterns that cover every skill type. Each archetype includes a structural template, real-world examples from verified skills, and the key patterns that make it work.

---

## Archetype 1: File Transformer

**What it does:** Reads, creates, edits, or converts files in specific formats.

**Verified examples:** docx, pptx, xlsx, pdf, pdf-reading

**Key patterns:**
- Quick Reference table routing to read/create/edit workflows
- Validation loop (create → validate → fix → finalize)
- Anti-pattern wall with wrong/right code examples
- Library-specific critical rules section
- Dependencies section listing required tools

### Template

```markdown
---
name: [format]-skill
description: "Use this skill any time a .[format] file is involved — as input, output, or both.
This includes: [list all operations]. Trigger whenever the user mentions [keywords],
references a .[format] filename, or asks for [output types]. Do NOT use for [adjacent formats]."
---

# [Format] Skill

## Quick Reference

| Task | Approach |
|------|----------|
| Read/analyze content | [tool/method] |
| Create new file | [tool/method] — see Creating below |
| Edit existing file | [tool/method] — see Editing below |
| Convert format | [tool/method] |

## Creating New Files

[Step-by-step with code examples]

### Validation
[How to verify the output is correct — always include this]

## Editing Existing Files

[Step-by-step workflow]

## Common Mistakes

[Anti-pattern wall: concrete wrong/right examples with explanations]

### Critical Rules
- [Rule with WHY it matters]
- [Rule with WHY it matters]

## Quality Checklist
- [ ] Output opens without errors in target application
- [ ] Formatting matches specifications
- [ ] [Domain-specific checks]

## Dependencies
- [Required tools and installation commands]
```

### What makes this archetype work

1. **The validation loop is non-negotiable.** Every file transformer skill must include a step where the output is checked. The docx skill validates XML structure; the xlsx skill checks for formula errors. Without this, broken files get delivered.

2. **Library quirks go in Critical Rules.** Every file library has gotchas — docx-js defaults to A4, WidthType.PERCENTAGE breaks in Google Docs, LibreOffice has sandboxed socket restrictions. These belong in a dedicated section because they're easy to miss but cause hard-to-debug failures.

3. **The Quick Reference table prevents wasted tokens.** Without it, Claude reads the entire skill before deciding which section applies. The table lets it jump to the right workflow immediately.

---

## Archetype 2: Creative Generator

**What it does:** Produces visual, written, or design output where aesthetic quality matters.

**Verified examples:** frontend-design, canvas-design, pptx (design sections)

**Key patterns:**
- Design-first philosophy phase before any code or content
- Bold aesthetic direction with specific anti-slop guardrails
- Color palette and typography guidance with concrete options
- "Unforgettable" differentiator question
- Anti-generic checklist

### Template

```markdown
---
name: [creative]-skill
description: "Create [distinctive/production-grade/beautiful] [output type] with high
[quality measure]. Use when the user asks to [trigger phrases]. Generates creative,
polished output that avoids generic AI aesthetics."
---

# [Creative Skill Name]

## Design Thinking

Before creating anything, commit to a conceptual direction:

1. **Purpose** — What problem does this solve? Who experiences it?
2. **Tone** — Pick a specific aesthetic direction (not "clean and modern"):
   [List 8-12 specific aesthetic options relevant to this domain]
3. **Differentiation** — What makes this UNFORGETTABLE?
4. **Constraints** — Technical requirements, audience, platform

## [Creation Process]

[Step-by-step with the design philosophy feeding into execution]

## Aesthetic Guidelines

### What to Pursue
[Specific, opinionated guidance — typography, color, composition, motion]

### What to Avoid (Anti-Slop)
NEVER default to:
- [Generic choice 1 with why it fails]
- [Generic choice 2 with why it fails]
- [Generic choice 3 with why it fails]

No two outputs should look the same. Vary [specific dimensions] across generations.

## [Palette / Typography / Layout Options]

[Concrete options table — not abstract principles, actual values to choose from]

## Quality Checklist
- [ ] Output has a clear aesthetic point-of-view
- [ ] No generic/default choices remain
- [ ] [Domain-specific quality checks]
```

### What makes this archetype work

1. **The design-first phase prevents convergence.** Without it, outputs cluster around the same "clean, modern, professional" look. The canvas-design skill proves this: forcing a full philosophy manifesto before any visual work produces dramatically more distinctive results.

2. **Concrete options beat abstract principles.** The pptx skill provides 10 named color palettes with hex values and 8 font pairings. This is more useful than "choose colors that match your topic" because it gives the consuming Claude specific starting points to riff on.

3. **Anti-slop guardrails work.** The frontend-design skill's blacklist of overused fonts and cliched color schemes measurably reduces generic output. Naming specific bad choices (Inter, purple gradients, centered layouts) is more effective than saying "be creative."

---

## Archetype 3: Workflow Orchestrator

**What it does:** Guides multi-stage processes where humans make decisions at each stage.

**Verified examples:** doc-coauthoring, skill-creator

**Key patterns:**
- Named stages with clear entry/exit conditions
- Context gathering before any output
- Structured feedback loops at each stage
- Progressive refinement (brainstorm → curate → draft → edit)
- Shorthand-friendly interaction (users can answer briefly)

### Template

```markdown
---
name: [workflow]-skill
description: "Guide users through [workflow description]. Use when user wants to
[trigger phrases]. This workflow helps users [value proposition]. Trigger when user
mentions [keywords] or similar [domain] tasks."
---

# [Workflow Name]

## Overview
[What this workflow does and the three stages]

## When to Offer This Workflow
**Trigger conditions:**
- [Explicit triggers]
- [Implicit triggers]
- [Context signals]

## Stage 1: [Context / Input Gathering]
**Goal:** [What this stage accomplishes]

### Questions to Ask
[Numbered list — max 5 initially, follow up as needed]

### Context Loading
[How to accept info dumps, file references, tool lookups]

**Exit condition:** [When to move on]

## Stage 2: [Creation / Refinement]
**Goal:** [What this stage accomplishes]

### For each section/component:
1. Clarifying questions
2. Brainstorm options (5-20 depending on complexity)
3. User curates (keep/remove/combine)
4. Draft
5. Iterative refinement via targeted edits

**Exit condition:** [When to move on]

## Stage 3: [Validation / Testing]
**Goal:** [What this stage accomplishes]

[How to verify the output works for its intended audience]

**Exit condition:** [Success criteria]

## Tips for Effective Guidance
- [Tone guidance]
- [How to handle deviations]
- [Context management across stages]
```

### What makes this archetype work

1. **Named stages create shared vocabulary.** When both Claude and the user can say "we're in Stage 2," it reduces confusion and makes it easy to skip or revisit stages intentionally.

2. **Shorthand-friendly interaction respects user time.** The doc-coauthoring skill explicitly tells users they can answer "1: yes, 2: no, 3: see #channel" — this dramatically increases completion rates for multi-stage workflows.

3. **Exit conditions prevent premature transitions.** Without clear criteria for when a stage is "done," either Claude rushes ahead or gets stuck in an infinite refinement loop.

---

## Archetype 4: Domain Knowledge

**What it does:** Encodes specialized expertise for consistent application across tasks.

**Verified examples:** chibitek-brand-guidelines, internal-comms, brand-guidelines

**Key patterns:**
- Authoritative source of truth with specific values (hex codes, font sizes, margins)
- Execution instructions for Claude ("When this skill is invoked, ALWAYS...")
- Document-type-specific formatting rules
- Quality checklist with verification steps
- Asset file paths for logos, templates, fonts

### Template

```markdown
---
name: [domain]-guidelines
description: "Applies [organization/domain] [standards/guidelines/rules] to
[output types]. Use whenever creating [deliverables] that require [domain] consistency.
Trigger on: [keywords], [document types], [contexts]."
---

# [Domain] Guidelines

## Purpose
[What these guidelines ensure and why consistency matters]

## Execution Instructions
**When this skill is invoked:**
1. Reference [asset files] at [paths]
2. Apply [standards] as defined below
3. Follow [principles] for content structure
4. Perform quality checklist before presenting output

## [Core Standards]

### [Standard Category 1 — e.g., Color Palette]
[Specific values with usage guidance]

### [Standard Category 2 — e.g., Typography]
[Specific values with hierarchy]

### [Standard Category 3 — e.g., Layout Rules]
[Specific values per document type]

## [Document Type]-Specific Guidelines

### [Type 1 — e.g., Presentations]
[Formatting rules specific to this output type]

### [Type 2 — e.g., Reports]
[Formatting rules specific to this output type]

## Content Principles
[Communication style, structure, tone — how content should be organized]

## Quality Checklist
**Visual Consistency:**
- [ ] [Check 1]
- [ ] [Check 2]

**Content Structure:**
- [ ] [Check 1]
- [ ] [Check 2]

**Professional Polish:**
- [ ] [Check 1]
- [ ] [Check 2]
```

### What makes this archetype work

1. **Specific values, not principles.** "Use brand colors" is useless. "#004F71 for headers, #00B398 for accents, #F2A900 for emphasis" is actionable. The Chibitek skill gets this right — every color has Pantone, CMYK, RGB, and Hex values plus usage guidance.

2. **Document-type routing.** The same brand guidelines apply differently to presentations vs. reports vs. emails. Dedicated sections per output type prevent misapplication.

3. **The quality checklist is the enforcement mechanism.** Without it, guidelines are suggestions. With it, they become requirements that get verified.

---

## Archetype 5: Integration Connector

**What it does:** Connects Claude to external services via APIs, MCP, or tool chains.

**Verified examples:** connect, mcp-builder

**Key patterns:**
- Authentication flow documentation
- Tool naming conventions for discoverability
- Error handling with actionable messages
- Pagination and response formatting
- Setup/installation instructions

### Template

```markdown
---
name: [service]-connector
description: "Connect Claude to [service] for [operations]. Use when the user
wants to [action verbs] in [service]. Supports [list of operations]."
---

# [Service] Connector

## Quick Start

### Setup
[Minimal steps to get connected — API key, install, configure]

### First Use
[What happens on first connection — auth flow, permissions]

## Supported Operations

| Operation | Description | Required Params |
|-----------|-------------|-----------------|
| [op 1] | [what it does] | [params] |
| [op 2] | [what it does] | [params] |

## [Core Operations — detailed]

[Each operation with examples, input/output, edge cases]

## Error Handling

| Error | Cause | Resolution |
|-------|-------|------------|
| [error 1] | [why] | [fix] |
| [error 2] | [why] | [fix] |

## Authentication
[How auth works, what persists, what expires]

## Troubleshooting
[Common issues and fixes]
```

### What makes this archetype work

1. **Auth flow must be documented explicitly.** The connect skill shows exactly what the user sees on first use ("Authorize here: [link]. Say 'connected' when done."). Without this, users get confused by OAuth redirects.

2. **Error tables with resolutions prevent dead ends.** The mcp-builder emphasizes "actionable error messages" — every error should guide toward a solution, not just describe the failure.

3. **Tool naming conventions matter for discoverability.** Consistent prefixes (e.g., `github_create_issue`, `github_list_repos`) help Claude find the right tool. The mcp-builder skill dedicates an entire section to this.
