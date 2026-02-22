#ifndef TERAZZO_H
#define TERAZZO_H

#include "./BSP/LCD/lcd.h"
#include "./SYSTEM/sys/sys.h"
#include "./BSP/OLED/OLED.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#define Pink                0xfe19  //粉色
#define LavenderBlush       0xff9e  //脸红的淡紫色
#define Thistle             0xddfb  //蓟
#define MediumSlateBlue     0x7b5d  //适中的板岩暗蓝灰色
#define Lavender            0xe73f  //熏衣草花的淡紫色
#define CornflowerBlue      0x64bd  //矢车菊的蓝色
#define LightSteelBlue      0xb63b  //淡钢蓝
#define LightCyan           0xe7ff  //淡青色
#define Auqamarin           0x7ff5  //绿玉\碧绿色
#define Khaki               0xf731  //卡其布
#define Moccasin            0xff36  //鹿皮鞋
#define LightSalmon         0xfd0f  //浅鲜肉(鲑鱼)色
#define LightCoral          0xf410  //淡珊瑚色



typedef struct {
    uint8_t r;
    uint16_t x;
    uint16_t y;
}Circle;

extern uint16_t terazzo_COLORS[13];


/*清空画布*/
void init_terazzo(void);

/* 在指定大小的画布内（尽可能地）画指定数量的多边形 */
void update_terazzo(uint16_t width,uint16_t height,uint16_t num);



#endif

