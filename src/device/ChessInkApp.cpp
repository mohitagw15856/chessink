#include "device/ChessInkApp.h"

#ifdef ARDUINO

#include <inkkit/Storage.h>

namespace chessink {

namespace {

constexpr const char* kAppRoot = "/chessink";
constexpr const char* kPuzzlesDir = "/chessink/puzzles";
constexpr const char* kStatsPath = "/chessink/stats.txt";

// Ecosystem button convention. TODO(hardware-test): confirm indices, and
// whether cursor-walking a 90-square board with six buttons feels usable or
// needs an accelerating repeat.
constexpr uint8_t kBtnBack = 0;
constexpr uint8_t kBtnSelect = 1;
constexpr uint8_t kBtnLeft = 2;
constexpr uint8_t kBtnRight = 3;
constexpr uint8_t kBtnUp = 4;
constexpr uint8_t kBtnDown = 5;

}  // namespace

void ChessInkApp::begin() {
  inkkit::sd::ensureDir(kAppRoot);
  inkkit::sd::ensureDir(kPuzzlesDir);
  std::string statsText;
  if (inkkit::sd::readWholeFile(kStatsPath, statsText)) {
    stats_ = SolveStats::parse(statsText);
  }
  refreshPacks();
  renderPacks(true);
}

void ChessInkApp::refreshPacks() {
  packNames_.clear();
  inkkit::sd::listFiles(kPuzzlesDir, ".pzl", [&](const std::string& path) {
    const size_t slash = path.find_last_of('/');
    packNames_.push_back(slash == std::string::npos ? path : path.substr(slash + 1));
  });
  if (packSel_ >= static_cast<int>(packNames_.size())) packSel_ = 0;
}

bool ChessInkApp::loadPack(const std::string& name) {
  std::string text;
  if (!inkkit::sd::readWholeFile((std::string(kPuzzlesDir) + "/" + name).c_str(), text)) {
    return false;
  }
  return parsePack(text, pack_);
}

void ChessInkApp::startPuzzle(size_t index) {
  puzzleIndex_ = index % pack_.puzzles.size();
  const Puzzle& p = pack_.puzzles[puzzleIndex_];
  solutionPly_ = 0;
  selected_ = -1;
  targets_.clear();
  if (pack_.game == Game::Chess) {
    chessPos_.setFen(p.fen);
    cursor_ = 28;  // e4-ish, middle of the board
  } else {
    xqPos_.setFen(p.fen);
    cursor_ = xiangqi::sqOf(4, 4);
  }
  screen_ = Screen::Puzzle;
  renderPuzzle(true);
}

void ChessInkApp::computeTargets() {
  targets_.clear();
  if (selected_ < 0) return;
  if (pack_.game == Game::Chess) {
    std::vector<chess::Move> moves;
    chessPos_.legalMoves(moves);
    for (const auto& m : moves) {
      if (m.from == selected_) targets_.push_back(m.to);
    }
  } else {
    std::vector<xiangqi::Move> moves;
    xqPos_.legalMoves(moves);
    for (const auto& m : moves) {
      if (m.from == selected_) targets_.push_back(m.to);
    }
  }
}

int ChessInkApp::cursorStep(int sq, int dx, int dy) const {
  if (pack_.game == Game::Chess) {
    int f = sq & 7;
    int r = sq >> 3;
    f = (f + dx + 8) % 8;
    r = (r + dy + 8) % 8;
    return r * 8 + f;
  }
  int f = xiangqi::fileOf(sq);
  int r = xiangqi::rankOf(sq);
  f = (f + dx + xiangqi::kFiles) % xiangqi::kFiles;
  r = (r + dy + xiangqi::kRanks) % xiangqi::kRanks;
  return xiangqi::sqOf(f, r);
}

void ChessInkApp::applyPlayerMove(const std::string& text) {
  const Puzzle& p = pack_.puzzles[puzzleIndex_];
  const std::string expected = p.solution[solutionPly_];

  if (text != expected) {
    finishPuzzle(false, expected);
    return;
  }

  // Play the (correct) move.
  if (pack_.game == Game::Chess) {
    chess::Move m;
    chess::Undo u;
    if (chessPos_.parseUci(text, m)) chessPos_.makeMove(m, u);
  } else {
    xiangqi::Move m;
    xiangqi::Undo u;
    if (xqPos_.parseMove(text, m)) xqPos_.makeMove(m, u);
  }
  ++solutionPly_;

  if (solutionPly_ >= p.solution.size()) {
    finishPuzzle(true, "");
    return;
  }
  opponentReply();
}

void ChessInkApp::opponentReply() {
  const Puzzle& p = pack_.puzzles[puzzleIndex_];
  const std::string reply = p.solution[solutionPly_];
  if (pack_.game == Game::Chess) {
    chess::Move m;
    chess::Undo u;
    if (chessPos_.parseUci(reply, m)) chessPos_.makeMove(m, u);
  } else {
    xiangqi::Move m;
    xiangqi::Undo u;
    if (xqPos_.parseMove(reply, m)) xqPos_.makeMove(m, u);
  }
  ++solutionPly_;
  renderPuzzle(false);
}

void ChessInkApp::finishPuzzle(bool solved, const std::string& refutation) {
  const Puzzle& p = pack_.puzzles[puzzleIndex_];
  stats_.record(solved, p.rating);
  inkkit::sd::writeWholeFile(kStatsPath, stats_.serialise());
  if (solved) {
    renderResult("Solved",
                 "Streak " + std::to_string(stats_.streak) + ", rating " +
                     std::to_string(stats_.rating));
  } else {
    renderResult("Not this time", "Expected " + refutation + ". Rating " +
                                      std::to_string(stats_.rating));
  }
}

void ChessInkApp::tick() {
  buttons_.update();

  switch (screen_) {
    case Screen::Packs: {
      if (buttons_.wasPressed(kBtnDown) && packSel_ + 1 < static_cast<int>(packNames_.size())) {
        ++packSel_;
        renderPacks(false);
      } else if (buttons_.wasPressed(kBtnUp) && packSel_ > 0) {
        --packSel_;
        renderPacks(false);
      } else if (buttons_.wasPressed(kBtnSelect) && !packNames_.empty()) {
        if (loadPack(packNames_[static_cast<size_t>(packSel_)])) {
          // Daily tactics: rotate by attempt count so each day advances.
          startPuzzle(stats_.attempted % pack_.puzzles.size());
        }
      }
      break;
    }
    case Screen::Puzzle: {
      int dx = 0;
      int dy = 0;
      if (buttons_.wasPressed(kBtnLeft)) dx = -1;
      if (buttons_.wasPressed(kBtnRight)) dx = 1;
      if (buttons_.wasPressed(kBtnUp)) dy = 1;
      if (buttons_.wasPressed(kBtnDown)) dy = -1;
      if (dx != 0 || dy != 0) {
        cursor_ = cursorStep(cursor_, dx, dy);
        renderPuzzle(false);
      } else if (buttons_.wasPressed(kBtnSelect)) {
        if (selected_ < 0) {
          selected_ = cursor_;
          computeTargets();
          if (targets_.empty()) selected_ = -1;  // nothing to move here
          renderPuzzle(false);
        } else if (cursor_ == selected_) {
          selected_ = -1;
          targets_.clear();
          renderPuzzle(false);
        } else {
          // Attempt the move (promotions default to queen).
          std::string text;
          if (pack_.game == Game::Chess) {
            chess::Move m;
            m.from = static_cast<uint8_t>(selected_);
            m.to = static_cast<uint8_t>(cursor_);
            text = chess::Position::uci(m);
            chess::Move probe;
            if (!chessPos_.parseUci(text, probe)) {
              if (!chessPos_.parseUci(text + "q", probe)) {
                selected_ = -1;
                targets_.clear();
                renderPuzzle(false);
                break;
              }
              text += "q";
            }
          } else {
            xiangqi::Move m;
            m.from = static_cast<uint8_t>(selected_);
            m.to = static_cast<uint8_t>(cursor_);
            text = xiangqi::Position::moveText(m);
            xiangqi::Move probe;
            if (!xqPos_.parseMove(text, probe)) {
              selected_ = -1;
              targets_.clear();
              renderPuzzle(false);
              break;
            }
          }
          selected_ = -1;
          targets_.clear();
          applyPlayerMove(text);
        }
      } else if (buttons_.wasPressed(kBtnBack)) {
        screen_ = Screen::Packs;
        renderPacks(true);
      }
      break;
    }
    case Screen::Result: {
      if (buttons_.wasPressed(kBtnSelect) && !pack_.puzzles.empty()) {
        startPuzzle(puzzleIndex_ + 1);
      } else if (buttons_.wasPressed(kBtnBack)) {
        screen_ = Screen::Packs;
        renderPacks(true);
      }
      break;
    }
  }
}

void ChessInkApp::renderPacks(bool full) {
  tr_.clear();
  tr_.textInverted(8, 6, "ChessInk  (SELECT opens a pack)", tr_.lineHeight() + 6);
  int y = 2 * tr_.lineHeight() + 10;
  if (packNames_.empty()) {
    tr_.text(8, y, "No packs in /chessink/puzzles.");
    tr_.text(8, y + tr_.lineHeight(), "Build one with: chessink pack");
  }
  for (size_t i = 0; i < packNames_.size(); ++i) {
    const std::string marker = (static_cast<int>(i) == packSel_) ? "> " : "  ";
    tr_.text(8, y, marker + packNames_[i]);
    y += tr_.lineHeight();
  }
  y += tr_.lineHeight();
  tr_.text(8, y, "Rating " + std::to_string(stats_.rating) + "  streak " +
                     std::to_string(stats_.streak) + "  solved " +
                     std::to_string(stats_.solved) + "/" + std::to_string(stats_.attempted));
  tr_.flush(full);
}

void ChessInkApp::renderPuzzle(bool full) {
  tr_.clear();
  const Puzzle& p = pack_.puzzles[puzzleIndex_];
  std::string head = pack_.title.empty() ? "Puzzle" : pack_.title;
  head += "  #" + std::to_string(puzzleIndex_ + 1) + "  (" + std::to_string(p.rating) + ")";
  tr_.textInverted(8, 6, head, tr_.lineHeight() + 6);

  const int oy = tr_.lineHeight() + 16;
  if (pack_.game == Game::Chess) {
    const int ox = (tr_.width() - 8 * BoardView::kChessTile) / 2;
    board_.drawChess(chessPos_, cursor_, selected_, targets_, ox, oy);
    tr_.text(8, tr_.height() - tr_.lineHeight() - 4,
             chessPos_.whiteToMove() ? "White to move." : "Black to move.");
  } else {
    const int ox = (tr_.width() - (xiangqi::kFiles - 1) * BoardView::kXqTile) / 2;
    board_.drawXiangqi(xqPos_, cursor_, selected_, targets_, ox, oy);
    tr_.text(8, tr_.height() - tr_.lineHeight() - 4,
             xqPos_.redToMove() ? "Red to move." : "Black to move.");
  }
  tr_.flush(full);
}

void ChessInkApp::renderResult(const std::string& headline, const std::string& detail) {
  screen_ = Screen::Result;
  tr_.clear();
  tr_.textInverted(8, 6, headline, tr_.lineHeight() + 6);
  tr_.text(8, tr_.height() / 2, detail);
  tr_.text(8, tr_.height() - 2 * tr_.lineHeight(), "SELECT next puzzle. BACK to packs.");
  tr_.flush(true);
}

}  // namespace chessink

#endif  // ARDUINO
