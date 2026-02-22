#include "PID.h"

//初始化结构体
void PID_InitStruct(PID_t* s){
    memset(s,0,sizeof(PID_t));
}


//添加算法改进选项
void PID_Enable_Opt(PID_t* s,uint8_t opt){
    s->Option|=opt;
}


//设置目标值
void PID_SetTarget(PID_t* s,float tar){
    s->Target = tar;
}

//更新实际值
void PID_UpdateActual(PID_t* s,float act){
    s->Actual_l = s->Actual_n;
    s->Actual_n = act;
}


//计算结果
void PID_UpdateOut(PID_t* s){
    
    float k=1;
    
    s->Error_l = s->Error_n;
    s->Error_n = s->Target - s->Actual_n;
    
    
    /*变速积分(可选)*/
    if(__PID_GET_OPTION(s,PID_OPT_CIR) != PID_RESET)
    {
        k = 1/( s->CIR_C*fabs(s->Error_n) + 1 );
        
    }
    
    /*误差积分（累加）*/
    s->ErrorInt += k * s->Error_n;
    
    
    /*积分分离(可选)*/
    if(__PID_GET_OPTION(s,PID_OPT_IS) != PID_RESET)
    {
        if(fabs(s->Error_n)>s->IS_A)
            s->ErrorInt=0;
    }
    
    /*积分限幅(可选)*/
    if(__PID_GET_OPTION(s,PID_OPT_II) != PID_RESET)
    {
        if(s->ErrorInt > s->II_MAX){s->ErrorInt = s->II_MAX;}
        if(s->ErrorInt < s->II_MIN){s->ErrorInt = s->II_MIN;}
    }
    
    /*计算微分项*/
     s->Difout_l = s->Difout_n;
     s->Difout_n = s->Kd * (s->Error_n - s->Error_l);
    
    /*微分先行(可选)*/
    if(__PID_GET_OPTION(s,PID_OPT_DL) != PID_RESET)
    {
        s->Difout_n = - s->Kd * (s->Actual_n - s->Actual_l);
    }
    
    /*不完全微分(可选)*/
    if(__PID_GET_OPTION(s,PID_OPT_ID) != PID_RESET)
    {
        s->Difout_n = (1-s->ID_A) * s->Difout_n + s->ID_A * s->Difout_l;
    }
    
    /*PID计算*/
    s->Out = s->Kp * s->Error_n + s->Ki * s->ErrorInt + s->Difout_n;
    
    /*输入死区(可选)*/
    if(__PID_GET_OPTION(s,PID_OPT_IDZ) != PID_RESET)
    {
        if(fabs(s->Error_n)<s->IDZ_A){
            s->Out=0;
        }
    }
    
    /*输出偏移(可选)*/
    if(__PID_GET_OPTION(s,PID_OPT_OO) != PID_RESET)
    {
        if(s->Out>0.5){
            s->Out+=s->OO_UP;
        }else if(s->Out<-0.5){
            s->Out-=s->OO_DOWN;
        }else{
            s->Out=0;
        }
    }
    
    /*输岀限幅*/
    if(s->Out > s->Out_max){s->Out = s->Out_max;}
    if(s->Out < s->Out_min){s->Out = s->Out_min;}
}








