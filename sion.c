#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAP_HEIGHT 150
#define MAP_WIDTH 300
#define HOUSE_COUNT 12
#define ZOMBIE_MAX 20
#define ZOMBIE_MIN 15
#define ITEM_COUNT 3

#define VIEW_HEIGHT 21
#define VIEW_WIDTH 51

#define HOUSE_COUNT 7
#define ZOMBIE_MAX 20
#define ZOMBIE_MIN 15
#define ITEM_COUNT 3

#define VIEW_ANGLE 0.2f

typedef enum {PLAYER = '8', ZOMBIE = 'z', ITEM = '!', WALL = '#', GROUND = '.'} type;
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
	dir lookDir;
} human;       

//직사각형 건물
typedef struct {
    int y1, x1;
    int y2, x2;
} Rect;

chtype map[MAP_HEIGHT][MAP_WIDTH];
human player = {.role = PLAYER, .point = (yx){10, 10}, .hp = 100, .lookDir = RIGHT};
void swap_int(int* a, int* b);
void init_UI();
void init_map(chtype map[MAP_HEIGHT][MAP_WIDTH]);
void draw_map(chtype map[MAP_HEIGHT][MAP_WIDTH]);
void draw_view_map(chtype map[MAP_HEIGHT][MAP_WIDTH], human player);
void draw_building(chtype map[MAP_HEIGHT][MAP_WIDTH], Rect b, int color_pair);
void HumanMove(human *player, int way);

int main() {
	system("clear");
	init_UI();
    initscr();
	nodelay(stdscr, TRUE);
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
	yx nextPoint;    
	while(true){
		//draw_map(map);
		draw_view_map(map, player);
		refresh();

    	int key = getch();

		if(key != -1){
			map[player.point.y][player.point.x] = GROUND | COLOR_PAIR(1);

			HumanMove(&player, key);
			mvprintw(55, 0, "y:%d, x:%d", player.point.y, player.point.x);

			map[player.point.y][player.point.x] =  PLAYER | COLOR_PAIR(3) | A_BOLD;
		}
	}
    endwin();
    return 0;
}
void swap_int(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}
void init_UI() {
    printf("==============================\n");
    printf("\tZOMBIE SURVIVAL\n");
    printf("==============================\n");
    printf("\t1. New Game\n");
    printf("\t2. Load Game (미구현) \n");
    printf("\t3. Options (미구현) \n");
    printf("\t4. Exit\n");
    printf("GAME START? : ");
    int n = 0;
    scanf("%d",&n);
    getchar();
    if (n==4) exit(0);
    else if (n==1) {
        return;
    }
    //else if (n==2) {}
    //else if (n==3) {}
}

void draw_building(chtype map[MAP_HEIGHT][MAP_WIDTH], Rect b, int color_pair) {
    // y1 ≤ y2, x1 ≤ x2 가정 (아니면 swap)
	if (b.y1 > b.y2) swap_int(&b.y1, &b.y2);
    if (b.x1 > b.x2) swap_int(&b.x1, &b.x2);
	// 상하 테두리
    for (int x=b.x1;x<=b.x2;++x) {
        map[b.y1][x] = '#' | COLOR_PAIR(color_pair);  // 상단
        map[b.y2][x] = '#' | COLOR_PAIR(color_pair);  // 하단
    }

    // 좌우 테두리
    for (int y=b.y1;y<=b.y2;++y) {
        map[y][b.x1] = '#' | COLOR_PAIR(color_pair);  // 좌측
        map[y][b.x2] = '#' | COLOR_PAIR(color_pair);  // 우측
    }
	// 가운데에 랜덤으로 구멍(문) 하나 뚫기
    int midx = (b.x1 + b.x2) / 2;
    int midy = (b.y1 + b.y2) / 2;
    int side = rand() % 4;  // 0: 상 1: 하 2: 좌 3: 우
    switch (side) {
        case 0:  // 상단 중앙
            map[b.y1][midx] = GROUND | COLOR_PAIR(1);
            break;
        case 1:  // 하단 중앙
            map[b.y2][midx] = GROUND | COLOR_PAIR(1);
            break;
        case 2:  // 좌측 중앙
            map[midy][b.x1] = GROUND | COLOR_PAIR(1);
            break;
        case 3:  // 우측 중앙
            map[midy][b.x2] = GROUND | COLOR_PAIR(1);
            break;
    }
}

