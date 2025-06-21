#include "game.h"

chtype map[MAP_HEIGHT][MAP_WIDTH];
human player = {.role = PLAYER, .point = {10, 10}, .hp = 100, .lookDir = RIGHT};
zombie_t zombies[ZOMBIE_MAX];
int zombie_count = 0;

double view_start_angle = RIGHT_VIEW_START_ANGLE;
double view_end_angle = RIGHT_VIEW_END_ANGLE; 