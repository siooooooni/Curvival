#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include "story.h"
#define MAP_HEIGHT 150 //맵 크기
#define MAP_WIDTH 300
#define HOUSE_COUNT 12 //집 개수

#define ZOMBIE_MAX 130 //좀비 개수 최소~최대
#define ZOMBIE_MIN 80

#define FZOMBIE_MAX 50 //빠른 좀비 개수 최소~최대 
#define FZOMBIE_MIN 40

#define ITEM_COUNT 20 //아이템 개수
#define VACCINE_COUNT 5 //각각 DEBUG 글자로 할 예정
#define TREE_COUNT 30 //트리 개수 30개
#define VIEW_HEIGHT 21 //플레이어 시야 크기
#define VIEW_WIDTH 51
#define VIEW_ANGLE 0.2f //플레이어 시야 각도
#define ROCK_COUNT 40 // 바위 개수
#define WATER_COUNT 30 // 물웅덩이 개수

#define NEAR_VIEW_SIZE 5 //주변 시야 범위
#define NEAR_VIEW_ADJ 5 //그래픽 조절값

#define UP_VIEW_START_ANGLE 10.0 //위쪽 시야 시작 각도
#define UP_VIEW_END_ANGLE 170.0 //위쪽 시야 끝 각도
#define LEFT_VIEW_START_ANGLE 100.0 //왼쪽 시야 시작 각도
#define LEFT_VIEW_END_ANGLE 260.0 //왼쪽 시야 끝 각도
#define DOWN_VIEW_START_ANGLE 190.0 //아래 시야 시작 각도
#define DOWN_VIEW_END_ANGLE 350.0 //아래 시야 끝 각도
#define RIGHT_VIEW_START_ANGLE 280.0 //오른쪽 시야 시작 각도
#define RIGHT_VIEW_END_ANGLE 80.0 //오른쪽 시야 끝 각도

#define VIEW_SHIFT_VEL 0.5 //시야 전환 애니메이션 속도

#define ZOMBIE_ACT_TIME 30 //좀비 분노 지속 시간
#define ZOMBIE_ACT_ADJ 5 //좀비 분노 조절값
#define ZOMBIE_ACT_BASE_DISTANCE 6 //기본 소음 범위
#define ZOMBIE_NOIZE_UNIT 1.0 //소음 발생 단위
#define ZOMBIE_GENERAL_SPEED 0.25 //일반적인 좀비 속도
#define ZOMBIE_CALM_DOWN_VAL 2.0 //소음이 줄어드는 속도
#define ZOMBIE_ACT_LIMIT 10.0 //좀비 분노 한계
#define ZOMBIE_DAMAGE 10

#define FZOMBIE_ACT_TIME 20 //좀비 분노 지속 시간  
#define FZOMBIE_ACT_ADJ 5 //좀비 분노 조절값    
#define FZOMBIE_ACT_BASE_DISTANCE 7 //기본 소음 범위      
#define FZOMBIE_NOIZE_UNIT 1.0 //소음 발생 단위     
#define FZOMBIE_GENERAL_SPEED 0.15 //일반적인 좀비 속도         
#define FZOMBIE_CALM_DOWN_VAL 2.0 //소음이 줄어드는 속도    
#define FZOMBIE_ACT_LIMIT 11.0 //좀비 분노 한계
#define FZOMBIE_DAMAGE 15

typedef enum {PLAYER = '8', ZOMBIE = 'z', FZOMBIE = 'f', ITEM = '!', WALL = '#', GROUND = '.', POND = '~', ROCK = '@'} type;
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
    int invincible;         // 무적 상태 여부 (1: 무적, 0: 일반)
    struct timespec invincible_end; // 무적 끝나는 시간
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
	int actAmt;
} zombie_t;

//아이템 구조체
typedef struct {
    int injection; //광범위 공격 : 1번
    int pointer; //총 : 2번
    int patch; //포션 : 3번
} item_list;
item_list itemList = {0,0,0};
chtype map[MAP_HEIGHT][MAP_WIDTH];
human player = {.role = PLAYER, .point = {10, 10}, .hp = 100, .lookDir = RIGHT};

zombie_t zombies[ZOMBIE_MAX];
int zombie_count = 0;
double zombie_act_distance = ZOMBIE_ACT_BASE_DISTANCE; //현재 소음 범위
double zombie_speed = ZOMBIE_GENERAL_SPEED; //좀비 속도

