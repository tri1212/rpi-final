#include "mbox.h"
#include "uart1.h"
#include "framebf.h"
#include "image.h"
#include "font.h"
#include "math.h"

// Screen info
unsigned int width, height, pitch;
/* Frame buffer address
 * (declare as pointer of unsigned char to access each byte) */
unsigned char *fb;
/**
 * Set screen resolution to 1024x768
 */
void framebf_init()
{
    mBuf[0] = 35 * 4; // Length of message in bytes
    mBuf[1] = MBOX_REQUEST;
    mBuf[2] = MBOX_TAG_SETPHYWH;  // Set physical width-height
    mBuf[3] = 8;                  // Value size in bytes
    mBuf[4] = 0;                  // REQUEST CODE = 0
    mBuf[5] = SCREEN_WIDTH;       // Value(width)
    mBuf[6] = SCREEN_HEIGHT;      // Value(height)
    mBuf[7] = MBOX_TAG_SETVIRTWH; // Set virtual width-height
    mBuf[8] = 8;
    mBuf[9] = 0;
    mBuf[10] = SCREEN_WIDTH;
    mBuf[11] = SCREEN_HEIGHT;
    mBuf[12] = MBOX_TAG_SETVIRTOFF; // Set virtual offset
    mBuf[13] = 8;
    mBuf[14] = 0;
    mBuf[15] = 0;                 // x offset
    mBuf[16] = 0;                 // y offset
    mBuf[17] = MBOX_TAG_SETDEPTH; // Set color depth
    mBuf[18] = 4;
    mBuf[19] = 0;
    mBuf[20] = COLOR_DEPTH;         // Bits per pixel
    mBuf[21] = MBOX_TAG_SETPXLORDR; // Set pixel order
    mBuf[22] = 4;
    mBuf[23] = 0;
    mBuf[24] = PIXEL_ORDER;
    mBuf[25] = MBOX_TAG_GETFB; // Get frame buffer
    mBuf[26] = 8;
    mBuf[27] = 0;
    mBuf[28] = 16;                // alignment in 16 bytes
    mBuf[29] = 0;                 // will return Frame Buffer size in bytes
    mBuf[30] = MBOX_TAG_GETPITCH; // Get pitch
    mBuf[31] = 4;
    mBuf[32] = 0;
    mBuf[33] = 0; // Will get pitch value here
    mBuf[34] = MBOX_TAG_LAST;
    // Call Mailbox
    if (mbox_call(ADDR(mBuf), MBOX_CH_PROP) // mailbox call is successful ?
        && mBuf[20] == COLOR_DEPTH          // got correct color depth ?
        && mBuf[24] == PIXEL_ORDER          // got correct pixel order ?
        && mBuf[28] != 0                    // got a valid address for frame buffer ?
    )
    {
        /* Convert GPU address to ARM address (clear higher address bits)
         * Frame Buffer is located in RAM memory, which VideoCore MMU
         * maps it to bus address space starting at 0xC0000000.
         * Software accessing RAM directly use physical addresses
         * (based at 0x00000000)
         */
        mBuf[28] &= 0x3FFFFFFF;
        // Access frame buffer as 1 byte per each address
        fb = (unsigned char *)((unsigned long)mBuf[28]);
        uart_puts("Got allocated Frame Buffer at RAM physical address: ");
        uart_hex(mBuf[28]);
        uart_puts("\n");
        uart_puts("Frame Buffer Size (bytes): ");
        uart_dec(mBuf[29]);
        uart_puts("\n");
        width = mBuf[5];  // Actual physical width
        height = mBuf[6]; // Actual physical height
        pitch = mBuf[33]; // Number of bytes per line
    }
    else
    {
        uart_puts("Unable to get a frame buffer with provided setting\n");
    }
}

void drawPixelARGB32(int x, int y, unsigned int attr)
{
    int offs = (y * pitch) + (COLOR_DEPTH / 8 * x);
    /* //Access and assign each byte
     *(fb + offs ) = (attr >> 0 ) & 0xFF; //BLUE
     *(fb + offs + 1) = (attr >> 8 ) & 0xFF; //GREEN
     *(fb + offs + 2) = (attr >> 16) & 0xFF; //RED
     *(fb + offs + 3) = (attr >> 24) & 0xFF; //ALPHA
     */
    // Access 32-bit together
    *((unsigned int *)(fb + offs)) = attr;
}

