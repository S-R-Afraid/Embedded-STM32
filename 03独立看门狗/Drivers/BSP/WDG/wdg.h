#ifndef __WDG_H
#define __WDG_H

#include "./SYSTEM/sys/sys.h"
#include "math.h"

void iwdg_init(uint8_t prer, uint16_t rlr);
void iwdg_feed(void);
void my_iwdg_init(double);
void iwdg_init_jcq(uint8_t prer, uint16_t rlr);
void iwdg_feed_jcq(void);

#endif



