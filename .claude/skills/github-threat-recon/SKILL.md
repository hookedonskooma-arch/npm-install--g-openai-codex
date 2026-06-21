---
name: github-threat-recon
description: "Full-spectrum GitHub repository threat analysis and code reconstruction. Use when a user shares a GitHub URL or pasted code and wants: nefarious intent detection, backdoor/malware/C2 scanning, supply chain risk assessment, feature documentation (current + planned), or code reconstruction with hardened security, efficiency, and stability. Triggers: 'analyze this repo', 'is this code safe', 'check for backdoors', 'threat analysis', 'document this codebase', 'rewrite with security', 'harden this code', 'find hidden features', 'extract the roadmap', 'supply chain check', 'malicious code', 'is this safe to install', 'audit this package', 'what features are planned', 'enhance this code', 'production harden', 'nefarious intent'. Do NOT use for: general debugging, writing features from scratch, or reviewing the user's own code unless they request threat analysis or reconstruction."
---

# GitHub Threat Reconnaissance & Code Reconstruction

Full-spectrum intelligence workflow for external GitHub repositories. Performs deep threat analysis, documents all features (current and future), and reconstructs code with security hardening, efficiency improvements, and production-grade stability. Built on OWASP Top 10 (2025), CWE Top 25, MITRE ATT&CK, and the Trail of Bits security research corpus.

## Quick Reference

| Trigger | Mode | What Happens |
|---------|------|--------------|
| "Is this safe?" / "check for backdoors" | THREAT mode | Full nefarious intent + supply chain scan |
| "What does this do?" / "document this repo" | RECON mode | Feature extraction + future roadmap analysis |
| "Rewrite with improvements" / "harden this" | RECONSTRUCT mode | Security + efficiency + stability rebuild |
| GitHub URL with no additional context | FULL mode | All three phases in sequence |
| Pasted code with security concerns | THREAT + RECONSTRUCT | Skip recon, focus on threat + rebuild |

When in doubt, run FULL mode. Cost is time, not accuracy.

---

## Phase 0: Ingestion

**Goal:** Acquire the repository content before any analysis begins.

### For GitHub URLs
```
1. web_fetch the raw GitHub URL or use bash_tool:
   git clone --depth 1 <repo_url> /tmp/recon_target
   
2. Run the fingerprint sweep:
   find /tmp/recon_target -type f | head -200
   cat /tmp/recon_target/package.json (or requirements.txt, go.mod, Cargo.toml)
   cat /tmp/recon_target/README.md
   
3. Capture the metadata block:
   - Star count, fork count, contributor count, age
   - Last commit date, commit frequency
   - License type
   - Dependency count and age
```

### For Pasted Code
Skip ingestion. Begin Phase 1 immediately with the provided content.

### Fingerprint Output (required before proceeding)
```
REPO FINGERPRINT
================
Name:          [repo name]
Language(s):   [primary + secondary]
Size:          [file count, LOC estimate]
Dependencies:  [count] ([ecosystem])
Entry Points:  [main files, scripts, executables]
Trust Signals: [stars, org, verified publisher, age]
Red Flags:     [0-day deps, no README, anonymous owner, etc.]
```

---

## Phase 1: Threat Analysis (THREAT Mode)

**Goal:** Detect nefarious intent, malicious patterns, and supply chain risk with zero false-negative tolerance.

Read `references/threat-taxonomy.md` for the complete detection ruleset.

### Layer 1: Intent Signals

These are the highest-signal indicators. Check first.

```
IDENTITY RISK
□ Anonymous maintainer with no public profile
□ Repository created within 30 days of first release
□ Sudden spike in stars (bot farming pattern)
□ Name closely resembles popular legitimate package (typosquatting)
□ Package name differs from GitHub repo name (dependency confusion)

BEHAVIORAL RISK
□ README promises capabilities disproportionate to codebase size
□ Claims to automate valuable/privileged actions without explanation
□ Excessive permissions requested vs. stated functionality
□ Social engineering language ("required", "urgent", "official replacement")
```

### Layer 2: Code Pattern Scan

Run every check in this layer. Never skip.

