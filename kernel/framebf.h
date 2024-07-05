//Use RGBA32 (32 bits for each pixel)
#define COLOR_DEPTH 32
//Pixel Order: BGR in memory order (little endian --> RGB in byte order)
#define PIXEL_ORDER 0

#define SCREEN_WIDTH 1060
#define SCREEN_HEIGHT 700

void framebf_init();
void drawPixelARGB32(int x, int y, unsigned int attr);
void drawRectARGB32(int x1, int y1, int x2, int y2, unsigned int attr, int fill);
void display_image(int x, int y, int image_width, int image_height, const unsigned int* image_data, int transparency, int rotation);
void display_video(int x, int y);
void draw_char(int x, int y, unsigned char ch, unsigned int attr, float scale);
void draw_string(int x, int y, char *s, unsigned int attr, int spacing, float scale);
void clear_screen();
void* memset(void* dest, int value, int count);
void* memcpy(void* dest, const void* src, int count);
