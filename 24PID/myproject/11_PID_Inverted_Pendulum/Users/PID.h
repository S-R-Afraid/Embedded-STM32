#ifndef PID_H
#define PID_H

#include "./SYSTEM/sys/sys.h"
#include "math.h"
#include "stdbool.h"
#include "float.h"
#include "stdlib.h"
#include "string.h"

#define PID_OK      1
#define PID_ERROR   0
#define PID_RESET   0
#define PID_SET     1
#define PID_OPT_II  1<<0    //积分限幅选项
#define PID_OPT_IS  1<<1    //积分分离，作用于积分值
#define PID_OPT_CIR 1<<2    //变速积分，作用于积分速度
#define PID_OPT_DL  1<<3    //微分先行
#define PID_OPT_ID  1<<4    //不完全微分
#define PID_OPT_OO  1<<5    //输出偏移
#define PID_OPT_IDZ 1<<6    //输入死区

#define __PID_GET_OPTION(__HANDLE__,__FLAG__)    (((__HANDLE__)->Option &(__FLAG__)) == (__FLAG__))

typedef struct {
    float Target,Actual_l,Actual_n,Out;   //目标，上一次的实际值,这一次实际值，输出
    float Kp,Ki,Kd;                     //比例项，积分项，微分项的权重
    float Error_n, Error_l, ErrorInt;     //本次误差，上次误差，误差积分
    float Difout_n,Difout_l;            //这次微分项值，上次值
    uint8_t Option;                     //选择哪些优化
    float ID_A;         //不完全微分参数,值越高，滤波效果越强
    float IDZ_A;        //输入死区参数
    float OO_UP,OO_DOWN;//输出偏移参数
    float CIR_C;        //变速积分参数
    float IS_A;         //积分分离参数
    float II_MAX,II_MIN;//积分限幅参数，上下限
    float Out_max,Out_min;//输出限幅
}PID_t;


//初始化结构体
void PID_InitStruct(PID_t* s);
//添加算法改进选项
void PID_Enable_Opt(PID_t* s,uint8_t opt);
//设置目标值
void PID_SetTarget(PID_t* s,float tar);
//更新实际值
void PID_UpdateActual(PID_t* s,float act);

//计算结果
void PID_UpdateOut(PID_t* s);











#endif


