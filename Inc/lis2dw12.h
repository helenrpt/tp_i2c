#ifndef LIS2DW12_H_
#define LIS2DW12_H_

#include <stdint.h>

// Fonctions d'initialisation et de lecture de l'accéléromètre LIS2DW12
void LIS2DW12_Init(uint8_t Address);
void Read_Accel(uint8_t Address, int16_t *ax, int16_t *ay, int16_t *az);
void Read_Accel_Float(uint8_t Address, float *ax_g, float *ay_g, float *az_g);

// Calcul du vecteur d'accélération (magnitude)
float Accel_Vector_Magnitude(float ax, float ay, float az);

// Détection de mouvement
uint8_t Detect_Movement(uint8_t Address, float threshold);

#endif
