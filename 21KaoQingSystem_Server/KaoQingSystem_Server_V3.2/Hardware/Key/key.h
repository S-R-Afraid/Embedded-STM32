#ifndef __KEY_H__
#define __KEY_H__

#include <stdint.h>
#include "main.h"
#include <stm32f1xx_hal.h>

#define Key_Press 1
#define Key_Release 2


uint8_t Key1_Status(void);
uint8_t Key2_Status(void);
uint8_t Key3_Status(void);

#endif

