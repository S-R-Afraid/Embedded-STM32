
#ifndef TEXT_ANIMATION_H
#define TEXT_ANIMATION_H

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/delay/delay.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "./BSP/LCD/lcd.h"

#define OBFU_NUM 15
#define PHRASES_NUM 3


struct Text_Animation{
	CHAR_LENGTH obfu_chars[OBFU_NUM+1];	//过渡用乱码字符串
	int start_tim;		//多少帧后开始渐变
	int end_tim;		//多少帧后结束渐变
	int disp_time;		//每句话显示帧数
	int loop;			//是否循环显示
	int idx;			//当前显示的是第几句话
	int fram;			//当前是第几帧
	char phrases[PHRASES_NUM][300];	//要显示的话
	char showstr[300];	//当前显示的内容
};
struct task{
    CHAR_LENGTH from;          //原字符
    CHAR_LENGTH to;            //目标字符
    int start;          //开始时间
    int end;            //结束时间
};



void update_showstr(struct Text_Animation *text);//更新显示字符串


    
extern struct Text_Animation text_animation;     /* 文字集 */















#endif












