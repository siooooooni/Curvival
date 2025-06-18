#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

#define MAP_HEIGHT 50
#define MAP_WIDTH 100

#define VIEW_HEIGHT 21
#define VIEW_WIDTH 51

typedef enum {PLAYER = '8', ZOMBIE = 'z', ITEM = '!', WALL = '#', GROUND = '.'} type;

typedef struct {
    int y;
    int x;
} yx;

typedef struct {
    type role;
    yx point;
    int hp;
} human;       

chtype map[MAP_HEIGHT][MAP_WIDTH];
human player = {.role = PLAYER, .point = (yx){10, 10}, .hp = 100};

void init_UI();
void init_map(chtype map[MAP_HEIGHT][MAP_WIDTH]);
void draw_map(chtype map[MAP_HEIGHT][MAP_WIDTH]);
void draw_view_map(chtype map[MAP_HEIGHT][MAP_WIDTH], yx playerPoint);
yx HumanMove(yx nextPoint, int way);

int main() {
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
