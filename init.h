#ifndef INIT_H
#define INIT_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
#endif
