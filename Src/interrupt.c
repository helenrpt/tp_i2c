
#include <stdint.h>
#include "stm32f446xx.h"
#include "interrupt.h"

extern uint32_t ticks;

static volatile uint8_t tim2_detection_flag = 0;

// Interrupt Handler for SysTick Interrupt
void SysTick_Handler(void){
	ticks++;
}

// Interrupt Handler for TIM2 - Flag toutes les secondes
void TIM2_IRQHandler(void)
{
	// Vérifier si c'est bien l'interruption de mise à jour
	if (TIM2->SR & TIM_SR_UIF)
	{
		// Effacer le flag d'interruption
		TIM2->SR &= ~TIM_SR_UIF;
		
		// Lever le flag pour déclencher la détection dans le main
		tim2_detection_flag = 1;
	}
}

// Fonction pour récupérer et effacer le flag
uint8_t TIM2_GetAndClearFlag(void)
{
	uint8_t flag = tim2_detection_flag;
	tim2_detection_flag = 0;
	return flag;
}

//////////////// HOW TO SETUP INTERUPT ? ///////////
// 1. Activate the NVIC IT :  NVIC_EnableIRQ().
// 2. Define the Event that generate IT : GPIO Rising Edge, Timer Update, USART RX not empty...
// 3. Write the corresponding ISR (Interrupt Sub-Routine) code. Do not forget to reset IT Flag.

///////////////         EXTI             //////////
// When using EXTI, to define the Event that generate IT (2), we need :
// a. Enable SYSCFG peripheral clock.
// b. Select the right PORT connected to EXTIx : SYSCFR->EXTICR.
// c. Unmask IT on EXTIx : EXTI->IMR.
// d. Select Rising or falling trigger edge :  EXTI->RTSR or EXTI->FTSR.

