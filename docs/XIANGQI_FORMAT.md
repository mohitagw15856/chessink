# Xiangqi puzzle source format

`chessink pack --game xiangqi` reads a plain-text file of blank-line
separated records:

```
fen 4k4/R3p4/9/9/9/9/9/9/9/3K5 r
moves a8a9
rating 900
themes mate mateIn1 rankMate
```

- `fen`: board rows from Black's base rank (9) down to Red's (0), digits for
  empty runs, letters `k a b/e n/h r c p` (upper case Red), then the side to
  move (`r`/`w` Red, anything else Black).
- `moves`: the solution, starting with the player's move (no Lichess-style
  setup move), in file-rank coordinates (`a8a9`): files a-i from Red's left,
  ranks 0-9 from Red's base.
- `#` lines are comments.

Every record in the shipped pack is validated by the real engine in
`test/host`: the whole line must be legal and mate-themed puzzles must end
in checkmate.
