#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAP_HEIGHT 150
#define MAP_WIDTH 300
#define HOUSE_COUNT 7
#define ZOMBIE_MAX 20
#define ZOMBIE_MIN 15
#define ITEM_COUNT 3

#define VIEW_HEIGHT 21
#define VIEW_WIDTH 51

typedef enum {PLAYER = '8', ZOMBIE = 'z', ITEM = '!', WALL = '#', GROUND = '.'} type;

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
} human;       

//직사각형 건물
typedef struct {
    int y1, x1;
    int y2, x2;
} Rect;

chtype map[MAP_HEIGHT][MAP_WIDTH];
human player = {.role = PLAYER, .point = (yx){10, 10}, .hp = 100};
void swap_int(int* a, int* b);
void init_UI();
void init_map(chtype map[MAP_HEIGHT][MAP_WIDTH]);
void draw_map(chtype map[MAP_HEIGHT][MAP_WIDTH]);
void draw_view_map(chtype map[MAP_HEIGHT][MAP_WIDTH], yx playerPoint);
void draw_building(chtype map[MAP_HEIGHT][MAP_WIDTH], Rect b, int color_pair);
yx HumanMove(yx nextPoint, int way);

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
		draw_view_map(map, player.point);
		refresh();

    	int key = getch();

		if(key != -1){
			map[player.point.y][player.point.x] = GROUND | COLOR_PAIR(1);

			nextPoint = HumanMove(player.point, key);
			player.point = nextPoint;
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
    for (int x = b.x1; x <= b.x2; x++) {
        map[b.y1][x] = '#' | COLOR_PAIR(color_pair);  // 상단
        map[b.y2][x] = '#' | COLOR_PAIR(color_pair);  // 하단
    }

    // 좌우 테두리
    for (int y = b.y1; y <= b.y2; y++) {
        map[y][b.x1] = '#' | COLOR_PAIR(color_pair);  // 좌측
        map[y][b.x2] = '#' | COLOR_PAIR(color_pair);  // 우측
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
        {5, 5, 10, 15},     // ↘ 정방향
        {20, 30, 15, 25},   // ↖ 역방향
        {40, 70, 45, 60},   // ↙ 정방향
        {60, 10, 55, 5},     // ↗ 역방향
		{80, 95, 86, 101},
		{110, 120, 115, 127},
		{63, 30, 73, 40},
    }; 
	for (int i=0;i<HOUSE_COUNT-1;++i) {
		draw_building(map,buildings[i],2);
	}
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

}

void draw_map(chtype map[MAP_HEIGHT][MAP_WIDTH]) {
    for (int i=0;i<MAP_HEIGHT;++i) {
        for (int j=0;j<MAP_WIDTH;++j) {
            mvaddch(i,j,map[i][j]);
        }
    }
}

yx HumanMove(yx nextPoint, int way) {
    if(way == 119){
        if((map[nextPoint.y - 1][nextPoint.x] & A_CHARTEXT) == GROUND){
            nextPoint.y--;
        }
    }
    else if(way == 100){
        if((map[nextPoint.y][nextPoint.x + 1] & A_CHARTEXT) == GROUND){
            nextPoint.x+=1;
        }
    }
    else if(way == 115){
        if((map[nextPoint.y + 1][nextPoint.x] & A_CHARTEXT) == GROUND){
            nextPoint.y+=1;
        }
    }
    else if(way == 97){
        if((map[nextPoint.y][nextPoint.x - 1] & A_CHARTEXT) == GROUND){
            nextPoint.x-=1;
        }
    }

    return nextPoint;
}

void draw_view_map(chtype map[MAP_HEIGHT][MAP_WIDTH], yx playerPoint){
	yx startPoint = {playerPoint.y - ((VIEW_HEIGHT-1) / 2), playerPoint.x - ((VIEW_WIDTH-1) / 2)};
	yx endPoint = {playerPoint.y + ((VIEW_HEIGHT-1) / 2), playerPoint.x + ((VIEW_WIDTH-1) / 2)};
	
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
	
	int y = 0, x = 0;
	for (int i = startPoint.y; i <= endPoint.y; ++i) {
        for (int j = startPoint.x; j<= endPoint.x; ++j) {
            mvaddch(y,x,map[i][j]);
			++x;
        }
		x = 0;
		++y;
    }
}
