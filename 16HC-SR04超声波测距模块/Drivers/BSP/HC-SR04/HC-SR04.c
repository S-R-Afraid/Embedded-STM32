#include "./BSP/HC-SR04/HC-SR04.h"






/*
在20°C情况下，声速的传播速度是1cm/58us，343M/s
*/
/*
工作过程：当给tirg一个不小于10us的高电平后，元件会发出一段（8个方波）的40KHZ的声波信号，
当检测到信号返回时，echo会持续性的高电平，高电平的时间就是信号发出到返回的时间
*/



double HC_SR04_distance = 0 ;

uint32_t HC_SR04_measure_result = 0;

void Init_HC_SR04(void){
    HC_SR04_GPIO_CLK_ENABLE();
    gtim_timx_cap_chy_init(0XFFFF, 72 - 1); /* 以1Mhz的频率计数 捕获 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 配置 TRIG 为推挽输出
    GPIO_InitStruct.Pin = HC_SR04_TRIG_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // 推挽输出模式
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(HC_SR04_TRIG_GPIO_PORT, &GPIO_InitStruct);

    // 配置 ECHO 为下拉输入
    GPIO_InitStruct.Pin = HC_SR04_ECHO_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // 输入模式
    //GPIO_InitStruct.Pull = GPIO_PULLDOWN;  // 启用内部下拉电阻
    HAL_GPIO_Init(HC_SR04_ECHO_GPIO_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(HC_SR04_TRIG_GPIO_PORT, HC_SR04_TRIG_GPIO_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HC_SR04_ECHO_GPIO_PORT, HC_SR04_ECHO_GPIO_PIN, GPIO_PIN_SET);
    
    
}



void HC_SR04_start_measure(void){
    g_timxchy_cap_sta = 0;
    g_timxchy_cap_val = 0 ;
    
    TRIG_SEND(1);
    delay_us(15);
    TRIG_SEND(0);
}



uint32_t HC_SR04_get_measure_result(void){
    //while((g_timxchy_cap_sta & 0X80) == 0);     //等待捕获完成
    
    uint32_t temp;
    temp = 0;
    temp = g_timxchy_cap_sta & 0X3F;/* 获得溢出次数 */
    temp *= 65536;                  /* 溢出时间总和 */
    temp += g_timxchy_cap_val;      /* 得到总的高电平时间 */
    g_timxchy_cap_sta = 0;          /* 准备下一次捕获*/
    return temp;
}

double HC_SR04_get_distance(){
    double dis = HC_SR04_get_measure_result();
    dis = dis / 58.0f ;
    return dis;
}

