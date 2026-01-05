#ifndef I2C_H_
#define I2C_H_


void I2C_Init (void);
void MPU_Write (uint8_t Address, uint8_t Reg, uint8_t Data);
void MPU_Read (uint8_t Address, uint8_t Reg, uint8_t *buffer, uint8_t size);
uint8_t MPU_FindAdress(uint8_t Address);


/*************  Private functions *************/
void I2C_Start (void);
void I2C_Write (uint8_t data);
void I2C_Address (uint8_t Address);
void I2C_Stop (void);
void I2C_WriteMulti (uint8_t *data, uint8_t size);
void I2C_Read (uint8_t Address, uint8_t *buffer, uint8_t size);
uint8_t I2C_FindAddress (uint8_t Address);


#endif
