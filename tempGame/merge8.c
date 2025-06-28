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

#define ZOMBIE_MAX 120 //좀비 개수 최소~최대
#define ZOMBIE_MIN 100

#define FZOMBIE_MAX 120 //빠른 좀비 개수 최소~최대 
#define FZOMBIE_MIN 100

#define ITEM_COUNT 150 //아이템 개수
#define VACCINE_COUNT 5 //각각 DEBUG 글자로 할 예정
#define TREE_COUNT 30 //트리 개수 30개
#define VIEW_HEIGHT 21 //플레이어 시야 크기
#define VIEW_WIDTH 51
#define VIEW_ANGLE 0.2f //플레이어 시야 각도
#define ROCK_COUNT 40 // 바위 개수
#define WATER_COUNT 30 // 물웅덩이 개수

#define NEAR_VIEW_SIZE 5 //주변 시야 범위
#define NEAR_VIEW_ADJ 5 //그래픽 조절값

#define NIGHT_VIEW_SIZE 4 // 밤 시야
#define NIGHT_NEAR_VIEW_SIZE 1 //밤 주변 시야

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
#define ZOMBIE_HP 4 //좀비 HP

#define FZOMBIE_ACT_TIME 20 //좀비 분노 지속 시간  
#define FZOMBIE_ACT_ADJ 5 //좀비 분노 조절값    
#define FZOMBIE_ACT_BASE_DISTANCE 7 //기본 소음 범위      
#define FZOMBIE_NOIZE_UNIT 1.0 //소음 발생 단위     
#define FZOMBIE_GENERAL_SPEED 0.15 //일반적인 좀비 속도         
#define FZOMBIE_CALM_DOWN_VAL 2.0 //소음이 줄어드는 속도    
#define FZOMBIE_ACT_LIMIT 11.0 //좀비 분노 한계
#define FZOMBIE_DAMAGE 15
#define FZOMBIE_HP 3 //빠른 좀비 HP

#define GUN ('#' | COLOR_PAIR(103)) //총 모양
#define INJECTION_RANGE 5 //인젝션 공격 범위 5x5가 기본
#define DAY_UNIT 25 //하루 시간

typedef enum {
	PLAYER = '8',
	ZOMBIE = 'z',
	FZOMBIE = 'f',
	ITEM = '!',
	
	CARPET = ',',
	GROUND = '.',
	SAND = 'd',
	LAWN = 'g',
	MOUNTAIN = 'm',
	
	WALL = '#',
	CORNER = 'X',
	GRASS = '*',
	DEAD_GRASS = 'Y',

	WATER = '~',
	ROCK = '@',
	
	BULLET_CHAR = '*',
	INJECTION = '$'
} type;

typedef enum {
	PLAYER_KEY = 1,
	ZOMBIE_KEY,
	FZOMBIE_KEY,
	ITEM_KEY,
	
	CARPET_KEY,
	GROUND_KEY,
	SAND_KEY,
	LAWN_KEY,
	MOUNTAIN_KEY,

	WALL_KEY,
	CORNER_KEY,
	GRASS_KEY,
	DEAD_GRASS_KEY,

	WATER_KEY,
	ROCK_KEY,

	BULLET_CHAR_KEY,
	INJECTION_KEY,
	
	BACKSHIN_KEY,
	CRAZY_ZOMBIE_KEY,
	CRAZY_FZOMBIE_KEY
} key;

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
typedef struct {
	int gun; //총알 수
	int vac[5]; //백신 D E B U G 배열 있으면 1, 없으면 0
	int injection; //인젝션 수 
	int packet; //패킷 수
    int UI_hp;
} UI_item;
UI_item item = {.gun = 30, .vac = {0}, .injection = 10, .packet = 1000,.UI_hp = 100};
chtype map[MAP_HEIGHT][MAP_WIDTH];
human player = {.role = PLAYER, .point = {21, 267}, .hp = 100, .lookDir = RIGHT};

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

int game_time = 0;
bool is_day = true;

void init_map_with_txt();
void swap_int(int* a, int* b); //정수값 swap
void init_UI(); //시작 UI
void spawn_object(chtype map[MAP_HEIGHT][MAP_WIDTH]); //맵 초기화
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
void game_UI();
void use_item(int);

