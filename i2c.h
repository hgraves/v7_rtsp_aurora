/*
 * @file i2c.h
 *
 *  Created on: Dec 18, 2015
 *      Author: Howard Graves
 */

#ifndef I2C_H_
#define I2C_H_

#include "XIic.h"
#include "xparameters.h"

// PCA9548 8-port IIC Switch
#define IIC_SWITCH_ADDRESS 0x74
// Connected to IIC Buses
// Bus 0
#define IIC_SI570_ADDRESS  0x5D
// Bus 1
#define IIC_FMC_HPC_ADDRESS 0x70
// Bus 2
#define IIC_FMC_LPC_ADDRESS 0x70
// Bus 3
#define IIC_EEPROM_ADDRESS 0x54
// Bus 4
#define IIC_SFP_ADDRESS 0x50
// Bus 5
#define IIC_ADV7511_ADDRESS 0x39
// Bus 6
#define IIC_DDR3_SPD_ADDRESS 0x50
#define IIC_DDR3_TEMP_ADDRESS 0x18
// Bus 7
#define IIC_SI5324_ADDRESS 0x68

#define IIC_BUS_0 0x01
#define IIC_BUS_1 0x02
#define IIC_BUS_2 0x04
#define IIC_BUS_3 0x08
#define IIC_BUS_4 0x10
#define IIC_BUS_5 0x20
#define IIC_BUS_6 0x40
#define IIC_BUS_7 0x80

typedef struct SI5324Info
{
	u8 regIndex;	/* Register Number */
	u8 value;		/* Value to be Written */
} SI5324Info;

typedef struct SI570Info
{
	u8 regIndex;	/* Register Number */
	u8 value;		/* Value to be Written */
} SI570Info;

int IicInit(XIic *);
int setupSI5324(XIic *);
int setupSI570(XIic *);
int i2cWriteData(XIic *, u8 *, u16);
int i2cReadData(XIic *, AddressType, u8 *, u16);
int i2cSetSI5324(XIic *pIIC);
int i2cSetSI570(XIic *pIIC);

#endif /* I2C_H_ */
