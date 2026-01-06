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

uint8_t devices[10];
uint8_t nbDevices;

int main(void)
{

	uint8_t movement_detected;
    float temp_float;
	
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
	
	printf("\r\n===== Measurements begin =====\r\n\r\n");

	while(1)
	{
		SYSTICK_Delay(500); // Delay 200ms
		
		movement_detected = Detect_Movement(0x19, 1.7f);
		
		if (movement_detected)
		{
			printf("[MOUVEMENT] \r\n");
		}
		else
		{
			printf("[IMMOBILE]  \r\n");
		}
		

        temp_float = STTS751_ReadTemperature(0x4A);
        int16_t temp_int = (int16_t)(temp_float * 10.0f);
        printf("Temp=%d.%d C\r\n", temp_int / 10, abs(temp_int % 10));
	}
}
