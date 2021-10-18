// Timer1.h
// Runs on LM4F120 or TM4C123

#include <stdint.h>
#include "tm4c123gh6pm.h"

#ifndef __TIMER1INTS_H__ // do not include more than once
#define __TIMER1INTS_H__

// ***************** Timer1_Init ****************
// Activate Timer1 interrupts to run user task periodically
// Inputs:  task is a pointer to a user function
//          period in units (1/clockfreq)
// Outputs: none
void Timer1_Init(void (*task)(void), uint32_t period);

#endif // __TIMER2INTS_H__
