#include <stdint.h>
#include "stm32f446xx.h"
#include "timer.h"

uint32_t SystemCoreClock = 16000000;
uint32_t ticks = 0;

//////////////////////////////////////////////
////////////// TIM2 MOVEMENT TIMER ///////////
//////////////////////////////////////////////

static volatile uint8_t movement_detected_flag = 0;

/**
 * Initialiser TIM2 pour générer une interruption toutes les secondes
 */
void TIM2_MovementDetection_Init(void)
{
    // 1. Activer l'horloge de TIM2
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    
    // 2. S'assurer que le timer est arrêté avant configuration
    TIM2->CR1 &= ~TIM_CR1_CEN;
    
    // 3. Réinitialiser le compteur
    TIM2->CNT = 0;
    
    // 4. Configurer le prescaler et la période
    // APB1 clock = 16 MHz (par défaut)
    // Prescaler = 16000 - 1 => fréquence = 16MHz / 16000 = 1 kHz (1ms)
    TIM2->PSC = 16000 - 1;
    
    // Auto-reload = 1000 - 1 => interruption toutes les 1000ms = 1s
    TIM2->ARR = 1000 - 1;
    
    // 5. Générer un événement de mise à jour pour charger les valeurs
    TIM2->EGR |= TIM_EGR_UG;
    
    // 6. Effacer le flag d'interruption de mise à jour
    TIM2->SR &= ~TIM_SR_UIF;
    
    // 7. Activer l'interruption de mise à jour
    TIM2->DIER |= TIM_DIER_UIE;  // Update interrupt enable
    
    // 8. Activer l'interruption TIM2 dans le NVIC
    NVIC_SetPriority(TIM2_IRQn, 1);
    NVIC_EnableIRQ(TIM2_IRQn);
    
    // 9. Démarrer le timer
    TIM2->CR1 |= TIM_CR1_CEN;  // Counter enable
}

/**
 * Récupérer le flag de mouvement
 */
uint8_t TIM2_GetMovementFlag(void)
{
    return movement_detected_flag;
}

/**
 * Effacer le flag de mouvement
 */
void TIM2_ClearMovementFlag(void)
{
    movement_detected_flag = 0;
}

/**
 * Fonction appelée par l'ISR pour mettre à jour le flag
 */
void TIM2_SetMovementFlag(uint8_t value)
{
    movement_detected_flag = value;
}


//////////////////////////////////////////////
////////////// SYSTICK TIMER /////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////

void SYSTICK_Init(void){
	SysTick_Config(SystemCoreClock / 1000);
}

/**
 * Millisecond delays with Systick Timer, blocking function
 * @param delay : milliseconds to wait
 */
void SYSTICK_Delay(uint32_t Delay)
{
	uint32_t tickstart = SYSTICK_Get();

	while((SYSTICK_Get() - tickstart) < Delay);
}

uint32_t SYSTICK_Get(void){
	return ticks;
}

//////////////////////////////////////////////
////////////// DW TIMER //////////////////////
//////////////////////////////////////////////

void DWT_Init(void)
{
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	//    DWT->LAR = 0xC5ACCE55;  // For Cortex M7
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
 * Microsecond delays with DW Timer, blocking function
 * @param _us : microseconds to wait
 */
void DWT_Delay(uint32_t _us)
{
	uint32_t startTick  = DWT->CYCCNT;
	uint32_t targetTick = DWT->CYCCNT + _us * (SystemCoreClock/1000000);

	// No overflow
	if (targetTick > startTick)
		while (DWT->CYCCNT < targetTick);

	// With overflow
	else
		while (DWT->CYCCNT > startTick || DWT->CYCCNT < targetTick);

}



// Get MCO HSE to PA8 (D7)
// the MCO1PRE[2:0] and MCO1[1:0]

