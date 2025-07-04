#include <stdio.h>
#include "init.h"
#include "map.h"

chtype map[MAP_HEIGHT][MAP_WIDTH];
int main() {
	init_UI();
    initscr();
    start_color();
    curs_set(0);

    // 색상 등록
    init_pair(1, 245, 245);  // 회색 글자, 회색 배경 : 빈칸
    init_pair(2, 15, 0);     // 흰색 글자(15번), 검은 배경(0번) : 벽
    init_pair(3, 21, 245);   // 파란색 글자(21번), 회색 배경 : 플레이어
    init_pair(4, 196, 245);  // 빨간색 글자, 회색 배경 : 좀비
    init_pair(5, 226, 245);  // 노란색 글자, 회색 배경 : 아이템

	//시작하고 출력
    init_map(map);
    draw_map(map);

    refresh();
    getch();
    endwin();
    return 0;
}
