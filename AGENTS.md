# Agent Setup — ESP32_WoL_IP_Fordwarder

## idf.py availability

`idf.py` is defined as a shell function in `~/.zshrc`:

```zsh
idf.py() {
    if [[ -z "${IDF_PATH}" ]]; then
        source "/home/javastral/.espressif/tools/activate_idf_v6.0.2.sh" > /dev/null 2>&1
    fi
    python3 "$IDF_PATH/tools/idf.py" "$@"
}
```

When running `idf.py` commands, source the activation script first if the function
is not available in your shell:

```bash
source /home/javastral/.espressif/tools/activate_idf_v6.0.2.sh > /dev/null 2>&1 && \
  python3 "$IDF_PATH/tools/idf.py" <command>
```

## Python venv note

If you get a warning about `python` vs `python3` venv mismatch:

```
'python' is currently active while the project was configured with 'python3'.
Run 'idf.py fullclean' to start again.
```

This is cosmetic — the build proceeds normally. If it blocks, use the explicit
`python3` invocation shown above instead of the `idf.py` function.

## Flash port

The ESP32 is connected at `/dev/ttyUSB0`.

## Build commands

```bash
# Build only
python3 "$IDF_PATH/tools/idf.py" build

# Build + flash
python3 "$IDF_PATH/tools/idf.py" flash -p /dev/ttyUSB0

# Build + flash + monitor (needs TTY — not usable in agent environments)
python3 "$IDF_PATH/tools/idf.py" -p /dev/ttyUSB0 flash monitor
```

## Doxygen docs

API docs are built with `doxygen` + `graphviz` (installed system-wide):

```bash
# Generate (output dir must exist first — doxygen cannot create it)
mkdir -p doc/doxygen
doxygen Doxyfile

# Output: doc/doxygen/html/index.html (gitignored)
```

- Keep the `Doxyfile` **minimal** — only non-default values. Do NOT run
  `doxygen -u Doxyfile`; it expands the file to ~3000 lines of defaults.
- CI builds and deploys docs on push to `main` via `.github/workflows/docs.yml`.
- Full guide: [doc/README.md](doc/README.md)

## API auth + endpoint behavior

- API routes (`/api/status`, `/api/wol`) require header auth:
  `X-API-Key: <WEB_API_TOKEN>`.
- Configure `WEB_API_TOKEN` in `main/.env` (see `main/.env.example`).
- API routes can return `503 Service Unavailable` with body
  `Low memory threshold reached` when free heap is below the safety guard.

## Project path

```
/home/javastral/GIT/Mocha/ESP32_WoL_IP_Fordwarder
```
