# Xiangqi move-generation test vectors

The Xiangqi engine is validated in `test/host` against the widely documented
perft values for the standard starting position (see e.g. the Xiangqi
sections of the chess-programming community wikis and the xqbase engine
documentation):

| Depth | Nodes |
|---|---|
| 1 | 44 |
| 2 | 1 920 |
| 3 | 79 666 |
| 4 | 3 290 240 |

plus targeted rule checks: the flying-general rule excludes moves that leave
the generals facing on an open file (verified on a two-generals position
where exactly the two sideways king steps remain legal), and every shipped
puzzle is validated end to end (legal line, true mate).

The move generator covers palace confinement (general, advisors), the
elephant river limit and blocked eye, horse-leg blocking, cannon screens,
and soldier promotion at the river.
