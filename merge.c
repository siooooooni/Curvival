#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define MAP_HEIGHT 150 //맵 크기
#define MAP_WIDTH 300
#define HOUSE_COUNT 12 //집 개수
#define ZOMBIE_MAX 200 //좀비 개수 최소~최대
#define ZOMBIE_MIN 100
#define ITEM_COUNT 20 //아이템 개수
#define VACCINE_COUNT 5 //각각 DEBUG 글자로 할 예정
#define VIEW_HEIGHT 21 //플레이어 시야 크기
#define VIEW_WIDTH 51
#define VIEW_ANGLE 0.2f //플레이어 시야 각도

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
    dir lookDir; //시야 방향
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
} zombie_t;

chtype map[MAP_HEIGHT][MAP_WIDTH];
human player = {.role = PLAYER, .point = {10, 10}, .hp = 100, .lookDir = RIGHT};
zombie_t zombies[ZOMBIE_MAX];
int zombie_count = 0;

void swap_int(int* a, int* b); //정수값 swap
void init_UI(); //시작 UI
void init_map(chtype map[MAP_HEIGHT][MAP_WIDTH]); //맵 초기화
void make_vaccine(chtype map[MAP_HEIGHT][MAP_WIDTH], char a); //백신 생성
void draw_building(chtype map[MAP_HEIGHT][MAP_WIDTH], Rect b, int color_pair); //건물 그리기
void draw_view_map(chtype map[MAP_HEIGHT][MAP_WIDTH], human player); //플레이어 시야 그리기
void move_zombies(chtype map[MAP_HEIGHT][MAP_WIDTH], yx playerPoint); //좀비 이동
void HumanMove(human *player, int way); //플레이어 이동

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
    init_pair(6, 46, 46);    // 초록색 글자, 초록색 배경 : 백신
    init_pair(7, 240, 240);  // 진한 회색 글자, 진한 회색 배경 : 시야 밖

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

