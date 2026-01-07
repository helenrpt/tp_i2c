
#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#include <stdint.h>

void SysTick_Handler(void);
uint8_t TIM2_GetAndClearFlag(void);

#endif
