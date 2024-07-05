#include "game.h"
#include "uart1.h"
#include "framebf.h"
#include "image.h"
#include "math.h"
#include "kernel.h"

static struct Object player;
static struct Object bullets[MAX_BULLETS];
static struct Enemy enemies[MAX_ENEMIES];

static enum state state = new_game;
static unsigned int enemy_count = 0;
static unsigned int bullet_count = 0;
static unsigned int game_round = 1;
static unsigned int game_score = 0;
static unsigned int player_hits = 0;

static unsigned int iframes_set = 0;
static unsigned int iframes_cycles = 0;

static unsigned int shoot_delay_set = 0;
static unsigned int shoot_delay_cycles = 0;

static unsigned int rand_seed = 123; // Seed for spawning enemies

void game()
{
    char c = '\0';

    init_enemy_sprites();

    while (1) {
        switch (state)
        {
        case new_game:
        {
            game_round = 1;
            game_score = 0;
            enemy_count = 0;
            bullet_count = 0;
            player_hits = 0;

            set_screen_color(BACKGROUND_COLOR);
            draw_string(SCREEN_WIDTH / 2 - 73, SCREEN_HEIGHT / 2 - 110, "DOOMSDAY", TEXT_COLOR, 2, 0.7);
            draw_string(SCREEN_WIDTH / 2 - 79, SCREEN_HEIGHT / 2 - 55, "Press enter to play", TEXT_COLOR, 2, 0.5);
            draw_string(SCREEN_WIDTH / 2 - 39, SCREEN_HEIGHT / 2 + 40, "Controls:", TEXT_COLOR, 2, 0.5);
            draw_string(SCREEN_WIDTH / 2 - 103, SCREEN_HEIGHT / 2 + 65, "Use W, A, S, D to move", TEXT_COLOR, 2, 0.5);
            draw_string(SCREEN_WIDTH / 2 - 103, SCREEN_HEIGHT / 2 + 95, "Use Q and E to rotate", TEXT_COLOR, 2, 0.5);
            draw_string(SCREEN_WIDTH / 2 - 103, SCREEN_HEIGHT / 2 + 125, "Use Space to shoot", TEXT_COLOR, 2, 0.5);
            drawRectARGB32(SCREEN_WIDTH / 2 - 115, SCREEN_HEIGHT / 2 + 34, SCREEN_WIDTH / 2 + 115, SCREEN_HEIGHT / 2 + 158, TEXT_COLOR, 0);

            c = uart_getc();
            if (c == '\n')       // check if the user pressed enter
                state = next_lv; // change the state to play

            break;
        }

        case next_lv: 
        {
            state = play; // change the state to play
            
            // Initialize level
            player.x = SCREEN_WIDTH / 2 - player_sprite.width;
            player.y = SCREEN_HEIGHT / 2 - player_sprite.height;
            player.w = player_sprite.width;
            player.h = player_sprite.height;
            player.bbox_w = player_sprite.width;
            player.bbox_h = player_sprite.height;
            player.angle = 0;
            bullet_count = 0;
            shoot_delay_set = 0;
            shoot_delay_cycles = 0;
            iframes_set = 0;
            iframes_cycles = 0;

            set_screen_color(BACKGROUND_COLOR);

            // Place player in the center of the screen
            display_image(player.x, player.y, player.bbox_w, player.bbox_h, player_sprite.data, 0, player.angle);

            // Update UI elements
            update_health();
            update_round();
            update_score();

            // Spawn enemies
            spawn_enemies(game_round);

            break;
        }

        case play:
        {
            // Start the polling timer
            set_wait_timer(1, POLLING_TIME_MS);

            c = get_uart(); // read the user input

            switch (c) // perform different actions based on the input
            {
            case 'w': // move the player up
                move_player(0, -PLAYER_SPEED);
                break;

            case 's': // move the player down
                move_player(0, PLAYER_SPEED);
                break;

            case 'a': // move the player left
                move_player(-PLAYER_SPEED, 0);
                break;

            case 'd': // move the player right
                move_player(PLAYER_SPEED, 0);
                break;

            case 'q': // rotate player 45 degrees counter-clockwise
                rotate_player(-PLAYER_ROTATION_STEP);
                break;

            case 'e': // rotate player 45 degrees clockwise
                rotate_player(PLAYER_ROTATION_STEP);
                break;

            case ' ': // shoot a bullet
                shoot_bullet();
                break;

            case 'p':         // quit the game
                state = quit; // change the state to quit
                break;

            default: // Do nothing for any other input
                break;
            }

            // Update bullets on screen
            update_bullets();

            // Update enemies on screen
            update_enemies();

            // Check if round is over
            if (enemy_count == 0) {
                if (game_round < MAX_ROUNDS) {
                    game_round++;
                    state = next_lv;
                } else {
                    state = win;
                }
            }

            // Wait for the polling timer to expire
            set_wait_timer(0, POLLING_TIME_MS);

            break;
        }

        case quit:
        {
            set_screen_color(BACKGROUND_COLOR);
            draw_string(SCREEN_WIDTH / 2 - 69, SCREEN_HEIGHT / 2 - 110, "Game Over", TEXT_COLOR, 2, 0.7);
            draw_string(SCREEN_WIDTH / 2 - 79, SCREEN_HEIGHT / 2 - 55, "Press enter to exit", TEXT_COLOR, 2, 0.5);
            c = uart_getc();
            if (c == '\n') {
                view = '0';         // change to main menu
                state = new_game;   // reset game
                return;             // exit the function and return to the main menu
            }
            break;
        }

        case win:
        {
            set_screen_color(BACKGROUND_COLOR);
            draw_string(SCREEN_WIDTH / 2 - 55, SCREEN_HEIGHT / 2 - 110, "You Win", TEXT_COLOR, 2, 0.7);
            draw_string(SCREEN_WIDTH / 2 - 79, SCREEN_HEIGHT / 2 - 55, "Press enter to exit", TEXT_COLOR, 2, 0.5);
            c = uart_getc();
            if (c == '\n') {
                view = '0';         // change to main menu
                state = new_game;   // reset game
                return;             // exit the function and return to the main menu
            }
            break;
        }

        default:
            set_screen_color(BACKGROUND_COLOR);
            break;
        }

        rand();
    }
}

