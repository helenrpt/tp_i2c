#include <stdint.h>
#include <stdlib.h>
#include <stm32f446xx.h>
#include <stdio.h>
#include "main.h"
#include "gpio.h"
#include "usart.h"
#include "timer.h"
#include "util.h"
#include "i2c.h"
#include "lis2dw12.h"
#include "STTS751.h"
#include "PCF8583.h"

uint8_t devices[10];
uint8_t nbDevices;
uint8_t alarm = 0;

int main(void)
{

	uint8_t movement_detected;
    float temp_float;
    uint8_t hour;
    uint8_t min;
    uint8_t second;
	
	FPU_Init();
	GPIO_Init();
	USART2_Init();
	SYSTICK_Init();
	I2C_Init();
	
	printf("\r\n===== Initialization =====\r\n");
	
	// Scanner le bus I2C pour détecter les périphériques
	printf("\r\nScan I2C...\r\n");
	nbDevices = I2C_Scan(devices);
	printf("Number of devices found: %d\r\n\r\n", nbDevices);
	
	// Initialize the sensors
	LIS2DW12_Init(0x19);     // Accelerometer
    STTS751_Init(0x4A);      // Temperature sensor
	PCF8583_Init();    
	
	RTC_SetTime(11, 00, 00);// RTC
	
	// Initialiser le timer de détection de mouvement (interruption toutes les 1s)
	TIM2_MovementDetection_Init();
	printf("TIM2 initialized for movement detection\r\n");
	
	printf("\r\n===== Measurements begin =====\r\n\r\n");

	uint8_t compteur_inactivite = 0;
	uint8_t alarm = 0;
	uint8_t display_counter = 0;

	while(1)
	{
		SYSTICK_Delay(100); // Petit délai pour ne pas surcharger le CPU
		display_counter++;
		
		// Vérifier si le flag TIM2 est levé (1 seconde écoulée)
		if (TIM2_GetAndClearFlag())
		{
			// Détecter le mouvement
			uint8_t movement = Detect_Movement(0x19, 1.7f);
			
			if (movement)
			{
				compteur_inactivite = 0;
				alarm = 0;
			}
			else
			{
				compteur_inactivite++;
				if (compteur_inactivite >= 15)
				{
					alarm = 1;
				}
			}
		}
		
		// Afficher toutes les 5 secondes (50 * 100ms = 5000ms)
		if (display_counter >= 50)
		{
			display_counter = 0;
			
			RTC_ReadTime(&hour, &min, &second);
			temp_float = STTS751_ReadTemperature(0x4A);
			int16_t temp_int = (int16_t)(temp_float * 10.0f);
			

			printf("\n\r[%02d:%02d:%02d] Temp: %d.%d C - Inactivite: %ds\r\n", 
				       hour, min, second, temp_int / 10, abs(temp_int % 10), compteur_inactivite);

			if (alarm) {
			printf("[ALARM] No movement detected for 15 seconds!\r\n");
			}

		}
		


	}
}
