#define POLLING_TIME_MS 40      // Polling time per game loop cycle (40ms -> 25 times/sec)

#define MAX_BULLETS 3           // Maximum number of bullets that can appear on the screen
#define BULLET_SPEED 30         // Bullet travel speed
#define SHOOT_DELAY 8           // Number of frames to wait between each shot (8 * 40ms = 320ms)

#define PLAYER_SPEED 10         // Player movement speed
#define PLAYER_ROTATION_STEP 45 // Player rotation amount (in degrees)
#define PLAYER_HP 3             // Player hit points
#define PLAYER_IFRAMES 25       // Number of invulnerability frames after getting hit (25 * 40ms = 1sec)

#define MAX_ENEMIES 10          // Maximum number of enemies
#define ENEMY_SPEED 3           // Enemy movement speed
#define ENEMY_HP 5              // Maximum hit points for each enemy
#define ENEMY_MIN_DISTANCE 40   // Minimum distance between enemies
#define POINTS_PER_ENEMY 50     // Points per enemy killed

#define MAX_ROUNDS 10           // Maximum number of rounds per game

#define TOP_PADDING_PX 60       // Padding at the top of the screen for the game UI

#define TEXT_COLOR 0x00FCFCFC
#define BACKGROUND_COLOR 0x00AB9278
#define HEALTH_BAR_COLOR 0x00FF0000

void game();
void move_player(int x_move, int y_move);
void rotate_player(int rotation);
void shoot_bullet();
void update_bullets();
void update_enemies();
void update_health();
void update_round();
void update_score();
void spawn_enemies(int n);
void set_screen_color(unsigned int color);

void int_to_str(int num, char *str);
unsigned int rand();
int rand_int(int min, int max);

struct Object {
    int x;
    int y;
    int w;
    int h;
    int bbox_w;    // bounding box width
    int bbox_h;    // bounding box height
    int angle;
};

struct Enemy {
    int x;
    int y;
    int w;
    int h;
    int hits;
    int index;      // enemy sprite index
};

enum state {
    new_game,
    play,
    next_lv,
    quit,
    win
};