int main() {
    system("clear");
    init_UI();
    initscr();
	noecho();
    nodelay(stdscr, TRUE);
    start_color();
    curs_set(0);
	
	//시야 안 - 낮 (0~)
	init_pair(PLAYER_KEY, 15, 240); //PLAYER: 파란 글자, 초록 배경
	init_pair(ZOMBIE_KEY, 88, 90); //ZOMBIE
	init_pair(FZOMBIE_KEY, 126, 125); //FZOMBIE
	init_pair(ITEM_KEY, 3, 11); //ITEM
	init_pair(CARPET_KEY, 245, 245); //CARPET
	init_pair(GROUND_KEY, 179, 179); //GROUND
	init_pair(SAND_KEY, 229, 229); //SAND
	init_pair(LAWN_KEY, 113, 113); //LAWN
	init_pair(MOUNTAIN_KEY, 237, 237); //MOUNTAIN
	init_pair(WALL_KEY, 255, 234); //WALL
	init_pair(CORNER_KEY, 251, 234); //CORNER
	init_pair(GRASS_KEY, 130, 28); //GRASS
	init_pair(DEAD_GRASS_KEY, 130, 229); //DEAD_GRASS
	init_pair(WATER_KEY, 39, 33); //WATER
	init_pair(ROCK_KEY, 241, 236); //ROCK
	init_pair(BULLET_CHAR_KEY, 165, 91); //BULLET_CHAR
	init_pair(INJECTION_KEY, 46, 22); //INGECTION
	init_pair(BACKSHIN_KEY, 50, 41); //백신
	init_pair(CRAZY_ZOMBIE_KEY, 9, 124); //CRAZY_ZOMBIE
	init_pair(CRAZY_FZOMBIE_KEY, 165, 89); //CRAZY_FZOMBIE
	
	//시야 밖 - 낮 (50~)
	init_pair(50+PLAYER_KEY, 20, 17); //PLAYER
    init_pair(50+ZOMBIE_KEY, 88, 89); //ZOMBIE
    init_pair(50+FZOMBIE_KEY, 126, 128); //FZOMBIE
    init_pair(50+ITEM_KEY, 184, 229); //ITEM
    init_pair(50+CARPET_KEY, 239, 239); //CARPET
    init_pair(50+GROUND_KEY, 94, 94); //GROUND
    init_pair(50+SAND_KEY, 144, 144); //SAND
    init_pair(50+LAWN_KEY, 70, 70); //LAWN
    init_pair(50+MOUNTAIN_KEY, 235, 235); //MOUNTAIN
    init_pair(50+WALL_KEY, 248, 234); //WALL
    init_pair(50+CORNER_KEY, 248, 234); //CORNER
    init_pair(50+GRASS_KEY, 28, 22); //GRASS
    init_pair(50+DEAD_GRASS_KEY, 130, 144); //DEAD_GRASS
    init_pair(50+WATER_KEY, 27, 26); //WATER
    init_pair(50+ROCK_KEY, 235, 235); //ROCK
    init_pair(50+BULLET_CHAR_KEY, 165, 162); //BULLET_CHAR
    init_pair(50+INJECTION_KEY, 46, 86); //INGECTION
    init_pair(50+BACKSHIN_KEY, 0, 0); //백신
    init_pair(50+CRAZY_ZOMBIE_KEY, 0, 0); //CRAZY_ZOMBIE
    init_pair(50+CRAZY_FZOMBIE_KEY, 0, 0); //CRAZY_FZOMBIE
	
	//시야 안 - 밤 (100~)
    init_pair(100+PLAYER_KEY, 255, 240); //PLAYER: 파란 글자, 초록 배경
    init_pair(100+ZOMBIE_KEY, 88, 89); //ZOMBIE
    init_pair(100+FZOMBIE_KEY, 126, 128); //FZOMBIE
    init_pair(100+ITEM_KEY, 3, 11); //ITEM
    init_pair(100+CARPET_KEY, 245, 245); //CARPET
    init_pair(100+GROUND_KEY, 179, 179); //GROUND
    init_pair(100+SAND_KEY, 229, 229); //SAND
    init_pair(100+LAWN_KEY, 113, 113); //LAWN
    init_pair(100+MOUNTAIN_KEY, 237, 237); //MOUNTAIN
    init_pair(100+WALL_KEY, 255, 234); //WALL
    init_pair(100+CORNER_KEY, 251, 234); //CORNER
    init_pair(100+GRASS_KEY, 130, 28); //GRASS
    init_pair(100+DEAD_GRASS_KEY, 130, 229); //DEAD_GRASS
    init_pair(100+WATER_KEY, 39, 33); //WATER
    init_pair(100+ROCK_KEY, 241, 236); //ROCK
    init_pair(100+BULLET_CHAR_KEY, 165, 91); //BULLET_CHAR
    init_pair(100+INJECTION_KEY, 46, 22); //INGECTION
    init_pair(100+BACKSHIN_KEY, 50, 41); //백신
    init_pair(100+CRAZY_ZOMBIE_KEY, 9, 124); //CRAZY_ZOMBIE
    init_pair(100+CRAZY_FZOMBIE_KEY, 165, 89); //CRAZY_FZOMBIE

    //시야 밖 - 밤 (150~)
    init_pair(150+PLAYER_KEY, 20, 17); //PLAYER
    init_pair(150+ZOMBIE_KEY, 88, 89); //ZOMBIE
    init_pair(150+FZOMBIE_KEY, 126, 128); //FZOMBIE
    init_pair(150+ITEM_KEY, 184, 229); //ITEM
    init_pair(150+CARPET_KEY, 237, 237); //CARPET
    init_pair(150+GROUND_KEY, 94, 94); //GROUND
    init_pair(150+SAND_KEY, 144, 144); //SAND
    init_pair(150+LAWN_KEY, 22, 22); //LAWN
    init_pair(150+MOUNTAIN_KEY, 235, 235); //MOUNTAIN
    init_pair(150+WALL_KEY, 244, 232); //WALL
    init_pair(150+CORNER_KEY, 244, 232); //CORNER
    init_pair(150+GRASS_KEY, 28, 22); //GRASS
    init_pair(150+DEAD_GRASS_KEY, 130, 144); //DEAD_GRASS
    init_pair(150+WATER_KEY, 20, 19); //WATER
    init_pair(150+ROCK_KEY, 236, 232); //ROCK
    init_pair(150+BULLET_CHAR_KEY, 165, 162); //BULLET_CHAR
    init_pair(150+INJECTION_KEY, 46, 86); //INGECTION
    init_pair(150+BACKSHIN_KEY, 0, 0); //백신
    init_pair(150+CRAZY_ZOMBIE_KEY, 0, 0); //CRAZY_ZOMBIE
    init_pair(150+CRAZY_FZOMBIE_KEY, 0, 0); //CRAZY_FZOMBIE

	//UI (200~)
	init_pair(200+1, COLOR_BLACK, COLOR_RED); // 빨간색 글자, 빨간색 배경 : HP
    init_pair(200+2, COLOR_BLACK, COLOR_GREEN);
	init_pair(200+3, COLOR_BLACK, 226);
	init_pair(200+4, COLOR_BLACK, 245);
	init_pair(200+5, COLOR_BLACK, 238);
	init_pair(200+6, COLOR_BLACK, 14);

	//시작하고 출력
    init_map_with_txt();
   	spawn_object(map);

	//좀비 시간
	struct timespec last_zombie_move, now; //초+나노초로 시간 저장하는 구조체
    clock_gettime(CLOCK_MONOTONIC, &last_zombie_move); //현재 시간 구조체에 저장
    struct timespec flast_zombie_move, fnow; //초+나노초로 시간 저장하는 구조체
    clock_gettime(CLOCK_MONOTONIC, &flast_zombie_move); //현재 시간 구조체에 저장
	
	//게임 시간
	struct timespec last_game_time, now_game_time;     
	clock_gettime(CLOCK_MONOTONIC, &last_game_time); 

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
        game_UI();
        item.UI_hp = player.hp; //UI에 hp 업데이트
        if (player.hp <= 0) you_die();
		
		//mvprintw(24, 0, "noize val: %lf\nzombie speed: %lf\n", zombie_act_distance, zombie_speed);
        //mvprintw(27, 0, "hp: %d\ngun : %d\ninjection : %d\npacket : %d", item.UI_hp,item.gun,item.injection,item.packet);

		//mvprintw(22, 0, "game_time(change day&night per %d): %d, is_day: %d\n", DAY_UNIT, game_time, is_day);	

		refresh();
        bool isTouched = check_touch();

        int key = getch(); //키보드 입력 받기
        if(key != -1){ //키보드 입력이 있다면
            if (key=='i') grab_item();
			else {
				use_item(key);
			}

            if(zombie_act_distance <= ZOMBIE_ACT_LIMIT) {
                zombie_act_distance += ZOMBIE_NOIZE_UNIT;
            }
			if(fzombie_act_distance <= FZOMBIE_ACT_LIMIT) {
                fzombie_act_distance += FZOMBIE_NOIZE_UNIT;
            }

			int pastY = player.point.y, pastX = player.point.x;
            HumanMove(&player, key);
            switch(map[player.point.y][player.point.x] & A_CHARTEXT) {
                case CARPET : map[pastY][pastX] = CARPET | COLOR_PAIR(CARPET_KEY); break;
                case GROUND : map[pastY][pastX] = GROUND | COLOR_PAIR(GROUND_KEY); break;
                case SAND : map[pastY][pastX] = SAND | COLOR_PAIR(SAND_KEY); break;
                case LAWN : map[pastY][pastX] = LAWN | COLOR_PAIR(LAWN_KEY); break;
                case MOUNTAIN : map[pastY][pastX] = MOUNTAIN | COLOR_PAIR(MOUNTAIN_KEY); break;
            }
            map[player.point.y][player.point.x] =  PLAYER | COLOR_PAIR(PLAYER_KEY) | A_BOLD;
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

		clock_gettime(CLOCK_MONOTONIC, &now_game_time);
        double game_time_elapsed = (now_game_time.tv_sec - last_game_time.tv_sec) + (now_game_time.tv_nsec - last_game_time.tv_nsec) / 1e9;
        if (game_time_elapsed > 1.0) {
			game_time += 1;
            last_game_time = now_game_time; //현재 시간 구조체에 저장
        
			if(game_time % DAY_UNIT == 0) {
            	is_day = !is_day;

				if(is_day) {
                    if(can_change_color()) {
                    }
                }
                else    {
                    if(can_change_color()) {
                    }
                }
        	}
		}

        struct timespec mnow;
        clock_gettime(CLOCK_MONOTONIC, &mnow);
        if (player.invincible) {
            if ((mnow.tv_sec > player.invincible_end.tv_sec) ||
                (mnow.tv_sec == player.invincible_end.tv_sec && now.tv_nsec > player.invincible_end.tv_nsec)) {
                player.invincible = 0; // 무적 해제
            }
        }
    }
    endwin();
    return 0;
}

