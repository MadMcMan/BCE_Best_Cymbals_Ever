# Vendored FX libraries

These sources are vendored (not git submodules) so the plugin builds without
network access. Every directory keeps its upstream licence file.

| Directory | Upstream | Commit | Licence | Used for |
|---|---|---|---|---|
| `cloudseedcore` | GhostNoteAudio/CloudSeedCore | cloned 2026-10-27 | MIT | `CloudSeed Reverb` |
| `signalsmith-basics` | Signalsmith-Audio/basics | cloned 2026-10-27 | MIT | `Signalsmith Chorus` |
| `signalsmith-dsp` | Signalsmith-Audio/dsp | cloned 2026-10-27 | MIT | Signalsmith support layer |
| `signalsmith-stretch` | Signalsmith-Audio/signalsmith-stretch | cloned 2026-10-27 | MIT | `Signalsmith Stretch` |
| `signalsmith-linear` | Signalsmith-Audio/linear | cloned 2026-10-27 | MIT | Stretch support layer |
| `soundpipe` | PaulBatchelor/Soundpipe | cloned 2026-10-27 | MIT | `Soundpipe Phaser` (selected module only) |

The Soundpipe copy is intentionally minimal: `base.h`, `phaser.h`,
`modules/phaser.c`, `CUI.h` and its licence. The full 16 MB tree is not needed
for the phaser entry.

CloudSeedCore needs `BUFFER_SIZE=1024` and `MAX_STR_SIZE=32` (the values from
its Visual Studio project); `VendoredFxSlot.cpp` supplies them before the
headers. Its inline `strcpy_s` formatter is not used by the wrapper.

To update a library, replace the directory contents and re-run the DSP,
VST3/CLAP/JACK and standalone builds.