zombie_t fzombies[FZOMBIE_MAX];
int fzombie_count = 0;
double fzombie_act_distance = FZOMBIE_ACT_BASE_DISTANCE; //현재 소음 범위
double fzombie_speed = FZOMBIE_GENERAL_SPEED; //좀비 속도

double view_start_angle = RIGHT_VIEW_START_ANGLE; //시야 시작 각도
double view_end_angle = RIGHT_VIEW_END_ANGLE; //시야 끝 각도

yx startPoint = {.y = 0, .x = 0};
yx endPoint = {.y = 0, .x = 0};

void swap_int(int* a, int* b); //정수값 swap
void init_UI(); //시작 UI
void init_map(chtype map[MAP_HEIGHT][MAP_WIDTH]); //맵 초기화
void make_vaccine(chtype map[MAP_HEIGHT][MAP_WIDTH], char a); //백신 생성
void draw_building(chtype map[MAP_HEIGHT][MAP_WIDTH], Rect b, int color_pair); //건물 그리기
void draw_view_map(chtype map[MAP_HEIGHT][MAP_WIDTH], human player); //플레이어 시야 그리기
void move_zombies(chtype map[MAP_HEIGHT][MAP_WIDTH], yx playerPoint); //좀비 이동
void HumanMove(human *player, int way); //플레이어 이동
void make_tree(chtype map[MAP_HEIGHT][MAP_WIDTH], int y, int x);
void make_obstacle(chtype map[MAP_HEIGHT][MAP_WIDTH]); // 장애물 생성
void line_view(yx point, double angle, double distance, int *r);
void player_view();
void move_fzombies(chtype map[MAP_HEIGHT][MAP_WIDTH], yx playerPoint);
int grab_item();
bool check_touch();
void you_die();

