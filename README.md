# ChessInk

**Chess and Xiangqi puzzles for the Xteink X4/X3.** Daily tactics on paper-
like ink: crisp board graphics, button-driven move input with legal-move
highlighting, full rules enforcement for both games, and streak/rating
tracking on the SD card.

Status: builds in CI, not yet verified on device.

Compatible with the CrossPoint ecosystem: same hardware, device layer from
[inkkit](https://github.com/mohitagw15856/inkkit) (pinned in
`platformio.ini`), files under `/chessink` on the SD card.

## The engines are the point

Move generation and validation live in pure, hardware-free modules
(`src/core/chess`, `src/core/xiangqi`) with full rules:

- **Chess**: castling legality, en passant (including the pinned-ep case),
  promotions. Validated against the standard perft suite (start position,
  Kiwipete, and three edge-case positions) in `test/host`.
- **Xiangqi**: palace confinement, elephant river and eye, horse legs,
  cannon screens, soldier promotion, flying-general rule. Validated against
  the documented start-position perft values 44 / 1 920 / 79 666 /
  3 290 240 (docs/XIANGQI_VECTORS.md).

Every shipped puzzle is engine-validated in the test suite: whole solution
line legal, and mate-themed puzzles proven to end in checkmate. Xiangqi
pieces render as traditional characters from pre-generated 1-bit bitmaps.

## Puzzles

Two starter packs ship in `puzzles/`:

- `chess-tactics-sample.pzl`: 14 curated tactics (back-rank, smothered,
  epaulette, ladder and fork patterns) in the Lichess data convention.
- `xiangqi-endgames.pzl`: 8 engine-verified endgame studies (rank mates,
  cannon mate, a flying-general mate-in-2).

These are samplers: the full "500 from the CC0 Lichess database" pack is a
one-liner once you download the (large) dump from
https://database.lichess.org/:

```sh
zstd -d lichess_db_puzzle.csv.zst
chessink pack lichess_db_puzzle.csv --game chess --title "Lichess 1600-1900" \
  --min-rating 1600 --max-rating 1900 --theme mateIn2 --limit 500 \
  --out /Volumes/SD/chessink/puzzles/lichess-mate2.pzl
```

Correspondence play is a documented v2 behind a feature flag
(docs/CORRESPONDENCE.md); no network code ships in v1.

## Install and flash

```sh
pio run -e xteink_x4              # or -e xteink_x3; run one at a time
pio run -e xteink_x4 -t upload
```

Copy `puzzles/*.pzl` to `/chessink/puzzles/`. Controls: arrows move the
cursor, SELECT picks up a piece (legal destinations get dots) and drops it,
BACK returns. Promotions default to queen.

## Companion

```sh
pip install -e "companion[test]"
chessink pack --help
```

`companion/tools/gen_glyphs.py` regenerates the Xiangqi glyph header from a
CJK font (the header is committed, so this is only needed to change glyphs).

## Documentation

- [docs/FORMAT.md](docs/FORMAT.md) - the .pzl format
- [docs/XIANGQI_FORMAT.md](docs/XIANGQI_FORMAT.md) - Xiangqi source format
- [docs/XIANGQI_VECTORS.md](docs/XIANGQI_VECTORS.md) - engine test vectors
- [docs/CORRESPONDENCE.md](docs/CORRESPONDENCE.md) - the v2 protocol design
- [docs/HARDWARE_TESTING.md](docs/HARDWARE_TESTING.md) - on-device checklist
- [docs/INKKIT_GAPS.md](docs/INKKIT_GAPS.md) - device-layer gaps for inkkit
- [ARCHITECTURE.md](ARCHITECTURE.md)

## Licence

MIT, see [LICENSE](LICENSE). The Lichess puzzle database is CC0. The 5x7
text renderer is shared ecosystem code originating in InkQuest (same
author, MIT).
