// ChessInk screens: pack list, puzzle board with cursor move input and
// legal-move highlighting, solve flow with refutation display, and
// SD-persisted stats (streak + rating estimate).
#pragma once

#ifdef ARDUINO

#include <string>
#include <vector>

#include <inkkit/Buttons.h>

#include "core/Puzzle.h"
#include "core/chess/Chess.h"
#include "core/xiangqi/Xiangqi.h"
#include "device/BoardView.h"
#include "device/TextRenderer.h"

namespace chessink {

class ChessInkApp {
 public:
  ChessInkApp(TextRenderer& tr, inkkit::Buttons& buttons)
      : tr_(tr), buttons_(buttons), board_(tr) {}

  void begin();
  void tick();

 private:
  enum class Screen : uint8_t { Packs, Puzzle, Result };

  void refreshPacks();
  bool loadPack(const std::string& name);
  void startPuzzle(size_t index);
  void computeTargets();
  int cursorStep(int sq, int dx, int dy) const;  // grid-aware cursor move
  void applyPlayerMove(const std::string& text);
  void opponentReply();
  void finishPuzzle(bool solved, const std::string& refutation);

  void renderPacks(bool full);
  void renderPuzzle(bool full);
  void renderResult(const std::string& headline, const std::string& detail);

  TextRenderer& tr_;
  inkkit::Buttons& buttons_;
  BoardView board_;

  Screen screen_ = Screen::Packs;
  std::vector<std::string> packNames_;
  int packSel_ = 0;

  Pack pack_;
  size_t puzzleIndex_ = 0;
  chess::Position chessPos_;
  xiangqi::Position xqPos_;
  size_t solutionPly_ = 0;

  int cursor_ = 0;
  int selected_ = -1;
  std::vector<uint8_t> targets_;

  SolveStats stats_;
};

}  // namespace chessink

#endif  // ARDUINO
