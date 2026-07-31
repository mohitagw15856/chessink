# ChessInk architecture

## Purity boundary

The reviewer's requirement drove the design: move generation/validation is
a pure, hardware-free module per game, and the native tests are the
authority. Both engines use the same shape: pseudo-legal generation plus a
make/undo legality filter. That trades speed for auditability; puzzle-app
loads are tiny, and perft (the correctness instrument) still runs the full
suite in under a second at -O2.

The filter approach buys correctness in the cases that kill hand-rolled
engines: chess pinned-en-passant and castling-through-check fall out of
"make the move, ask if the king is attacked", and the Xiangqi
flying-general rule is one extra predicate evaluated after every trial
move (which also makes rank-mate compositions work: an escape square that
looks shielded by the general itself is correctly found attacked after the
general steps away).

Everything shipped is validated by the engines themselves in test/host:
solution lines must be legal, mate-themed puzzles must end in true
checkmate. Two composition bugs were caught this way before first commit
(a blocked scholar's-mate file; illegal generals-facing start positions).

## Rendering

Chess pieces are bold letters on outline/filled tiles: honest, crisp 1-bit
e-ink design instead of downscaled anti-aliased sprites. Xiangqi uses the
traditional characters, pre-rendered to 20x20 1-bit bitmaps at build time
(companion/tools/gen_glyphs.py) because the ecosystem has no runtime CJK
path yet (docs/INKKIT_GAPS.md); red pieces are outline discs, black filled.

## Device flow

Pack list -> puzzle (cursor + select input, legal-target dots, scripted
opponent replies from the solution line) -> result (streak + Elo-style
rating persisted at /chessink/stats.txt). "Daily tactics" rotates the
starting puzzle by attempt count, so the pack advances day by day without
needing the clock.

## Fork vs standalone

Standalone firmware on inkkit, like the rest of the ecosystem: a puzzle
app shares no application logic with an EPUB reader, and the device layer
is already common via inkkit.
