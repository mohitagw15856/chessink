from chessink import lichess


def test_parse_csv_and_fields():
    text = ("PuzzleId,FEN,Moves,Rating,RatingDeviation,Popularity,NbPlays,Themes,GameUrl,OpeningTags\n"
            "abc,6k1/8/8/8/8/8/8/4R2K b - - 0 1,a7a6 e1e8,1200,80,90,10,mate backRank,url,\n")
    ps = lichess.parse_csv(text)
    assert len(ps) == 1
    assert ps[0].moves == ["a7a6", "e1e8"]
    assert ps[0].rating == 1200


def test_apply_uci_simple_and_side_toggle():
    fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    out = lichess.apply_uci(fen, "e2e4")
    assert " b " in out
    assert "/4P3/" in out          # pawn landed on e4
    assert out.split()[3] == "e3"  # double push exposes the ep square


def test_apply_uci_castling_moves_rook():
    fen = "4k3/8/8/8/8/8/8/4K2R w K - 0 1"
    out = lichess.apply_uci(fen, "e1g1")
    board = out.split()[0]
    assert board.endswith("5RK1")  # rook f1, king g1
    assert out.split()[2] == "-"   # right consumed


def test_apply_uci_en_passant_removes_pawn():
    fen = "4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1"
    out = lichess.apply_uci(fen, "e5d6")
    board = out.split()[0]
    assert board == "4k3/8/3P4/8/8/8/8/4K3"  # d5 pawn gone, ours on d6


def test_apply_uci_promotion():
    fen = "4k3/P7/8/8/8/8/8/4K3 w - - 0 1"
    out = lichess.apply_uci(fen, "a7a8q")
    assert out.split()[0].startswith("Q3k3")
