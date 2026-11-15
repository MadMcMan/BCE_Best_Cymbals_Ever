# MadMcMan's BCE - 4-Stage Linear Synthesis Engine

MadMcMan's BCE stands for Best Cymbals Ever.

A high-performance cymbal/hi-hat synthesizer built on the **DISTRHO Plugin Framework (DPF)** and **LVGL**. It uses a four-stage linear synthesis workflow plus a modular FX chain and modulation matrix to simulate the complex inharmonic resonances of metallic percussion.

## Inspiration

This instrument was strongly influenced by the ideas in this video:

- https://www.youtube.com/watch?v=netcpYINyBQ

## Sponsor This Project

- **MadMcMan's BCE**: [GitHub Sponsors](https://github.com/sponsors/MadMcMan) · [Patreon](https://www.patreon.com/MadMcMan)
- **falkTX / DPF**: [GitHub Sponsors](https://github.com/sponsors/falkTX) · [Patreon](https://www.patreon.com/falkTX)
- **Airwindows (Chris Johnson)**: [Patreon](https://www.patreon.com/airwindows)
- **Zion Jaymes**: [Patreon](https://www.patreon.com/ZionJaymes)

## Credits and Support

- **falkTX** — DPF and DPF-Widgets: https://github.com/sponsors/falkTX · https://www.patreon.com/falkTX
- **Zion Jaymes** — original cymbal synthesis inspiration: https://www.patreon.com/ZionJaymes
- **Airwindows (Chris Johnson)** — Airwindows DSP: https://www.patreon.com/airwindows
- **Surge Synth Team** — sst-effects, sst-filters, and the Surge XT effect implementations: https://github.com/surge-synthesizer
- **Signalsmith Audio (Geraint Luff)** — Signalsmith Chorus, Stretch, and support layers: https://github.com/Signalsmith-Audio
- **GhostNoteAudio** — CloudSeedCore reverb: https://github.com/GhostNoteAudio/CloudSeedCore
- **Paul Batchelor** — Soundpipe phaser: https://github.com/PaulBatchelor/Soundpipe
- **elDoof** — HumpCrusher distortion: https://github.com/elDoof/HumpCrusher
- **LVGL** — embedded UI framework: https://github.com/sponsors/lvgl
- **fmt** — formatting library: https://github.com/fmtlib/fmt

Where a project has a GitHub Sponsors or Patreon page, the link above points to it; otherwise it points to the upstream project.

## Signal Flow

1.  **Stage 1: IMPULSES** - Synthesize the initial stick strike and shape it with a 5-band EQ, always-on HPF/LPF, tilt control, and real-time EQ graph editing.
2.  **Stage 2: DISPERSION** - Two independent dispersion engines (`DISP 1` / `DISP 2`) with vibrato, chaos, phase distortion, and multi-stage Phase Nudge controls for temporal and spectral smearing.
3.  **Stage 3: FILTERS** - Parallel comb/filter bank tuned for cymbal-like inharmonic behavior, with per-group and per-stage shaping controls.
4.  **Stage 4: COLOUR** - Final comb bank and colouring EQ for the overall spectrum and output character.
5.  **FX CHAIN** - Six reorderable modular FX slots with AirSuiteUniversal, Surge, internal-filter, and vendored external effects.
6.  **MOD** - Eleven modulation sources (LFO, ADSR, MSEG, STEP, random, and live performance inputs), 48 routes, and four macros.
7.  **PRESET** - Preset browser/loading/saving, Init state, preset metadata, and IR export.

## Features

- **551 AirSuiteUniversal Types**: The modular FX system exposes 551 selectable AirSuiteUniversal types (`COUNT = 554`, including `NONE` and two reserved enum holes).
- **6-Slot Modular FX Chain**: Each slot selects from AirSuiteUniversal, Surge XT, project internal filters, or vendored effects, exposes its algorithm parameters plus wet and bypass, and can be drag-reordered in the chain.
- **Shared EQ Editor System**: Stage 1 and Stage 4 colouring use aligned EQ UX, graph interactions, and control behavior.
- **FX Type Selection**: Choose per-slot algorithm types with searchable picker controls and per-slot parameter editing.
- **Direct Graph Editing**: HPF/LPF and EQ points are draggable in-graph for fast tonal shaping.
- **Customizable Preset Storage**: Presets are written to a per‑user directory (Documents/BCE/Presets by default); location may be overridden at runtime with the `CYMBALS_PRESET_DIR` environment variable or by editing the path in the UI.
- **Directional Arc Drag**: Radial controls use consistent drag semantics (up/right increase, down/left decrease).
- **Dual Dispersion + Nudge Engines**: Independent Dispersion 1 and Dispersion 2 tabs with macro/random tools.
- **Comb Bank Shaping**: Group controls, randomization, and Bessel tuning workflows for resonance design.
- **Reverb Suite**: Airwindows-based multi-algorithm reverb stage.
- **Button Debounce Hardening**: Rapid-click protection in UI click handlers to improve host/plugin stability.
- **Final FX Routing + UI Reordering**: The FX/REV order toggle changes both the audio routing and the UI panel order, with a subtle animated feedback when the order changes.
- **7-Tab LVGL Interface**:
  - Stage 1: `IMPULSES`
  - Stage 2: `DISPERSION` (`DISP 1` / `DISP 2` sub-tabs)
  - Stage 3: `FILTERS`
  - Stage 4: `COLOUR`
  - `FX CHAIN`
  - `MOD`
  - `PRESET`
- **Multi-Format Support**: DPF targets for VST3, CLAP, LV2, JACK, and DSSI (DSSI only when liblo is available).

## Imported FX and DSP Libraries

The FX chain and filter models include these imported or vendored sources:

| Source | Used for | License |
|---|---|---|
| Airwindows | 551 AirSuiteUniversal types, including reverb, filter, dynamics, tape, saturation, distortion, and utility algorithms | MIT |
| Surge XT `sst-effects` | Dual Delay, Flanger, Floaty Delay, Phaser, Reverb 1, Reverb 2, Bonsai, Rotary Speaker, Treemonster | GPLv3 |
| `sst-filters` | Korg35, diode ladder, OB-Xd, TriPole, and Cytomic tilt filter models | GPLv3 |
| `sst-basic-blocks`, `sst-waveshapers` | Support layers used by the sst FX/filter integrations | GPLv3 |
| CloudSeedCore | CloudSeed Reverb | MIT |
| Signalsmith Audio | Signalsmith Chorus and Signalsmith Stretch, with the Signalsmith DSP/linear support layers | MIT |
| Soundpipe | Soundpipe Phaser | MIT |
| `simde` | Portable SIMD/SSE emulation for ARM and universal macOS builds | MIT |
| HumpCrusher | HumpCrusher distortion | AGPLv3 |
| Project internal | 33 native EQ/filter models and 23 filterbank topologies | AGPLv3 |

The project-owned `cymbals-ui` control kit is MIT; see the License section for the combined distribution terms.

## Building

### Prerequisites
- **Compiler**: MinGW-w64 (GCC) with C++17 (C++20 is used for the sst-effects translation unit)
- **Tools**: CMake 3.15+, Git
- **Submodules**: DPF, LVGL, DPF-Widgets, Airwindows, and simde
- **Python**: not required to configure or build on Linux, macOS, or Windows

### Build Steps

From the repository root:

```powershell
# Fetch the pinned dependency submodules
git submodule update --init --recursive

# Configure from the repository root (the only supported entrypoint)
cmake -S . -B build-lvgl -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release `
  -DCONFIG_LV_BUILD_EXAMPLES=OFF -DCONFIG_LV_BUILD_DEMOS=OFF `
  -DCONFIG_LV_USE_THORVG_INTERNAL=OFF

# Build the plugin targets
cmake --build build-lvgl --config Release --target `
  MadMcManBCE-vst3 MadMcManBCE-clap MadMcManBCE-jack
```

Outputs land in `build-lvgl/bin/`. The `-lv2` target is also available; `-dssi` is generated when liblo is present. VST3, CLAP, and JACK are the primary local build gate.

### Canonical Build Path (Required)

- The only supported CMake entrypoint is the repository root [CMakeLists.txt](CMakeLists.txt).
- The plugin-local path [plugins/Cymbals/CMakeLists.txt](plugins/Cymbals/CMakeLists.txt) is deprecated and intentionally blocked with a hard error.
- If a build is started from [plugins/Cymbals/CMakeLists.txt](plugins/Cymbals/CMakeLists.txt), rerun from the repository root with `cmake -S . -B build-lvgl` and then build targets from that directory.

### macOS Build (Ninja)

From the repository root:

```bash
git submodule update --init --recursive

cmake -S . -B build-mac -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCONFIG_LV_BUILD_EXAMPLES=OFF \
  -DCONFIG_LV_BUILD_DEMOS=OFF \
  -DCONFIG_LV_USE_THORVG_INTERNAL=OFF

cmake --build build-mac --config Release --target \
  MadMcManBCE-vst3 MadMcManBCE-clap MadMcManBCE
```

## CI/CD and Releases

GitHub Actions now uses a multi-platform build and packaging workflow inspired by larger DPF projects:

- Workflow file: `.github/workflows/build-main.yml`
- Build jobs: Linux, macOS, and Windows
- Artifacts: packaged per-platform plugin bundles/binaries
- Release: tag-driven asset publishing on `v*` tags

Trigger policy:

- push and pull_request do not trigger the build workflow.
- v* tags trigger the build and release flow.

## Release Packaging

Release bundles are produced by the tag-driven GitHub Actions workflow. Local packaging helpers are kept outside the public tree; the CI workflow and installer scripts under `scripts/installer_*` remain the release source of truth.

## Repository Layout

- `plugins/Cymbals/src/` - Core DSP and UI source.
- `plugins/Cymbals/src/ui/tabs/` - LVGL stage-tab builders for `IMPULSES`, `DISPERSION`, `FILTERS`, `COLOUR`, and `FX CHAIN`; the `MOD` and `PRESET` surfaces are built from their respective UI modules in `PluginUI.cpp`.
- `plugins/Cymbals/src/cymbals-ui/` - Project-owned LVGL control kit (knobs, styles, shared palette).
- `deps/DPF`, `deps/lvgl`, `deps/dpf-widgets`, `deps/airwindows`, `deps/simde` - Pinned dependency submodules.
- `deps/vendored-fx/` - Header/source-only FX dependencies included directly by CMake.
- `deps/sst-*`, `deps/fmt/` - Vendored header-only DSP/support libraries.

## Shared UI Kit

UI knobs/controls live in-tree at `plugins/Cymbals/src/cymbals-ui`. The root `CMakeLists.txt` adds that directory after LVGL; kit changes ship in the same commit as the plugin code they serve. The kit's style/palette/knob implementation is the single source of truth.

## Technical Details
- **Sample Rate**: Optimized for 96kHz internal processing.
- **Optimization**: Minimal LVGL build (no examples, no demos, no ThorVG) for fast compilation.
- **Framework**: Built on the open-source DISTRHO Plugin Framework.

## License
The combined plugin is distributed under the GNU Affero General Public License v3.0; see [LICENSE](LICENSE).

- Project-owned `cymbals-ui` kit: MIT (see `plugins/Cymbals/src/cymbals-ui/LICENSE`).

Third-party terms:
- `DPF` and `DPF-Widgets`: ISC-style permissive licenses.
- `LVGL`, `Airwindows`, `fmt`, `simde`, CloudSeedCore, Signalsmith, and Soundpipe: MIT.
- `sst-effects`, `sst-basic-blocks`, `sst-filters`, `sst-waveshapers`, and the Surge FX extraction: GPLv3.
- `UnifiedFilter` contains GPLv3 adaptations of sst-filters models (Korg35, diode ladder, OB-Xd, TriPole, Cytomic tilt).
- HumpCrusher: AGPLv3.

The corresponding license files are kept in their dependency directories, and source for the GPL/AGPL components is included in this repository.
