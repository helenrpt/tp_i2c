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

uint8_t Decimal_to_BCD(uint8_t decimal)
{
    return ((decimal / 10) << 4) | (decimal % 10);
}

void RTC_ReadTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds)
{
    uint8_t buffer[3];

    // Lire à partir de l'adresse 0x02 (secondes), puis 0x03 (minutes), puis 0x04 (heures)
    MPU_Read(PCF8583_ADDRESS, 0x02, buffer, 3);

    // Conversion BCD -> décimal avec masquage approprié
    *seconds = BCD_to_Decimal(buffer[0] & 0x7F);  // Registre 0x02: secondes
    *minutes = BCD_to_Decimal(buffer[1] & 0x7F);  // Registre 0x03: minutes
    *hours   = BCD_to_Decimal(buffer[2] & 0x3F);  // Registre 0x04: heures (format 24h)
}

void PCF8583_Init(void)
{
    // Registre de contrôle/statut (adresse 0x00)
    // Bit 7 = 0 : Comptage activé
    // Bits 5-4 = 00 : Mode horloge 32.768 kHz
    // Bit 2 = 0 : Pas de masquage
    
    // 1. Arrêter l'horloge pour configurer (bit 7 = 1)
    MPU_Write(PCF8583_ADDRESS, 0x00, 0x80);  // Stop counting
    SYSTICK_Delay(10);
    
    // 2. Réinitialiser les registres de temps à 00:00:00
    MPU_Write(PCF8583_ADDRESS, 0x01, 0x00);  // Centièmes de seconde = 0
    MPU_Write(PCF8583_ADDRESS, 0x02, 0x00);  // Secondes = 0
    MPU_Write(PCF8583_ADDRESS, 0x03, 0x00);  // Minutes = 0
    MPU_Write(PCF8583_ADDRESS, 0x04, 0x00);  // Heures = 0
    SYSTICK_Delay(10);
    
    // 3. Démarrer l'horloge en mode 32.768 kHz (bits 5-4 = 00, bit 7 = 0)
    MPU_Write(PCF8583_ADDRESS, 0x00, 0x00);  // Start counting, mode horloge
    SYSTICK_Delay(10);
}

void RTC_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    // Arrêter l'horloge (bit 7 = 1)
    MPU_Write(PCF8583_ADDRESS, 0x00, 0x80);
    SYSTICK_Delay(10);
    
    // Écrire l'heure en format BCD
    MPU_Write(PCF8583_ADDRESS, 0x02, Decimal_to_BCD(seconds));
    MPU_Write(PCF8583_ADDRESS, 0x03, Decimal_to_BCD(minutes));
    MPU_Write(PCF8583_ADDRESS, 0x04, Decimal_to_BCD(hours));
    SYSTICK_Delay(10);
    
    // Redémarrer l'horloge (bit 7 = 0, mode 32.768 kHz)
    MPU_Write(PCF8583_ADDRESS, 0x00, 0x00);
}


