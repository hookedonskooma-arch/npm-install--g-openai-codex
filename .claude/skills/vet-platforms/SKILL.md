---
name: vet-platforms
description: "Run a structured AI platform safety and trust assessment on any website or tool. Use when a user asks whether an AI tool, platform, or service is safe to use, trustworthy, or worth signing up for. Triggers on: 'is this AI safe', 'can I trust this platform', 'vet this tool', 'check this website', 'is [URL] legit', 'should I use this AI', 'due diligence on this tool', 'look into this platform', 'is this safe for my data', 'who owns this AI', 'is this a scam', 'research this tool', 'AI platform check', 'safety check', or any time a URL is shared with intent to evaluate safety or legitimacy. Also triggers when a client or user describes an AI tool by name and wants a trust opinion. Do NOT use for general web research, cybersecurity audits of internal systems, or code security reviews — those are separate workflows."
---

# Vet Platforms — AI Platform Safety & Trust Assessment

## Overview

This skill runs a structured due diligence check on any AI platform, tool, or service. It produces a plain-language safety report covering company identity, funding, data practices, privacy, security posture, and red flags — culminating in a trust rating a non-technical user can act on.

Built for Chibitek client-facing use. Output should be clear, jargon-free, and honest — including gut-check observations when something feels off even if it can't be fully verified.

---

## Quick Reference

| Situation | Approach |
|-----------|----------|
| User provides a URL | Run all 8 assessment sections against that URL |
| User provides a tool name only (no URL) | Search for the official site first, confirm before proceeding |
| Web search is unavailable | Flag clearly — note results are limited to training data and may be outdated |
| Platform is brand new / obscure, little data found | Flag this explicitly — low info availability is itself a yellow flag |
| Platform involves PHI, client data, or financial info | Escalate risk threshold — recommend professional review before use |
| User is in a regulated industry (healthcare, finance, legal) | Add compliance note at the end of the report |

---

## Assessment Workflow

Run these 8 sections in order. Use web search when available. Be thorough but plain-spoken.

---

### 1. Company Identity
- Who owns or operates this platform?
- Where is the company incorporated or headquartered?
- Who are the founders or key leadership? Do they have verifiable, credible backgrounds?
- Is this a funded startup, an established company, or an anonymous entity?
- Is there a real team page, a physical address, and working contact information on the site?

**Why this matters:** Legitimate platforms stand behind their identity. Anonymous ownership is a significant red flag regardless of how polished the product looks.

---

### 2. Funding & Financial Backing
- Is the company VC-backed, bootstrapped, or institutionally funded?
- Who are the known investors? Are any concerning (foreign state entities, controversial firms, opaque shell structures)?
- Is the business model transparent enough to explain how they sustain operations?

**Why this matters:** Funding sources reveal incentives. A platform with no clear revenue model and no known investors either hasn't found product-market fit or is monetizing something it isn't disclosing.

---

### 3. Business Model
- How does this platform make money?
- If the product is free — what exactly is being monetized? Ads, data, premium upsell, enterprise contracts?
- Does the pricing or business model make sense for a real, sustainable company?

**Why this matters:** "Free" is never actually free. If you can't identify the revenue model in under two minutes, assume data monetization until proven otherwise.

---

### 4. Data & Privacy
- Is there a Privacy Policy? Is it easy to find, clearly written, and recently updated?
- Does the platform state whether user data (prompts, files, inputs) is used to train AI models?
- Can users opt out of data training?
- What data is collected — email, usage patterns, prompts, uploaded files, location?
- Do they share or sell data to third parties?
- Do they claim GDPR, CCPA, or HIPAA compliance? Can they back it up?

**Why this matters:** This is the core risk for most users. A vague or hard-to-find Privacy Policy is a yellow flag. No opt-out from training data is a significant concern for business users.

---

### 5. Terms of Service
- Are the Terms easy to find and reasonably written?
- Do they claim unusually broad rights over user-submitted content or outputs?
- What is the data retention policy — how long do they keep your data, and can you delete it?
- What happens to your data if the company shuts down or is acquired?

