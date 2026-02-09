#include "./BSP/PLAY_BEEP/BEEPSOUND.h"


/*

t是一个音符所需要的最小时间单位（ms），
    如过你的乐谱里有八分音符和十六分音符，那么这里应该填16分音符的持续时间；
    同时，一个八分音符拆分成两个连续的十六分音符。

当出现小于-20的数值时，视为结束队列，所以请确保你的音符数组以-20(或更小)结尾，否侧会出现崩溃错误

使用TIM2作为每个基本单位的定时器，使用TIM3输出PWM波形

*/


int* list;
uint16_t time;
float v=0.4;                                //音量，建议调小一点。最小值为0，最大值为1

float h_list[21]={                            /*简谱低音do到高音do（C大调）*/
    130.81,146.83,164.81,174.61,196.00,220,246.94,
    261.63,293.66,329.63,349.23,392.00,440,493.88,
    523.25,587.33,659.26,698.46,783.99,880,987.77,
};
void play_beep_sound_init(uint16_t t,int* list_p){
    list=list_p;
    time=t;
    v*=0.2;
    gtim_tim2_int_init(time*10, 7200-1);     //10Khz 的计数频率
    gtim_timx_pwm_chy_init(38-1, 7200 - 1);/* 10Khz的计数频率,263.15hz的PWM.接近C4 */
    /*修改比较值为0，即音量为0*/
    __HAL_TIM_SET_COMPARE(&g_timx_pwm_chy_handle, GTIM_TIMX_PWM_CHY, 0);
}



void play_next(){
    static int no=0;
    __HAL_TIM_SET_COMPARE(&g_timx_pwm_chy_handle, GTIM_TIMX_PWM_CHY, 0);
    printf("%d\t\n",list[no]);
    if(no>=0){
        
        if(list[no]<=-20){
            no=-1;
            play_beep(0);
            return;
        }
        
        play_beep(list[no]);
        no++;
    }
    
}

void play_beep(int s){
    float h=10000;
    __HAL_TIM_SET_COUNTER(&g_timx_pwm_chy_handle, 0);   /* PWM定时器计数器清零 */
    if(s==0){
        /*修改比较值为0，即音量为0*/
        __HAL_TIM_SET_COMPARE(&g_timx_pwm_chy_handle, GTIM_TIMX_PWM_CHY, 0);
    }else{
        if(s<0){
            s=-1-s;
        
        }else if(s>0){
            s=s+6;
        }
        h=10000/h_list[s];
        /*修改重装载值*/
        __HAL_TIM_SetAutoreload(&g_timx_pwm_chy_handle,h);
        
        /*修改比较值*/
        __HAL_TIM_SET_COMPARE(&g_timx_pwm_chy_handle, GTIM_TIMX_PWM_CHY, h*v);
    }
    
}



