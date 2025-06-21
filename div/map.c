#include "map.h"
#include "utils.h"

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
    //아이템 및 가구 생성 (집 안에 먼저, 나머지는 랜덤)
    int items_placed_in_houses = 0;
    // 그려진 건물(HOUSE_COUNT-1개) 내부에 아이템 및 가구 배치
    for (int i = 0; i < HOUSE_COUNT - 1; ++i) {
        // 집 내부의 유효한 좌표 범위 계산
        int y_start = buildings[i].y1 + 1;
        int y_end = buildings[i].y2 - 1;
        int x_start = buildings[i].x1 + 1;
        int x_end = buildings[i].x2 - 1;

        if (y_start > y_end || x_start > x_end) {
            continue; // 집이 너무 작으면 스킵
        }

        // 아이템 배치 시도 (ITEM_COUNT 한도 내에서)
        if (items_placed_in_houses < ITEM_COUNT) {
            int y, x;
            int attempts = 0;
            const int max_attempts = 50;
            
            do {
                y = (rand() % (y_end - y_start + 1)) + y_start;
                x = (rand() % (x_end - x_start + 1)) + x_start;
                attempts++;
            } while (map[y][x] != (GROUND | COLOR_PAIR(1)) && attempts < max_attempts);

            if (attempts < max_attempts) {
                map[y][x] = ITEM | COLOR_PAIR(5) | A_BLINK;
                items_placed_in_houses++;
            }
        }

        // 가구 배치 시도
        {
            int y, x;
            int attempts = 0;
            const int max_attempts = 50;
            
            do {
                y = (rand() % (y_end - y_start + 1)) + y_start;
                x = (rand() % (x_end - x_start + 1)) + x_start;
                attempts++;
            } while (map[y][x] != (GROUND | COLOR_PAIR(1)) && attempts < max_attempts);

            if (attempts < max_attempts) {
                map[y][x] = FURNITURE | COLOR_PAIR(10);
            }
        }
    }

    // 나머지 아이템은 맵의 다른 곳에 랜덤하게 생성
    int remaining_items = ITEM_COUNT - items_placed_in_houses;
    for (int it = 0; it < remaining_items; ++it) {
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
                if(rock_pattern[dy][dx]) map[y+dy][x+dx] = 'O' | COLOR_PAIR(8) | A_BOLD;
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
                if(water_pattern[dy][dx]) map[y+dy][x+dx] = '~' | COLOR_PAIR(9);
            }
            water_cnt++;
        }
    }
} 