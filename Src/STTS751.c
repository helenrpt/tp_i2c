#include <stdint.h>
#include <stm32f446xx.h>
#include <stdio.h>
#include <math.h>
#include "lis2dw12.h"
#include "i2c.h"
#include "timer.h"


// ========== CAPTEUR DE TEMPÉRATURE STTS751 ==========
void STTS751_Init(uint8_t Address)
{
    uint8_t data;
   
    if (!MPU_FindAdress(Address << 1))
    {
        printf("Erreur: STTS751 non détecté à l'adresse 0x%02X\r\n", Address);
        return;
    }


    MPU_Read(Address << 1, 0x0F, &data, 1);
    printf("STTS751 Product ID: 0x%02X\r\n", data);

    MPU_Read(Address << 1, 0xFE, &data, 1);
    if (data == 0x53)
        printf("STTS751 Manufacturer ID correct: 0x%02X\r\n", data);
    else
        printf("Erreur Manufacturer ID: 0x%02X\r\n", data);

    
    //activate sensor in continuous mode
    // Configuration Register (0x03): 0x00 for continuous mode, 1 conversion/sec
    data = 0x00;
    MPU_Write(Address << 1, 0x03, data);

    //Define resolution (0x08): 12 bits
    data = 0x03; // 12 bits (0.0625°C resolution)
    MPU_Write(Address << 1, 0x08, data);

    printf("STTS751 initialized correctly\r\n");
}

float STTS751_ReadTemperature(uint8_t Address)
{
    uint8_t temp_high, temp_low;
    int16_t raw_temp;
    float temperature;

    // High byte of temperature (0x00)
    MPU_Read(Address << 1, 0x00, &temp_high, 1);
    
    // Low byte of temperature (0x02)
    MPU_Read(Address << 1, 0x02, &temp_low, 1);

    // Combine the two bytes (high part = integer, low part = fraction)
    // The low part contains the 4 most significant bits as fraction
    raw_temp = (int16_t)((int8_t)temp_high); // cast to int8_t to handle sign
    
    // Add the fractional part (4 MSB bits of temp_low)
    temperature = (float)raw_temp + ((temp_low >> 4) * 0.0625f);

    return temperature;
}
