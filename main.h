/**
 * main.h
 */

#ifndef _MAIN_H
#define _MAIN_H	1

#include <stddef.h>

void postQuit();
void main_switchtomenu();
void main_switchtogame();
void main_switchtomapsel();
int main_loadmap(int num);
int main_restartmap();
int main_nextmap();
int main_prevmap();

#endif

