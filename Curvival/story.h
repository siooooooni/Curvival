#ifndef STORY
#define STORY
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <math.h>

#define CENTER_X 40
#define CENTER_Y 12
#define RADIUS 10
#define ARMS 6

void draw_spiral_feathers() {
    initscr();
    start_color();
    curs_set(0);
    noecho();
    use_default_colors();

    for (int i=1;i<=ARMS;++i) {
        init_pair(i, i % 7 + 1, -1); // 다양한 색상
    }

    for (int angle=0;angle<360;angle+=5) {
        clear();
        for (int a=0;a<ARMS;++a) {
            double theta = (angle + a*(360/ARMS)) * M_PI / 180.0;
            for (int r=1;r<RADIUS;++r) {
                int x = CENTER_X + r * cos(theta + 0.2 * sin(r / 2.0));
                int y = CENTER_Y + r * sin(theta + 0.2 * sin(r / 2.0));
                attron(COLOR_PAIR((a%6) + 1));
                mvaddch(y, x, '*');
                attroff(COLOR_PAIR((a%6) + 1));
            }
        }
        refresh();
        usleep(70000); // 약 0.07초 프레임 딜레이
    }
    endwin();
}
void story_waiting() {
	char pass;
    scanf("%c",&pass);
	while (1) { if (pass=='\n') break;}
	system("clear");
}
void init_story() {
	system("clear");
	printf("프롤로그 – 나무 인형이 도착하던 날\n");
    sleep(1);
	printf("그날, 나는 단지 메일 하나를 열었을 뿐이었다.\n");
    sleep(1);
    printf("제목은 별것 아니었다.\n");
    sleep(1);
    printf("“무료 성능 향상 패치! 지금 다운로드하세요.”\n");
    sleep(1);
    printf("첨부파일은 TrojanHorse.exe\n");
    sleep(1);
    printf("귀엽게 생긴 나무 인형 모양의 프로그램 아이콘이었다.\n");
    sleep(1);
    printf("의심은 들었지만, 클릭했다. 그 순간부터...\n");
    sleep(1);
	printf("Enter로 계속...\n");
	story_waiting();
    
	printf("내 컴퓨터의 세상은 뒤틀리기 시작했다.\n");
    sleep(1);
    printf("화면이 깜빡이고, 익숙하던 프로그램들이 오작동하더니,\n");
	printf("       _______________________\n");
	printf("      |                       |\n");
	printf("      |    [ SYSTEM ERROR ]   |\n");
	printf("      |_______________________|\n");
	printf("      |  ___ ___ ___ ___ ___  |\n");
	printf("      | |___|___|___|___|___| |\n");
	printf("      |_______________________|\n");
	printf("         ||             ||\n");
	printf("         ||             ||\n");
	printf("         []             []\n");
	sleep(1);
    printf("내 바탕화면 안에서… 비명이 들리기 시작했다.\n");	
    printf("Enter로 계속...\n");
	story_waiting();

    printf("나는 안다.\n");
    sleep(1);
    printf("지금 이 컴퓨터가 완전히 감염되면,\n");
    sleep(1);
    printf("내 업무, 내 기억, 나의 삶이 전부 날아간다.\n");
    sleep(1);
    printf("그래서 나는 직접 컴퓨터 내부로 접속하기로 했다.\n");
    sleep(1);
	printf("Enter로 계속...\n");
	story_waiting();

	draw_spiral_feathers();
	sleep(1);
    printf("디버깅 툴을 응급 패치해 나를 의식으로 전송한다.\n");
    sleep(1);
    printf("이제부터 나는 백신 프로그램 “DEBUG”로서 이 디지털 세상에 들어가,\n");
    sleep(1);
    printf("시스템을 수리하고, 감염된 좀비 코드를 정화해야 한다.\n");
    sleep(1);
    printf("\n[Enter] 키를 눌러 디지털 세계에 진입하십시오...\n");
	story_waiting();
}
#endif