void move_player(int x_move, int y_move) 
{
    int player_move_x = player.x + x_move;
    int player_move_y = player.y + y_move;

    if (player_move_x >= 0 && 
        player_move_x + player.w <= SCREEN_WIDTH && 
        player_move_y >= TOP_PADDING_PX && 
        player_move_y + player.h <= SCREEN_HEIGHT)
    {
        // Calculate the coordinates of the player's bounding box
        int player_bbox_x = player.x - (player.bbox_w - player.w) / 2;
        int player_bbox_y = player.y - (player.bbox_h - player.h) / 2;

        // Draw a rectangle to remove the player from the previous position
        drawRectARGB32(player_bbox_x, player_bbox_y, player_bbox_x + player.bbox_w, player_bbox_y + player.bbox_h, BACKGROUND_COLOR, 1);

        // Update player's position
        player.x = player_move_x;
        player.y = player_move_y;

        // Draw the player in the new position
        display_image(player.x, player.y, player.w, player.h, player_sprite.data, 0, player.angle);
    }
}

void rotate_player(int rotation) 
{   
    // Calculate the coordinates of the player's bounding box
    int player_bbox_x = player.x - (player.bbox_w - player.w) / 2;
    int player_bbox_y = player.y - (player.bbox_h - player.h) / 2;

    // Draw a rectangle to remove the player from the previous position
    drawRectARGB32(player_bbox_x, player_bbox_y, player_bbox_x + player.bbox_w, player_bbox_y + player.bbox_h, BACKGROUND_COLOR, 1);

    // Update player's angle
    // Map angle to 0-360 range
    player.angle += rotation;
    player.angle %= 360;
    if (player.angle < 0) player.angle += 360;

    // Convert the angle to radians
    double rad = player.angle * DEG_TO_RAD;
    double angle_sin = sin(rad);
    double angle_cos = cos(rad);

    // Calculate the dimensions of the bounding box around the player based on the player's current angle
    player.bbox_w = ceil(fabs(player.w * angle_cos) + fabs(player.h * angle_sin));
    player.bbox_h = ceil(fabs(player.w * angle_sin) + fabs(player.h * angle_cos));

    // Display rotated player sprite
    display_image(player.x, player.y, player.w, player.h, player_sprite.data, 0, player.angle);
}

