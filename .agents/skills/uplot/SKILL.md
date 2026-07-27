---
name: uplot
description: "Trigger: uPlot, chart, plot, time-series graph, line chart, heap chart, memory history, dashboard graph, streaming chart, uplot.min.js. Write or modify uPlot chart code against the exact vendored version in main/web/ by fetching live docs from the internet — never rely on training-data API memory."
license: MIT
metadata:
  author: mochapulse
  version: "1.0"
---

## Activation Contract

Load this skill when writing, reviewing, or debugging uPlot chart code in this
project — the heap-history chart in `main/web/app.js`, new charts on the
dashboard, or upgrading the vendored library.

The project vendors uPlot as static files; the API surface you must target is
the **vendored version**, not whatever version your training data knows.
uPlot's option/hook API is deep and changes between releases — always verify
against live docs pinned to the vendored tag.

## Hard Rules

1. **Read the vendored version first.** Line 1 of `main/web/uplot.min.js`:
   `/*! https://github.com/leeoniya/uPlot (vX.Y.Z) */`. That `X.Y.Z` is the
   tag you pin every doc fetch to. Current: **1.6.32**.
2. **Never invent options from memory.** If you cannot fetch docs, say so
   instead of guessing a hook name or options shape.
3. **Global, not module.** The vendored build is the IIFE — code calls
   `new uPlot(...)` on the global. No `import`/`require`, no bundler.
4. **Data is aligned arrays, never objects.** `data = [xArr, y1Arr, y2Arr]`,
   all same length, x ascending. JSON rows must be converted first.
5. **Width is explicit.** uPlot does not auto-size. Set `width` from
   `container.clientWidth` and handle window resize with `setSize()`.
6. **Script order matters.** `/uplot.min.js` must load before `/app.js`
   (already wired in `index.html` — preserve it).

## Live Doc Sources (fetch in this order)

| Source | URL / ID | Use for |
|---|---|---|
| Context7 | `/leeoniya/uplot` (version 1.6.32, ~600 snippets) | How-to patterns with runnable examples — resolve-library-id then query-docs with one specific topic per call |
| Typed API (source of truth) | `https://raw.githubusercontent.com/leeoniya/uPlot/<TAG>/dist/uPlot.d.ts` | Exact option names, hook signatures, return types — fully commented |
| Concepts doc | `https://raw.githubusercontent.com/leeoniya/uPlot/<TAG>/docs/README.md` | Scales, series, axes, hooks lifecycle overview |
| Demo sources | `https://raw.githubusercontent.com/leeoniya/uPlot/<TAG>/demos/<file>` | Working code for streaming, sync-cursor, bands, zoom |
| Live demos | `https://leeoniya.github.io/uPlot/demos/index.html` | Visual check of what an option does |

Notes:

- Git tags have **no `v` prefix** — use `/1.6.32/`, not `/v1.6.32/`.
- Replace `<TAG>` with the version read from the vendored file (Hard Rule 1).
- If Context7 is unavailable, any web-fetch tool works against the raw
  GitHub URLs.
- The `.d.ts` is ~2000 lines — fetch it, then grep/read the section you need
  (e.g. search `interface Series` or `Hooks.Arrays`) instead of reading it all.

## Verified Workflow

1. Read line 1 of `main/web/uplot.min.js` → version tag.
2. API question? Query Context7 `/leeoniya/uplot` for the pattern.
3. Exact signature/option needed? Fetch the pinned `uPlot.d.ts` and read the
   relevant interface.
4. Write code matching the existing dashboard style: ES5 `var`, function
   declarations, dashboard palette (`#38bdf8` accent, `#34d399` success,
   `#334155` grid, `#64748b` axis text) — see `main/web/app.js` `initChart()`.
5. Verify: `node --check main/web/app.js` (and any edited JS).
6. Do NOT run `idf.py build` unless the user asks — this skill covers JS/docs
   work only.

## Project Integration Facts

- Vendored: `main/web/uplot.min.js` (~51 KB) + `main/web/uplot.min.css`
  (~2 KB), embedded in flash via `EMBED_FILES` in `main/CMakeLists.txt`.
- Served at `/uplot.min.js` and `/uplot.min.css` by the static table in
  `main/utils/web_API.c` (`s_files[]`). New static files need: EMBED_FILES
  entry + extern `_binary_<base_with_underscores>_{start,end}` + table row.
- Existing chart: `#heap-chart` div in the Status tab, ring buffer
  (`MAX_POINTS = 360`, 30 min at 5 s poll) in `app.js`, updated from
  `refreshStatus()` via `updateChart(data)`. History lives only in the
  browser — nothing is stored on the ESP32.

## Upgrading the Vendored Library

```bash
TAG=1.6.33  # example — check https://api.github.com/repos/leeoniya/uPlot/tags
curl -sL -o main/web/uplot.min.js  "https://cdn.jsdelivr.net/npm/uplot@$TAG/dist/uPlot.iife.min.js"
curl -sL -o main/web/uplot.min.css "https://cdn.jsdelivr.net/npm/uplot@$TAG/dist/uPlot.min.css"
node --check main/web/uplot.min.js
```

Always pin `$TAG` explicitly — jsdelivr `latest` will silently drift. Update
the version number in: this skill (Hard Rule 1), `README.md` project tree,
and re-check flash budget (firmware bin + ~53 KB must stay under the 1 MB
factory partition).

## Gotchas

- **No-data init:** pass `null` (or `[]`) as data and define `range`
  functions on x and y scales that return defaults when `dataMin == null`
  (official pattern: `demos/no-data.html`). Do NOT pass `[[],[],[]]` — an
  array of empty arrays crashes the constructor with
  `TypeError: Cannot read properties of undefined (reading 'length')`.
  Guard `dataMin === dataMax` (single point) by padding the x range.
- **Mutate via `setData`, never re-create.** Push/shift the same arrays and
  call `chart.setData([ts, free, min])` — destroying/re-creating per poll
  leaks canvases.
- **Time axis:** `scales: { x: { time: true } }` with Unix-second timestamps.
  uPlot formats tick labels itself; only override `values` on the y-axis
  (e.g. byte formatting).
- **Legend live values:** `legend: { show: true, live: true }` updates the
  legend under the cursor for free.
- **Custom tick text:** axis `values: (self, splits) => string[]` — return
  exactly one string per split.
- **Dashboard must survive lib load failure:** guard with
  `if (typeof uPlot === "undefined") return;` (pattern already in `app.js`).
- **Flash budget check after upgrade:** `ls -la build/*.bin` — binary must
  stay under 1,048,576 bytes (current ~870 KB with uPlot included).