void init_map(chtype map[MAP_HEIGHT][MAP_WIDTH]) {
	for (int i=0;i<MAP_HEIGHT;++i) {                                        
		for (int j=0;j<MAP_WIDTH;++j) {                                         
			map[i][j] = GROUND | COLOR_PAIR(1); //빈칸     
		}  
	}                     
	for (int i=0;i<MAP_WIDTH;++i) map[0][i] = WALL | COLOR_PAIR(2); //벽
    for (int i=0;i<MAP_HEIGHT;++i) map[i][0] = WALL | COLOR_PAIR(2);    
	for (int i=0;i<MAP_WIDTH;++i) map[MAP_HEIGHT-1][i] = WALL | COLOR_PAIR(2);                                                  
	for (int i=0;i<MAP_HEIGHT;++i) map[i][MAP_WIDTH-1] = WALL | COLOR_PAIR(2);

    //건물 생성 (직사각형) : HOUSE_COUNT
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
   	//건물 생성 리스트 (왼쪽 위 사각형, 오른쪽 아래 사각형)
	Rect buildings[HOUSE_COUNT] = {
    	{  5,   5,  10,  15},   // 좌상단
    	{  8,  50,  15,  70},   // 상단 중간
    	{  2, 120,  10, 150},   // 상단 우측
    	{ 20, 180,  30, 210},   // 중상 우측
    	{ 35,  30,  45,  75},   // 중간 왼쪽
    	{ 60,  80,  72, 110},   // 중간
    	{ 90,  10, 105,  45},   // 중하단 왼쪽
    	{110, 140, 125, 170},   // 중하단 우측
    	{130, 200, 145, 240},   // 하단 우측
    	{ 40, 250,  55, 290},   // 중간 우측 극단
    	{ 80, 200,  95, 260},   // 중하단 우측
    	{ 60, 150,  75, 190},   // 중단 우측
	};
	for (int i=0;i<HOUSE_COUNT-1;++i) {
		draw_building(map,buildings[i],2);
	}
	//좀비 랜덤 생성 : ZOMBIE_MIN ~ ZOMBIE_MAX
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
        } while (map[y][x] != (GROUND | COLOR_PAIR(1))); //빈 공간이어야함.
        map[y][x] = ZOMBIE | COLOR_PAIR(4);
    }
    //아이템 랜덤 생성 : ITEM_COUNT
    for (int it=0;it<ITEM_COUNT;++it) {
        int y,x;
        do {
            y = rand() % (MAP_HEIGHT - 2) + 1;
            x = rand() % (MAP_WIDTH  - 2) + 1;
        } while (map[y][x] != (GROUND | COLOR_PAIR(1)));
        map[y][x] = ITEM | COLOR_PAIR(5) | A_BLINK;
    }

	map[10][10] = PLAYER | COLOR_PAIR(3) | A_BOLD; //플레이어
    map[48][30] = ZOMBIE | COLOR_PAIR(4); //좀비
    map[30][30] = ITEM | COLOR_PAIR(5) | A_BLINK; //아이템
}

void draw_map(chtype map[MAP_HEIGHT][MAP_WIDTH]) {
    for (int i=0;i<MAP_HEIGHT;++i) {
        for (int j=0;j<MAP_WIDTH;++j) {
            mvaddch(i,j,map[i][j]);
        }
    }
}

void HumanMove(human *player, int way) {
    if(way == UP){ // 'w' 누르면
        if((map[(player -> point).y - 1][(player -> point).x] & A_CHARTEXT) == GROUND){
            (player -> point).y--;
			player -> lookDir = UP;
        }
    }
    else if(way == RIGHT){ // 'd' 누르면
        if((map[(player -> point).y][(player -> point).x + 1] & A_CHARTEXT) == GROUND){
            (player -> point).x++;
			player -> lookDir = RIGHT;
        }
    }
    else if(way == DOWN){ // 's' 누르면
        if((map[(player -> point).y + 1][(player -> point).x] & A_CHARTEXT) == GROUND){
            (player -> point).y+=1;
			player -> lookDir = DOWN;
        }
    }
    else if(way == LEFT){ // 'a' 누르면
        if((map[(player -> point).y][(player -> point).x - 1] & A_CHARTEXT) == GROUND){
            (player -> point).x-=1;
			player -> lookDir = LEFT;
        }
    }
}

