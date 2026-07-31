// Board rendering for both games into the 1-bit framebuffer.
//
// Chess: 8x8 tiles, pieces as bold letters (white = outline tile, black =
// filled tile with inverted letter). Xiangqi: 9x10 intersections with the
// traditional characters from the generated 20x20 glyphs (XqGlyphs.h), red
// pieces in outline circles, black pieces in filled circles.
#pragma once

#ifdef ARDUINO

#include <string>
#include <vector>

#include "core/Puzzle.h"
#include "core/chess/Chess.h"
#include "core/xiangqi/Xiangqi.h"
#include "device/TextRenderer.h"

namespace chessink {

class BoardView {
 public:
  explicit BoardView(TextRenderer& tr) : tr_(tr) {}

  // Renders a chess position; `cursor`/`selected` are squares or -1;
  // `targets` are highlighted legal destinations.
  void drawChess(const chess::Position& pos, int cursor, int selected,
                 const std::vector<uint8_t>& targets, int originX, int originY);

  void drawXiangqi(const xiangqi::Position& pos, int cursor, int selected,
                   const std::vector<uint8_t>& targets, int originX, int originY);

  static constexpr int kChessTile = 40;
  static constexpr int kXqTile = 36;

 private:
  void tileMarkers(int x, int y, int size, bool cursor, bool selected, bool target);
  void glyph20(int x, int y, const uint32_t* rows, bool invert);
  TextRenderer& tr_;
};

}  // namespace chessink

#endif  // ARDUINO
