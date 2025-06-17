#ifndef MAP_H
#define MAP_H

#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <time.h>

#define MAP_HEIGHT 50
#define MAP_WIDTH 100
#define HOUSE_COUNT 7
#define ZOMBIE_MAX 20
#define ZOMBIE_MIN 15
#define ITEM_COUNT 3
//chtype map[MAP_HEIGHT][MAP_WIDTH];
enum Object {EMPTY = '.',WALL = '#', ZOMBIE = 'z', PLAYER = '8', ITEM = '!'};
void init_map(chtype map[MAP_HEIGHT][MAP_WIDTH]) {
    for (int i=0;i<MAP_HEIGHT;++i) {
        for (int j=0;j<MAP_WIDTH;++j) {
            map[i][j] = '.' | COLOR_PAIR(1); //빈칸
        }
    }
    for (int i=0;i<MAP_WIDTH;++i) map[0][i] = WALL | COLOR_PAIR(2); //벽
    for (int i=0;i<MAP_HEIGHT;++i) map[i][0] = WALL | COLOR_PAIR(2);
    for (int i=0;i<MAP_WIDTH;++i) map[MAP_HEIGHT-1][i] = WALL | COLOR_PAIR(2);
    for (int i=0;i<MAP_HEIGHT;++i) map[i][MAP_WIDTH-1] = WALL | COLOR_PAIR(2);
	//건물 4개 생성 (직사각형)
	srand(time(NULL));
	int w,h,ry,rx;
	h = 6; w = 12; //중앙 건물
	ry = MAP_HEIGHT/2 - h/2;
	rx = MAP_WIDTH/2 - w/2;
	for (int i=ry;i<ry+h;++i) {
		for (int j=rx;j<rx+w;++j) {
			if (i==ry || i==ry+h-1 || j==rx || j==rx+w-1) map[i][j] = WALL | COLOR_PAIR(2);
		}
	}
	// 나머지 3개 건물 (랜덤 위치 및 크기)
    for (int b=1;b<HOUSE_COUNT;++b) {
        h = rand() % 5 + 5;        // 높이 4~7
        w = rand() % 12 + 5;        // 너비 6~13
        ry = rand() % (MAP_HEIGHT - h - 2) + 1;
        rx = rand() % (MAP_WIDTH  - w - 2) + 1;
        for (int i=ry;i<ry+h;++i) {
            for (int j=rx;j<rx+w;++j) {
                if (i==ry || i==ry+h-1 || j==rx || j==rx+w-1)
                    map[i][j] = WALL | COLOR_PAIR(2);
            }
        }
    }
	//플레이어 위치
	map[25][50] = '8' | COLOR_PAIR(3) | A_BOLD; //플레이어
    
	//좀비 랜덤 생성 : ZOMBIE_MIN ~ ZOMBIE_MAX
	int zombieCount = rand() % (ZOMBIE_MAX - ZOMBIE_MIN + 1) + ZOMBIE_MIN;
	for (int z=0;z<zombieCount;++z) {
        int y,x;
        do {
            y = rand() % (MAP_HEIGHT - 2) + 1; //벽은 제외
            x = rand() % (MAP_WIDTH  - 2) + 1;
        } while (map[y][x] != (EMPTY | COLOR_PAIR(1))); //빈 공간이어야함.
        map[y][x] = ZOMBIE | COLOR_PAIR(4);
    }
	//아이템 랜덤 생성 : ITEM_COUNT
    for (int it=0;it<ITEM_COUNT;++it) {
        int y,x;
        do {
            y = rand() % (MAP_HEIGHT - 2) + 1;
            x = rand() % (MAP_WIDTH  - 2) + 1;
        } while (map[y][x] != (EMPTY | COLOR_PAIR(1)));
        map[y][x] = ITEM | COLOR_PAIR(5) | A_BLINK;
    }
}
void draw_map(chtype map[MAP_HEIGHT][MAP_WIDTH]) {
    for (int i=0;i<MAP_HEIGHT;++i) {
        for (int j=0;j<MAP_WIDTH;++j) {
            mvaddch(i,j,map[i][j]);
        }
    }
}/*
int main() {
    initscr();
    start_color();
    curs_set(0);

    // 색상 등록
    init_pair(1, 245, 245);  // 회색 글자, 회색 배경 : 빈칸
    init_pair(2, 15, 0);     // 흰색 글자(15번), 검은 배경(0번) : 벽
    init_pair(3, 21, 245);   // 파란색 글자(21번), 회색 배경 : 플레이어
    init_pair(4, 196, 245);  // 빨간색 글자, 회색 배경 : 좀비
    init_pair(5, 226, 245);  // 노란색 글자, 회색 배경 : 아이템

    init_map(map);
    draw_map(map);

    refresh();
    getch();
    endwin();
    return 0;
}*/
#endif
