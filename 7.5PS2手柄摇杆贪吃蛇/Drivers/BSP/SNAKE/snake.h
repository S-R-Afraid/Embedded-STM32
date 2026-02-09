#ifndef SNAKE_H
#define SNAKE_H
#include "./SYSTEM/sys/sys.h"
#include "./BSP/OLED/OLED.h"
#include "./SYSTEM/delay/delay.h"
#include <time.h>
#include <stdlib.h>


#define BOARD_W     16                      /* 场地宽度(单位方格) */
#define BOARD_H     8                       /* 场地高度 */
#define SNAKE_LEN   BOARD_W*BOARD_H         /* 蛇最大长度 */
#define SNAKE_W     7                       /* 上色格子宽度 */
#define OLED_W      128                     /* OLED屏幕宽度 */
#define OLED_H      64                      /* OLED屏幕高度 */


struct Body{
    int x,y;
};
extern int game_end;                    /* 游戏结束标志 */

extern int score;                           /* 分数 */

extern int snake_length;                    /* 当前蛇的长度 */

extern struct Body bodys[SNAKE_LEN];        /*身体数组*/

extern uint8_t food_x,food_y;               /* 食物位置 */

extern int forward;                         /* 前进方向 */

/* 摇杆 */
extern     uint16_t PS2_xy_value[2];               /* 0x 1y */
extern     uint8_t PS2_z;                      /* z轴是否被按下 */
extern     float PS2_x_f,PS2_y_f;          /* xy的相对值，取值0~1 */

void Ini_snake();                           /* 初始化蛇和食物的位置 */

int Ask_body_here(int x,int y);             /*查询某个位置是否有身体，用于刷新食物位置*/

int Random(int n);

void PutFood();                             /* 放置食物 */

void Draw();                                /* 总绘制函数 */

void DrawSqure(int x,int y);                /* 绘制方块 */

void DrawBoard();                           /* 绘制棋盘格子 */

void DrawBody();                            /* 绘制身体 */

void DrawFood();                            /* 绘制食物 */

void Forward(int d);                        /* 前进函数，改变头的位置 */

void Update(void);                          /*更新身体位置*/

void Increase(void);                        /*增长身体*/

int Judge(void);                            /* 判断是否碰到身体，是否吃到食物 */

void GameEnd(void);                         /* 游戏结束，显示分数 */









#endif // SNAKE_H