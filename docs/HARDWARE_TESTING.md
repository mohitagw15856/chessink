# Hardware testing checklist

The two engines are exhaustively host-tested (perft vectors, pack
validation); what needs a device is interaction and rendering, each a
TODO(hardware-test):

## Build and flash

```sh
pio run -e xteink_x4            # run envs one at a time
pio run -e xteink_x4 -t upload
```

Copy `puzzles/*.pzl` to `/chessink/puzzles/` on the SD card.

## Items to verify on device

- Boot order; pack list renders; stats round-trip on SD.
- Board legibility on the panel: chess letter tiles and the 20x20 hanzi
  glyphs at arm's length; dark-square dot texture vs ghosting.
- Cursor walking with six buttons across 64/90 squares: speed, wrap
  behaviour, whether auto-repeat is needed.
- Legal-move highlighting correctness matches the engines (spot checks).
- Solve flow: wrong move shows the expected refutation; streak/rating
  persist across sleep.
- Fast vs full refresh cadence between moves and between puzzles.
