
#ifndef SAND_H
#define SAND_H

#include "./BSP/MPU6050/MPU6050.h"
#include "math.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "./BSP/LCD/lcd.h"
#include "./SYSTEM/delay/delay.h"

/* 宏定义 */
#define SAND_NUM  520    //沙子的数量
#define BOX_LEN 30    //沙漏每部分的长度有几个单位，不包括边缘，详情见init_block()
#define SAND_LEN 5      //沙子的长度



/* 结构体 */
typedef struct{
    double cosx;
    double cosy;
    double cosz;

}BoxAngel;//姿态角


typedef struct{
    int flag;
    int snum;
    int broad[BOX_LEN+2][BOX_LEN+2];
}SandBlock;//沙漏块

/* 全局变量 */
extern BoxAngel boxangel;
extern SandBlock block1,block2;
extern int sand_clock_time;

extern int forward;                         /* 前进方向 */
extern int model;   /*模式选择，是MPU6050读取0还是摇杆读取1*/
/* 摇杆 */
extern     uint16_t PS2_xy_value[2];               /* 0x 1y */
extern     uint8_t PS2_z;                      /* z轴是否被按下 */
extern     float PS2_x_f,PS2_y_f;          /* xy的相对值，取值0~1 */

/* 函数声明 */
void update_angle(void);

void init_block(void);

void update_local(SandBlock *block);

void draw_sandblock(int zx,int zy,SandBlock *block);

void draw_sandblock2(int zx,int zy,SandBlock *block);

void draw_sand(int zx,int zy,uint16_t color);

void sand_clock(void);

#endif


