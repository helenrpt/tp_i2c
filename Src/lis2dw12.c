#include <stdint.h>
#include <stm32f446xx.h>
#include <stdio.h>
#include <math.h>
#include "lis2dw12.h"
#include "i2c.h"
#include "timer.h"

// ========== INITIALISATION LIS2DW12 ==========
void LIS2DW12_Init(uint8_t Address)
{
    uint8_t data;

    //check if device is connected
    if (!MPU_FindAdress(Address << 1))
    {
        printf("Erreur: LIS2DW12 non détecté à l'adresse 0x%02X\r\n", Address);
        return;
    }

    // check WHO_AM_I register (0x0F) - should return 0x44
    MPU_Read(Address << 1, 0x0F, &data, 1);
    if (data == 0x44)
        printf("LIS2DW12 WHO_AM_I correct: 0x%02X\r\n", data);
    else
    {
        printf("Erreur WHO_AM_I: 0x%02X (attendu 0x44)\r\n", data);
        return;
    }

    //  Soft reset
    data = 0x40; // SOFT_RESET bit
    MPU_Write(Address << 1, 0x25, data);
    SYSTICK_Delay(10); // Wait for reset to complete

    // 4 Enable Block Data Update (BDU) in CTRL2 (0x21)
    // BDU prevents reading inconsistent data during update
    data = 0x08; // Bit 3 = BDU
    MPU_Write(Address << 1, 0x21, data);

    //  Configurer CTRL6 (0x25): Plage ±4g (selon doc ST)
    // Bits [5:4] = 01 (±4g)
    data = 0x10; // 00010000
    MPU_Write(Address << 1, 0x25, data);

    // Configure CTRL1 (0x20): ODR = 50 Hz, High-Performance mode (according to ST documentation)
    // Bits [7:4] = 0100 (50 Hz), Bits [3:2] = 01 (High-Performance mode)
    data = 0x44; // 01000100
    MPU_Write(Address << 1, 0x20, data);

    // Attendre stabilisation (100ms comme recommandé)
    SYSTICK_Delay(100);

    printf("LIS2DW12 initialized: 50Hz, ±4g, High-Performance mode\r\n");
}

// ========== LECTURE DES DONNÉES BRUTES ==========
void Read_Accel(uint8_t Address, int16_t *ax, int16_t *ay, int16_t *az)
{
    uint8_t buffer_x[2], buffer_y[2], buffer_z[2];
    uint8_t status;
    
    // Attendre que les données soient prêtes (Data Ready)
    // Registre STATUS (0x27), bit DRDY (bit 0)
    uint32_t timeout = 10000;
    do {
        MPU_Read(Address << 1, 0x27, &status, 1);
        timeout--;
    } while (!(status & 0x01) && timeout > 0);

    // Lire chaque axe séparément (contournement du problème d'auto-incrément)
    // OUT_X_L (0x28) et OUT_X_H (0x29)
    MPU_Read(Address << 1, 0x28, buffer_x, 2);
    
    // OUT_Y_L (0x2A) et OUT_Y_H (0x2B)
    MPU_Read(Address << 1, 0x2A, buffer_y, 2);
    
    // OUT_Z_L (0x2C) et OUT_Z_H (0x2D)
    MPU_Read(Address << 1, 0x2C, buffer_z, 2);

    // Conversion little-endian (LSB d'abord, puis MSB)
    // Mode High-Performance = 14 bits alignés à gauche sur 16 bits
    int16_t raw_x = (int16_t)(buffer_x[1] << 8 | buffer_x[0]);
    int16_t raw_y = (int16_t)(buffer_y[1] << 8 | buffer_y[0]);
    int16_t raw_z = (int16_t)(buffer_z[1] << 8 | buffer_z[0]);
    
    // Diviser par 4 pour obtenir la valeur 14 bits (données alignées à gauche)
    *ax = raw_x >> 2;
    *ay = raw_y >> 2;
    *az = raw_z >> 2;
}

// ========== LECTURE DES DONNÉES EN G ==========
void Read_Accel_Float(uint8_t Address, float *ax_g, float *ay_g, float *az_g)
{
    int16_t ax, ay, az;
    
    Read_Accel(Address, &ax, &ay, &az);
    
    // Conversion en g (±4g sur 14 bits en mode High-Performance)
    // Sensibilité = 4g / 2^13 = 0.000488 g/LSB (2^13 car données signées sur 14 bits)
    float sensitivity = 0.000488f; // en g/LSB pour ±4g
    
    *ax_g = (float)ax * sensitivity;
    *ay_g = (float)ay * sensitivity;
    *az_g = (float)az * sensitivity;
}

// ========== CALCUL DU VECTEUR D'ACCÉLÉRATION ==========
// Calcul de la magnitude selon doc ST
float Accel_Vector_Magnitude(float ax, float ay, float az)
{
    return sqrtf(ax * ax + ay * ay + az * az);
}

// ========== DÉTECTION DE MOUVEMENT ==========
// Basé sur la méthode de la documentation ST : moyennage de 3 échantillons
uint8_t Detect_Movement(uint8_t Address, float threshold)
{
    static float vector_baseline = 0.0f;
    static uint8_t first_call = 1;
    static uint8_t immobile_count = 0;
    float ax, ay, az;
    float samples_x[3], samples_y[3], samples_z[3];
    float avg_x = 0.0f, avg_y = 0.0f, avg_z = 0.0f;
    float current_vector, vector_diff;
    
    // Acquérir 3 échantillons et faire la moyenne (réduit de 5 à 3 pour plus de réactivité)
    for (int i = 0; i < 3; i++)
    {
        Read_Accel_Float(Address, &samples_x[i], &samples_y[i], &samples_z[i]);
        avg_x += samples_x[i];
        avg_y += samples_y[i];
        avg_z += samples_z[i];
        SYSTICK_Delay(20); // 20ms entre chaque échantillon (50Hz ODR)
    }
    
    // Calculer la moyenne
    avg_x /= 3.0f;
    avg_y /= 3.0f;
    avg_z /= 3.0f;
    
    // Calculer le vecteur d'accélération (magnitude)
    current_vector = Accel_Vector_Magnitude(avg_x, avg_y, avg_z);
    
    // Premier appel : établir la baseline
    if (first_call)
    {
        vector_baseline = current_vector;
        first_call = 0;
        return 0; // Pas de mouvement au premier appel
    }
    
    // Calculer la différence avec la baseline
    vector_diff = fabsf(current_vector - vector_baseline);
    
    // Si mouvement détecté
    if (vector_diff > threshold)
    {
        // Mise à jour rapide de la baseline pendant le mouvement
        vector_baseline = 0.7f * vector_baseline + 0.3f * current_vector;
        immobile_count = 0;
        return 1; // Mouvement détecté
    }
    else
    {
        // Pas de mouvement - compter les frames immobiles
        immobile_count++;
        
        // Mise à jour lente de la baseline quand immobile
        // Plus réactive après quelques frames immobiles
        if (immobile_count > 3)
        {
            vector_baseline = 0.85f * vector_baseline + 0.15f * current_vector;
        }
        else
        {
            vector_baseline = 0.95f * vector_baseline + 0.05f * current_vector;
        }
        
        return 0; // Pas de mouvement
    }
}
