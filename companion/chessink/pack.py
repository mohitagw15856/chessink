"""Builds .pzl packs from Lichess CSV (chess) or the documented text format
(Xiangqi). See docs/FORMAT.md and docs/XIANGQI_FORMAT.md."""

from __future__ import annotations

from pathlib import Path

from . import lichess

MAGIC = "PZL 1"


def build_chess_pack(csv_text: str, title: str, min_rating: int = 0,
                     max_rating: int = 9999, theme: str | None = None,
                     limit: int = 500) -> str:
    puzzles = []
    for p in lichess.parse_csv(csv_text):
        if not (min_rating <= p.rating <= max_rating):
            continue
        if theme and theme not in p.themes.split():
            continue
        if len(p.moves) < 2:
            continue
        fen_after = lichess.apply_uci(p.fen, p.moves[0])
        puzzles.append((fen_after, p.moves[1:], p.rating, p.themes))
        if len(puzzles) >= limit:
            break
    if not puzzles:
        raise ValueError("no puzzles matched the filters")
    return _serialise(title, "chess", puzzles)


def parse_xiangqi_text(text: str) -> list[tuple[str, list[str], int, str]]:
    """Blank-line separated records of `fen`, `moves`, `rating`, `themes`
    lines (see docs/XIANGQI_FORMAT.md). Solutions start with the player's
    move directly (no Lichess-style setup move)."""
    puzzles = []
    fen, moves, rating, themes = None, [], 1500, ""
    def flush():
        nonlocal fen, moves, rating, themes
        if fen and moves:
            puzzles.append((fen, moves, rating, themes))
        fen, moves, rating, themes = None, [], 1500, ""
    for raw in text.splitlines():
        line = raw.strip()
        if not line:
            flush()
            continue
        if line.startswith("#"):
            continue
        key, _, value = line.partition(" ")
        if key == "fen":
            fen = value.strip()
        elif key == "moves":
            moves = value.split()
        elif key == "rating":
            rating = int(value)
        elif key == "themes":
            themes = value.strip()
    flush()
    if not puzzles:
        raise ValueError("no puzzles found")
    return puzzles


def build_xiangqi_pack(text: str, title: str, limit: int = 500) -> str:
    puzzles = parse_xiangqi_text(text)[:limit]
    return _serialise(title, "xiangqi", puzzles)


def _serialise(title: str, game: str, puzzles) -> str:
    out = [MAGIC, f"M title {title}", f"M game {game}"]
    for fen, moves, rating, themes in puzzles:
        out.append("%%")
        out.append(f"F {fen}")
        out.append("X " + " ".join(moves))
        out.append(f"R {rating}")
        if themes:
            out.append(f"T {themes}")
    return "\n".join(out) + "\n"
