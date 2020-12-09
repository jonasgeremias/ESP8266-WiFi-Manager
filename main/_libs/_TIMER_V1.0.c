#ifndef __TIMER_0__
#ifndef TIMER_ISR_VOID
#define TIMER_0_ERROR
#error "defina a funcao TIMER_ISR_VOID"
#endif

#ifndef TIMER_0_ERROR
#include "driver/hw_timer.h"

void IRAM_ATTR vTimerCallback(xTimerHandle xTimer)
{
	TIMER_ISR_VOID;
}

void init_timer_us(int timer_us)
{
	hw_timer_init(vTimerCallback, NULL);
	hw_timer_alarm_us(timer_us, 1); // Com reload
}

#endif
#endif
#define __TIMER_0__