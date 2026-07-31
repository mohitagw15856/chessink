"""Lichess puzzle CSV handling.

The Lichess puzzle database (CC0) stores each puzzle as the FEN *before* the
opponent's last move, with `Moves` starting from that opponent move; the
solver answers from move two. The pack builder applies the first move
mechanically (the data is trusted) and stores the resulting position with the
remaining moves as the solution line.
"""

from __future__ import annotations

import csv
from dataclasses import dataclass


@dataclass
class LichessPuzzle:
    puzzle_id: str
    fen: str
    moves: list[str]
    rating: int
    themes: str


def parse_csv(text: str) -> list[LichessPuzzle]:
    out: list[LichessPuzzle] = []
    reader = csv.reader(text.splitlines())
    header = next(reader, None)
    if header is None:
        return out
    idx = {name: i for i, name in enumerate(header)}
    for row in reader:
        if not row:
            continue
        out.append(
            LichessPuzzle(
                puzzle_id=row[idx.get("PuzzleId", 0)],
                fen=row[idx.get("FEN", 1)],
                moves=row[idx.get("Moves", 2)].split(),
                rating=int(row[idx.get("Rating", 3)] or 1500),
                themes=row[idx.get("Themes", 7)] if idx.get("Themes", 7) < len(row) else "",
            )
        )
    return out


def _parse_board(fen: str):
    fields = fen.split()
    rows = fields[0].split("/")
    board: dict[tuple[int, int], str] = {}
    for r, row in enumerate(rows):  # r=0 is rank 8
        f = 0
        for c in row:
            if c.isdigit():
                f += int(c)
            else:
                board[(f, 7 - r)] = c
                f += 1
    return board, fields


def _emit_board(board: dict[tuple[int, int], str]) -> str:
    rows = []
    for r in range(7, -1, -1):
        run = 0
        row = ""
        for f in range(8):
            piece = board.get((f, r))
            if piece is None:
                run += 1
            else:
                if run:
                    row += str(run)
                    run = 0
                row += piece
        if run:
            row += str(run)
        rows.append(row)
    return "/".join(rows)


def apply_uci(fen: str, uci: str) -> str:
    """Applies one trusted UCI move to a FEN, returning the new FEN.

    Handles promotion, castling rook hops, en passant capture, castling-right
    and en-passant-field bookkeeping. No legality checking: the move comes
    from the Lichess database.
    """
    board, fields = _parse_board(fen)
    side = fields[1]
    castling = fields[2] if len(fields) > 2 else "-"
    ff, fr = ord(uci[0]) - 97, int(uci[1]) - 1
    tf, tr = ord(uci[2]) - 97, int(uci[3]) - 1
    piece = board.pop((ff, fr))

    # En passant: pawn moves diagonally onto an empty square.
    if piece.lower() == "p" and ff != tf and (tf, tr) not in board:
        board.pop((tf, fr), None)

    # Promotion.
    if len(uci) == 5:
        piece = uci[4].upper() if side == "w" else uci[4].lower()

    board[(tf, tr)] = piece

    # Castling: king moves two files; hop the rook.
    if piece.lower() == "k" and abs(tf - ff) == 2:
        rank = fr
        if tf == 6:
            board[(5, rank)] = board.pop((7, rank))
        else:
            board[(3, rank)] = board.pop((0, rank))

    # Castling rights.
    rights = set(castling) - {"-"}
    if piece == "K":
        rights -= {"K", "Q"}
    if piece == "k":
        rights -= {"k", "q"}
    for corner, right in [((7, 0), "K"), ((0, 0), "Q"), ((7, 7), "k"), ((0, 7), "q")]:
        if (ff, fr) == corner or (tf, tr) == corner:
            rights.discard(right)

    # En passant availability after a double push.
    ep = "-"
    if piece.lower() == "p" and abs(tr - fr) == 2:
        ep = uci[0] + ("3" if side == "w" else "6")

    new_side = "b" if side == "w" else "w"
    rights_text = "".join(c for c in "KQkq" if c in rights) or "-"
    return f"{_emit_board(board)} {new_side} {rights_text} {ep} 0 1"