void init_map_with_txt() {
	FILE *fp = fopen("world_map.txt", "r");
    if (fp == NULL) {
        perror("파일 열기 실패");
        return;
    }

    char line[MAP_WIDTH + 2]; // 한 줄을 읽어올 버퍼. +2는 \n과 \0 문자를 위함.
    for (int i = 0; i < MAP_HEIGHT; i++) {
        if (fgets(line, sizeof(line), fp) == NULL) {
            // 파일의 끝에 도달했거나 오류 발생 시 중단
            break;
        }

        for (int j = 0; j < MAP_WIDTH; j++) {
            char c = line[j];

            // 파일에서 읽어온 줄의 끝에 도달하면
            if (c == '\n' || c == '\r' || c == '\0') {
                // 해당 줄의 나머지 맵 공간을 GROUND로 채움
                for (int k = j; k < MAP_WIDTH; k++) {
                    map[i][k] = GROUND | COLOR_PAIR(GROUND_KEY);
                }
                break; // 다음 줄로 넘어감
            }
			
			switch(c) {
				case CARPET : map[i][j] = CARPET | COLOR_PAIR(CARPET_KEY); break; 
				case GROUND : map[i][j] = GROUND | COLOR_PAIR(GROUND_KEY); break;
				case SAND : map[i][j] = SAND | COLOR_PAIR(SAND_KEY); break;
				case LAWN : map[i][j] = LAWN | COLOR_PAIR(LAWN_KEY); break;
				case MOUNTAIN : map[i][j] = MOUNTAIN | COLOR_PAIR(MOUNTAIN_KEY); break;
				case WALL : map[i][j] = WALL | COLOR_PAIR(WALL_KEY); break;
				case CORNER : map[i][j] = CORNER | COLOR_PAIR(CORNER_KEY); break;
				case GRASS : map[i][j] = GRASS | COLOR_PAIR(GRASS_KEY); break;
				case DEAD_GRASS : map[i][j] = DEAD_GRASS | COLOR_PAIR(DEAD_GRASS_KEY); break;
				case WATER : map[i][j] = WATER | COLOR_PAIR(WATER_KEY); break;
				case ROCK : map[i][j] = ROCK | COLOR_PAIR(ROCK_KEY); break;
                case 'D' : map[i][j] = 'D' | COLOR_PAIR(BACKSHIN_KEY); break;
				case 'E' : map[i][j] = 'E' | COLOR_PAIR(BACKSHIN_KEY); break;
				case 'B' : map[i][j] = 'B' | COLOR_PAIR(BACKSHIN_KEY); break;
				case 'U' : map[i][j] = 'U' | COLOR_PAIR(BACKSHIN_KEY); break;
				case 'G' : map[i][j] = 'G' | COLOR_PAIR(BACKSHIN_KEY); break;
				default:
                    map[i][j] = GROUND | COLOR_PAIR(GROUND_KEY); break;
			}
        }
    }

    fclose(fp);
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

//플레이어 이동
void HumanMove(human *player, int way) {
    if(way == UP){ // 'w' 누르면
        if(
			(map[(player -> point).y - 1][(player -> point).x] & A_CHARTEXT) == CARPET ||
			(map[(player -> point).y - 1][(player -> point).x] & A_CHARTEXT) == GROUND ||
			(map[(player -> point).y - 1][(player -> point).x] & A_CHARTEXT) == SAND ||
			(map[(player -> point).y - 1][(player -> point).x] & A_CHARTEXT) == LAWN ||
			(map[(player -> point).y - 1][(player -> point).x] & A_CHARTEXT) == MOUNTAIN
		){
            (player -> point).y--;
            player -> lookDir = UP;
        }
    }
    else if(way == RIGHT){ // 'd' 누르면
        if(
			(map[(player -> point).y][(player -> point).x + 1] & A_CHARTEXT) == CARPET ||
            (map[(player -> point).y][(player -> point).x + 1] & A_CHARTEXT) == GROUND ||
            (map[(player -> point).y][(player -> point).x + 1] & A_CHARTEXT) == SAND ||
            (map[(player -> point).y][(player -> point).x + 1] & A_CHARTEXT) == LAWN ||
            (map[(player -> point).y][(player -> point).x + 1] & A_CHARTEXT) == MOUNTAIN
		){
            (player -> point).x++;
            player -> lookDir = RIGHT;
        }
    }
    else if(way == DOWN){ // 's' 누르면
        if(
			(map[(player -> point).y + 1][(player -> point).x] & A_CHARTEXT) == CARPET ||
            (map[(player -> point).y + 1][(player -> point).x] & A_CHARTEXT) == GROUND ||
            (map[(player -> point).y + 1][(player -> point).x] & A_CHARTEXT) == SAND ||
            (map[(player -> point).y + 1][(player -> point).x] & A_CHARTEXT) == LAWN ||
            (map[(player -> point).y + 1][(player -> point).x] & A_CHARTEXT) == MOUNTAIN
		){
            (player -> point).y++;
            player -> lookDir = DOWN;
        }
    }
    else if(way == LEFT){ // 'a' 누르면
        if(
			(map[(player -> point).y][(player -> point).x - 1] & A_CHARTEXT) == CARPET ||
            (map[(player -> point).y][(player -> point).x - 1] & A_CHARTEXT) == GROUND ||
            (map[(player -> point).y][(player -> point).x - 1] & A_CHARTEXT) == SAND ||
            (map[(player -> point).y][(player -> point).x - 1] & A_CHARTEXT) == LAWN ||
            (map[(player -> point).y][(player -> point).x - 1] & A_CHARTEXT) == MOUNTAIN
		){
            (player -> point).x--;
            player -> lookDir = LEFT;
        }
    }
}

void spawn_object(chtype map[MAP_HEIGHT][MAP_WIDTH]) {
    //좀비 랜덤 생성 : ZOMBIE_MIN ~ ZOMBIE_MAX
    int zombieCount = rand() % (ZOMBIE_MAX - ZOMBIE_MIN + 1) + ZOMBIE_MIN;
    zombie_count = zombieCount;
    int zidx = 0;
    for (int z=0;z<zombieCount;++z) {
        int y,x;
        do {
            y = rand() % (MAP_HEIGHT - 2) + 1; //벽은 제외
            x = rand() % (MAP_WIDTH  - 2) + 1;
        } while (
            (map[y][x] & A_CHARTEXT) != CARPET &&
            (map[y][x] & A_CHARTEXT) != GROUND &&
            (map[y][x] & A_CHARTEXT) != SAND &&
            (map[y][x] & A_CHARTEXT) != LAWN &&
            (map[y][x] & A_CHARTEXT) != MOUNTAIN
        ); //빈 공간이어야함
        map[y][x] = ZOMBIE | COLOR_PAIR(ZOMBIE_KEY);
        //좀비 구조체 배열에 좀비 좌표 저장
        zombies[zidx].point.y = y;
        zombies[zidx].point.x = x;
        zombies[zidx].alive = 1;
        zombies[zidx].hp = ZOMBIE_HP;
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
        } while (
            (map[y][x] & A_CHARTEXT) != CARPET &&
            (map[y][x] & A_CHARTEXT) != GROUND &&
            (map[y][x] & A_CHARTEXT) != SAND &&
            (map[y][x] & A_CHARTEXT) != LAWN &&
            (map[y][x] & A_CHARTEXT) != MOUNTAIN
        ); //빈 공간이어야함
        map[y][x] = FZOMBIE | COLOR_PAIR(FZOMBIE_KEY);
        //좀비 구조체 배열에 좀비 좌표 저장
        fzombies[fzidx].point.y = y;
        fzombies[fzidx].point.x = x;
        fzombies[fzidx].alive = 1;
        fzombies[fzidx].hp = FZOMBIE_HP;
        fzombies[fzidx].actAmt = 0;
        fzidx++; //인덱스 증가
    }
    //아이템 랜덤 생성 : ITEM_COUNT
    for (int it=0;it<ITEM_COUNT;++it) {
        int y,x;
        do {
            y = rand() % (MAP_HEIGHT - 2) + 1;
            x = rand() % (MAP_WIDTH  - 2) + 1;
        } while (
            (map[y][x] & A_CHARTEXT) != CARPET &&
            (map[y][x] & A_CHARTEXT) != GROUND &&
            (map[y][x] & A_CHARTEXT) != SAND &&
            (map[y][x] & A_CHARTEXT) != LAWN &&
            (map[y][x] & A_CHARTEXT) != MOUNTAIN
        );
        map[y][x] = ITEM | COLOR_PAIR(ITEM_KEY) | A_BLINK;
    }
    map[player.point.y][player.point.x] = PLAYER | COLOR_PAIR(PLAYER_KEY) | A_BOLD; //플레이어
}

