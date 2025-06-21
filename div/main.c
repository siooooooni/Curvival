#include "game.h"
#include "ui.h"
#include "map.h"
#include "player.h"
#include "view.h"
#include "zombie.h"

int main() {
    system("clear");
    init_UI();
    initscr();
	noecho();
    nodelay(stdscr, TRUE);
    start_color();
    curs_set(0);

    // 색상 등록
    init_pair(1, 245, 245);  // 회색 글자, 회색 배경 : 빈칸
    init_pair(2, 15, 235);     // 흰색 글자(15번), 검은 배경(0번) : 벽
    init_pair(3, 21, 245);   // 파란색 글자(21번), 회색 배경 : 플레이어
    init_pair(4, 196, 245);  // 빨간색 글자, 회색 배경 : 좀비
    init_pair(5, 226, 245);  // 노란색 글자, 회색 배경 : 아이템
    init_pair(6, 46, 46);    // 초록색 글자, 초록색 배경 : 백신
    init_pair(7, 240, 240);  // 진한 회색 글자, 진한 회색 배경 : 시야 밖
	init_pair(8, 236, 236); // 바위(진한 회색)
    init_pair(9, 39, 39);   // 물웅덩이(파란색)
    init_pair(10, 130, 245); // 가구(갈색)
    
	init_pair(100, 243, 243);  // 회색 글자, 회색 배경 : 빈칸
    init_pair(200, 243, 232);     // 흰색 글자(15번), 검은 배경(0번) : 벽

	//시작하고 출력
    init_map(map);
    struct timespec last_zombie_move, now; //초+나노초로 시간 저장하는 구조체
    clock_gettime(CLOCK_MONOTONIC, &last_zombie_move); //현재 시간 구조체에 저장
    while(1){
        draw_view_map(map, player);
        refresh();
        int key = getch(); //키보드 입력 받기
        if(key != -1){ //키보드 입력이 있다면
            map[player.point.y][player.point.x] = GROUND | COLOR_PAIR(1);
            HumanMove(&player, key);
            mvprintw(55, 0, "y:%d, x:%d", player.point.y, player.point.x);
            map[player.point.y][player.point.x] =  PLAYER | COLOR_PAIR(3) | A_BOLD;
        }

        //좀비 움직이는 함수
        clock_gettime(CLOCK_MONOTONIC, &now);
        double elapsed = (now.tv_sec - last_zombie_move.tv_sec) + (now.tv_nsec - last_zombie_move.tv_nsec) / 1e9;
        if (elapsed > 0.3) { //0.3초마다 좀비 움직이는 함수 실행
            move_zombies(map, player.point);
            last_zombie_move = now; //현재 시간 구조체에 저장
        }
    }
    endwin();
    return 0;
} 