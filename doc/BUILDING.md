# Building the Doxygen documentation (local)

This project uses [Doxygen](https://www.doxygen.nl/) to generate HTML API docs
from the `/** ... */` comments in the C source tree. The workflow is configured
in `Doxyfile` and runs on CI via `.github/workflows/docs.yml`, but you can also
build the docs locally to preview changes before pushing.

## Quick start

```bash
# 1. Install dependencies
sudo apt-get install doxygen graphviz          # Linux (Ubuntu/Debian)
brew install doxygen graphviz                  # macOS
choco install doxygen graphviz                  # Windows (Chocolatey)

# 2. Generate from the repo root
doxygen Doxyfile

# 3. Open in browser
xdg-open docs/doxygen/html/index.html          # Linux
open docs/doxygen/html/index.html              # macOS
start docs/doxygen/html/index.html             # Windows
```

## Output structure

```
docs/doxygen/
└── html/
    ├── index.html          ← Main landing page (from doc/mainpage.md)
    ├── files.html          ← File list
    ├── globals.html        ← Functions, variables, macros
    ├── annotated.html      ← Data structures (structs, enums)
    ├── pages.html          ← Related pages
    ├── *.svg               ← Call-graph / include-graph diagrams
    └── search/             ← Client-side search index
```

> `docs/doxygen/` is in `.gitignore` — it is never committed.

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

## Troubleshooting

**`error: tag OUTPUT_DIRECTORY ... does not exist and cannot be created`**

```bash
mkdir -p docs/doxygen
doxygen Doxyfile
```

**`warning: Tag '...' has become obsolete`**

The GitHub runner ships a newer doxygen version than the `Doxyfile` was written
for. Run `doxygen -u Doxyfile` to auto-migrate the config file, then review the
diff before committing.

**No call graphs?** Install `graphviz` (`dot` binary must be on `$PATH`).

## CI pipeline

```
push to main
  │
  ▼
Install doxygen + graphviz (apt)
  │
  ▼
mkdir -p docs/doxygen
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
