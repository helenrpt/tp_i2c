#include <stdint.h>
#include <stm32f446xx.h>
#include <stdio.h>
#include "main.h"
#include "gpio.h"
#include "usart.h"
#include "timer.h"
#include "util.h"
#include "i2c.h"

uint8_t I2C_Scan(uint8_t *foundAddresses)
{
    uint8_t count = 0;

    for (uint8_t addr = 0x08; addr <= 0x77; addr++)
    {
        if (MPU_FindAdress(addr << 1))
        {
            foundAddresses[count++] = addr;
            printf("I2C device found at address: 0x%02X\r\n", addr);
        }
    }


    return count;
}

uint8_t devices[10];
uint8_t nbDevices;


void LIS2DW12_Init(uint8_t Address)
{
    uint8_t data;

    // 1️⃣ Vérifier si le capteur répond
    if (!MPU_FindAdress(Address << 1))
    {
        printf("Erreur: LIS2DW12 non détecté à l'adresse 0x%02X\r\n", Address);
        return;
    }

    // 2️⃣ Activer le capteur
    // CTRL1 (0x20) : ODR = 100 Hz, mode normal, axes XYZ activés
    data = 0x60;
    MPU_Write(Address << 1, 0x20, data);

    // 3️⃣ Configurer le range ±2g
    // CTRL6 (0x25) : FS = ±2g
    data = 0x00;
    MPU_Write(Address << 1, 0x25, data);

    // 4️⃣ Désactiver filtre high-pass (optionnel)
    // CTRL2 (0x21) = 0
    data = 0x00;
    MPU_Write(Address << 1, 0x21, data);

    // 5️⃣ Vérifier WHO_AM_I (0x0F)
    MPU_Read(Address << 1, 0x0F, &data, 1);
    if (data == 0x44)
        printf("LIS2DW12 initialisé correctement\r\n");
    else
        printf("Erreur WHO_AM_I: 0x%02X\r\n", data);
}

void Read_Accel(uint8_t Address, int16_t *ax, int16_t *ay, int16_t *az)
{
    uint8_t buffer[6];

    // Lire 6 octets (X, Y, Z)
    MPU_Read(Address << 1, 0x28 | 0x80, buffer, 6); // bit 7 = auto-incrément

    // Conversion little-endian
    *ax = (int16_t)(buffer[1] << 8 | buffer[0]);
    *ay = (int16_t)(buffer[3] << 8 | buffer[2]);
    *az = (int16_t)(buffer[5] << 8 | buffer[4]);
}


int main(void)
{
	int16_t ax, ay, az;
	FPU_Init();
	GPIO_Init();
	USART2_Init();
	SYSTICK_Init();
	I2C_Init();
	LIS2DW12_Init(0x19);


	while(1){
        SYSTICK_Delay(1000);
        Read_Accel(0x19, &ax, &ay, &az);
        printf("AX=%d AY=%d AZ=%d\r\n", ax, ay, az);

          // délai 100 ms

	}
}