```
OBFUSCATION PATTERNS
□ Base64 encoded strings that decode to executable content
□ eval(), exec(), Function() with dynamic string construction
□ String.fromCharCode() chains
□ Hex-encoded payloads
□ Multi-layer encoding (base64 → gzip → base64)
□ Variable names that appear random or meaninglessly short across the codebase

EXFILTRATION PATTERNS
□ HTTP/HTTPS calls to external domains not listed in README
□ DNS lookups to non-infrastructure domains
□ WebSocket connections to external endpoints
□ Environment variable harvesting (process.env dumping, os.environ scraping)
□ File system reads outside expected working directories
□ Credential file targeting (~/.ssh/, ~/.aws/, ~/.npmrc, .env files)
□ Clipboard access
□ Screen capture or input recording

PERSISTENCE PATTERNS
□ Cron job installation
□ LaunchAgent/LaunchDaemon plist creation (macOS)
□ Windows Registry modifications
□ Startup script modification (.bashrc, .zshrc, .profile injection)
□ Kernel module loading
□ Scheduled task creation

PRIVILEGE ESCALATION
□ sudo calls without explicit user consent flow
□ setuid/setgid manipulation
□ Token theft from environment
□ SSH key injection
□ Cloud credential role assumption without consent

COMMAND & CONTROL (C2) PATTERNS
□ Polling loops to external URLs
□ Dynamic domain generation (DGA patterns)
□ IRC/Discord/Telegram bot channel connections
□ Encrypted tunneling setup (custom TLS, Tor integration)
□ Reverse shell setup (nc, bash -i, socat patterns)
□ Process hollowing or injection APIs
```

### Layer 3: Dependency Audit

```
FOR EACH DEPENDENCY:
□ Verify package exists on official registry (npm, PyPI, crates.io, etc.)
□ Check package age — new packages (<6 months) with wide permissions = risk
□ Check maintainer history — abandoned then re-adopted packages = risk
□ Check version pinning — unpinned ranges allow silent injection
□ Cross-reference known malicious package databases:
   - OSV.dev (osv.dev/list)
   - Socket.dev threat intel
   - Snyk vulnerability database
□ Check for install scripts (postinstall, setup.py install hooks)
   — These execute code at install time, not runtime
```

### Layer 4: Infrastructure Analysis

```
□ Hard-coded IPs or domains — classify each as legitimate CDN or suspicious
□ API endpoints — are they documented? Do they match stated function?
□ Webhook URLs — where do they point?
□ File paths — are they accessing system-level locations?
□ Ports — are non-standard ports opened?
```

### Threat Scoring

After all four layers, produce a Threat Assessment:

```
THREAT ASSESSMENT
=================
Overall Risk:     [ CRITICAL | HIGH | MEDIUM | LOW | CLEAN ]

Findings:
  CRITICAL: [count] — [brief list]
  HIGH:     [count] — [brief list]
  MEDIUM:   [count] — [brief list]
  LOW:      [count] — [brief list]

Verdict: [ DO NOT INSTALL | INSTALL WITH CAUTION | CONDITIONALLY SAFE | SAFE ]

Top 3 Findings:
1. [Finding name] — [CWE/ATT&CK reference] — [exact file:line]
2. [Finding name] — [CWE/ATT&CK reference] — [exact file:line]
3. [Finding name] — [CWE/ATT&CK reference] — [exact file:line]
```

---

## Phase 2: Feature Reconnaissance (RECON Mode)

**Goal:** Produce a complete feature map — documented, implied, and planned.

Read `references/feature-extractor.md` for extraction methodology.

### Current Feature Extraction

```
SOURCE 1 — README and Docs
□ Every bullet point, heading, and feature claim
□ Usage examples → infer capabilities from examples, not just descriptions
□ Configuration options → each config key is a feature
□ API documentation → each endpoint is a feature

SOURCE 2 — Code Structure
□ Every public function/class/method = a feature or sub-feature
□ Every route/endpoint/handler
□ Every CLI flag and command
□ Every config schema field
□ Event emitters/listeners → infer the event-driven feature surface

SOURCE 3 — Tests
□ Test names often describe features more precisely than docs
□ Integration tests reveal the expected end-to-end workflows
□ Mock objects reveal what external systems the code interacts with
```

