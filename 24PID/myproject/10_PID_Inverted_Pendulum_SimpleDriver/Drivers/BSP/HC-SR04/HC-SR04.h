#ifndef HC_SR04_H
#define HC_SR04_H
#include "./SYSTEM/sys/sys.h"
#include "./BSP/TIMER/gtim.h"
#include "./SYSTEM/delay/delay.h"

#define HC_SR04_TRIG_GPIO_PORT                  GPIOD
#define HC_SR04_TRIG_GPIO_PIN                   GPIO_PIN_11
#define HC_SR04_ECHO_GPIO_PORT                  GPIOB
#define HC_SR04_ECHO_GPIO_PIN                   GPIO_PIN_6
#define HC_SR04_GPIO_CLK_ENABLE()               do{ __HAL_RCC_GPIOD_CLK_ENABLE();__HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)   /* 所在IO口时钟使能 */
#define TRIG_SEND(x)                            do{HAL_GPIO_WritePin(HC_SR04_TRIG_GPIO_PORT,HC_SR04_TRIG_GPIO_PIN,(x?GPIO_PIN_SET:GPIO_PIN_RESET)); }while(0)



extern double HC_SR04_distance ;

extern uint32_t HC_SR04_measure_result ;



void Init_HC_SR04(void);

void HC_SR04_start_measure(void);

uint32_t HC_SR04_get_measure_result(void);

double HC_SR04_get_distance(void);













#endif