void drawRectARGB32(int x1, int y1, int x2, int y2, unsigned int attr, int fill)
{
    x1 = min(SCREEN_WIDTH, max(0, x1));
    y1 = min(SCREEN_HEIGHT, max(0, y1));
    x2 = max(0, min(SCREEN_WIDTH, x2));
    y2 = max(0, min(SCREEN_HEIGHT, y2));
    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            if (fill)
                drawPixelARGB32(x, y, attr);
            else if ((x == x1 || x == x2) || (y == y1 || y == y2))
                drawPixelARGB32(x, y, attr);
        }
    }
}

void display_image(int x, int y, int image_width, int image_height, const unsigned int *image_data, int transparency, int rotation) {
    // Calculate the rotation in radians
    double rotation_rad = -rotation * DEG_TO_RAD;

    // Calculate the maximum rotated dimensions
    double cos_rotation = cos(rotation_rad);
    double sin_rotation = sin(rotation_rad);

    // Calculate the center coordinates of the image
    int center_x = x + image_width / 2;
    int center_y = y + image_height / 2;

    // Calculate the maximum bounds of the rotated image
    int half_bbox_w = ceil((fabs(image_width * cos_rotation) + fabs(image_height * sin_rotation)) / 2.0);
    int half_bbox_h = ceil((fabs(image_width * sin_rotation) + fabs(image_height * cos_rotation)) / 2.0);

    // Adjust the start and end points of the loops
    int start_x = min(SCREEN_WIDTH, max(0, center_x - half_bbox_w));
    int start_y = min(SCREEN_HEIGHT, max(0, center_y - half_bbox_h));
    int end_x = max(0, min(SCREEN_WIDTH, center_x + half_bbox_w));
    int end_y = max(0, min(SCREEN_HEIGHT, center_y + half_bbox_h));

    for (int j = start_y; j < end_y; j++) {
        for (int i = start_x; i < end_x; i++) {
            // Translate the pixel coordinates relative to the center
            int translated_i = i - center_x;
            int translated_j = j - center_y;

            // Apply rotation to the translated coordinates
            int rotated_i = round(translated_i * cos_rotation - translated_j * sin_rotation + center_x);
            int rotated_j = round(translated_i * sin_rotation + translated_j * cos_rotation + center_y);

            // Check if the rotated coordinates are within the image bounds
            if (rotated_i >= x && rotated_i < x + image_width && rotated_j >= y && rotated_j < y + image_height) {
                unsigned int color = image_data[(rotated_j - y) * image_width + (rotated_i - x)];
                if (!transparency || color)
                    drawPixelARGB32(i, j, color);
            }
        }
    }
}

void display_video(int x, int y)
{
    int delay_ms = (1.0 / VIDEO_FPS) * 1000;
    
    for (int i = 0; i < VIDEO_FRAMES; i++) {
        set_wait_timer(1, delay_ms);
        if (uart_isReadByteReady()) return;
        display_image(x, y, VIDEO_WIDTH, VIDEO_HEIGHT, video_data[i], 0, 0);
        set_wait_timer(0, delay_ms);
    }
}

void draw_char(int x, int y, unsigned char ch, unsigned int attr, float scale)
{
    if (ch < UNICODE_FIRST || ch > UNICODE_LAST) return;

    int char_index = ch - UNICODE_FIRST;
    int glyph_index = glyph_desc[char_index].glyph_index;
    int glyph_width = glyph_desc[char_index].width_px;

    for (int row = 0; row < HEIGHT_PX; row++) {
        for (int col = 0; col < glyph_width; col++) {
            int pixel_x = (int)(x + col * scale);
            int pixel_y = (int)(y + row * scale);
            int bitmap_byte = glyph_index + row * ((glyph_width + 7) / 8) + col / 8;

            if (bitmap[bitmap_byte] & (1 << (7 - col % 8))) {
                drawRectARGB32(pixel_x, pixel_y, pixel_x + (int)scale, pixel_y + (int)scale, attr, 1);
            }
        }
    }
}

void draw_string(int x, int y, char *s, unsigned int attr, int spacing, float scale)
{
    if (s == 0) return;

    int curr_x = x;
    
    while (*s != '\0') {
        draw_char(curr_x, y, *s, attr, scale);
        curr_x += (glyph_desc[*s - UNICODE_FIRST].width_px + spacing) * scale;
        s++;
    }
}

void clear_screen()
{
    drawRectARGB32(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1);
}

void* memset(void* dest, int value, int count)
{
    char* ptr = (char*)dest;
    for (int i = 0; i < count; i++) {
        *ptr++ = (char)value;
    }
    return dest;
}

void* memcpy(void* dest, const void* src, int n)
{
    char* dest_ptr = (char*)dest;
    const char* src_ptr = (const char*)src;
    for (int i = 0; i < n; i++) {
        *dest_ptr++ = *src_ptr++;
    }
    return dest;
}