void draw_view_map(chtype map[MAP_HEIGHT][MAP_WIDTH], human player){
	yx startPoint = {player.point.y - ((VIEW_HEIGHT-1) / 2), player.point.x - ((VIEW_WIDTH-1) / 2)}; // 왼쪽 위 좌표
	yx endPoint = {player.point.y + ((VIEW_HEIGHT-1) / 2), player.point.x + ((VIEW_WIDTH-1) / 2)}; // 오른쪽 아래 좌표
	
	// 좌표들이 맵을 벗어날 경우 예외 처리하는 코드
	if(startPoint.y < 0){
		endPoint.y = VIEW_HEIGHT - 1;
		startPoint.y = 0;
	}
	if(startPoint.x < 0){
		endPoint.x = VIEW_WIDTH - 1;
		startPoint.x = 0;
    }
	if(endPoint.y >= MAP_HEIGHT){
		startPoint.y = MAP_HEIGHT - VIEW_HEIGHT;
		endPoint.y = MAP_HEIGHT - 1;
	}
    if(endPoint.x >= MAP_WIDTH){
		startPoint.x = MAP_WIDTH - VIEW_WIDTH;
		endPoint.x = MAP_WIDTH - 1;
	}
	
	// 화면에 출력
	int y = 0, x = 0;
	for (int i = startPoint.y; i <= endPoint.y; ++i) {
        for (int j = startPoint.x; j<= endPoint.x; ++j) {
            mvaddch(y,x,'.');
			++x;
        }
		x = 0;
		++y;
    }
	
	if(player.lookDir == UP) {
		for(float a = VIEW_ANGLE; a < 7; a += 0.1*a) {
			float ty = 0, tx = 0;
			int ry = (int)(player.point.y - ty), rx = (int)(player.point.x - tx);
			while(
				(ry >= startPoint.y) && 
				(rx >= startPoint.x) && 
				(((map[ry][rx] & A_CHARTEXT) == PLAYER) || ((map[ry][rx] & A_CHARTEXT) == GROUND))
			) {
				ry = (int)(player.point.y - ty);
				rx = (int)(player.point.x - tx);
				mvaddch(ry - startPoint.y, rx - startPoint.x, map[ry][rx]);
				
				if(a < 1) tx += 0.2;
				else tx += 1 / (a*2);
				ty = a * tx;
			}
		}

		for(float a = VIEW_ANGLE; a < 7; a += 0.1*a) {
            float ty = 0, tx = 0;
            int ry = (int)(player.point.y - ty), rx = (int)(player.point.x + tx);
            while(
                (ry >= startPoint.y) &&
                (rx <= endPoint.x) &&
                (((map[ry][rx] & A_CHARTEXT) == PLAYER) || ((map[ry][rx] & A_CHARTEXT) == GROUND))
            ) {
                ry = (int)(player.point.y - ty);
                rx = (int)(player.point.x + tx);
                mvaddch(ry - startPoint.y, rx - startPoint.x, map[ry][rx]);
                
				if(a < 1) tx += 0.2;
                else tx += 1 / (a*2);
				ty = a * tx;
            }
        }
		
		for(int tx = -1; tx <= 1; ++tx) {
            int ty = 0;
			int ry = player.point.y, rx = player.point.x + tx;
            while(
                (ry >= startPoint.y) &&
                (rx <= endPoint.x) &&
				(rx >= 0) &&
                (((map[ry][rx] & A_CHARTEXT) == PLAYER) || ((map[ry][rx] & A_CHARTEXT) == GROUND))
            ) {
                ry = player.point.y - ty;
                mvaddch(ry - startPoint.y, rx - startPoint.x, map[ry][rx]);

                ++ty;
            }
        }
	}
	else if(player.lookDir == RIGHT) {
        for(float a = VIEW_ANGLE + 1; a < 15; a += 0.1*a) {
            float ty = 0, tx = 0;
            int ry = (int)(player.point.y - tx), rx = (int)(player.point.x + ty);
            while(
                (ry >= startPoint.y) &&
                (rx <= endPoint.x) &&
                (((map[ry][rx] & A_CHARTEXT) == PLAYER) || ((map[ry][rx] & A_CHARTEXT) == GROUND))
            ) {
                ry = (int)(player.point.y - tx);
                rx = (int)(player.point.x + ty);
                mvaddch(ry - startPoint.y, rx - startPoint.x, map[ry][rx]);
				
				if(a < 1) tx += 0.2;
                else tx += 1 / (a*2);
                ty = a * tx;
            }
        }
		
        for(float a = VIEW_ANGLE + 1; a < 15; a += 0.1*a) {
            float ty = 0, tx = 0;
            int ry = (int)(player.point.y + tx), rx = (int)(player.point.x + ty);
            while(
                (ry <= endPoint.y) &&
                (rx <= endPoint.x) &&
                (((map[ry][rx] & A_CHARTEXT) == PLAYER) || ((map[ry][rx] & A_CHARTEXT) == GROUND))
            ) {
                ry = (int)(player.point.y + tx);
                rx = (int)(player.point.x + ty);
                mvaddch(ry - startPoint.y, rx - startPoint.x, map[ry][rx]);

                if(a < 1) tx += 0.2;
                else tx += 1 / (a*2);
				ty = a * tx;
            }
        }

        for(int tx = -1; tx <= 1; ++tx) {
            int ty = 0;
            int ry = player.point.y - tx, rx = player.point.x;
            while(
                (rx <= endPoint.x) &&
                (ry <= endPoint.y) &&
                (ry >= 0) &&
				(((map[ry][rx] & A_CHARTEXT) == PLAYER) || ((map[ry][rx] & A_CHARTEXT) == GROUND))
            ) {
                rx = player.point.x + ty;
                mvaddch(ry - startPoint.y, rx - startPoint.x, map[ry][rx]);

                ++ty;
            }
        }
    }
	else if(player.lookDir == DOWN) {
        for(float a = VIEW_ANGLE; a < 7; a += 0.1*a) {
            float ty = 0, tx = 0;
            int ry = (int)(player.point.y + ty), rx = (int)(player.point.x - tx);
            while(
                (ry >= startPoint.y) &&
                (rx >= startPoint.x) &&
                (((map[ry][rx] & A_CHARTEXT) == PLAYER) || ((map[ry][rx] & A_CHARTEXT) == GROUND))
            ) {
                ry = (int)(player.point.y + ty);
                rx = (int)(player.point.x - tx);
                mvaddch(ry - startPoint.y, rx - startPoint.x, map[ry][rx]);

                if(a < 1) tx += 0.2;
                else tx += 1 / (a*2);
                ty = a * tx;
            }
        }

        for(float a = VIEW_ANGLE; a < 7; a += 0.1*a) {
            float ty = 0, tx = 0;
            int ry = (int)(player.point.y + ty), rx = (int)(player.point.x + tx);
            while(
                (ry >= startPoint.y) &&
                (rx <= endPoint.x) &&
                (((map[ry][rx] & A_CHARTEXT) == PLAYER) || ((map[ry][rx] & A_CHARTEXT) == GROUND))
            ) {
                ry = (int)(player.point.y + ty);
                rx = (int)(player.point.x + tx);
                mvaddch(ry - startPoint.y, rx - startPoint.x, map[ry][rx]);

                if(a < 1) tx += 0.2;
                else tx += 1 / (a*2);
                ty = a * tx;
            }
        }

        for(int tx = -1; tx <= 1; ++tx) {
            int ty = 0;
            int ry = player.point.y, rx = player.point.x + tx;
            while(
                (ry >= startPoint.y) &&
                (rx <= endPoint.x) &&
                (rx >= 0) &&
                (((map[ry][rx] & A_CHARTEXT) == PLAYER) || ((map[ry][rx] & A_CHARTEXT) == GROUND))
            ) {
                ry = player.point.y + ty;
                mvaddch(ry - startPoint.y, rx - startPoint.x, map[ry][rx]);

                ++ty;
            }
        }
    }
	else if(player.lookDir == LEFT) {
        for(float a = VIEW_ANGLE + 1; a < 15; a += 0.1*a) {
            float ty = 0, tx = 0;
            int ry = (int)(player.point.y - tx), rx = (int)(player.point.x - ty);
            while(
                (ry >= startPoint.y) &&
                (rx <= endPoint.x) &&
                (((map[ry][rx] & A_CHARTEXT) == PLAYER) || ((map[ry][rx] & A_CHARTEXT) == GROUND))
            ) {
                ry = (int)(player.point.y - tx);
                rx = (int)(player.point.x - ty);
                mvaddch(ry - startPoint.y, rx - startPoint.x, map[ry][rx]);

                if(a < 1) tx += 0.2;
                else tx += 1 / (a*2);
                ty = a * tx;
            }
        }

        for(float a = VIEW_ANGLE + 1; a < 15; a += 0.1*a) {
            float ty = 0, tx = 0;
            int ry = (int)(player.point.y + tx), rx = (int)(player.point.x - ty);
            while(
                (ry <= endPoint.y) &&
                (rx <= endPoint.x) &&
                (((map[ry][rx] & A_CHARTEXT) == PLAYER) || ((map[ry][rx] & A_CHARTEXT) == GROUND))
            ) {
                ry = (int)(player.point.y + tx);
                rx = (int)(player.point.x - ty);
                mvaddch(ry - startPoint.y, rx - startPoint.x, map[ry][rx]);

                if(a < 1) tx += 0.2;
                else tx += 1 / (a*2);
                ty = a * tx;
            }
        }

        for(int tx = -1; tx <= 1; ++tx) {
            int ty = 0;
            int ry = player.point.y - tx, rx = player.point.x;
            while(
                (rx <= endPoint.x) &&
                (ry <= endPoint.y) &&
                (ry >= 0) &&
                (((map[ry][rx] & A_CHARTEXT) == PLAYER) || ((map[ry][rx] & A_CHARTEXT) == GROUND))
            ) {
                rx = player.point.x - ty;
                mvaddch(ry - startPoint.y, rx - startPoint.x, map[ry][rx]);

                ++ty;
            }
        }
    }
}
