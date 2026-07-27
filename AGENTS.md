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

## Project path

```
/home/javastral/GIT/Mocha/ESP32_WoL_IP_Fordwarder
```
