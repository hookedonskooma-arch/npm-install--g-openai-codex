# Quality Gates

Pre-built quality checklists organized by output type. Include the relevant checklist in any skill that produces that output type, then customize with domain-specific items.

---

## Documents (.docx, .pdf)

### Structure
- [ ] Document opens without errors in target application (Word, Google Docs, Preview)
- [ ] Page size matches specification (US Letter vs. A4)
- [ ] Margins are consistent throughout
- [ ] Headers and footers appear on all pages (if specified)
- [ ] Page numbers are correct and sequential
- [ ] Table of contents matches actual headings (if present)

### Typography
- [ ] Font family is consistent throughout (no fallback fonts visible)
- [ ] Heading hierarchy is correct (H1 > H2 > H3)
- [ ] Body text size is readable (11-12pt for print, 14-16pt for screen)
- [ ] No orphaned headings (heading at bottom of page with content on next)

### Content
- [ ] No placeholder text remains ("Lorem ipsum", "[TODO]", "[INSERT]")
- [ ] All hyperlinks are functional
- [ ] Images render correctly (not broken or distorted)
- [ ] Tables have consistent formatting and alignment

### Accessibility
- [ ] Images have alt text (if applicable)
- [ ] Color is not the only means of conveying information
- [ ] Headings use built-in heading styles (not just bold text)

---

## Spreadsheets (.xlsx, .csv)

### Data Integrity
- [ ] Zero formula errors (#REF!, #DIV/0!, #VALUE!, #N/A, #NAME?)
- [ ] Formulas reference correct cells (no off-by-one errors)
- [ ] Consistent formulas across all projection periods
- [ ] Edge cases handled (zero values, negative numbers, empty cells)
- [ ] No unintended circular references

### Formatting
- [ ] Professional, consistent font throughout
- [ ] Number formats match content type (currency, percentage, date)
- [ ] Years formatted as text strings (not comma-separated numbers)
- [ ] Negative numbers use parentheses, not minus signs (for financial)
- [ ] Column widths accommodate content without truncation

### Structure
- [ ] Headers are frozen (if data extends beyond one screen)
- [ ] Sheet tabs are named descriptively
- [ ] Assumptions are in separate cells with references (not hardcoded)
- [ ] Color coding follows convention (blue for inputs, black for formulas)

### Validation
- [ ] Recalculated with LibreOffice/Excel to verify formula results
- [ ] Spot-checked calculations manually for accuracy
- [ ] Data types are correct (numbers as numbers, not text)

---

## Presentations (.pptx)

### Visual Design
- [ ] Color palette is consistent throughout (no off-brand colors)
- [ ] Typography is consistent (one header font, one body font)
- [ ] Every slide has a visual element (no text-only slides)
- [ ] Slide titles convey the takeaway (not just the topic)
- [ ] Logo placement is consistent

### Content
- [ ] One key message per slide
- [ ] Maximum 3-5 bullet points per slide
- [ ] No walls of text (60% visual, 40% text rule)
- [ ] Speaker notes added where needed
- [ ] No placeholder content remains

### Structure
- [ ] Title slide includes all required elements
- [ ] Section breaks separate logical groups
- [ ] Flow is logical (problem → solution → evidence → action)
- [ ] Slide count is appropriate for time allotment (~1 slide per minute)

### Technical
- [ ] Opens correctly in PowerPoint, Google Slides, and Keynote
- [ ] Images are embedded (not linked to external files)
- [ ] Fonts are available on target systems (or embedded)
- [ ] File size is reasonable (under 50MB unless image-heavy)

---

## Web / Frontend (HTML, React, CSS)

### Functionality
- [ ] All interactive elements work (buttons, forms, navigation)
- [ ] No console errors in browser developer tools
- [ ] State management works correctly (data persists/resets as expected)
- [ ] Edge cases handled (empty states, loading states, error states)

### Visual Quality
- [ ] Aesthetic has a clear point-of-view (not generic)
- [ ] Responsive at common breakpoints (mobile, tablet, desktop)
- [ ] Typography is readable at all sizes
- [ ] Color contrast meets accessibility standards
- [ ] Animations enhance UX (not just decorative)

### Code Quality
- [ ] No hardcoded values that should be configurable
- [ ] CSS uses variables for theming consistency
- [ ] No inline styles where classes would be cleaner
- [ ] Component structure is logical and maintainable

### Anti-Slop Check
- [ ] Not using Inter, Roboto, or Arial as the display font
- [ ] Not using purple gradients on white backgrounds
- [ ] Layout has visual tension (not everything centered)
- [ ] Design feels specific to this context (not interchangeable)

---

## API / Integration Code

### Authentication
- [ ] Credentials are not hardcoded in source code
- [ ] Token expiration is handled with refresh logic
- [ ] Auth errors produce clear user-facing messages
- [ ] API keys are stored in environment variables

### Error Handling
- [ ] Every API call has error handling (try/catch or equivalent)
- [ ] Errors include context (which operation, what failed, suggested fix)
- [ ] Network timeouts are handled gracefully
- [ ] Rate limiting is respected with retry logic

### Data Handling
- [ ] Pagination is implemented for list endpoints
- [ ] Response data is validated before use
- [ ] Large responses are filtered/truncated to relevant fields
- [ ] No sensitive data in logs or error messages

### Reliability
- [ ] Works with valid inputs
- [ ] Handles invalid inputs gracefully (not crashes)
- [ ] Idempotent operations are safe to retry
- [ ] Partial failures don't corrupt state

---

## Workflow Outputs

### Completeness
- [ ] All stages were offered to the user
- [ ] Context from earlier stages informed later stages
- [ ] No information was asked for twice
- [ ] User had opportunity to review and refine

### Quality
- [ ] Final output matches the stated purpose
- [ ] Content is specific and actionable (not generic filler)
- [ ] Formatting matches the target use case
- [ ] Length is appropriate (not padded, not truncated)

### User Experience
- [ ] User could skip optional steps without friction
- [ ] Feedback was incorporated correctly
- [ ] Iterations made meaningful improvements
- [ ] Process concluded cleanly with clear deliverable

---

## How to Use Quality Gates

When building a skill:

1. Identify the output type(s) the skill produces
2. Copy the relevant checklist section into the skill's Quality Checklist
3. Add domain-specific items (e.g., brand compliance, regulatory requirements)
4. Remove items that don't apply to this specific skill
5. Order items by importance (most critical first)

The checklist should be the last step before delivering output. It serves as both a self-check for the consuming Claude instance and a reference for the user to verify quality.
