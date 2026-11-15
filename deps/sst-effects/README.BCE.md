# sst-effects (vendored)

Header-only Surge XT effect implementations used by MadMcManBCE's FX slots.

- Upstream: https://github.com/surge-synthesizer/sst-effects
- Commit: `987b39dcb2e2e61e6225c3bc390fd00da824d226`
- Vendored: 2026-10-25 (unmodified `include/` tree; see `LICENSE.md`)
- Requires C++20: `plugins/Cymbals/src/surge/SurgeFx.cpp` is the only translation
  unit that includes these headers and is compiled with `-std=c++20` by a
  per-source CMake property. The rest of the plugin stays C++17.
