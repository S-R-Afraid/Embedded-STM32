#ifndef __KEY_H
#define __KEY_H

#define KEY_CLICK		0x01
#define KEY_DOUBLE		0x02
#define KEY_LONG		0x04

void Key_Init(void);
uint8_t Key_Check(uint8_t n, uint8_t Event);
void Key_Clear(void);
void Key_Tick(void);

#endif
