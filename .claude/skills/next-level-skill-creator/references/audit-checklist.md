# Skill Audit Checklist

A comprehensive framework for evaluating skill quality. Run this audit when improving an existing skill or before finalizing a new one.

---

## Scoring

Each category is scored Pass / Partial / Fail. A production-grade skill passes all categories.

---

## 1. Structural Completeness

### Quick Reference Table
- [ ] **Present** — Skill has a decision-routing table at the top
- [ ] **Complete** — Every mode of operation is listed
- [ ] **Actionable** — Each row points to a specific section or approach

**Pass:** Table exists, covers all modes, each row routes clearly.
**Partial:** Table exists but incomplete or has dead-end rows.
**Fail:** No routing table; consuming Claude must read the entire skill to decide what to do.

### Anti-Pattern Wall
- [ ] **Present** — Skill includes a "Common Mistakes" or "Critical Rules" section
- [ ] **Concrete** — Shows wrong examples with explanations, not just "don't do X"
- [ ] **Why-Driven** — Each anti-pattern explains why the wrong way fails

**Pass:** Concrete wrong/right examples with reasoning.
**Partial:** Lists rules without examples or without explaining why.
**Fail:** No anti-pattern section; mistakes are not anticipated.

### Quality Checklist
- [ ] **Present** — Skill includes a verification checklist at or near the end
- [ ] **Specific** — Checklist items are verifiable (not "looks good")
- [ ] **Output-Typed** — Checklist matches the actual output format

**Pass:** Checklist with verifiable, output-specific items.
**Partial:** Checklist exists but items are vague or generic.
**Fail:** No quality gate; output is unvalidated.

### Validation Loop
- [ ] **Present** — For file-producing skills, a create → validate → fix cycle exists
- [ ] **Automated** — Validation uses a script or tool, not just manual inspection
- [ ] **Actionable on Failure** — Validation errors include remediation steps

**Pass:** Automated validation with clear fix paths.
**Partial:** Validation mentioned but manual or incomplete.
**Fail:** No validation step; broken outputs get delivered.

---

## 2. Description Quality

### Triggering Coverage
- [ ] **Core operations** listed (create, read, edit, convert, etc.)
- [ ] **Casual references** covered (indirect mentions, filename references)
- [ ] **Adjacent tasks** excluded with negative triggers
- [ ] **Edge cases** addressed (non-obvious contexts where the skill applies)

**Pass:** Comprehensive coverage with positive and negative triggers.
**Partial:** Core operations listed but edge cases or negative triggers missing.
**Fail:** Generic description that could match too broadly or too narrowly.

### Push Factor
- [ ] **Aggressive enough** — Claude tends to under-trigger; description pushes for inclusion
- [ ] **Verb coverage** — Multiple phrasings of the same intent included
- [ ] **Context signals** — Non-keyword contexts that should trigger are listed

**Pass:** Pushy description that errs on the side of triggering.
**Partial:** Adequate but could miss edge cases.
**Fail:** Passive description that relies on exact keyword matches.

### Character Budget
- [ ] **Under 1024 characters** — Hard limit
- [ ] **No wasted words** — Every phrase contributes to triggering accuracy
- [ ] **No angle brackets** — Technical limitation; < and > are not allowed

---

## 3. Progressive Disclosure

### SKILL.md Length
- [ ] **Under 300 lines** — Core procedures and routing only
- [ ] **No duplicated content** — Information lives in one place
- [ ] **Clear pointers** — References to external files include "Read [file] for [purpose]"

**Pass:** SKILL.md is lean with clear routing to reference files.
**Partial:** Somewhat long but organized.
**Fail:** Monolithic SKILL.md with everything in one file; consumes excessive context.

### Reference File Organization
- [ ] **References exist** for detailed procedures (in `references/`)
- [ ] **Scripts exist** for deterministic/repeated code (in `scripts/`)
- [ ] **Assets exist** for output resources (in `assets/`)
- [ ] **Large references** (300+ lines) include a table of contents or grep patterns

