#ifndef MAZE_H
#define MAZE_H

#include "stdlib.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LCD/lcd.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./BSP/MyRandom/MyRandom.h"
#include "./BSP/LED/led.h"
#include <string.h>


#define MAP_LENGTH  10  /*纵向长度，对应i*/
#define MAP_WIDTH   10  /*横宽度，对应j*/
#define MAP_BLOCK_WIDTH 20  /*格子宽度*/
#define MAP_LX      50 
#define MAP_LY      50  /*左上角坐标*/
typedef struct{
    int i1,j1,i2,j2;
}MAZE_WALL;

extern uint32_t maze_map[MAP_LENGTH][MAP_WIDTH];


void DFS_generate(int16_t x1,int16_t y1,int16_t x2,int16_t y2,PRNG_SEED seed);

void ergodic_separate_generate(int16_t x1,int16_t y1,int16_t x2,int16_t y2,PRNG_SEED seed);
















#endif
