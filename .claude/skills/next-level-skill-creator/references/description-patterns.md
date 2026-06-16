# Description Patterns

The `description` field in SKILL.md frontmatter is the primary mechanism that determines whether Claude invokes a skill. It is always in context (~100 words max, 1024 characters hard limit). Every word must earn its place.

---

## The Anatomy of a Great Description

A production-grade description has four components:

### 1. Core Function (what it does)
One sentence stating what the skill enables.

### 2. Trigger Phrases (when to use it)
Explicit list of phrases, keywords, contexts, and file types that should activate the skill. Be pushy — Claude tends to under-trigger, so err on the side of inclusion.

### 3. Negative Triggers (when NOT to use it)
Explicit list of adjacent tasks or formats where this skill should NOT activate. This prevents false positives.

### 4. Edge Case Coverage (the non-obvious triggers)
Casual mentions, indirect references, and situations where a user needs the skill without naming it.

---

## Patterns from the Best Descriptions

### Pattern: File Format Skill (docx, pptx, xlsx)

**What works:**
- Lists every operation: "creating, reading, parsing, extracting, editing, modifying, updating, combining, splitting"
- Covers casual mentions: "even casually (like 'the xlsx in my downloads')"
- Covers indirect needs: "even if the extracted content will be used elsewhere, like in an email or summary"
- Explicit negative triggers: "Do NOT trigger when the primary deliverable is a Word document, HTML report..."

**Example (pptx):**
```
Use this skill any time a .pptx file is involved in any way — as input, output,
or both. This includes: creating slide decks, pitch decks, or presentations;
reading, parsing, or extracting text from any .pptx file (even if the extracted
content will be used elsewhere, like in an email or summary); editing, modifying,
or updating existing presentations; combining or splitting slide files; working
with templates, layouts, speaker notes, or comments. Trigger whenever the user
mentions "deck," "slides," "presentation," or references a .pptx filename,
regardless of what they plan to do with the content afterward. If a .pptx file
needs to be opened, created, or touched, use this skill.
```

### Pattern: Creative Skill (frontend-design)

**What works:**
- States the quality bar: "distinctive, production-grade"
- Lists concrete trigger contexts: "websites, landing pages, dashboards, React components, HTML/CSS layouts"
- Anti-generic commitment: "avoids generic AI aesthetics"

**Example (frontend-design):**
```
Create distinctive, production-grade frontend interfaces with high design quality.
Use this skill when the user asks to build web components, pages, artifacts,
posters, or applications (examples include websites, landing pages, dashboards,
React components, HTML/CSS layouts, or when styling/beautifying any web UI).
Generates creative, polished code and UI design that avoids generic AI aesthetics.
```

### Pattern: Workflow Skill (doc-coauthoring)

**What works:**
- Lists document types: "documentation, proposals, technical specs, decision docs"
- States the value: "helps users efficiently transfer context, refine content, verify the doc works"
- Multiple trigger phrasings: "writing docs, creating proposals, drafting specs"

### Pattern: Domain Knowledge Skill (brand-guidelines)

**What works:**
- Clear scope: "presentations, reports, and documents"
- Specificity: names the organization and what the standards cover

---

## Writing Rules

### Be Pushy (Claude Under-Triggers)
Claude has a tendency to not invoke skills when they would be helpful. Combat this by making descriptions slightly aggressive about when to trigger.

**Weak:** "Helps with PDF tasks"
**Strong:** "Use this skill whenever the user wants to do anything with PDF files. This includes reading or extracting text/tables from PDFs, combining or merging multiple PDFs into one, splitting PDFs apart, rotating pages, adding watermarks, creating new PDFs, filling PDF forms, encrypting/decrypting PDFs, extracting images, and OCR on scanned PDFs. If the user mentions a .pdf file or asks to produce one, use this skill."

### Cover the Full Verb Space
Users express the same intent many ways. Cover all of them:
- "create / make / build / generate / produce / write / draft"
- "edit / modify / update / change / fix / revise / tweak"
- "read / analyze / extract / parse / review / inspect / examine"

### Include Casual and Indirect References
- "even casually (like 'the xlsx in my downloads')"
- "regardless of what they plan to do with the content afterward"
- "even if the user doesn't explicitly ask for a 'dashboard'"

### Negative Triggers Prevent Conflicts
When two skills could plausibly trigger on the same query, negative triggers resolve the conflict:
- "Do NOT use for PDFs, spreadsheets, Google Docs, or general coding tasks"
- "Do NOT trigger when the primary deliverable is a Word document"

### Max 1024 Characters
This is a hard limit. If the description exceeds it, prioritize:
1. Core function (keep)
2. Top trigger phrases (keep)
3. Negative triggers (keep)
4. Edge cases (trim if needed)

---

## Description Optimization Process

After the skill is functionally complete:

1. **Generate 20 eval queries** — 10 should-trigger, 10 should-not-trigger
2. **Make them realistic** — include file paths, personal context, casual speech, typos
3. **Focus on edge cases** — the obvious triggers work; test the near-misses
4. **Review with the user** — bad eval queries produce bad descriptions
5. **Iterate** — test, measure trigger rate, adjust wording, repeat

### What Makes a Good Eval Query

**Bad (too obvious):**
```json
{"query": "Create a PDF", "should_trigger": true}
{"query": "Write a poem", "should_trigger": false}
```

**Good (realistic, edge-case focused):**
```json
{"query": "ok so my boss just sent me this xlsx file (its in my downloads, called something like 'Q4 sales final FINAL v2.xlsx') and she wants me to add a column that shows the profit margin as a percentage", "should_trigger": true}
{"query": "I have some CSV data about customer churn rates, can you help me build a machine learning model to predict which customers will leave?", "should_trigger": false}
```

The should-not-trigger queries should be near-misses — they share keywords with the skill but actually need something different.