**Pass:** Clean separation between routing (SKILL.md), documentation (references/), code (scripts/), and output resources (assets/).
**Partial:** Some separation but could be cleaner.
**Fail:** Everything in SKILL.md or files with no clear organization.

---

## 4. Instruction Quality

### Why-Driven Instructions
- [ ] **Reasoning present** — Instructions explain why, not just what
- [ ] **Context given** — Constraints are explained (e.g., "this breaks in Google Docs")
- [ ] **Minimal ALL-CAPS** — Rare, reserved for genuinely critical safety issues

**Pass:** Instructions read like guidance from an expert who understands the constraints.
**Partial:** Some instructions explain why, others are bare commands.
**Fail:** Littered with ALWAYS/NEVER/MUST without reasoning; reads like a compliance checklist.

### Imperative Form
- [ ] **Verb-first** — "To accomplish X, do Y" not "You should do X"
- [ ] **Consistent** — Same voice throughout
- [ ] **Instructional** — Reads as a procedure manual, not a conversation

### Example Quality
- [ ] **Concrete** — Examples use real values, not placeholders
- [ ] **Correct** — Examples actually work if copied and executed
- [ ] **Paired** — Wrong/right examples shown side-by-side where relevant

---

## 5. Creative Quality (Creative Generator skills only)

### Design-First Phase
- [ ] **Mandatory** — Cannot skip to execution without conceptual planning
- [ ] **Specific options** — Lists concrete aesthetic directions, not just "be creative"
- [ ] **Differentiator question** — Asks "what makes this unforgettable?"

### Anti-Slop Guardrails
- [ ] **Named bad defaults** — Specific fonts, colors, patterns to avoid
- [ ] **Variety enforcement** — Instruction to vary outputs across generations
- [ ] **Quality bar stated** — "Production-grade," "meticulously crafted," etc.

### Concrete Options
- [ ] **Color palettes** with hex values and usage guidance
- [ ] **Font pairings** with specific names and sizes
- [ ] **Layout patterns** with descriptions

---

## 6. Domain Accuracy (Domain Knowledge skills only)

### Source of Truth
- [ ] **Values are specific** — Hex codes, Pantone numbers, font sizes, margins
- [ ] **Asset paths documented** — Logo files, templates, fonts are referenced with full paths
- [ ] **Document-type routing** — Different rules for presentations vs. reports vs. other formats

### Enforcement
- [ ] **Quality checklist** verifies brand/domain compliance
- [ ] **Execution instructions** tell Claude what to do when the skill is invoked
- [ ] **Priority order** is clear when guidelines conflict

---

## 7. Integration Quality (Connector skills only)

### Setup
- [ ] **Minimal steps** — Fewest possible steps to get connected
- [ ] **Auth flow documented** — What the user sees and does on first connection
- [ ] **Persistence clarity** — What persists vs. what expires

### Error Handling
- [ ] **Error table** with cause and resolution for each failure mode
- [ ] **Actionable messages** — Errors guide toward solutions
- [ ] **Graceful degradation** — Partial failures don't halt everything

---

## Running the Audit

To audit a skill:

1. Read the SKILL.md and all reference files
2. Score each category above as Pass / Partial / Fail
3. Prioritize fixes: Fail items first, then Partial
4. Apply the archetype-specific sections only if relevant
5. Report findings with specific recommendations

### Output Format

```
## Skill Audit: [skill-name]

### Summary
- Structural Completeness: [Pass/Partial/Fail]
- Description Quality: [Pass/Partial/Fail]
- Progressive Disclosure: [Pass/Partial/Fail]
- Instruction Quality: [Pass/Partial/Fail]
- [Archetype-specific]: [Pass/Partial/Fail]

### Findings
[Specific issues and recommendations]

### Priority Fixes
1. [Most impactful fix]
2. [Second most impactful]
3. [Third]
```