void shoot_bullet()
{
    if (bullet_count < MAX_BULLETS && !shoot_delay_set) {
        int bullet_w = bullet_sprite.width;
        int bullet_h = bullet_sprite.height;

        // Convert the angle to radians
        double rad = player.angle * DEG_TO_RAD;
        double angle_sin = sin(rad);
        double angle_cos = cos(rad);

        // Set the bullet's initial position to the player's center
        bullets[bullet_count].x = player.x + (player.w / 2) - (bullet_w / 2);
        bullets[bullet_count].y = player.y;

        // Set bullet dimensions
        bullets[bullet_count].w = bullet_w;
        bullets[bullet_count].h = bullet_h;

        // Calculate the dimensions of the bullet's bounding box based on the player's current angle
        bullets[bullet_count].bbox_w = ceil(fabs(bullet_w * angle_cos) + fabs(bullet_h * angle_sin));
        bullets[bullet_count].bbox_h = ceil(fabs(bullet_w * angle_sin) + fabs(bullet_h * angle_cos));

        // Set bullet angle
        // Map angle to 0-360 range
        bullets[bullet_count].angle = player.angle - 90;
        bullets[bullet_count].angle %= 360;
        if (bullets[bullet_count].angle < 0) bullets[bullet_count].angle += 360;

        // Increment the bullet counter to fire the bullet
        bullet_count++;

        // Begin shoot delay
        shoot_delay_set = 1;
    }
}

void update_bullets()
{
    // Handle shoot delay
    if (shoot_delay_set && ++shoot_delay_cycles >= SHOOT_DELAY) {
        shoot_delay_set = 0;
        shoot_delay_cycles = 0;
    }

    // Update bullet positions
    for (int i = 0; i < bullet_count; i++) {
        struct Object* bullet = &bullets[i];

        // Calculate bullet bounding box coordinates
        int bullet_bbox_x = bullet->x - (bullet->bbox_w - bullet->w) / 2;
        int bullet_bbox_y = bullet->y - (bullet->bbox_h - bullet->h) / 2;

        // Remove bullet from the previous position
        int bullet_start_x = player.x + (player.w / 2) - (bullet->w / 2);
        if (bullet->y != player.y || bullet->x != bullet_start_x)
            drawRectARGB32(bullet_bbox_x, bullet_bbox_y, bullet_bbox_x + bullet->bbox_w, bullet_bbox_y + bullet->bbox_h, BACKGROUND_COLOR, 1);

        // Calculate the movement in x and y directions based on the angle and speed
        int move_x = round(cos(bullet->angle * DEG_TO_RAD) * BULLET_SPEED);
        int move_y = round(sin(bullet->angle * DEG_TO_RAD) * BULLET_SPEED);

        // Update bullet position
        bullet->x += move_x;
        bullet->y += move_y;

        // Update bullet bounding box coordinates
        bullet_bbox_x += move_x;
        bullet_bbox_y += move_y;

        // Ensure that the bullet is within screen bounds
        if (bullet_bbox_x >= 0 && 
            bullet_bbox_x + bullet->bbox_w <= SCREEN_WIDTH && 
            bullet_bbox_y >= TOP_PADDING_PX && 
            bullet_bbox_y + bullet->bbox_h <= SCREEN_HEIGHT) {

            // Calculate bullet hit box coordinates
            int min_dimension = min(bullet->w, bullet->h) / 2;
            int bullet_hbox_x = (bullet->x + bullet->w / 2) - min_dimension;
            int bullet_hbox_y = (bullet->y + bullet->h / 2) - min_dimension;

            // Detect enemy hit
            int hit = 0;
            for (int j = 0; j < enemy_count; j++) {
                struct Enemy* enemy = &enemies[j];

                // Calculate the collision detection area for the enemy
                if (bullet_hbox_x >= enemy->x - enemy->w && 
                    bullet_hbox_x <= enemy->x + enemy->w &&
                    bullet_hbox_y >= enemy->y - enemy->h && 
                    bullet_hbox_y <= enemy->y + enemy->h) {

                    // Bullet hit an enemy
                    enemy->hits++;
                    hit = 1;

                    // Check if enemy HP is depleted
                    if (enemy->hits >= ENEMY_HP) {
                        // Remove the enemy from the game
                        drawRectARGB32(enemy->x, enemy->y - 9, enemy->x + enemy->w, enemy->y + enemy->h, BACKGROUND_COLOR, 1);
                        enemy_count--;
                        enemies[j] = enemies[enemy_count];

                        // Increment score and update UI
                        game_score += POINTS_PER_ENEMY;
                        update_score();
                    }

                    // Remove the bullet from active bullets
                    bullet_count--;
                    bullets[i] = bullets[bullet_count];
                    i--;
                    break; // Exit the loop since each bullet can only hit one enemy
                }
            }
            // Do not redraw the bullet if a hit is detected
            if (hit) continue;

            // drawRectARGB32(bullet_bbox_x, bullet_bbox_y, bullet_bbox_x + bullet->bbox_w, bullet_bbox_y + bullet->bbox_h, 0x0000FF00, 1);

            // Draw the bullet in the new position
            display_image(bullet->x, bullet->y, bullet->w, bullet->h, bullet_sprite.data, 1, bullet->angle + 90);

            // drawRectARGB32(bullet_hbox_x, bullet_hbox_y, bullet_hbox_x + bullet->w, bullet_hbox_y + bullet->w, 0x00FF0000, 1);
        } else {
            // Remove the bullet from active bullets if the bullet is out of bounds
            bullet_count--;
            bullets[i] = bullets[bullet_count];
            i--;
        }
    }
}