### Future Feature Extraction

This is where most reviewers stop. Don't.

```
EXTRACTION TARGETS
□ TODO / FIXME / HACK / XXX comments → planned work
□ Commented-out code blocks → abandoned or deferred features
□ Stub functions (raise NotImplementedError, throw new Error('Not implemented'))
□ Feature flags / toggles (especially disabled ones)
□ README sections marked "Coming soon", "Planned", "Roadmap", "WIP"
□ CHANGELOG entries with "Planned" or future dates
□ GitHub Issues referenced in comments (fetch them if public)
□ Version constants with increments not yet shipped
□ Database migration files with schema extensions not yet wired up
□ Configuration options that exist but have no handler
□ Dead code paths that appear to be unfinished implementations
```

### Feature Map Output

```
FEATURE MAP
===========
[REPO NAME] — v[version]

CURRENT FEATURES
----------------
Core:
  - [Feature 1]: [1-sentence description] [source: file:line]
  - [Feature 2]: [1-sentence description] [source: file:line]

Supporting:
  - [Feature N]: [description] [source]

FUTURE / PLANNED FEATURES
--------------------------
Explicitly Planned (from TODO/docs):
  - [Feature]: [source: file:line or issue #]

Implied / In-Progress:
  - [Feature]: [evidence] [source: file:line]

Abandoned / Deferred:
  - [Feature]: [evidence of abandonment] [source: file:line]

INTEGRATION SURFACE
-------------------
External Systems: [list every API, database, queue, service touched]
Protocols:        [HTTP, WebSocket, gRPC, etc.]
Data Formats:     [JSON, protobuf, CSV, etc.]
```

---

## Phase 3: Code Reconstruction (RECONSTRUCT Mode)

**Goal:** Rewrite the codebase with security hardening, efficiency improvements, and production-grade stability — preserving all legitimate functionality.

Read `references/reconstruction-protocol.md` for transformation rules.

### Pre-Reconstruction Checklist

Before writing a single line, confirm:

```
□ Phase 1 (Threat) complete — know exactly what to remove and why
□ Phase 2 (Recon) complete — know exactly what to preserve and enhance
□ Target language/runtime confirmed — match original unless instructed to migrate
□ Dependency decisions made:
    Keep: [list]
    Replace: [original] → [safer alternative]
    Remove: [list with reason]
```

### Reconstruction Protocol

**Step 1: Security Hardening (apply to every file)**

```
INPUT VALIDATION
- All external inputs validated against strict schema before use
- Never trust environment variables without validation
- Sanitize before logging (no credential leakage in logs)
- Validate file paths against allowlist (prevent path traversal)

AUTHENTICATION & AUTHORIZATION
- Replace hardcoded credentials with env var references + validation
- Add rate limiting to all network-facing handlers
- Verify all token/key usages use constant-time comparison
- Remove any backdoor authentication bypasses found in Phase 1

DEPENDENCY HARDENING
- Pin all dependencies to exact versions (no ranges)
- Replace deprecated or high-risk packages with maintained alternatives
- Remove unused dependencies (attack surface reduction)
- Add integrity hashes where supported (package-lock.json, pip hash)

SECRETS MANAGEMENT
- Move all hardcoded secrets to environment variable references
- Add .env.example with placeholder values (never real values)
- Add .gitignore entries for all secret file patterns
- Add secret scanning pre-commit hook config
```

**Step 2: Efficiency Improvements**

```
ALGORITHMIC
- Replace O(n²) patterns with O(n log n) or better where applicable
- Cache repeated expensive computations (memoization with TTL)
- Replace synchronous blocking I/O with async patterns
- Eliminate redundant DB queries (N+1 problem detection)
- Batch API calls where sequential calls exist

RESOURCE MANAGEMENT
- Ensure all file handles, DB connections, network sockets are closed
- Add connection pooling for database and HTTP clients
- Add timeouts to all network operations (no indefinite waits)
- Implement backoff + retry for transient failures
```

**Step 3: Stability Improvements**

