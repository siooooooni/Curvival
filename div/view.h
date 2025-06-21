#ifndef VIEW_H
#define VIEW_H

#include "game.h"

void draw_view_map(chtype map[MAP_HEIGHT][MAP_WIDTH], human player); //플레이어 시야 그리기
void line_view(double angle, double distance, yx startPoint, yx endPoint);
void player_view(yx startPoint, yx endPoint);

#endif // VIEW_H 