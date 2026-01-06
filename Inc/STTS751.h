#ifndef stts751_H_
#define stts751_H_

#include <stdint.h>

void STTS751_Init(uint8_t Address);
float STTS751_ReadTemperature(uint8_t Address);


#endif