int main() {
    system("clear");
    init_UI();
    initscr();
	noecho();
    nodelay(stdscr, TRUE);
    start_color();
    curs_set(0);

    //시야 안 색상
    init_pair(1, 245, 245);  // 회색 글자, 회색 배경 : 땅
    init_pair(2, 15, 235);     // 흰색 글자(15번), 검은 배경(0번) : 벽
    init_pair(3, 21, 245);   // 파란색 글자(21번), 회색 배경 : 플레이어
    init_pair(4, 88, 245);  // 빨간색 글자, 회색 배경 : 좀비
    init_pair(5, 226, 245);  // 노란색 글자, 회색 배경 : 아이템
    init_pair(6, 46, 46);    // 초록색 글자, 초록색 배경 : 백신
    init_pair(7, 240, 240);  // 진한 회색 글자, 진한 회색 배경 : 시야 밖
    init_pair(8, 245, 236); // 바위(진한 회색)
    init_pair(9, 45, 39);   // 물웅덩이(파란색)
	init_pair(10, 54, 245); // 빠른 좀비
    
	//시야 밖 색상 (100 + 기존색상번호)
	init_pair(101, 243, 243);  // 땅
    init_pair(102, 243, 232);  // 벽
	init_pair(108, 238, 238);  // 바위
	init_pair(109, 238, 238);  // 물웅덩이

	//좀비 분노 색상
	init_pair(104, 196, 245); //좀비 분노
	init_pair(110, 201, 245); //빠른 좀비 분노

	//시작하고 출력
    init_map(map);
    make_obstacle(map); // 장애물 생성
    struct timespec last_zombie_move, now; //초+나노초로 시간 저장하는 구조체
    clock_gettime(CLOCK_MONOTONIC, &last_zombie_move); //현재 시간 구조체에 저장
    struct timespec flast_zombie_move, fnow; //초+나노초로 시간 저장하는 구조체
    clock_gettime(CLOCK_MONOTONIC, &flast_zombie_move); //현재 시간 구조체에 저장
	while(1){
        startPoint = (yx){player.point.y - ((VIEW_HEIGHT-1) / 2), player.point.x - ((VIEW_WIDTH-1) / 2)}; // 왼쪽 위 좌표
    	endPoint = (yx){player.point.y + ((VIEW_HEIGHT-1) / 2), player.point.x + ((VIEW_WIDTH-1) / 2)}; // 오른쪽 아래 좌표
    	// 좌표들이 맵을 벗어날 경우 예외 처리
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

		draw_view_map(map, player);
        
		mvprintw(22, 0, "noize val: %lf\nzombie speed: %lf\n", zombie_act_distance, zombie_speed);

		refresh();
        bool isTouched = check_touch();
        if (isTouched) {
            //0.5초간 무적 상태?
        }

        int key = getch(); //키보드 입력 받기
        if(key != -1){ //키보드 입력이 있다면
            if (key=='5') grab_item();
			if(zombie_act_distance <= ZOMBIE_ACT_LIMIT) {
                zombie_act_distance += ZOMBIE_NOIZE_UNIT;
            }
			if(fzombie_act_distance <= FZOMBIE_ACT_LIMIT) {
                fzombie_act_distance += FZOMBIE_NOIZE_UNIT;
            }


            map[player.point.y][player.point.x] = GROUND | COLOR_PAIR(1);
            HumanMove(&player, key);
            map[player.point.y][player.point.x] =  PLAYER | COLOR_PAIR(3) | A_BOLD;
        }

        //좀비 움직이는 함수
        clock_gettime(CLOCK_MONOTONIC, &now);
        double elapsed = (now.tv_sec - last_zombie_move.tv_sec) + (now.tv_nsec - last_zombie_move.tv_nsec) / 1e9;
        if (elapsed > zombie_speed) {
            if(zombie_act_distance > ZOMBIE_ACT_BASE_DISTANCE) { //현재 소음 범위가 기본 소음 범위를 넘어섰다면
            	zombie_act_distance -= ZOMBIE_CALM_DOWN_VAL; // 소음 범위 줄이기
        	}
        	else {
            	zombie_act_distance = ZOMBIE_ACT_BASE_DISTANCE;
        	}

			move_zombies(map, player.point);
            last_zombie_move = now; //현재 시간 구조체에 저장
        }

		clock_gettime(CLOCK_MONOTONIC, &fnow);
        double felapsed = (fnow.tv_sec - flast_zombie_move.tv_sec) + (fnow.tv_nsec - flast_zombie_move.tv_nsec) / 1e9;
        if (felapsed > fzombie_speed) {
            if(fzombie_act_distance > FZOMBIE_ACT_BASE_DISTANCE) { //현재 소음 범위가 기본 소음 범위를 넘어섰다면
                fzombie_act_distance -= FZOMBIE_CALM_DOWN_VAL; // 소음 범위 줄이기
            }
            else {
                fzombie_act_distance = FZOMBIE_ACT_BASE_DISTANCE;
            }

            move_fzombies(map, player.point);
            flast_zombie_move = fnow; //현재 시간 구조체에 저장
        }

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        if (player.invincible) {
            if ((now.tv_sec > player.invincible_end.tv_sec) ||
                (now.tv_sec == player.invincible_end.tv_sec && now.tv_nsec > player.invincible_end.tv_nsec)) {
                player.invincible = 0; // 무적 해제
            }
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

//시작할 때 뜨는 UI
void init_UI() {
	while (1) {	
    	printf("==============================\n");
    	printf("\tZOMBIE SURVIVAL\n");
    	printf("==============================\n");
    	printf("\t0. Prologue\n");
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
		else if (n==0) {
			init_story();
		}
		//else if (n==2) {}
    	//else if (n==3) {}
	}
}

//건물 그리기
void draw_building(chtype map[MAP_HEIGHT][MAP_WIDTH], Rect b, int color_pair) {
    // y1 ≤ y2, x1 ≤ x2 가정 (아니면 swap)
    if (b.y1 > b.y2) swap_int(&b.y1, &b.y2);
    if (b.x1 > b.x2) swap_int(&b.x1, &b.x2);
    // 상하 테두리
    for (int x=b.x1;x<=b.x2;++x) {
        map[b.y1][x] = WALL | COLOR_PAIR(color_pair);  // 상단
        map[b.y2][x] = WALL | COLOR_PAIR(color_pair);  // 하단
    }
    // 좌우 테두리
    for (int y=b.y1;y<=b.y2;++y) {
        map[y][b.x1] = WALL | COLOR_PAIR(color_pair);  // 좌측
        map[y][b.x2] = WALL | COLOR_PAIR(color_pair);  // 우측
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

//초기 맵 제작 함수
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
    int zombieCount = rand() % (ZOMBIE_MAX - ZOMBIE_MIN + 1) + ZOMBIE_MIN;
    zombie_count = zombieCount;
    int zidx = 0;
    for (int z=0;z<zombieCount;++z) {
        int y,x;
        do {
            y = rand() % (MAP_HEIGHT - 2) + 1; //벽은 제외
            x = rand() % (MAP_WIDTH  - 2) + 1;
        } while (map[y][x] != (GROUND | COLOR_PAIR(1))); //빈 공간이어야함
        map[y][x] = ZOMBIE | COLOR_PAIR(4);
        //좀비 구조체 배열에 좀비 좌표 저장
        zombies[zidx].point.y = y;
        zombies[zidx].point.x = x;
        zombies[zidx].alive = 1;
		zombies[zidx].actAmt = 0;
        zidx++; //인덱스 증가
    }
	
	int fzombieCount = rand() % (FZOMBIE_MAX - FZOMBIE_MIN + 1) + FZOMBIE_MIN;
    fzombie_count = fzombieCount;
    int fzidx = 0;
    for (int z=0;z<fzombieCount;++z) {
        int y,x;
        do {
            y = rand() % (MAP_HEIGHT - 2) + 1; //벽은 제외
            x = rand() % (MAP_WIDTH  - 2) + 1;
        } while (map[y][x] != (GROUND | COLOR_PAIR(1))); //빈 공간이어야함
        map[y][x] = FZOMBIE | COLOR_PAIR(10);
        //좀비 구조체 배열에 좀비 좌표 저장
        fzombies[fzidx].point.y = y;
        fzombies[fzidx].point.x = x;
        fzombies[fzidx].alive = 1;
        fzombies[fzidx].actAmt = 0;
        fzidx++; //인덱스 증가
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
    //백신 랜덤 생성 : VACCINE_COUNT : 5
    char temp[5] = {'D','E','B','U','G'};
    for (int i=0;i<VACCINE_COUNT;++i) {
        make_vaccine(map,temp[i]);
    }
    map[player.point.y][player.point.x] = PLAYER | COLOR_PAIR(3) | A_BOLD; //플레이어
    //장애물 랜덤 생성
    make_obstacle(map);
}

//백신 알파벳 별로 만들기 위해 함수 작성함.
void make_vaccine(chtype map[MAP_HEIGHT][MAP_WIDTH],char a) {
    int y,x;
    do {
        y = rand() % (MAP_HEIGHT - 2) + 1;
        x = rand() % (MAP_WIDTH  - 2) + 1;
    } while (map[y][x] != (GROUND | COLOR_PAIR(1)));
    map[y][x] = a | COLOR_PAIR(6) | A_BLINK;
}

//플레이어 이동
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
            (player -> point).y++;
            player -> lookDir = DOWN;
        }
    }
    else if(way == LEFT){ // 'a' 누르면
        if((map[(player -> point).y][(player -> point).x - 1] & A_CHARTEXT) == GROUND){
            (player -> point).x--;
            player -> lookDir = LEFT;
        }
    }
}
//플레이어 시야 그리기 (ray casting 방식)
void draw_view_map(chtype map[MAP_HEIGHT][MAP_WIDTH], human player){
    // 시야가 닿지 않는 곳은 어둡게 처리
    int y = 0, x = 0;
    for (int i = startPoint.y; i <= endPoint.y; ++i) {
        for (int j = startPoint.x; j <= endPoint.x; ++j) {
            char c = map[i][j] & A_CHARTEXT;
            if(c == GROUND) {
                mvaddch(y, x, GROUND | COLOR_PAIR(101));
            }
            else if(c == WALL) {
                mvaddch(y, x, WALL | COLOR_PAIR(102));
            }
			else if(c == ROCK) {
				mvaddch(y, x, ROCK | COLOR_PAIR(108));
			}
			else if(c == POND) {
				mvaddch(y, x, POND | COLOR_PAIR(109));
			}
            else {
                mvaddch(y, x, GROUND | COLOR_PAIR(101));
            }

            ++x;
        }
        x = 0;
        ++y;
    }

	mvaddch(player.point.y - startPoint.y, player.point.x - startPoint.x, map[player.point.y][player.point.x]);

    // 시야 각도에 따라 ray casting 방식으로 시야를 그림
	if(player.lookDir == UP) {
		if(UP_VIEW_START_ANGLE != view_start_angle){
			if((UP_VIEW_START_ANGLE + 180 > view_start_angle) && (view_start_angle > UP_VIEW_START_ANGLE)) {
				view_start_angle -= VIEW_SHIFT_VEL;
				view_end_angle -= VIEW_SHIFT_VEL;
			}
			else {
				view_start_angle += VIEW_SHIFT_VEL;
				view_end_angle += VIEW_SHIFT_VEL;
			}
		}
	}
    else if(player.lookDir == RIGHT) {
		if(RIGHT_VIEW_START_ANGLE != view_start_angle){
            if((RIGHT_VIEW_START_ANGLE - 180 < view_start_angle) && (view_start_angle < RIGHT_VIEW_START_ANGLE)) {
				view_start_angle += VIEW_SHIFT_VEL;
            	view_end_angle += VIEW_SHIFT_VEL;
			}
            else {
                view_start_angle -= VIEW_SHIFT_VEL;
            	view_end_angle -= VIEW_SHIFT_VEL;
			}
        }
	}
    else if(player.lookDir == DOWN) {
		if(DOWN_VIEW_START_ANGLE != view_start_angle){
            if((DOWN_VIEW_START_ANGLE - 180 < view_start_angle) && (view_start_angle < DOWN_VIEW_START_ANGLE)) {
            	view_start_angle += VIEW_SHIFT_VEL;
                view_end_angle += VIEW_SHIFT_VEL;
			}
            else {
            	view_start_angle -= VIEW_SHIFT_VEL;
                view_end_angle -= VIEW_SHIFT_VEL;
			}
        }
	}
    else if(player.lookDir == LEFT) {
		if(LEFT_VIEW_START_ANGLE != view_start_angle){
            if((LEFT_VIEW_START_ANGLE + 180 > view_start_angle) && (view_start_angle > LEFT_VIEW_START_ANGLE)) {
            	view_start_angle -= VIEW_SHIFT_VEL;
                view_end_angle -= VIEW_SHIFT_VEL;
			}
            else {
            	view_start_angle += VIEW_SHIFT_VEL;
                view_end_angle += VIEW_SHIFT_VEL;
			}
        }
	}
	//0~360도 범위를 벗어날 경우 조정
	if(view_start_angle < 0) view_start_angle = 360;
    else if(view_start_angle > 360) view_start_angle = 0;

	if(view_end_angle < 0) view_end_angle = 360;
    else if(view_end_angle > 360) view_end_angle = 0;

	player_view();
}
void player_view() { //플레이어 시야
	if(view_start_angle > view_end_angle) {
		for(double angle = 0.0; angle < view_end_angle; angle += 0.1) {
			line_view(player.point, angle, 1000, NULL);
		}
		for(double angle = 0.0; angle < view_start_angle; angle += 0.1) {
			line_view(player.point, angle, NEAR_VIEW_SIZE, NULL);
		}
		for(double angle = view_start_angle; angle < 360.0; angle += 0.1) {
			line_view(player.point, angle, 1000, NULL);
		}
	}
	else {
		for(double angle = 0.0; angle < view_start_angle; angle += 0.1) {
    		line_view(player.point, angle, NEAR_VIEW_SIZE, NULL);
    	}
    	for(double angle = view_start_angle; angle < view_end_angle; angle += 0.1) {
    		line_view(player.point, angle, 1000, NULL);
    	}
    	for(double angle = view_end_angle; angle < 360.0; angle += 0.1) {
        	line_view(player.point, angle, NEAR_VIEW_SIZE, NULL);
    	}
	}
}
void line_view(yx point, double angle, double distance, int *r) { //일직선 시야 검색 (distance 거리만큼 검색함) (r은 검색중 장애물을 만났는지 여부를 저장함)
	if((int)angle % 90 == 0) angle += 0.5;

	double a = tan(angle*(M_PI / 180.0));
	yx quadrant = {.y = 1, .x = 1};
	
	if((0 < angle) && (angle < 90)) {
		quadrant = (yx){.y = 1, .x = 1};
	}
	else if((90 < angle) && (angle < 180)) {
		quadrant = (yx){.y = 1, .x = -1};
	}
	else if((180 < angle) && (angle < 270)) {
        quadrant = (yx){.y = -1, .x = -1};
    }
	else if((270 < angle) && (angle < 360)) {
        quadrant = (yx){.y = -1, .x = 1};
    }
	
	double ty = 0, tx = 0;
	int ry = point.y - (int)ty, rx = point.x + (int)tx;
	
	while((startPoint.y <= ry) && (ry <= endPoint.y) && (startPoint.x <= rx) && (rx < endPoint.x)) { //화면 이탈 방지
		if(((map[ry][rx] & A_CHARTEXT) == WALL) || ((map[ry][rx] & A_CHARTEXT) == ROCK)) { //장애물 감지
        	if(r != NULL) *r = 1;
			break;
		}

		double nty = a * (tx + quadrant.x);
		while(abs((int)nty - (int)ty) >= 1) {
			if(((map[ry][rx] & A_CHARTEXT) == WALL) || ((map[ry][rx] & A_CHARTEXT) == ROCK)) { //장애물 감지
				if(r != NULL) *r = 1;
				return;
			}

			if((startPoint.y >= ry) || (ry >= endPoint.y) || (startPoint.x >= rx) || (rx >= endPoint.x) || (sqrt((ty*ty) + (tx*tx/NEAR_VIEW_ADJ)) > distance)) { //화면 이탈 방지와 거리 이탈 방지
                return;
            }

			ty += quadrant.y;
			ry = point.y - (int)ty;
        	rx = point.x + (int)tx;
			
			if(r == NULL) mvaddch(ry - startPoint.y, rx - startPoint.x, map[ry][rx]);
		}
		
		if(sqrt((ty*ty) + (tx*tx/NEAR_VIEW_ADJ)) > distance) { //거리 이탈 방지
            return;
        }

		tx += quadrant.x;
		ty = a * tx;
        ry = point.y - (int)ty;
        rx = point.x + (int)tx;
		
		if(r == NULL) mvaddch(ry - startPoint.y, rx - startPoint.x, map[ry][rx]);
	}
}
//좀비 움직이는 함수 : zombie_count으로 모든 좀비 하나하나씩 이동방향을 y축 우선으로 결정하여 이동가능한지 체크하고 옮긴다.
void move_zombies(chtype map[MAP_HEIGHT][MAP_WIDTH], yx playerPoint) {
    for (int i = 0; i < zombie_count; ++i) {
        if (!zombies[i].alive) continue; //좀비가 죽은 놈이면 끝
        //시야 내에 있는지 확인
        int ny = zombies[i].point.y;
        int nx = zombies[i].point.x;

		if(zombies[i].actAmt == 0) { // 좀비 비활성화 상태 움직임
			int is_act = 0;

			double distance = sqrt((ny - player.point.y) * (ny - player.point.y) + (nx - player.point.x) * (nx - player.point.x)); //zombie와 플레이어 거리계산 (수정 필요) 
			if(sqrt((ny - player.point.y) * (ny - player.point.y) + (nx - player.point.x) * (nx - player.point.x)/ZOMBIE_ACT_ADJ) <= zombie_act_distance) { //좀비가 플레이어를 보았는지 확인하는 코드
				double angle = atan2(ny - player.point.y, nx - player.point.x) * (180.0 / M_PI) + 180.0;
            	int is_blocked = 0;
            	line_view(zombies[i].point, angle, distance, &is_blocked);

            	if(is_blocked == 0) { //좀비가 플레이어를 봤다면
                	zombies[i].actAmt = ZOMBIE_ACT_TIME;
					is_act = 1;
            	}
        	}
			
			if(!is_act) { //좀비가 플레이어를 못 봤다면
				int chooseDyDx = rand()%6;

				switch(chooseDyDx) {
					case 0: ++ny; break;
					case 1: --ny; break;
					case 2: ++nx; break;
					case 3: --nx; break;
					default: break;
				}
			}
		}
		if(zombies[i].actAmt > 0) { // 좀비 활성화 상태 움직임
			int chooseDyDx = rand()%2;
        	
			if (chooseDyDx) {
            	if (playerPoint.y < ny) ny--;
            	else if (playerPoint.y > ny) ny++;
        	}
        	else {
            	if (playerPoint.x < nx) nx--;
            	else if (playerPoint.x > nx) nx++;
        	}	
			
			--(zombies[i].actAmt);
		}

        // 이동 가능 체크 (벽, 다른 좀비, 플레이어 등)
        int can_move = 1;
        if ((map[ny][nx] & A_CHARTEXT) != GROUND) can_move = 0; // &로 문자 코드만 추출해서 땅이 아니면 이동 불가
        //다른 좀비와 겹치는지 확인
        for (int j = 0; j < zombie_count; ++j) {
            if (i != j && zombies[j].alive && zombies[j].point.y == ny && zombies[j].point.x == nx) {
                can_move = 0;
                break;
            }
        }
        //플레이어와 겹치는지 확인
        if (player.point.y == ny && player.point.x == nx) can_move = 0;
        //이동 가능하면 이동
        if (can_move) {
            map[zombies[i].point.y][zombies[i].point.x] = GROUND | COLOR_PAIR(1);
            zombies[i].point.y = ny;
            zombies[i].point.x = nx;
            if(zombies[i].actAmt > 0) map[ny][nx] = ZOMBIE | COLOR_PAIR(104);
			else map[ny][nx] = ZOMBIE | COLOR_PAIR(4);
        }
    }
}
void move_fzombies(chtype map[MAP_HEIGHT][MAP_WIDTH], yx playerPoint) {
    for (int i = 0; i < fzombie_count; ++i) {
        if (!fzombies[i].alive) continue; //좀비가 죽은 놈이면 끝
        //시야 내에 있는지 확인
        int ny = fzombies[i].point.y;
        int nx = fzombies[i].point.x;

        if(fzombies[i].actAmt == 0) { // 좀비 비활성화 상태 움직임
            int is_act = 0;

            double distance = sqrt((ny - player.point.y) * (ny - player.point.y) + (nx - player.point.x) * (nx - player.point.x)); //zombie와 플레이어 거리계산 (수정 필요)
            if(sqrt((ny - player.point.y) * (ny - player.point.y) + (nx - player.point.x) * (nx - player.point.x)/FZOMBIE_ACT_ADJ) <= fzombie_act_distance) { //좀비가 플레이어를 보았는지 확인하는 코드
                double angle = atan2(ny - player.point.y, nx - player.point.x) * (180.0 / M_PI) + 180.0;
                int is_blocked = 0;
                line_view(fzombies[i].point, angle, distance, &is_blocked);

                if(is_blocked == 0) { //좀비가 플레이어를 봤다면
                    fzombies[i].actAmt = FZOMBIE_ACT_TIME;
                    is_act = 1;
                }
            }

            if(!is_act) { //좀비가 플레이어를 못 봤다면
                int chooseDyDx = rand()%6;

                switch(chooseDyDx) {
                    case 0: ++ny; break;
                    case 1: --ny; break;
                    case 2: ++nx; break;
                    case 3: --nx; break;
                    default: break;
                }
            }
        }
        if(fzombies[i].actAmt > 0) { // 좀비 활성화 상태 움직임
            int chooseDyDx = rand()%2;

            if (chooseDyDx) {
                if (playerPoint.y < ny) ny--;
                else if (playerPoint.y > ny) ny++;
            }
            else {
                if (playerPoint.x < nx) nx--;
                else if (playerPoint.x > nx) nx++;
            }

            --(fzombies[i].actAmt);
        }

        // 이동 가능 체크 (벽, 다른 좀비, 플레이어 등)
        int can_move = 1;
        if ((map[ny][nx] & A_CHARTEXT) != GROUND) can_move = 0; // &로 문자 코드만 추출해서 땅이 아니면 이동 불가
        //다른 좀비와 겹치는지 확인
        for (int j = 0; j < fzombie_count; ++j) {
            if (i != j && fzombies[j].alive && fzombies[j].point.y == ny && fzombies[j].point.x == nx) {
                can_move = 0;
                break;
            }
        }
        //플레이어와 겹치는지 확인
        if (player.point.y == ny && player.point.x == nx) can_move = 0;
        //이동 가능하면 이동
        if (can_move) {
            map[fzombies[i].point.y][fzombies[i].point.x] = GROUND | COLOR_PAIR(1);
            fzombies[i].point.y = ny;
            fzombies[i].point.x = nx;
            if(fzombies[i].actAmt > 0) map[ny][nx] = FZOMBIE | COLOR_PAIR(110);
            else map[ny][nx] = FZOMBIE | COLOR_PAIR(10);
        }
    }
}
// 장애물 생성 함수
void make_obstacle(chtype map[MAP_HEIGHT][MAP_WIDTH]) {
    // 바위 패턴(3x3)
    int rock_pattern[3][3] = {
        {0,1,0},
        {1,1,1},
        {1,1,1}
    };
    // 물웅덩이 패턴(3x3)
    int water_pattern[3][3] = {
        {1,1,1},
        {1,1,1},
        {1,1,1}
    };
    // 바위(진한 회색, COLOR_PAIR(12))
    int rock_cnt = 0, try_limit = 10000;
    while(rock_cnt < ROCK_COUNT && try_limit--) {
        int y = rand() % (MAP_HEIGHT - 4) + 1;
        int x = rand() % (MAP_WIDTH - 4) + 1;
        int can_place = 1;
        for(int dy=0; dy<3; ++dy) for(int dx=0; dx<3; ++dx) {
            if(rock_pattern[dy][dx] && (map[y+dy][x+dx] & A_CHARTEXT) != GROUND) can_place = 0;
        }
        if(can_place) {
            for(int dy=0; dy<3; ++dy) for(int dx=0; dx<3; ++dx) {
                if(rock_pattern[dy][dx]) map[y+dy][x+dx] = ROCK | COLOR_PAIR(8) | A_BOLD;
            }
            rock_cnt++;
        }
    }
    // 물웅덩이(파란색, COLOR_PAIR(11))
    int water_cnt = 0;
    while(water_cnt < WATER_COUNT && try_limit--) {
        int y = rand() % (MAP_HEIGHT - 4) + 1;
        int x = rand() % (MAP_WIDTH - 4) + 1;
        int can_place = 1;
        for(int dy=0; dy<3; ++dy) for(int dx=0; dx<3; ++dx) {
            if(water_pattern[dy][dx] && (map[y+dy][x+dx] & A_CHARTEXT) != GROUND) can_place = 0;
        }
        if(can_place) {
            for(int dy=0; dy<3; ++dy) for(int dx=0; dx<3; ++dx) {
                if(water_pattern[dy][dx]) map[y+dy][x+dx] = POND | COLOR_PAIR(9);
            }
            water_cnt++;
        }
    }
}

int grab_item() { //아이템 랜덤 줍기
    int dx[] = {1,0,-1,0};
    int dy[] = {0,1,0,-1};
    for (int dir=0;dir<4;++dir) { //경계값 검사는 안해도 된다. 어차피 끝부분은 벽이다.
        int nx = player.point.x + dx[dir];
        int ny = player.point.y + dy[dir];
        if ((map[ny][nx] & A_CHARTEXT) == '!') {
            int k = rand()%3;
            if (!k) itemList.injection += 1;
            else if (k==1) itemList.pointer += 1;
            else itemList.patch += 1;
            map[ny][nx] = GROUND | COLOR_PAIR(1);
        }
    }
}

bool check_touch() {
    if (player.invincible) return 0; // 무적이면 데미지 없음

    int dx[] = {1,0,-1,0};
    int dy[] = {0,1,0,-1};
    for (int dir=0;dir<4;++dir) {
        int nx = player.point.x + dx[dir];
        int ny = player.point.y + dy[dir];
        char who = map[ny][nx] & A_CHARTEXT;
        if (who == 'z') {
            player.hp -= ZOMBIE_DAMAGE;
            if (player.hp < 0) you_die();
            // 무적 시작
            player.invincible = 1;
            clock_gettime(CLOCK_MONOTONIC, &player.invincible_end);
            player.invincible_end.tv_nsec += 500000000; // 0.5초
            if (player.invincible_end.tv_nsec >= 1000000000) {
                player.invincible_end.tv_sec += 1;
                player.invincible_end.tv_nsec -= 1000000000;
            }
            return 1;
        }
        else if (who == 'f') {
            player.hp -= FZOMBIE_DAMAGE;
            if (player.hp < 0) you_die();
            // 무적 시작
            player.invincible = 1;
            clock_gettime(CLOCK_MONOTONIC, &player.invincible_end);
            player.invincible_end.tv_nsec += 500000000; // 0.5초
            if (player.invincible_end.tv_nsec >= 1000000000) {
                player.invincible_end.tv_sec += 1;
                player.invincible_end.tv_nsec -= 1000000000;
            }
            return 1;
        }
    }
    return 0;
}
void you_die() {
    endwin(); // ncurses 모드 해제
    system("clear");
    printf("\n\n");
    printf(" __     ______  _    _   _      ____   _____ ______ \n");
    printf(" \\ \\   / / __ \\| |  | | | |    / __ \\ / ____|  ____|\n");
    printf("  \\ \\_/ / |  | | |  | | | |   | |  | | (___ | |__   \n");
    printf("   \\   /| |  | | |  | | | |   | |  | |\\___ \\|  __|  \n");
    printf("    | | | |__| | |__| | | |___| |__| |____) | |____ \n");
    printf("    |_|  \\____/ \\____/  |______\\____/|_____/|______|\n");
    printf("\n");
    printf("                GAME OVER...\n");
    printf("\n");
    printf("         당신은 좀비에게 잡아먹혔습니다...\n");
    printf("\n\n");
    printf("         다시 도전하려면 프로그램을 재시작하세요!\n");
    printf("\n\n");
    sleep(5);
    exit(0);
}

