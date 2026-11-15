#!/bin/sh
# Copy knob section from src/UIWidgets.cpp to .claude/skills/cymbals-knobs/references/knob-spec.md
# Fails CI if drifted — run this after every knob change.
set -e
echo "sync-knob-spec: manual — re-copy src/UIWidgets.cpp:118-214 into knob-spec.md"
echo "No auto-copy to avoid accidental truncation; verify diff:"
git -C E:/dev/deps/cymbals-ui diff -- .claude/skills/cymbals-knobs/references/knob-spec.md || true
