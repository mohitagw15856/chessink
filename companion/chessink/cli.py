"""Command line interface: `chessink pack`."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

from . import pack as pack_mod


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="chessink", description="Build .pzl packs")
    sub = parser.add_subparsers(dest="command", required=True)

    b = sub.add_parser("pack", help="build a puzzle pack")
    b.add_argument("source", type=Path,
                   help="Lichess CSV export (chess) or Xiangqi text file")
    b.add_argument("--game", choices=["chess", "xiangqi"], required=True)
    b.add_argument("--title", required=True)
    b.add_argument("--out", required=True, type=Path)
    b.add_argument("--min-rating", type=int, default=0)
    b.add_argument("--max-rating", type=int, default=9999)
    b.add_argument("--theme")
    b.add_argument("--limit", type=int, default=500)

    args = parser.parse_args(argv)
    text = args.source.read_text(encoding="utf-8")
    try:
        if args.game == "chess":
            doc = pack_mod.build_chess_pack(text, args.title, args.min_rating,
                                            args.max_rating, args.theme, args.limit)
        else:
            doc = pack_mod.build_xiangqi_pack(text, args.title, args.limit)
    except ValueError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(doc, encoding="utf-8")
    print(args.out)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
