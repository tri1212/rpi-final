#include "framebf.h"

// #define ENABLE_VIDEO

#define VIDEO_WIDTH 640
#define VIDEO_HEIGHT 368
#define VIDEO_FRAMES 50
#define VIDEO_FPS 12

#define MAX_IMAGES 3
#define MAX_ENEMY_SPRITES 5

struct Image {
    int width;
    int height;
    const unsigned int* data;
};

extern struct Image images[];
extern struct Image enemy_sprites[];
extern const unsigned int video_data[VIDEO_FRAMES][VIDEO_WIDTH * VIDEO_HEIGHT];

extern const struct Image player_sprite;
extern const struct Image bullet_sprite;
extern const struct Image heart_sprite;

void init_images();
void init_enemy_sprites();
