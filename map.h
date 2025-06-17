#ifndef MAP_H
#define MAP_H

#include <stdio.h>
#include <string.h>
#include <ncurses.h>

#define MAP_HEIGHT 50
#define MAP_WIDTH 100
//chtype map[MAP_HEIGHT][MAP_WIDTH];
void init_map(chtype map[MAP_HEIGHT][MAP_WIDTH]) {
    for (int i=0;i<MAP_HEIGHT;++i) {
        for (int j=0;j<MAP_WIDTH;++j) {
            map[i][j] = '.' | COLOR_PAIR(1); //빈칸
        }
    }
    for (int i=0;i<MAP_WIDTH;++i) map[0][i] = '#' | COLOR_PAIR(2); //벽
    for (int i=0;i<MAP_HEIGHT;++i) map[i][0] = '#' | COLOR_PAIR(2);
    for (int i=0;i<MAP_WIDTH;++i) map[MAP_HEIGHT-1][i] = '#' | COLOR_PAIR(2);
    for (int i=0;i<MAP_HEIGHT;++i) map[i][MAP_WIDTH-1] = '#' | COLOR_PAIR(2);
    map[10][10] = '8' | COLOR_PAIR(3) | A_BOLD; //플레이어
    map[48][30] = 'z' | COLOR_PAIR(4); //좀비
    map[30][30] = '!' | COLOR_PAIR(5) | A_BLINK; //아이템

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