void swap_int(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

//시작할 때 뜨는 UI
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
        zidx++; //인덱스 증가
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
    yx startPoint = {player.point.y - ((VIEW_HEIGHT-1) / 2), player.point.x - ((VIEW_WIDTH-1) / 2)}; // 왼쪽 위 좌표
    yx endPoint = {player.point.y + ((VIEW_HEIGHT-1) / 2), player.point.x + ((VIEW_WIDTH-1) / 2)}; // 오른쪽 아래 좌표
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
    // 화면에 기본적으로 진한 회색 '.' 출력
    int y = 0, x = 0;
    for (int i = startPoint.y; i <= endPoint.y; ++i) {
        for (int j = startPoint.x; j<= endPoint.x; ++j) {
            mvaddch(y,x,'.' | COLOR_PAIR(7));
            ++x;
        }
        x = 0;
        ++y;
    }
    // 시야 각도에 따라 ray casting 방식으로 시야를 그림
    if(player.lookDir == UP) {
        for(float a = VIEW_ANGLE; a < 7; a += 0.1*a) {
            float ty = 0, tx = 0;
            int ry = (int)(player.point.y - ty), rx = (int)(player.point.x - tx);
            while((ry >= startPoint.y) && (ry <= endPoint.y) && (rx >= startPoint.x) && (rx <= endPoint.x) && 
                  ((map[ry][rx] & A_CHARTEXT) == PLAYER || (map[ry][rx] & A_CHARTEXT) == GROUND)) {
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
            while((ry >= startPoint.y) && (ry <= endPoint.y) && (rx >= startPoint.x) && (rx <= endPoint.x) && 
                  ((map[ry][rx] & A_CHARTEXT) == PLAYER || (map[ry][rx] & A_CHARTEXT) == GROUND)) {
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
            while((ry >= startPoint.y) && (ry <= endPoint.y) && (rx >= startPoint.x) && (rx <= endPoint.x) && 
                  ((map[ry][rx] & A_CHARTEXT) == PLAYER || (map[ry][rx] & A_CHARTEXT) == GROUND)) {
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
            while((ry >= startPoint.y) && (ry <= endPoint.y) && (rx >= startPoint.x) && (rx <= endPoint.x) && 
                  ((map[ry][rx] & A_CHARTEXT) == PLAYER || (map[ry][rx] & A_CHARTEXT) == GROUND)) {
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
            while((ry >= startPoint.y) && (ry <= endPoint.y) && (rx >= startPoint.x) && (rx <= endPoint.x) && 
                  ((map[ry][rx] & A_CHARTEXT) == PLAYER || (map[ry][rx] & A_CHARTEXT) == GROUND)) {
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
            while((ry >= startPoint.y) && (ry <= endPoint.y) && (rx >= startPoint.x) && (rx <= endPoint.x) && 
                  ((map[ry][rx] & A_CHARTEXT) == PLAYER || (map[ry][rx] & A_CHARTEXT) == GROUND)) {
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
            while((ry >= startPoint.y) && (ry <= endPoint.y) && (rx >= startPoint.x) && (rx <= endPoint.x) && 
                  ((map[ry][rx] & A_CHARTEXT) == PLAYER || (map[ry][rx] & A_CHARTEXT) == GROUND)) {
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
            while((ry >= startPoint.y) && (ry <= endPoint.y) && (rx >= startPoint.x) && (rx <= endPoint.x) && 
                  ((map[ry][rx] & A_CHARTEXT) == PLAYER || (map[ry][rx] & A_CHARTEXT) == GROUND)) {
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
            while((ry >= startPoint.y) && (ry <= endPoint.y) && (rx >= startPoint.x) && (rx <= endPoint.x) && 
                  ((map[ry][rx] & A_CHARTEXT) == PLAYER || (map[ry][rx] & A_CHARTEXT) == GROUND)) {
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
            while((ry >= startPoint.y) && (ry <= endPoint.y) && (rx >= startPoint.x) && (rx <= endPoint.x) && 
                  ((map[ry][rx] & A_CHARTEXT) == PLAYER || (map[ry][rx] & A_CHARTEXT) == GROUND)) {
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
            while((ry >= startPoint.y) && (ry <= endPoint.y) && (rx >= startPoint.x) && (rx <= endPoint.x) && 
                  ((map[ry][rx] & A_CHARTEXT) == PLAYER || (map[ry][rx] & A_CHARTEXT) == GROUND)) {
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
            while((ry >= startPoint.y) && (ry <= endPoint.y) && (rx >= startPoint.x) && (rx <= endPoint.x) && 
                  ((map[ry][rx] & A_CHARTEXT) == PLAYER || (map[ry][rx] & A_CHARTEXT) == GROUND)) {
                rx = player.point.x - ty;
                mvaddch(ry - startPoint.y, rx - startPoint.x, map[ry][rx]);
                ++ty;
            }
        }
    }
}

//좀비 움직이는 함수 : zombie_count으로 모든 좀비 하나하나씩 이동방향을 y축 우선으로 결정하여 이동가능한지 체크하고 옮긴다.
void move_zombies(chtype map[MAP_HEIGHT][MAP_WIDTH], yx playerPoint) {
    for (int i = 0; i < zombie_count; ++i) {
        if (!zombies[i].alive) continue; //좀비가 죽은 놈이면 끝
        //시야 내에 있는지 확인
        int dy = abs(zombies[i].point.y - playerPoint.y);
        int dx = abs(zombies[i].point.x - playerPoint.x);
        if (dy > VIEW_HEIGHT/2 || dx > VIEW_WIDTH/2) continue;
        //이동 방향 결정
        int ny = zombies[i].point.y;
        int nx = zombies[i].point.x;
        // 랜덤으로 이동
        int chooseDyDx = rand()%2;
        if (chooseDyDx) {
            if (playerPoint.y < ny) ny--;
            else if (playerPoint.y > ny) ny++;
        }
        else {
            if (playerPoint.x < nx) nx--;
            else if (playerPoint.x > nx) nx++;
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
            map[ny][nx] = ZOMBIE | COLOR_PAIR(4);
        }
    }
} 
