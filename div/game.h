#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

#define MAP_HEIGHT 150 //맵 크기
#define MAP_WIDTH 300
#define HOUSE_COUNT 12 //집 개수
#define ZOMBIE_MAX 200 //좀비 개수 최소~최대
#define ZOMBIE_MIN 100
#define ITEM_COUNT 20 //아이템 개수
#define VACCINE_COUNT 5 //각각 DEBUG 글자로 할 예정
#define TREE_COUNT 30 //트리 개수 30개
#define VIEW_HEIGHT 21 //플레이어 시야 크기
#define VIEW_WIDTH 51
#define ROCK_COUNT 40 // 바위 개수
#define WATER_COUNT 30 // 물웅덩이 개수

#define NEAR_VIEW_SIZE 5
#define NEAR_VIEW_ADJ 5

#define UP_VIEW_START_ANGLE 10.0
#define UP_VIEW_END_ANGLE 170.0
#define LEFT_VIEW_START_ANGLE 100.0
#define LEFT_VIEW_END_ANGLE 260.0
#define DOWN_VIEW_START_ANGLE 190.0
#define DOWN_VIEW_END_ANGLE 350.0
#define RIGHT_VIEW_START_ANGLE 280.0
#define RIGHT_VIEW_END_ANGLE 80.0

#define VIEW_SHIFT_VEL 0.5

typedef enum {PLAYER = '8', ZOMBIE = 'z', ITEM = '!', WALL = '#', GROUND = '.', TREE = 'T', FURNITURE = 'F'} type;
typedef enum {UP = 119, RIGHT = 100, DOWN = 115, LEFT = 97} dir;

//좌표
typedef struct {
    int y;
    int x;
} yx;

//플레이어
typedef struct {
    type role;
    yx point;
    int hp;
    dir lookDir; //시야 방향
} human;

//직사각형 건물
typedef struct {
    int y1, x1;
    int y2, x2;
} Rect;

//좀비 구조체
typedef struct {
    yx point;
    int alive;
    int hp;
} zombie_t;

extern chtype map[MAP_HEIGHT][MAP_WIDTH];
extern human player;
extern zombie_t zombies[ZOMBIE_MAX];
extern int zombie_count;

extern double view_start_angle;
extern double view_end_angle;

#endif // GAME_H 