void update_enemies()
{
    // Calculate player bounding box coordinates
    int player_bbox_x = player.x - (player.bbox_w - player.w) / 2;
    int player_bbox_y = player.y - (player.bbox_h - player.h) / 2;

    // Handle i-frames after the player is hit by an enemy
    if (iframes_set && ++iframes_cycles >= PLAYER_IFRAMES) {
        iframes_cycles = 0;
        iframes_set = 0;
    }

    for (int i = 0; i < enemy_count; i++) {
        struct Enemy* enemy = &enemies[i];

        // Draw a rectangle to remove the enemy from the old position
        drawRectARGB32(enemy->x, enemy->y - 9, enemy->x + enemy->w, enemy->y + enemy->h, BACKGROUND_COLOR, 1);

        // Calculate the direction vector towards the player
        double dx = player.x - enemy->x;
        double dy = player.y - enemy->y;

        // Calculate the angle between the enemy and the player
        double angle = atan2(dy, dx);

        // Calculate the movement in x and y directions based on the angle and speed
        int new_x = enemy->x + ceil(cos(angle) * ENEMY_SPEED);
        int new_y = enemy->y + ceil(sin(angle) * ENEMY_SPEED);

        // Check for collision with obstacles (other enemies)
        for (int j = 0; j < enemy_count; j++) {
            // Skip self
            if (j == i) continue;

            // Calculate the direction vector away from the obstacle
            double obstacle_dx = enemy->x - enemies[j].x;
            double obstacle_dy = enemy->y - enemies[j].y;

            // If the obstacle is within a certain range, adjust the movement direction away from it
            if (pow(obstacle_dx, 2) + pow(obstacle_dy, 2) < pow(ENEMY_MIN_DISTANCE, 2)) {
                double avoid_angle = atan2(obstacle_dy, obstacle_dx);
                new_x += ceil(cos(avoid_angle));
                new_y += ceil(sin(avoid_angle));
            }
        }

        // Ensure that the new position is within the screen bounds
        if (new_x >= 0 && 
            new_x <= SCREEN_WIDTH - enemy->w && 
            new_y >= TOP_PADDING_PX && 
            new_y <= SCREEN_HEIGHT - enemy->h) 
        {
            enemy->x = new_x;
            enemy->y = new_y;
        }

        // Check for player collision
        if (!iframes_set &&
            enemy->x < player_bbox_x + player.bbox_w &&
            enemy->x + enemy->w > player_bbox_x &&
            enemy->y < player_bbox_y + player.bbox_h &&
            enemy->y + enemy->h > player_bbox_y)
        {
            // Enemy collided with player
            player_hits++;

            update_health();
            if (player_hits >= PLAYER_HP) {
                state = quit;
                return;
            }

            // Begin i-frames counter
            iframes_set = 1;
        }

        // Display the updated enemy position
        display_image(enemy->x, enemy->y, enemy->w, enemy->h, enemy_sprites[enemy->index].data, 0, 0);

        // Draw enemy health bar
        int health_bar_x = enemy->x + (int)(enemy->w * ((ENEMY_HP - enemy->hits) / (float)ENEMY_HP));
        drawRectARGB32(enemy->x, enemy->y - 9, health_bar_x, enemy->y - 4, HEALTH_BAR_COLOR, 1);
        drawRectARGB32(enemy->x, enemy->y - 9, enemy->x + enemy->w, enemy->y - 4, TEXT_COLOR, 0);
    }
}

