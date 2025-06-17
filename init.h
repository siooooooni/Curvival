#ifndef INIT_H
#define INIT_H
#include <stdio.h>
#include <string.h>
char print[200] = "==============================\n\tZOMBIE SURVIVAL\n==============================\n   1. New Game\n   2. Load Game\n   3. Options\n   4. Exit\n==============================\nPlease select: ";
void init_print() {
	printf("%s",print);
}
#endif