```
ERROR HANDLING
- Every network call wrapped in try/catch with specific error types
- Every file operation has graceful failure path
- No unhandled promise rejections / uncaught exceptions
- Errors logged with context, not just message strings
- User-facing errors never expose internal stack traces

OBSERVABILITY
- Structured logging (JSON) at appropriate levels (debug/info/warn/error)
- Request/response logging for all external calls (sanitized)
- Health check endpoint for services
- Startup validation (fail fast if config is missing)

TESTING SURFACE
- Add type annotations / JSDoc to all public interfaces
- Document preconditions and postconditions for complex functions
- Add input/output examples in comments for non-obvious logic
```

### Reconstruction Output Format

```
## Reconstruction Report

### Summary of Changes
| Category | Changes Made | Rationale |
|----------|-------------|-----------|
| Security | [N] changes | [top finding addressed] |
| Efficiency | [N] changes | [top improvement] |
| Stability | [N] changes | [top improvement] |
| Dependencies | [N] changed | [replaced/removed list] |

### Removed (Threat-Related)
- [File:line]: [what was removed] — [threat it represented]

### Security Hardening Applied
- [File:line]: [change] — [why]

### Efficiency Improvements
- [File:line]: [change] — [measured or estimated impact]

### Stability Improvements
- [File:line]: [change] — [failure mode addressed]

---
[RECONSTRUCTED CODE FOLLOWS]
---
```

---

## Common Mistakes

### Never analyze without ingestion
```
Wrong: Looking at one file and drawing conclusions about the whole repo
Why:   Malicious code is often in dependency install scripts or secondary
       files designed to hide from casual inspection.
Right: Always fingerprint the full file tree before analysis.
```

### Never skip the dependency audit
```
Wrong: "The main code looks clean, LGTM"
Why:   The ClawHavoc campaign and most supply chain attacks embed payloads
       in dependencies, not the primary codebase.
Right: Audit every dependency. The threat is usually not where you expect.
```

### Never reconstruct before threat analysis
```
Wrong: Rewriting code that contains a backdoor, preserving the backdoor
Why:   If Phase 1 is skipped, malicious patterns get cleaned up and
       professionalized in the reconstruction. You hand back polished malware.
Right: Complete Phase 1 fully. Explicitly list removals before rewriting.
```

### Never mark "CLEAN" without running all four threat layers
```
Wrong: "No eval() or exec() found, looks safe"
Why:   Modern supply chain attacks use install hooks, encoded payloads,
       and dependency hijacking — none of which appear in main source files.
Right: Run all four layers. A partial audit is not an audit.
```

### Never produce a feature map from README alone
```
Wrong: Summarizing only what the README says
Why:   The most important features — and the planned ones — are in the
       code, tests, TODOs, and issue references. README is marketing.
Right: Cross-reference README with code structure, tests, and comments.
```

---

## Quality Gates

### Threat Analysis Gate
- [ ] All four layers completed (intent, code, dependencies, infrastructure)
- [ ] Every finding has: file:line, CWE/ATT&CK reference, severity
- [ ] Threat score produced before proceeding
- [ ] Verdict stated explicitly (DO NOT INSTALL / SAFE / etc.)

### Feature Map Gate
- [ ] Current features sourced from code, not just README
- [ ] Future features extracted from TODOs, stubs, flags, and comments
- [ ] Every feature has a source reference (file:line or doc section)
- [ ] Integration surface fully mapped

### Reconstruction Gate
- [ ] All threat findings from Phase 1 addressed (removed or replaced)
- [ ] Every removed element documented with rationale
- [ ] Dependencies pinned to exact versions
- [ ] Error handling present on every I/O operation
- [ ] No hardcoded secrets remain
- [ ] Summary change table produced before code output

---

## Dependencies

- `bash_tool` + `git` for repo cloning and file inspection
- `web_fetch` for GitHub raw content and READMEs
- `grep` / `find` for pattern scanning
- Internet access for dependency vulnerability cross-reference
- `references/threat-taxonomy.md` — complete detection ruleset
- `references/feature-extractor.md` — extraction methodology
- `references/reconstruction-protocol.md` — transformation rules
- `references/report-template.md` — output format templates
