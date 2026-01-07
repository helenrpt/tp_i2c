#ifndef PCF8583_H_
#define PCF8583_H_

#include <stdint.h>

uint8_t BCD_to_Decimal(uint8_t bcd);
void RTC_ReadTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds);

#endif