// Update player's health
void update_health()
{
    // Calculate number of hearts to display
    int health = PLAYER_HP - player_hits;

    // Clear all hearts
    for (int i = 0; i < PLAYER_HP; i++) {
        int x = 12 + 40 * i;
        drawRectARGB32(x, 12, x + heart_sprite.width, 12 + heart_sprite.height, BACKGROUND_COLOR, 1);
    }

    // Draw hearts
    for (int i = 0; i < health; i++) {
        display_image(12 + 40 * i, 12, heart_sprite.width, heart_sprite.height, heart_sprite.data, 1, 0);
    }
}

// Update round text
void update_round()
{
    char round_str[10];
    int_to_str(game_round, round_str);

    int start_x = (SCREEN_WIDTH / 2) - 38;
    drawRectARGB32(start_x, 12, start_x + 120, 36, BACKGROUND_COLOR, 1);
    draw_string(start_x, 12, "Round:", TEXT_COLOR, 2, 0.5);
    draw_string(start_x + 71, 12, round_str, TEXT_COLOR, 2, 0.5);
}

// Update score text
void update_score()
{
    char score_str[10];
    int_to_str(game_score, score_str);

    int start_x = (SCREEN_WIDTH / 2) + 390;
    drawRectARGB32(start_x, 12, start_x + 120, 36, BACKGROUND_COLOR, 1);
    draw_string(start_x, 12, "Score:", TEXT_COLOR, 2, 0.5);
    draw_string(start_x + 66, 12, score_str, TEXT_COLOR, 2, 0.5);
}

void set_screen_color(unsigned int color)
{
    drawRectARGB32(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color, 1);
}

void spawn_enemies(int n)
{
    int max_enemies = min(MAX_ENEMIES, n);
    int spawned_enemies = 0;

    while (spawned_enemies < max_enemies) {
        int x, y;
        int rand_enemy_index = rand_int(0, MAX_ENEMY_SPRITES - 1);
        int enemy_w = enemy_sprites[rand_enemy_index].width;
        int enemy_h = enemy_sprites[rand_enemy_index].height;

        // Generate random positions for the enemy along the edges of the screen
        int side = rand_int(0, 3);
        switch (side)
        {
        case 0: // Spawn enemy on the top edge
            x = rand_int(0, SCREEN_WIDTH - enemy_w);
            y = TOP_PADDING_PX;
            break;

        case 1: // Spawn enemy on the right edge
            x = SCREEN_WIDTH - enemy_w;
            y = rand_int(TOP_PADDING_PX, SCREEN_HEIGHT - enemy_h);
            break;

        case 2: // Spawn enemy on the bottom edge
            x = rand_int(0, SCREEN_WIDTH - enemy_w);
            y = SCREEN_HEIGHT - enemy_h;
            break;
        
        default: // Spawn enemy on the left edge
            x = 0;
            y = rand_int(TOP_PADDING_PX, SCREEN_HEIGHT - enemy_h);
            break;
        }

        // Check if the new enemy is too close to any existing enemy
        int too_close = 0;
        for (int i = 0; i < spawned_enemies; i++) {
            int dist_x = abs(x - enemies[i].x);
            int dist_y = abs(y - enemies[i].y);
            int min_dist_x = (enemy_w + ENEMY_MIN_DISTANCE) / 2;
            int min_dist_y = (enemy_h + ENEMY_MIN_DISTANCE) / 2;

            if (dist_x < min_dist_x && dist_y < min_dist_y) {
                too_close = 1;
                break;
            }
        }

        if (too_close) continue;

        // Spawn the enemy
        enemies[spawned_enemies].x = x;
        enemies[spawned_enemies].y = y;
        enemies[spawned_enemies].w = enemy_w;
        enemies[spawned_enemies].h = enemy_h;
        enemies[spawned_enemies].hits = 0;
        enemies[spawned_enemies].index = rand_enemy_index;
        spawned_enemies++;
    }
    enemy_count = max_enemies;
}

void int_to_str(int num, char *str)
{
    int i = 0;
    int is_negative = 0;

    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    do {
        str[i++] = num % 10 + '0';
        num /= 10;
    } while (num);

    if (is_negative)
        str[i++] = '-';

    for (int j = 0; j < i / 2; j++) {
        char tmp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = tmp;
    }

    str[i] = '\0';
}

unsigned int rand()
{
    rand_seed = (1664525 * rand_seed + 1013904223) % 4294967296;
    return rand_seed;
}

int rand_int(int min, int max)
{
    return (rand() % (max - min + 1)) + min;
}
