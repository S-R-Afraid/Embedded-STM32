#include "PID.h"
#include "math.h"

void PID_Clear(PID_t *p)
{
	p->Err0 = 0;
	p->Err1 = 0;
	p->ErrInt = 0;
}

void PID_Update(PID_t *p)
{
	p->Err1 = p->Err0;
	p->Err0 = p->Target - p->Actual;
	
	if (p->Ki != 0)
	{
		if (p->ErrIntThreshold == 0)
		{
			p->ErrInt += p->Err0;
		}
		else if (fabs(p->Err0) < p->ErrIntThreshold)
		{
			p->ErrInt += p->Err0;
		}
		else
		{
			p->ErrInt = 0;
		}
	}
	else
	{
		p->ErrInt = 0;
	}
	
	p->Out = p->Kp * p->Err0
		   + p->Ki * p->ErrInt
		   + p->Kd * (p->Err0 - p->Err1);
	
	if (p->Out > p->OutMax) {p->Out = p->OutMax;}
	if (p->Out < p->OutMin) {p->Out = p->OutMin;}
}

