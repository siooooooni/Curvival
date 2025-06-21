#ifndef MAP_H
#define MAP_H

#include "game.h"

void init_map(chtype map[MAP_HEIGHT][MAP_WIDTH]); //맵 초기화
void make_vaccine(chtype map[MAP_HEIGHT][MAP_WIDTH], char a); //백신 생성
void draw_building(chtype map[MAP_HEIGHT][MAP_WIDTH], Rect b, int color_pair); //건물 그리기
void make_tree(chtype map[MAP_HEIGHT][MAP_WIDTH], int y, int x);
void make_obstacle(chtype map[MAP_HEIGHT][MAP_WIDTH]); // 장애물 생성

#endif // MAP_H 