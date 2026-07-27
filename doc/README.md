# Doxygen documentation

This project uses [Doxygen](https://www.doxygen.nl/) to generate HTML API docs
from the `/** ... */` comments in the C source tree. The docs are rebuilt on
every push to `main` and deployed to GitHub Pages automatically via
`.github/workflows/docs.yml`, but you can also build them locally to preview
changes before pushing.

## Live docs

```
https://mochapulse.github.io/ESP32_WoL_IP_Fordwarder/
```

## Quick start

```bash
# 1. Install dependencies
sudo apt-get install doxygen graphviz          # Linux (Ubuntu/Debian)
brew install doxygen graphviz                  # macOS
choco install doxygen graphviz                  # Windows (Chocolatey)

# 2. Generate from the repo root
mkdir -p doc/doxygen
doxygen Doxyfile

# 3. Open in browser
xdg-open doc/doxygen/html/index.html           # Linux
open doc/doxygen/html/index.html               # macOS
start doc/doxygen/html/index.html              # Windows
```

> **First run?** You must create the output directory before running `doxygen Doxyfile`.
> It is gitignored — each fresh clone needs to create it.

## Directory layout

| Directory | Purpose | Tracked? |
|-----------|---------|----------|
| `doc/` | **Source** docs — `README.md`, `mainpage.md` | Yes |
| `doc/doxygen/` | **Generated** HTML output | No (gitignored) |

## Output structure

```
doc/doxygen/
└── html/
    ├── index.html          ← Main landing page (from doc/mainpage.md)
    ├── files.html          ← File list
    ├── globals.html        ← Functions, variables, macros
    ├── annotated.html      ← Data structures (structs, enums)
    ├── pages.html          ← Related pages
    ├── *.svg               ← Call-graph / include-graph diagrams
    └── search/             ← Client-side search index
```

> `doc/doxygen/` is in `.gitignore` — it is never committed.

## What gets documented

The Doxyfile is configured with:

| Setting | Value | Effect |
|---------|-------|--------|
| `INPUT` | `main/` `doc/` | Scans all `.c`, `.h`, `.md` in these dirs |
| `RECURSIVE` | `YES` | Walks into `utils/`, `web/` |
| `EXTRACT_ALL` | `YES` | Documents everything, even undocumented symbols |
| `EXTRACT_STATIC` | `YES` | Includes `static` functions and variables |
| `OPTIMIZE_OUTPUT_FOR_C` | `YES` | C-specific formatting |
| `SOURCE_BROWSER` | `YES` | Syntax-highlighted source in the output |
| `CALL_GRAPH` | `YES` | Function call graphs (needs `graphviz`) |
| `GENERATE_LATEX` | `NO` | HTML-only (skip PDF generation) |

## Custom main page

The landing page content comes from `doc/mainpage.md`. Edit that file to change
what appears at `index.html`. Doxygen renders it as Markdown with its own
formatting extensions.

## Keeping the Doxyfile lean

The `Doxyfile` in this repo is **minimal** — it only sets values that differ
from Doxygen's defaults. This keeps it readable and easy to diff in code review.

```bash
# DO NOT run this — it replaces our minimal config with 3000 lines of defaults:
# doxygen -u Doxyfile

# To see what defaults your installed Doxygen version uses:
doxygen -g - | head -30
```

When a Doxygen upgrade introduces new tags or deprecates old ones, review the
warnings from a manual `doxygen Doxyfile` run and update the file by hand rather
than auto-migrating.

## Troubleshooting

**`error: tag OUTPUT_DIRECTORY ... does not exist and cannot be created`**

You skipped step 2 in Quick start. Run:

```bash
mkdir -p doc/doxygen
```

then re-run `doxygen Doxyfile`.

**`warning: Unsupported xml/html tag <something> found`**

Angle brackets in Doxygen comments need escaping. Replace `<name>` with
`\<name\>` or use `\c <name>`. See `web_API.c` line 12 for an example
of the objcopy symbol notation fixed for this.

**`warning: Tag '...' has become obsolete`**

The installed Doxygen is newer than what the `Doxyfile` targets. Delete the
obsolete tag line from `Doxyfile` — do **not** run `doxygen -u` (see
"Keeping the Doxyfile lean" above).

**No call graphs?** Install `graphviz` (`dot` binary must be on `$PATH`).

**Docs look stale after editing source?** The output directory is never
cleaned automatically. For a fresh build:

```bash
rm -rf doc/doxygen/html && doxygen Doxyfile
```

## CI pipeline

```
push to main
  │
  ▼
Install doxygen + graphviz (apt)
  │
  ▼
mkdir -p doc/doxygen
  │
  ▼
doxygen Doxyfile
  │
  ▼
peaceiris/actions-gh-pages → gh-pages branch
  │
  ▼
GitHub Pages serves from gh-pages
```

On PRs the workflow uploads a preview artifact (`doxygen-html`) instead of
deploying — download it from the Actions tab to preview the docs before merge.

## GitHub Pages setup (one-time)

1. **Enable GitHub Pages** — repo Settings → Pages → Source = "Deploy from a branch",
   Branch = `gh-pages`, folder = `/ (root)`. Save.
2. **Configure permissions** — Settings → Actions → General → Workflow permissions =
   "Read and write permissions". Save.
3. Push to `main` — the workflow publishes on `gh-pages` and GitHub Pages serves it.
