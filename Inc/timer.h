

#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>

void SYSTICK_Init(void);
void SYSTICK_Delay(uint32_t Delay);
uint32_t SYSTICK_Get(void);

void DWT_Init(void);
void DWT_Delay(uint32_t _us);
void TIM2_MovementDetection_Init(void);
uint8_t TIM2_GetAndClearFlag(void);


#endif
