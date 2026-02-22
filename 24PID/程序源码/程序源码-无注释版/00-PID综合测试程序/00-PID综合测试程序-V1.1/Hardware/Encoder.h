#ifndef __ENCODER_H
#define __ENCODER_H

void Encoder_Init(void);
int16_t Encoder_GetLocation(uint8_t n);
void Encoder_SetLocation(uint8_t n, int16_t Location);

#endif
