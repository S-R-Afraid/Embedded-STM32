#ifndef __WDG_H
#define __WDG_H

#include "./SYSTEM/sys/sys.h"
#include "math.h"
#include "./BSP/LED/led.h"
#include "./BSP/KEY/key.h"

void iwdg_init(uint8_t prer, uint16_t rlr);
void iwdg_feed(void);
void my_iwdg_init(double);
void iwdg_init_jcq(uint8_t prer, uint16_t rlr);
void iwdg_feed_jcq(void);
void wwdg_init(uint8_t tr, uint8_t wr, uint32_t fprer);
void wwdg_init_jcq(uint8_t tr, uint8_t wr, uint8_t fprer);
void wwdg_set_counter_jcq(uint8_t cnt);

#endif



