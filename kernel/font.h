#define UNICODE_FIRST 32	/*First Unicode letter in this font*/
#define UNICODE_LAST 126	/*Last Unicode letter in this font*/
#define HEIGHT_PX 48	    /*Font height in pixels*/
#define GLYPH_COUNT 95		/*Number of glyphs in the font*/

struct glyph_desc {
  unsigned int width_px;
  unsigned int glyph_index;
};

extern const unsigned int bitmap[];
extern const struct glyph_desc glyph_desc[];