**Why this matters:** Broad IP or content rights in the ToS can mean the platform owns or can use anything you submit. Acquisition clauses matter because a safe platform today can be acquired by a less safe operator tomorrow.

---

### 6. Security & Trust Signals
- Do they publish a security policy?
- Do they hold SOC 2 Type II or ISO 27001 certifications?
- Is there a responsible disclosure or bug bounty program?
- Is there a documented breach history or known security incidents?

**Why this matters:** Certifications aren't foolproof, but they signal a company that invests in security practices. The absence of any security documentation on a platform asking for sensitive data is a red flag.

---

### 7. Reputation & Red Flags
- Is there credible press coverage from reputable tech sources?
- Are there known controversies, legal issues, or regulatory actions?
- Is the domain newly registered, or does it have an established history?
- Does the site feel legitimate — real team bios, verifiable history, consistent branding?
- Are there user complaints on forums, Reddit, or review sites worth noting?

**Why this matters:** Reputation signals accumulate over time. A platform with no press, no community presence, and a recently registered domain warrants significant skepticism.

---

### 8. Verdict

Deliver:

- **Trust Rating:** Low Risk / Moderate Risk / High Risk / Insufficient Data
- **Top 3 Reasons** for the rating (plain language)
- **Key Concerns** a non-technical user should know before signing up
- **Recommendation:** Safe to use / Use with caution / Do not use / Escalate for professional review

---

## Output Format

Structure the report with clear section headers matching the 8 categories above. End with the Verdict block. Use plain language throughout — write as if explaining to a smart but non-technical business owner.

Example Verdict block:

```
VERDICT: Moderate Risk

Top 3 Reasons:
1. No information on whether prompts are used for model training, and no opt-out option found.
2. Company founded in 2023 with limited press coverage and anonymous investor backing.
3. Terms of Service grant broad rights to user-submitted content with no acquisition clause.

Recommendation: Use with caution. Avoid submitting sensitive client data or proprietary information until the company publishes clearer data usage policies. Consult your IT advisor before using in a business context.
```

---

## Common Mistakes

**Do NOT:**
- Skip sections because the platform looks polished — design quality is not a trust signal
- Give a "Low Risk" rating just because the platform is well-known — even major platforms have problematic data practices
- Treat absence of information as neutral — if a Privacy Policy doesn't exist or can't be found, say so explicitly
- Use jargon like "exfiltration vector" or "threat surface" — the output is for non-technical readers
- Hedge so much the verdict is useless — the user needs a clear recommendation

**DO:**
- Flag gut-check observations even if you can't fully verify them
- Note when web search is unavailable and results may be outdated
- Recommend professional review when PHI, financial data, or regulated use cases are involved
- Use the phrase "Insufficient Data" rather than "Unknown" — it communicates that low info availability is itself meaningful

---

## Regulated Industry Add-On

If the user is in healthcare, finance, legal, or education, append this note to the Verdict:

> **Regulated Industry Note:** This platform has not been verified for use with [PHI / financial records / legal documents / student data]. Before using it in a professional context, verify compliance certifications directly with the vendor and consult your compliance officer or legal counsel. Chibitek recommends a formal vendor risk assessment for any AI platform handling regulated data.

---

## Quality Checklist

Before delivering the report, verify:

- [ ] All 8 sections addressed — no section skipped without explanation
- [ ] Verdict includes Trust Rating, Top 3 Reasons, and a clear Recommendation
- [ ] Language is plain — no unexplained technical jargon
- [ ] Web search availability noted if limited
- [ ] Low data availability called out as a yellow flag if applicable
- [ ] Regulated industry note added if relevant
- [ ] Report does not contradict itself between sections

---

## Disclaimer (include at end of every report)

> This assessment is provided for informational purposes only and does not constitute legal, security, or compliance advice. Results are based on publicly available information and AI-generated analysis, which may be incomplete or outdated. Chibitek makes no warranties regarding accuracy. Users are solely responsible for decisions made based on this report. For critical business, legal, or security decisions, consult a qualified professional.

---

## Dependencies

- Web search enabled (strongly recommended — results are significantly limited without it)
- No external APIs or libraries required
- Works in Claude, ChatGPT, or any capable LLM with access to web search
