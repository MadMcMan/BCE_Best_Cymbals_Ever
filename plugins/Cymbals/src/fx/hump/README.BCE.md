# HumpCrusher DSP (vendored)

- Upstream: https://github.com/elDoof/HumpCrusher
- Commit: `00c7439c7d8dc82cbd7bf787cf74e8420a5ef8da`
- Vendored: 2026-10-25 — `src/dsp/*.h` only (JUCE-free, header-only).
- License: GNU AGPLv3 (upstream requires it for JUCE 8). See `LICENSE`.

Used by `plugins/Cymbals/src/fx/InternalFxSlot.cpp` for the HumpCrusher FX
entry. The combined MadMcManBCE binary is therefore distributed under
AGPLv3 terms; keep the source available (this repository).
