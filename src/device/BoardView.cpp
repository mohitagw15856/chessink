#include "device/BoardView.h"

#ifdef ARDUINO

#include <algorithm>

#include "device/XqGlyphs.h"

namespace chessink {

namespace {

bool contains(const std::vector<uint8_t>& v, int sq) {
  return std::find(v.begin(), v.end(), static_cast<uint8_t>(sq)) != v.end();
}

char chessLetter(int8_t p) {
  const char names[] = " PNBRQK";
  const int a = p > 0 ? p : -p;
  return names[a];
}

}  // namespace

void BoardView::tileMarkers(int x, int y, int size, bool cursor, bool selected, bool target) {
  if (cursor) {
    // Double border.
    for (int i = 0; i < 2; ++i) {
      tr_.hline(x + i, y + i, size - 2 * i);
      tr_.hline(x + i, y + size - 1 - i, size - 2 * i);
      for (int yy = y + i; yy < y + size - i; ++yy) {
        tr_.pixel(x + i, yy);
        tr_.pixel(x + size - 1 - i, yy);
      }
    }
  }
  if (selected) {
    // Corner ticks.
    for (int i = 0; i < 6; ++i) {
      tr_.pixel(x + i, y);
      tr_.pixel(x, y + i);
      tr_.pixel(x + size - 1 - i, y + size - 1);
      tr_.pixel(x + size - 1, y + size - 1 - i);
    }
  }
  if (target) {
    // Centre dot.
    const int cx = x + size / 2;
    const int cy = y + size / 2;
    for (int dy = -2; dy <= 2; ++dy) {
      for (int dx = -2; dx <= 2; ++dx) {
        if (dx * dx + dy * dy <= 4) tr_.pixel(cx + dx, cy + dy);
      }
    }
  }
}

void BoardView::glyph20(int x, int y, const uint32_t* rows, bool invert) {
  for (int gy = 0; gy < kXqGlyphSize; ++gy) {
    const uint32_t row = rows[gy];
    for (int gx = 0; gx < kXqGlyphSize; ++gx) {
      const bool on = row & (1u << (kXqGlyphSize - 1 - gx));
      if (on != invert) tr_.pixel(x + gx, y + gy);
    }
  }
}

void BoardView::drawChess(const chess::Position& pos, int cursor, int selected,
                          const std::vector<uint8_t>& targets, int ox, int oy) {
  const int T = kChessTile;
  for (int rank = 7; rank >= 0; --rank) {
    for (int file = 0; file < 8; ++file) {
      const int sq = rank * 8 + file;
      const int x = ox + file * T;
      const int y = oy + (7 - rank) * T;

      // Checkerboard: dark squares get a sparse dot texture that reads as
      // grey on the panel without hiding the pieces.
      const bool dark = ((file + rank) & 1) == 0;
      if (dark) {
        for (int yy = 2; yy < T - 2; yy += 4) {
          for (int xx = 2; xx < T - 2; xx += 4) tr_.pixel(x + xx, y + yy);
        }
      }
      tr_.hline(x, y, T);
      tr_.hline(x, y + T - 1, T);
      for (int yy = y; yy < y + T; ++yy) {
        tr_.pixel(x, yy);
        tr_.pixel(x + T - 1, yy);
      }

      const int8_t p = pos.at(sq);
      if (p != 0) {
        const bool black = p < 0;
        if (black) {
          // Filled inner tile, letter knocked out.
          for (int yy = 4; yy < T - 4; ++yy) tr_.hline(x + 4, y + yy, T - 8);
          std::string s(1, chessLetter(p));
          // Letter in the middle via the text renderer, inverted look.
          tr_.textInverted(x + T / 2 - 3, y + T / 2 - 5, s, 12);
        } else {
          std::string s(1, chessLetter(p));
          tr_.text(x + T / 2 - 3, y + T / 2 - 5, s);
        }
      }
      tileMarkers(x, y, T, sq == cursor, sq == selected, contains(targets, sq));
    }
  }
}

void BoardView::drawXiangqi(const xiangqi::Position& pos, int cursor, int selected,
                            const std::vector<uint8_t>& targets, int ox, int oy) {
  const int T = kXqTile;
  // Grid: 9 files x 10 ranks drawn as intersections.
  for (int r = 0; r < xiangqi::kRanks; ++r) {
    tr_.hline(ox, oy + r * T, (xiangqi::kFiles - 1) * T + 1);
  }
  for (int f = 0; f < xiangqi::kFiles; ++f) {
    for (int yy = 0; yy <= (xiangqi::kRanks - 1) * T; ++yy) {
      // River gap between ranks 4 and 5 except the edges.
      const int rankPos = yy / T;
      if (rankPos == 4 && yy % T != 0 && f != 0 && f != xiangqi::kFiles - 1) continue;
      tr_.pixel(ox + f * T, oy + yy);
    }
  }

  for (int sq = 0; sq < xiangqi::kSquares; ++sq) {
    const int8_t p = pos.at(sq);
    const int f = xiangqi::fileOf(sq);
    const int r = xiangqi::rankOf(sq);
    // Screen coordinates: rank 9 at the top (Black), rank 0 bottom (Red).
    const int cx = ox + f * T;
    const int cy = oy + (xiangqi::kRanks - 1 - r) * T;

    if (p != 0) {
      const bool red = p > 0;
      const int R = 13;
      // Disc: outline for red, filled for black.
      for (int dy = -R; dy <= R; ++dy) {
        for (int dx = -R; dx <= R; ++dx) {
          const int d2 = dx * dx + dy * dy;
          if (d2 > R * R) continue;
          const bool edge = d2 >= (R - 1) * (R - 1);
          if (!red || edge) {
            if (red && !edge) continue;
            tr_.pixel(cx + dx, cy + dy);
          }
        }
      }
      const int a = p > 0 ? p : -p;
      const uint32_t* rows = kXqGlyphs[a][red ? 0 : 1];
      if (rows) glyph20(cx - kXqGlyphSize / 2, cy - kXqGlyphSize / 2, rows, !red);
    }
    const int half = T / 2 - 2;
    tileMarkers(cx - half, cy - half, 2 * half, sq == cursor, sq == selected,
                contains(targets, sq));
  }
}

}  // namespace chessink

#endif  // ARDUINO
