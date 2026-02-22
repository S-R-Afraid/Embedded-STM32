#ifndef __PID_H
#define __PID_H

typedef struct {
	float Target;
	float Actual;
	float Out;
	
	float Err0;
	float Err1;
	float ErrInt;
	
	float ErrIntThreshold;
	
	float Kp;
	float Ki;
	float Kd;
	
	float OutMax;
	float OutMin;
} PID_t;

void PID_Clear(PID_t *p);
void PID_Update(PID_t *p);

#endif
