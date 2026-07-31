from pathlib import Path

import pytest

from chessink import pack
from chessink.cli import main

REPO = Path(__file__).resolve().parents[2]


def test_build_chess_pack_from_repo_source():
    csv_text = (REPO / "puzzles/src/chess-sample.csv").read_text()
    doc = pack.build_chess_pack(csv_text, "T")
    lines = doc.splitlines()
    assert lines[0] == "PZL 1"
    assert "M game chess" in lines
    assert doc.count("%%") == 14
    # Setup move applied: solutions start with the player's move.
    assert "X e1e8" in doc


def test_filters():
    csv_text = (REPO / "puzzles/src/chess-sample.csv").read_text()
    doc = pack.build_chess_pack(csv_text, "T", min_rating=1000)
    assert doc.count("%%") == 5
    doc = pack.build_chess_pack(csv_text, "T", theme="backRankMate")
    assert doc.count("%%") == 4
    with pytest.raises(ValueError):
        pack.build_chess_pack(csv_text, "T", min_rating=4000)


def test_xiangqi_pack_from_repo_source():
    text = (REPO / "puzzles/src/xiangqi-endgames.txt").read_text()
    doc = pack.build_xiangqi_pack(text, "XQ")
    assert "M game xiangqi" in doc
    assert doc.count("%%") == 8
    assert "X g0g9 d9d8 a1d1" in doc


def test_cli_roundtrip(tmp_path):
    out = tmp_path / "sample.pzl"
    rc = main(["pack", str(REPO / "puzzles/src/chess-sample.csv"), "--game", "chess",
               "--title", "Sample", "--out", str(out), "--limit", "3"])
    assert rc == 0
    assert out.read_text().count("%%") == 3
