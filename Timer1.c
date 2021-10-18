// Timer1.c
// Runs on LM4F120 or TM4C123

#include <stdint.h>

#include "tm4c123gh6pm.h"

void (*PeriodicTask1)(void);

// ***************** TIMER1_Init ****************
// Activate TIMER1 interrupts to run user task periodically
// Inputs:  task is a pointer to a user function
//          period in units (1/clockfreq)
// Outputs: none
void Timer1_Init(void (*task)(void), uint32_t period){
  unsigned long volatile delay;
  SYSCTL_RCGC1_R |= (1 << 17); 
  delay = SYSCTL_RCGC1_R;   // 0) activate TIMER1
  PeriodicTask1 = task;          // user function
  TIMER1_CTL_R = 0x00000000;    // 1) disable TIMER1A during setup
  TIMER1_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER1_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER1_TAILR_R = period-1;    // 4) reload value
  TIMER1_TAPR_R = 0;            // 5) bus clock resolution
  TIMER1_ICR_R = 0x00000001;    // 6) clear TIMER1A timeout flag
  TIMER1_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI5_R = (NVIC_PRI5_R&0xFFFF00FF)|0x00A0A000; // 8) priority 5
  // interrupts enabled in the main program after all devices initialized
  // vector number 37, interrupt number 21
  NVIC_EN0_R = 1<<21;           // 9) enable IRQ 21 in NVIC
}

void Timer1A_Handler(void){
  TIMER1_ICR_R = 0x00000001;  // acknowledge
  (*PeriodicTask1)();
}
