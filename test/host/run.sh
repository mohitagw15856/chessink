#!/usr/bin/env bash
# Build and run the ChessInk host tests: perft vectors for both engines and
# engine validation of the shipped puzzle packs. Needs only a host compiler.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
root="$(cd "$here/../.." && pwd)"

CXX="${CXX:-g++}"
out="$(mktemp -d)"
trap 'rm -rf "$out"' EXIT

echo "Compiling host tests (CXX=$CXX, -O2 for perft)"
"$CXX" -std=gnu++2a -O2 -Wall -Wextra -Werror \
  -I"$root/src" \
  "$root/src/core/chess/Chess.cpp" \
  "$root/src/core/xiangqi/Xiangqi.cpp" \
  "$root/src/core/Puzzle.cpp" \
  "$here/test_core.cpp" \
  -o "$out/test_core"

"$out/test_core" "$root"/puzzles/*.pzl