//플레이어 시야 그리기 (ray casting 방식)
void draw_view_map(chtype map[MAP_HEIGHT][MAP_WIDTH], human player){
    // 시야가 닿지 않는 곳은 어둡게 처리
    int y = 0, x = 0;
    for (int i = startPoint.y; i <= endPoint.y; ++i) {
        for (int j = startPoint.x; j <= endPoint.x; ++j) {
            // 플레이어 위치면 플레이어만 출력
            if (i == player.point.y && j == player.point.x) {
                mvaddch(y, x, '8' | COLOR_PAIR(PLAYER_KEY) | A_BOLD);
            } else {
                char c = map[i][j] & A_CHARTEXT;
                if(is_day) {
                    switch(c) {
                        case CARPET : mvaddch(y, x, CARPET | COLOR_PAIR(CARPET_KEY + 50)); break;
                        case GROUND : mvaddch(y, x, GROUND | COLOR_PAIR(GROUND_KEY + 50)); break;
                        case SAND : mvaddch(y, x, SAND | COLOR_PAIR(SAND_KEY + 50)); break;
                        case LAWN : mvaddch(y, x, LAWN | COLOR_PAIR(LAWN_KEY + 50)); break;
                        case MOUNTAIN : mvaddch(y, x, MOUNTAIN | COLOR_PAIR(MOUNTAIN_KEY + 50)); break;
                        case WALL : mvaddch(y, x, WALL | COLOR_PAIR(WALL_KEY + 50)); break;
                        case CORNER : mvaddch(y, x, CORNER | COLOR_PAIR(CORNER_KEY + 50)); break;
                        case GRASS : mvaddch(y, x, GRASS | COLOR_PAIR(GRASS_KEY + 50)); break;
                        case DEAD_GRASS : mvaddch(y, x, DEAD_GRASS | COLOR_PAIR(DEAD_GRASS_KEY + 50)); break;
                        case WATER : mvaddch(y, x, WATER | COLOR_PAIR(WATER_KEY + 50)); break;
                        case ROCK : mvaddch(y, x, ROCK | COLOR_PAIR(ROCK_KEY + 50)); break;
                    }
                } else {
                    switch(c) {
                        case CARPET : mvaddch(y, x, CARPET | COLOR_PAIR(CARPET_KEY + 150)); break;
                        case GROUND : mvaddch(y, x, GROUND | COLOR_PAIR(GROUND_KEY + 150)); break;
                        case SAND : mvaddch(y, x, SAND | COLOR_PAIR(SAND_KEY + 150)); break;
                        case LAWN : mvaddch(y, x, LAWN | COLOR_PAIR(LAWN_KEY + 150)); break;
                        case MOUNTAIN : mvaddch(y, x, MOUNTAIN | COLOR_PAIR(MOUNTAIN_KEY + 150)); break;
                        case WALL : mvaddch(y, x, WALL | COLOR_PAIR(WALL_KEY + 150)); break;
                        case CORNER : mvaddch(y, x, CORNER | COLOR_PAIR(CORNER_KEY + 150)); break;
                        case GRASS : mvaddch(y, x, GRASS | COLOR_PAIR(GRASS_KEY + 150)); break;
                        case DEAD_GRASS : mvaddch(y, x, DEAD_GRASS | COLOR_PAIR(DEAD_GRASS_KEY + 150)); break;
                        case WATER : mvaddch(y, x, WATER | COLOR_PAIR(WATER_KEY + 150)); break;
                        case ROCK : mvaddch(y, x, ROCK | COLOR_PAIR(ROCK_KEY + 150)); break;
                    }
                }
            }
            ++x;
        }
        x = 0;
        ++y;
    }

	//mvaddch(player.point.y - startPoint.y, player.point.x - startPoint.x, map[player.point.y][player.point.x]);

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
	if(!is_day) {
		if(view_start_angle > view_end_angle) {  
			for(double angle = 0.0; angle < view_end_angle; angle += 0.1) {
            	line_view(player.point, angle, NIGHT_VIEW_SIZE, NULL);
        	}
        	for(double angle = 0.0; angle < view_start_angle; angle += 0.1) {
            	line_view(player.point, angle, NIGHT_NEAR_VIEW_SIZE, NULL);
        	}
        	for(double angle = view_start_angle; angle < 360.0; angle += 0.1) {
            	line_view(player.point, angle, NIGHT_VIEW_SIZE, NULL);
        	}
    	}
    	else {
        	for(double angle = 0.0; angle < view_start_angle; angle += 0.1) {
            	line_view(player.point, angle, NIGHT_NEAR_VIEW_SIZE, NULL);
        	}
        	for(double angle = view_start_angle; angle < view_end_angle; angle += 0.1) {
            	line_view(player.point, angle, NIGHT_VIEW_SIZE, NULL);
        	}
        	for(double angle = view_end_angle; angle < 360.0; angle += 0.1) {
            	line_view(player.point, angle, NIGHT_NEAR_VIEW_SIZE, NULL);
        	}
    	}

		return;
	}

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
		if(
			(map[ry][rx] & A_CHARTEXT) == WALL || 
			(map[ry][rx] & A_CHARTEXT) == CORNER ||
			(map[ry][rx] & A_CHARTEXT) == GRASS ||
			(map[ry][rx] & A_CHARTEXT) == DEAD_GRASS ||
			(map[ry][rx] & A_CHARTEXT) == ROCK
		) { //장애물 감지
        	if(r != NULL) *r = 1;
			break;
		}

		double nty = a * (tx + quadrant.x);
		while(abs((int)nty - (int)ty) >= 1) {
			if(
            	(map[ry][rx] & A_CHARTEXT) == WALL ||
            	(map[ry][rx] & A_CHARTEXT) == CORNER ||
            	(map[ry][rx] & A_CHARTEXT) == GRASS ||
            	(map[ry][rx] & A_CHARTEXT) == DEAD_GRASS ||
            	(map[ry][rx] & A_CHARTEXT) == ROCK
        	) { //장애물 감지
            	if(r != NULL) *r = 1;
            	return;
        	}
			
			if((startPoint.y >= ry) || (ry >= endPoint.y) || (startPoint.x >= rx) || (rx >= endPoint.x) || (sqrt((ty*ty) + (tx*tx/NEAR_VIEW_ADJ)) > distance)) { //화면 이탈 방지와 거리 이탈 방지
                return;
            }

			ty += quadrant.y;
			ry = point.y - (int)ty;
        	rx = point.x + (int)tx;
			
			if(r == NULL) {
                if((startPoint.y <= ry) && (ry <= endPoint.y) && (startPoint.x <= rx) && (rx <= endPoint.x)) {
                    mvaddch(ry - startPoint.y, rx - startPoint.x, map[ry][rx]);
                }
            }
		}
		
		if(sqrt((ty*ty) + (tx*tx/NEAR_VIEW_ADJ)) > distance) { //거리 이탈 방지
            return;
        }

		tx += quadrant.x;
		ty = a * tx;
        ry = point.y - (int)ty;
        rx = point.x + (int)tx;
		
		if(r == NULL) {
            if((startPoint.y <= ry) && (ry <= endPoint.y) && (startPoint.x <= rx) && (rx <= endPoint.x)) {
                mvaddch(ry - startPoint.y, rx - startPoint.x, map[ry][rx]);
            }
        }
	}
}
//좀비 움직이는 함수 : zombie_count으로 모든 좀비 하나하나씩 이동방향을 y축 우선으로 결정하여 이동가능한지 체크하고 옮긴다.
void move_zombies(chtype map[MAP_HEIGHT][MAP_WIDTH], yx playerPoint) {
    for (int i = 0; i < zombie_count; ++i) {
        if (!zombies[i].alive) continue; //좀비가 죽은 놈이면 끝
        //시야 내에 있는지 확인
        int ny = zombies[i].point.y;
        int nx = zombies[i].point.x;

		int pastY = ny;
		int pastX = nx;		

		if(zombies[i].actAmt == 0) { // 좀비 비활성화 상태 움직임
			int is_act = 0;

			double distance = sqrt((ny - player.point.y) * (ny - player.point.y) + (nx - player.point.x) * (nx - player.point.x)); //zombie와 플레이어 거리계산 (수정 필요) 
			if((ny - player.point.y) * (ny - player.point.y) + (nx - player.point.x) * (nx - player.point.x)/ZOMBIE_ACT_ADJ <= zombie_act_distance*zombie_act_distance) { //좀비가 플레이어를 보았는지 확인하는 코드
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
        if (
			(map[ny][nx] & A_CHARTEXT) != CARPET &&
			(map[ny][nx] & A_CHARTEXT) != GROUND &&
			(map[ny][nx] & A_CHARTEXT) != SAND &&
			(map[ny][nx] & A_CHARTEXT) != LAWN &&
			(map[ny][nx] & A_CHARTEXT) != MOUNTAIN
		) can_move = 0; // &로 문자 코드만 추출해서 땅이 아니면 이동 불가
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
           	switch(map[ny][nx] & A_CHARTEXT) {
                case CARPET : map[pastY][pastX] = CARPET | COLOR_PAIR(CARPET_KEY); break;
                case GROUND : map[pastY][pastX] = GROUND | COLOR_PAIR(GROUND_KEY); break;
                case SAND : map[pastY][pastX] = SAND | COLOR_PAIR(SAND_KEY); break;
                case LAWN : map[pastY][pastX] = LAWN | COLOR_PAIR(LAWN_KEY); break;
                case MOUNTAIN : map[pastY][pastX] = MOUNTAIN | COLOR_PAIR(MOUNTAIN_KEY); break;
            }

			zombies[i].point.y = ny;
            zombies[i].point.x = nx;
            if(zombies[i].actAmt > 0) map[ny][nx] = ZOMBIE | COLOR_PAIR(CRAZY_ZOMBIE_KEY);
			else map[ny][nx] = ZOMBIE | COLOR_PAIR(ZOMBIE_KEY);
        }
    }
}

void move_fzombies(chtype map[MAP_HEIGHT][MAP_WIDTH], yx playerPoint) {
    for (int i = 0; i < zombie_count; ++i) {
        if (!fzombies[i].alive) continue; //좀비가 죽은 놈이면 끝
        //시야 내에 있는지 확인
        int ny = fzombies[i].point.y;
        int nx = fzombies[i].point.x;

        int pastY = ny;
        int pastX = nx;

        if(fzombies[i].actAmt == 0) { // 좀비 비활성화 상태 움직임
            int is_act = 0;

            double distance = sqrt((ny - player.point.y) * (ny - player.point.y) + (nx - player.point.x) * (nx - player.point.x)); //zombie와 플레이어 거리계산 (수정 필요)
            if((ny - player.point.y) * (ny - player.point.y) + (nx - player.point.x) * (nx - player.point.x)/FZOMBIE_ACT_ADJ <= fzombie_act_distance*fzombie_act_distance) { //좀비가 플레이어를 보았는지 확인하는 코드
                double angle = atan2(ny - player.point.y, nx - player.point.x) * (180.0 / M_PI) + 180.0;
                int is_blocked = 0;
                line_view(zombies[i].point, angle, distance, &is_blocked);

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
        if (
            (map[ny][nx] & A_CHARTEXT) != CARPET &&
            (map[ny][nx] & A_CHARTEXT) != GROUND &&
            (map[ny][nx] & A_CHARTEXT) != SAND &&
            (map[ny][nx] & A_CHARTEXT) != LAWN &&
            (map[ny][nx] & A_CHARTEXT) != MOUNTAIN
        ) can_move = 0; // &로 문자 코드만 추출해서 땅이 아니면 이동 불가
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
			switch(map[ny][nx] & A_CHARTEXT) { 
				case CARPET : map[pastY][pastX] = CARPET | COLOR_PAIR(CARPET_KEY); break;   
				case GROUND : map[pastY][pastX] = GROUND | COLOR_PAIR(GROUND_KEY); break;   
				case SAND : map[pastY][pastX] = SAND | COLOR_PAIR(SAND_KEY); break;  
				case LAWN : map[pastY][pastX] = LAWN | COLOR_PAIR(LAWN_KEY); break;   
				case MOUNTAIN : map[pastY][pastX] = MOUNTAIN | COLOR_PAIR(MOUNTAIN_KEY); break;   
			}

            fzombies[i].point.y = ny;
            fzombies[i].point.x = nx;
            if(fzombies[i].actAmt > 0) map[ny][nx] = FZOMBIE | COLOR_PAIR(CRAZY_FZOMBIE_KEY);
            else map[ny][nx] = FZOMBIE | COLOR_PAIR(FZOMBIE_KEY);
        }
    }
}

int grab_item() { //아이템 랜덤 줍기
    int dx[] = {1,0,-1,0};
    int dy[] = {0,1,0,-1};
    for (int dir=0;dir<4;++dir) { //경계값 검사는 안해도 된다. 어차피 끝부분은 벽이다.
        int nx = player.point.x + dx[dir];
        int ny = player.point.y + dy[dir];
        char cur = map[ny][nx] & A_CHARTEXT;
        if (cur == '!') {
            int k = rand()%3;
            if (!k) item.gun += 20;
            else if (k==1) item.injection += 2;
            else item.packet += 1;
            map[ny][nx] = GROUND | COLOR_PAIR(GROUND_KEY);
        }
        else if (cur=='D') {
            item.vac[0] += 1;
			map[ny][nx] = GROUND | COLOR_PAIR(GROUND_KEY);
        }
        else if (cur=='E') {
            item.vac[1] += 1;
			map[ny][nx] = GROUND | COLOR_PAIR(GROUND_KEY);
        }
        else if (cur=='B') {
            item.vac[2] += 1;
			map[ny][nx] = GROUND | COLOR_PAIR(GROUND_KEY);
        }
        else if (cur=='U') {
            item.vac[3] += 1;
			map[ny][nx] = GROUND | COLOR_PAIR(GROUND_KEY);
        }
        else if (cur=='G') {
            item.vac[4] += 1;
			map[ny][nx] = GROUND | COLOR_PAIR(GROUND_KEY);
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
        if (who == ZOMBIE) {
            player.hp -= ZOMBIE_DAMAGE;
            // 무적 시작
            player.invincible = 1;
            clock_gettime(CLOCK_MONOTONIC, &player.invincible_end);
            player.invincible_end.tv_nsec += 1000000000; // 0.5초
            if (player.invincible_end.tv_nsec >= 1000000000) {
                player.invincible_end.tv_sec += 1;
                player.invincible_end.tv_nsec -= 1000000000;
            }
            return 1;
        }
        else if (who == FZOMBIE) {
            player.hp -= FZOMBIE_DAMAGE;
            // 무적 시작
            player.invincible = 1;
            clock_gettime(CLOCK_MONOTONIC, &player.invincible_end);
            player.invincible_end.tv_nsec += 1000000000; // 0.5초
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

void game_UI() {
    //HP바
    mvprintw(1, VIEW_WIDTH + 2, "HP");
    //DAYNNIGHT BAR
    mvprintw(4, VIEW_WIDTH + 2, "DAY & NIGHT");

    int hp_pixel = 28 * (player.hp/100.0);
    for(int i = 2; i <= 3; ++i) {
        for(int j = VIEW_WIDTH + 2; j < VIEW_WIDTH + 2 + hp_pixel; ++j) {
            mvaddch(i, j, ' ' | COLOR_PAIR(200+1));
        }
        for(int j = VIEW_WIDTH+ 2 + hp_pixel; j < VIEW_WIDTH + 2 + 28; ++j) {
            mvaddch(i, j, ' ' | COLOR_PAIR(200+4));
        }
    }

    int daynight_pixel = 28 * ((game_time%DAY_UNIT)/(double)DAY_UNIT);
    for(int i = 5; i <= 5; ++i) {
        if(is_day) {
            for(int j = VIEW_WIDTH + 2; j < VIEW_WIDTH + 2 + daynight_pixel; ++j) {
                mvaddch(i, j, ' ' | COLOR_PAIR(200+3));
            }
        }
        else {
            for(int j = VIEW_WIDTH + 2; j < VIEW_WIDTH + 2 + daynight_pixel; ++j) {
                mvaddch(i, j, ' ' | COLOR_PAIR(200+5));
            }
        }

        for(int j = VIEW_WIDTH + 2 + daynight_pixel; j < VIEW_WIDTH + 2 + 28; ++j) {
            mvaddch(i, j, ' ' | COLOR_PAIR(200+4));
        }
    }
    
    //아이템칸
    for(int i = 7; i <= 10; ++i) {
		/*
        for(int j = 0; j <= 7; j++) {
            mvaddch(i, VIEW_WIDTH + j + 2, '.' | COLOR_PAIR(200+4));
        }

        mvaddch(7, VIEW_WIDTH + 2 + 1, '_' | COLOR_PAIR(BULLET_CHAR_KEY));
        mvaddch(7, VIEW_WIDTH + 2 + 6, '_' | COLOR_PAIR(BULLET_CHAR_KEY));
        for(int j = 0; j <= 6; ++j) mvaddch(8, VIEW_WIDTH + 2 + j, GUN);
        mvaddch(8, VIEW_WIDTH + 2 + 7, 'P' | COLOR_PAIR(BULLET_CHAR_KEY));

        mvaddch(9, VIEW_WIDTH + 2, '/' | COLOR_PAIR(BULLET_CHAR_KEY));
        mvaddch(9, VIEW_WIDTH + 2 + 4, '/' | COLOR_PAIR(BULLET_CHAR_KEY));
        mvaddch(10, VIEW_WIDTH + 2 + 2, '/' | COLOR_PAIR(BULLET_CHAR_KEY));

        mvaddch(9, VIEW_WIDTH + 2 + 1, GUN);
        mvaddch(9, VIEW_WIDTH + 2 + 2, GUN);
        mvaddch(10, VIEW_WIDTH + 2, GUN);
        mvaddch(10, VIEW_WIDTH + 2 + 1, GUN);

        mvaddch(9, VIEW_WIDTH + 2 + 3, 'Z' | COLOR_PAIR(BULLET_CHAR_KEY));

        mvaddch(10, VIEW_WIDTH + 2 + 6, (char)(item.gun / 10 + '0') | COLOR_PAIR(BULLET_CHAR_KEY));
        mvaddch(10, VIEW_WIDTH + 2 + 7, (char)(item.gun % 10 + '0') | COLOR_PAIR(BULLET_CHAR_KEY));
		*/

		for(int j = 0; j <= 7; j++) {
            mvaddch(i, VIEW_WIDTH + j + 2, '.' | COLOR_PAIR(BULLET_CHAR_KEY));
        }
        mvaddch(10, VIEW_WIDTH + 2 + 6, (char)(item.gun / 10 + '0') | COLOR_PAIR(200+4));
        mvaddch(10, VIEW_WIDTH + 2 + 7, (char)(item.gun % 10 + '0') | COLOR_PAIR(200+4));
		for(int j = 0; j <= 7; j++) {
            mvaddch(i, VIEW_WIDTH + j + 2 + 10, '.' | COLOR_PAIR(INJECTION_KEY));
        }
        mvaddch(10, VIEW_WIDTH + 2 + 6 + 10, (char)(item.injection / 10 + '0') | COLOR_PAIR(200+4));
        mvaddch(10, VIEW_WIDTH + 2 + 7 + 10, (char)(item.injection % 10 + '0') | COLOR_PAIR(200+4));
        for(int j = 0; j <= 7; j++) {
            mvaddch(i, VIEW_WIDTH + j + 2 + 20, '.' | COLOR_PAIR(200+6));
        }
        mvaddch(10, VIEW_WIDTH + 2 + 6 + 20, (char)(item.packet / 10 + '0') | COLOR_PAIR(200+4));
        mvaddch(10, VIEW_WIDTH + 2 + 7 + 20, (char)(item.packet % 10 + '0') | COLOR_PAIR(200+4));
    }

    //모은 백신 조각

    int vac_list_a[5][14] = { {13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 17},{13, 13, 13, 13, 14, 15, 15, 15, 15, 16, 17, 17, 17, 17},{13, 13, 13, 14, 14, 15, 15, 15, 16, 16, 17, 17, 17},{13, 13, 14, 14, 15, 15, 16, 16, 17, 17},{13, 13, 14, 15, 15, 15, 16, 16, 17, 17} };
    int vac_list_b[5][14] = { {0, 1, 2, 0, 3, 0, 3, 0, 3, 0, 1, 2}, {0, 1, 2, 3, 0, 0, 1, 2, 3, 0, 1, 2, 3}, {0, 1, 2, 0, 3, 0, 1, 2, 0, 3, 0, 1, 2}, {0, 3, 0, 3, 0, 3, 0, 3, 1, 2}, {1, 2, 0, 0, 2, 3, 0, 3, 1, 2} };


    for(int i = 0; i < 5; ++i) {
        if(!item.vac[i]) {
            for(int j = 0; j < 14; ++j) {
                if(vac_list_a[i][j] == 0) break;
                mvaddch(vac_list_a[i][j] - 1, VIEW_WIDTH + 2 + vac_list_b[i][j] + i * 6, '.' | COLOR_PAIR(200+4));
            }
        }
        else {
            for(int j = 0; j < 14; ++j) {
                if(vac_list_a[i][j] == 0) break;
                mvaddch(vac_list_a[i][j] - 1, VIEW_WIDTH + 2 + vac_list_b[i][j] + i * 6, '.' | COLOR_PAIR(200+2));
            }
        }
    }
}

void use_item(int item_num) {
    if(item_num == 'j') {
        if(item.gun >= 1) { 
            // 총알 1개 사용
            --item.gun;

            // 플레이어가 바라보는 방향으로 10칸 탐색
            int dx = 0, dy = 0;
			int gunDistance = 7;
            if(player.lookDir == UP) dy = -1;
            else if(player.lookDir == DOWN) dy = 1;
            else if(player.lookDir == LEFT) {
				dx = -1;
				gunDistance = 13;
			}
            else if(player.lookDir == RIGHT) {
				dx = 1;
				gunDistance = 13;
			}

            int px = player.point.x;
            int py = player.point.y;

            for(int i = 1; i <= gunDistance; ++i) {
                int nx = px + dx * i;
                int ny = py + dy * i;
                if(nx < 0 || nx >= MAP_WIDTH || ny < 0 || ny >= MAP_HEIGHT) break;
                
                // 시각적 이펙트 표시
                mvaddch(ny - startPoint.y, nx - startPoint.x, BULLET_CHAR | COLOR_PAIR(BULLET_CHAR_KEY) | A_BOLD);
                refresh();
                usleep(5000); // 0.005초
                
                char who = map[ny][nx] & A_CHARTEXT;
                if(who == 'z') {
					bool is_alive = true;
                    // 좀비 배열에서 해당 좌표의 좀비를 찾아서 alive = 0
                    for(int j = 0; j < zombie_count; ++j) {
                        if(zombies[j].alive && (zombies[j].point.y == ny) && (zombies[j].point.x == nx)) {
                            --(zombies[j].hp);
                            if(zombies[j].hp <= 0) {
								zombies[j].alive = 0;
								is_alive = false;
							}
							break;
                        }
                    }
                    // 맵에서 좀비 제거
					if(!is_alive) {
						map[ny][nx] = GROUND | COLOR_PAIR(GROUND_KEY);
                    	break; // 한 마리만 제거
					}
                }
                else if(who == 'f') {
					bool is_alive = true;
                    // 빠른 좀비 죽이기 코드
                    for(int j = 0; j < fzombie_count; ++j) {
                        if(fzombies[j].alive && (fzombies[j].point.y == ny) && (fzombies[j].point.x == nx)) {
							--(fzombies[j].hp);
                            if(fzombies[j].hp <= 0) {
								fzombies[j].alive = 0;
                                is_alive = false;
                            }
                            break;
						}
                    }

                	if(!is_alive) {
                        map[ny][nx] = GROUND | COLOR_PAIR(GROUND_KEY);
                        break; // 한 마리만 제거
                    }
				}
				else if(who=='#' || who=='@') break;
            }
        }
    }
    else if(item_num == 'k') {
        if(item.injection >= 1) {
            --item.injection;
			int px = player.point.x;
			int py = player.point.y;
			
            // Pattern 1 data
            char pattern1[3][5] = {
                {'/', '*', '#', '*', '\\'},
                {'*', '#', ' ', '#', '*'},
                {'\\', '*', '#', '*', '/'}
            };

            // Display Pattern 1
            attron(COLOR_PAIR(INJECTION_KEY)); // Assuming color pair 3 is for green text
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -2; dx <= 2; ++dx) {
                    int nx = px + dx;
                    int ny = py + dy;
                    if(nx < 0 || nx >= MAP_WIDTH || ny < 0 || ny >= MAP_HEIGHT) continue;
                    mvaddch(ny - startPoint.y, nx - startPoint.x, pattern1[dy + 1][dx + 2]);
					/*
                    for (int i = 0; i < zombie_count; ++i) {
                        if (zombies[i].alive && zombies[i].point.y == ny && zombies[i].point.x == nx) {
                            --(zombies[i].hp);
                            if(zombies[i].hp <= 0) {
                                zombies[i].alive = 0;
                            	map[ny][nx] = GROUND | COLOR_PAIR(1);
							}
                        }
                    }

                    for (int i = 0; i < fzombie_count; ++i) {
                        if (fzombies[i].alive && fzombies[i].point.y == ny && fzombies[i].point.x == nx) {
                        	--(fzombies[i].hp);
                            if(fzombies[i].hp <= 0) {
                                fzombies[i].alive = 0;
                                map[ny][nx] = GROUND | COLOR_PAIR(1);
                            }
						}
                    }
					*/
                }
            }
            attroff(COLOR_PAIR(INJECTION_KEY));
            refresh();

            usleep(200000);

            // Pattern 2 data
            char pattern2[5][7] = {
                {'/', '*', '#', '#', '#', '*', '\\'},
                {'/', '*', '#', ' ', '#', '*', '\\'},
                {'#', '#', ' ', ' ', ' ', '#', '#'},
                {'\\', '*', '#', ' ', '#', '*', '/'},
                {'\\', '*', '#', '#', '#', '*', '/'}
            };
            
            // Display Pattern 2 (clears old pattern area implicitly)
            attron(COLOR_PAIR(INJECTION_KEY));
            for (int dy = -2; dy <= 2; ++dy) {
                for (int dx = -3; dx <= 3; ++dx) {
                    int nx = px + dx;
                    int ny = py + dy;
                    if(nx < 0 || nx >= MAP_WIDTH || ny < 0 || ny >= MAP_HEIGHT) continue;
                    
                    mvaddch(ny - startPoint.y, nx - startPoint.x, pattern2[dy + 2][dx + 3]);
                    // Kill normal zombies in the effect area
                    for (int i = 0; i < zombie_count; ++i) {
                        if (zombies[i].alive && zombies[i].point.y == ny && zombies[i].point.x == nx) {
                        	--(zombies[i].hp);
                            if(zombies[i].hp <= 0) {
                                zombies[i].alive = 0;
                                map[ny][nx] = GROUND | COLOR_PAIR(GROUND_KEY);
                            }
						}
                    }

                    // Kill fast zombies in the effect area
                    for (int i = 0; i < fzombie_count; ++i) {
                        if (fzombies[i].alive && fzombies[i].point.y == ny && fzombies[i].point.x == nx) {
                        	--(fzombies[i].hp);
                            if(fzombies[i].hp <= 0) {
                                fzombies[i].alive = 0;
                                map[ny][nx] = GROUND | COLOR_PAIR(GROUND_KEY);
                            }
						}
                    }
                }
            }
            attroff(COLOR_PAIR(INJECTION_KEY));
            refresh();
        }
    }
    else if(item_num == 'l') {
        if(item.packet >= 1) {
            --item.packet;
            player.hp += 10;
            // 만약 최대 HP가 100이라면
            if(player.hp > 100) player.hp = 100;
        }
    }
}
