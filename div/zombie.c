#include "zombie.h"

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