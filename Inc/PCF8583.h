#ifndef PCF8583_H_
#define PCF8583_H_

#include <stdint.h>

void PCF8583_Init(void);
void RTC_ReadTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds);
void RTC_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
uint8_t BCD_to_Decimal(uint8_t bcd);
uint8_t Decimal_to_BCD(uint8_t decimal);

#endif
