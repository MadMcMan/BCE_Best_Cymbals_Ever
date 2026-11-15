#!/bin/sh
# Fail if any lv_arc_create outside src/UIWidgets.cpp
set -e
hits=$(grep -r "lv_arc_create" --include="*.cpp" --include="*.hpp" E:/dev/deps/cymbals-ui | grep -v "src/UIWidgets.cpp" || true)
if [ -n "$hits" ]; then
  echo "lint-knobs: bare lv_arc_create found outside src/UIWidgets.cpp:"
  echo "$hits"
  exit 1
fi
echo "lint-knobs: OK"
