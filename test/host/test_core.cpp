// Host tests: chess perft vectors, Xiangqi perft vectors, puzzle-format
// parsing, rating maths, and full engine validation of the shipped packs
// (all solution moves legal; mate-themed puzzles end in checkmate).
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "core/Puzzle.h"
#include "core/chess/Chess.h"
#include "core/xiangqi/Xiangqi.h"

using namespace chessink;

static int g_checks = 0;
#define CHECK(cond)                                                        \
  do {                                                                     \
    ++g_checks;                                                            \
    if (!(cond)) {                                                         \
      std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
      return 1;                                                            \
    }                                                                      \
  } while (0)

static int testChessPerft() {
  struct V { const char* fen; int depth; uint64_t want; };
  const V vs[] = {
      {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -", 1, 20},
      {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -", 2, 400},
      {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -", 3, 8902},
      {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -", 4, 197281},
      {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", 1, 48},
      {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", 2, 2039},
      {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", 3, 97862},
      {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", 4, 43238},
      {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", 5, 674624},
      {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 4, 422333},
      {"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 3, 62379},
  };
  for (const auto& v : vs) {
    chess::Position p;
    CHECK(p.setFen(v.fen));
    CHECK(p.perft(v.depth) == v.want);
  }
  return 0;
}

static int testXiangqiPerft() {
  xiangqi::Position p;
  const uint64_t want[] = {44, 1920, 79666, 3290240};
  for (int d = 1; d <= 4; ++d) {
    CHECK(p.perft(d) == want[d - 1]);
  }
  // Rule spot checks: flying general forbids the facing file.
  xiangqi::Position facing;
  CHECK(facing.setFen("4k4/9/9/9/9/9/9/9/9/4K4 r"));
  std::vector<xiangqi::Move> moves;
  facing.legalMoves(moves);
  // Red general on e0 faces e9: may not stay on the e-file is not required,
  // but moving along e (e0->e1) keeps facing and must be excluded.
  for (const auto& m : moves) {
    CHECK(!(xiangqi::fileOf(m.to) == 4 && xiangqi::fileOf(m.from) == 4));
  }
  CHECK(moves.size() == 2);  // d0 and f0 only
  return 0;
}

static int testPuzzleFormatAndRating() {
  Pack pack;
  CHECK(!parsePack("nope", pack));
  CHECK(parsePack("PZL 1\nM title T\nM game xiangqi\n%%\nF 4k4/9/9/9/9/9/9/9/9/4K4 r\nX a0a1\nR 1234\n", pack));
  CHECK(pack.game == Game::Xiangqi);
  CHECK(pack.puzzles.size() == 1);
  CHECK(pack.puzzles[0].rating == 1234);

  CHECK(updateRating(1500, 1500, true) == 1516);
  CHECK(updateRating(1500, 1500, false) == 1484);
  CHECK(updateRating(1500, 2100, false) > 1495);  // little loss vs monsters

  SolveStats s;
  s.record(true, 1200);
  s.record(true, 1200);
  s.record(false, 1200);
  CHECK(s.streak == 0 && s.solved == 2 && s.attempted == 3);
  SolveStats r = SolveStats::parse(s.serialise());
  CHECK(r.rating == s.rating && r.solved == 2);
  return 0;
}

static bool isMate(chess::Position& p) {
  std::vector<chess::Move> moves;
  p.legalMoves(moves);
  return moves.empty() && p.inCheck(p.whiteToMove());
}

static bool isMateXq(xiangqi::Position& p) {
  std::vector<xiangqi::Move> moves;
  p.legalMoves(moves);
  return moves.empty();
}

static int validatePackFile(const char* path) {
  std::ifstream in(path);
  CHECK(in.good());
  std::stringstream ss;
  ss << in.rdbuf();
  Pack pack;
  CHECK(parsePack(ss.str(), pack));
  CHECK(!pack.puzzles.empty());

  for (const auto& puzzle : pack.puzzles) {
    const bool wantsMate = puzzle.themes.find("mate") != std::string::npos;
    if (pack.game == Game::Chess) {
      chess::Position p;
      CHECK(p.setFen(puzzle.fen));
      for (const auto& text : puzzle.solution) {
        chess::Move m;
        CHECK(p.parseUci(text, m));  // every solution move must be legal
        chess::Undo u;
        p.makeMove(m, u);
      }
      if (wantsMate) CHECK(isMate(p));
    } else {
      xiangqi::Position p;
      CHECK(p.setFen(puzzle.fen));
      for (const auto& text : puzzle.solution) {
        xiangqi::Move m;
        CHECK(p.parseMove(text, m));
        xiangqi::Undo u;
        p.makeMove(m, u);
      }
      if (wantsMate) CHECK(isMateXq(p));
    }
  }
  std::printf("  pack ok: %s (%zu puzzles)\n", path, pack.puzzles.size());
  return 0;
}

int main(int argc, char** argv) {
  if (testChessPerft()) return 1;
  if (testXiangqiPerft()) return 1;
  if (testPuzzleFormatAndRating()) return 1;
  // Shipped packs, paths passed by run.sh.
  for (int i = 1; i < argc; ++i) {
    if (validatePackFile(argv[i])) return 1;
  }
  std::printf("%d checks, 0 failures\n", g_checks);
  return 0;
}
