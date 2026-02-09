
#ifndef BEEPSOUND_H
#define BEEPSOUND_H

#include "./BSP/TIMER/gtim.h"
#include "./BSP/TIMER/btim.h"
void play_beep_sound_init(uint16_t t,int* list);

void play_next(void);

void play_beep(int s);



#endif


