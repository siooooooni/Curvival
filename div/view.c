#include "view.h"

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
        for (int j = startPoint.x; j <= endPoint.x; ++j) {
			char c = map[i][j] & A_CHARTEXT;
			if(c == GROUND) {
				mvaddch(y, x, GROUND | COLOR_PAIR(100));
			}
            else if(c == WALL) {
                mvaddch(y, x, WALL | COLOR_PAIR(200));
            }
			else {
				mvaddch(y, x, GROUND | COLOR_PAIR(100));
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
	
	if(view_start_angle < 0) view_start_angle = 360;
    else if(view_start_angle > 360) view_start_angle = 0;

	if(view_end_angle < 0) view_end_angle = 360;
    else if(view_end_angle > 360) view_end_angle = 0;

	player_view(startPoint, endPoint);
}
void player_view(yx startPoint, yx endPoint) {
	if(view_start_angle > view_end_angle) {
		for(double angle = 0.0; angle < view_end_angle; angle += 0.1) {
			line_view(angle, 1000, startPoint, endPoint);
		}
		for(double angle = 0.0; angle < view_start_angle; angle += 0.1) {
			line_view(angle, NEAR_VIEW_SIZE, startPoint, endPoint);
		}
		for(double angle = view_start_angle; angle < 360.0; angle += 0.1) {
			line_view(angle, 1000, startPoint, endPoint);
		}
	}
	else {
		for(double angle = 0.0; angle < view_start_angle; angle += 0.1) {
    		line_view(angle, NEAR_VIEW_SIZE, startPoint, endPoint);
    	}
    	for(double angle = view_start_angle; angle < view_end_angle; angle += 0.1) {
    		line_view(angle, 1000, startPoint, endPoint);
    	}
    	for(double angle = view_end_angle; angle < 360.0; angle += 0.1) {
        	line_view(angle, NEAR_VIEW_SIZE, startPoint, endPoint);
    	}
	}
}
void line_view(double angle, double distance, yx startPoint, yx endPoint) {
	if((int)angle % 90 == 0) angle += 0.5;

	double a = tan(angle*(M_PI / 180.0));
	yx quadrant;
	
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
	int ry = player.point.y - (int)ty, rx = player.point.x + (int)tx;
	
	while((startPoint.y <= ry) && (ry <= endPoint.y) && (startPoint.x <= rx) && (rx < endPoint.x)) {
		if((map[ry][rx] & A_CHARTEXT) == WALL) {
        	break;
		}

		double nty = a * (tx + quadrant.x);
		while(abs((int)nty - (int)ty) >= 1) {
			if((map[ry][rx] & A_CHARTEXT) == WALL) {
				return;
			}

			if((startPoint.y >= ry) || (ry >= endPoint.y) || (startPoint.x >= rx) || (rx >= endPoint.x) || (sqrt((ty*ty) + (tx*tx/NEAR_VIEW_ADJ)) > distance)) {
                return;
            }

			ty += quadrant.y;
			ry = player.point.y - (int)ty;
        	rx = player.point.x + (int)tx;
			
			mvaddch(ry - startPoint.y, rx - startPoint.x, map[ry][rx]);
		}
		
		if(sqrt((ty*ty) + (tx*tx/NEAR_VIEW_ADJ)) > distance) {
            return;
        }

		tx += quadrant.x;
		ty = a * tx;
        ry = player.point.y - (int)ty;
        rx = player.point.x + (int)tx;
		
		mvaddch(ry - startPoint.y, rx - startPoint.x, map[ry][rx]);
	}
} 