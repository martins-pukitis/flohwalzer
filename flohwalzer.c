// flohwalzer.c
// Plays Flohwalzer (https://en.wikipedia.org/wiki/Flohwalzer) in
// forwards or backwards direction depending on the button pressed
// Runs on LM4F120/TM4C123

// October 18, 2021
// ******* Required Hardware I/O connections*******************
// play flohwalzer in forwards direction button connected to PE0
// play flohwalzer in backwards direction button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "Sound.h"

//#define DEBUG
#define TRUE 1
#define FALSE 0

// The #define statement SYSDIV2 initializes
// the PLL to the desired frequency.
#define SYSDIV2 4
// bus frequency is 400MHz/(SYSDIV2+1) = 400MHz/(4+1) = 80 MHz

// Place the processor in low-power sleep mode while it waits for an interrupt
void WaitForInterrupt(void);

// **************Systick_Init*********************
// Initialize Systick periodic interrupts
// Also calls DAC_Init() to initialize DAC
// Input: none
// Output: none
// reload value of 2666666 = 30 Hz
void Systick_Init(void){
  NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
  NVIC_ST_RELOAD_R = 2666666;// reload value
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
  NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R&0x00FFFFFF; // priority 0
  NVIC_ST_CTRL_R = 0x0007; // enable,core clock, and interrupts
}



//------------Switch_Init------------
// Initialize the two switches used to play the flohwalzer forwards PE0 and backwards PE1, use positive logic
// Input: none
// Output: none
void Switch_Init(void){ 
  unsigned long volatile delay;
  SYSCTL_RCGCGPIO_R |= 0x10;           // Port E clock
  delay = SYSCTL_RCGCGPIO_R;           // wait 3-5 bus cycles
  GPIO_PORTE_DIR_R &= ~0x03;        // PE1,0 inputs
  GPIO_PORTE_PDR_R |= 0x03;         // enable pull-down
  GPIO_PORTE_AFSEL_R &= ~0x03;      // not alternative
  GPIO_PORTE_AMSEL_R &= ~0x03;      // no analog
  GPIO_PORTE_PCTL_R &= ~0x000000FF; // bits for PE1,PE0
  GPIO_PORTE_DEN_R |= 0x03;         // enable PE1,PE0
}


void SysTick_Handler(void){  // runs at 30 Hz
  static unsigned long gpio_old = 0, gpio;

  gpio = GPIO_PORTE_DATA_R;
  /* if PE0 pressed */
  if (!(gpio_old & 0x1) && (gpio & 0x1))
    flohwalzerSound(FORWARDS);

  /* if PE1 pressed */
  if (!(gpio_old & 0x2) && (gpio & 0x2))
    flohwalzerSound(BACKWARDS);

  gpio_old = gpio;
}

// configure the system to get its clock from the PLL
void PLL_Init(void){
  // 0) configure the system to use RCC2 for advanced features
  //    such as 400 MHz PLL and non-integer System Clock Divisor
  SYSCTL_RCC2_R |= SYSCTL_RCC2_USERCC2;
  // 1) bypass PLL while initializing
  SYSCTL_RCC2_R |= SYSCTL_RCC2_BYPASS2;
  // 2) select the crystal value and oscillator source
  SYSCTL_RCC_R &= ~SYSCTL_RCC_XTAL_M;   // clear XTAL field
  SYSCTL_RCC_R += SYSCTL_RCC_XTAL_16MHZ;// configure for 16 MHz crystal
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_OSCSRC2_M;// clear oscillator source field
  SYSCTL_RCC2_R += SYSCTL_RCC2_OSCSRC2_MO;// configure for main oscillator source
  // 3) activate PLL by clearing PWRDN
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_PWRDN2;
  // 4) set the desired system divider and the system divider least significant bit
  SYSCTL_RCC2_R |= SYSCTL_RCC2_DIV400;  // use 400 MHz PLL
  SYSCTL_RCC2_R = (SYSCTL_RCC2_R&~0x1FC00000) // clear system clock divider field
                  + (SYSDIV2<<22);      // configure for 80 MHz clock
  // 5) wait for the PLL to lock by polling PLLLRIS
  while((SYSCTL_RIS_R&SYSCTL_RIS_PLLLRIS)==0){};
  // 6) enable use of PLL by clearing BYPASS
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_BYPASS2;
}

int main(void){
  PLL_Init();
  Sound_Init();
  Switch_Init();
  Systick_Init();

  while (TRUE)
    WaitForInterrupt();
}

