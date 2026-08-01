# The .pzl puzzle-pack format

A pack is a UTF-8 text file at `/chessink/puzzles/<name>.pzl`.

Line 1: magic `PZL 1`. Header (before the first `%%`):

| Line | Meaning |
|---|---|
| `M title <text>` | pack title |
| `M game chess` or `M game xiangqi` | which engine validates and renders |

Each puzzle starts with `%%`:

| Line | Meaning |
|---|---|
| `F <fen>` | the position the solver faces (their side to move) |
| `X <m1> <m2> ...` | solution line; m1 is the player's move, m2 the scripted reply, alternating |
| `R <rating>` | difficulty estimate (Elo-like) |
| `T <themes>` | space-separated theme tags (packs built from Lichess keep its tags) |

Moves are long algebraic: chess `e2e4`/`e7e8q`, Xiangqi `a8a9` with files
a-i from Red's left and ranks 0 (Red base) to 9. Unknown tags are skipped.

Note the Lichess convention difference: the Lichess database stores the
position *before* the opponent's setup move. `chessink pack` applies that
first move at build time, so on the device every puzzle opens with the
solver to move.
