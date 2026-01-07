#include <stdint.h>
#include <stm32f446xx.h>
#include <stdio.h>
#include <math.h>
#include "i2c.h"
#include "timer.h"


#define HOURS_COUNTER_ADRESS 0x04
#define PCF8583_ADDRESS 0xA0
// ========== INITIALISATION PCF8583 ==========
/*
void PCF8583_Init(uint8_t Address){


}
*/
uint8_t BCD_to_Decimal(uint8_t bcd)
{
        return ((bcd >> 4) * 10) + (bcd & 0x0F);
}
void RTC_ReadTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds)
{
    uint8_t buffer[3];



    // Positionner le pointeur interne du PCF8583 au registre des secondes
   I2C_Start();
    I2C_Address(PCF8583_ADDRESS);   // Write mode
    I2C_Write(0x02);         // Registre secondes (PCF8583)
    I2C_Start();             // Repeated start

 // Lire secondes, minutes, heures
    I2C_Read(PCF8583_ADDRESS | 0x01, buffer, 3);
    I2C_Stop();



    // Conversion BCD -> décimal
    *seconds = BCD_to_Decimal(buffer[0] & 0x7F);
    *minutes = BCD_to_Decimal(buffer[1] & 0x7F);
    *hours   = BCD_to_Decimal(buffer[2] & 0x3F);
}


