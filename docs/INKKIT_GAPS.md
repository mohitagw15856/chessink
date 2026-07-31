# inkkit gaps

ChessInk depends on inkkit v0.1.0-rc1. Gaps found:

## 1. No text or font engine (sixth app), and no CJK glyph path

The 5x7 ASCII renderer is copied again. ChessInk additionally had to
pre-generate 20x20 hanzi bitmaps (companion/tools/gen_glyphs.py) for the
Xiangqi pieces because there is no way to render CJK at runtime. A shared
inkkit text module with the ecosystem .cpfont loader would remove both.

## 2. No primitive drawing helpers

Board rendering wanted lines, rectangles, filled circles and bit-blits;
inkkit exposes only the raw framebuffer. Every app now carries a private
pixel/hline layer inside its TextRenderer copy. A tiny inkkit::gfx
(pixel/hline/vline/rect/disc/blit1bpp) would be enough.

## 3. No key-value settings helper (third app)

stats.txt parsing is the same hand-rolled loop again.
