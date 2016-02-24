/*
 * @file i2c.c
 * @brief i2c functions
 *
 *  Created on: Dec 18, 2015
 *      Author: Howard Graves
 */

#include "common.h"
#include "i2c.h"

static void iicSendHandler(XIic *);
static void iicReceiveHandler(XIic *);
static void iicStatusHandler(XIic * , int );

/*****************************************************************************/
/**
* @brief initialize the i2c controller
* This function initializes for I2C controller.
*
* @param	pIIC contains the address I2C controller
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*
******************************************************************************/
int IicInit(XIic *pIIC) {

	int Status;

	/*
	 * Initialize the IIC device Instance.
	 */
	Status = XIic_Initialize(pIIC, XPAR_IIC_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		if(Status == XST_DEVICE_NOT_FOUND) {
			xil_printf("XIic_Initialize FAILED - Device not found\n");
		} else {
			xil_printf("XIic_Initialize FAILED - Device not started\n");
		}
		return XST_FAILURE;
	}

	// Release reset on the PCA9548 IIC Switch
	XIic_SetGpOutput(pIIC, 0xFF);

	/*
	 * setup interrupt handlers
	 */
	XIic_SetSendHandler(pIIC, pIIC,(XIic_Handler) iicSendHandler);
	XIic_SetRecvHandler(pIIC, pIIC,(XIic_Handler) iicReceiveHandler);
	XIic_SetStatusHandler(pIIC, pIIC,(XIic_StatusHandler) iicStatusHandler);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief setup the si5324
* This function programs the SI5324.
*
* @param	pIIC contains the address I2C controller
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*
******************************************************************************/
int setupSI5324(XIic *pIIC) {

	int Status;
	u8 WriteBuffer[sizeof(AddressType) + PAGE_SIZE];
	int index;

	SI5324Info si5324InitTable[] = {
			{  0, 0x54},	/* Register 0 */
			{  1, 0xE4},	/* Register 1 */
			{  2, 0x32},	/* Register 2 */
			{  3, 0x15},	/* Register 3 */
			{  4, 0x92},	/* Register 4 */
			{  5, 0xed},	/* Register 5 */
			{  6, 0x2d},	/* Register 6 */
			{  7, 0x2a},	/* Register 7 */
			{  8, 0x00},	/* Register 8 */
			{  9, 0xc0},	/* Register 9 */
			{ 10, 0x08},	/* Register 10 */
			{ 11, 0x40},	/* Register 11 */
			{ 19, 0x29},	/* Register 19 */
			{ 20, 0x3e},	/* Register 20 */
			{ 21, 0xff},	/* Register 21 */
			{ 22, 0xdf},	/* Register 22 */
			{ 23, 0x1f},	/* Register 23 */
			{ 24, 0x3f},	/* Register 24 */
			{ 25, 0xA0},	/* Register 25 */
			{ 31, 0x00},	/* Register 31 */
			{ 32, 0x00},	/* Register 32 */
			{ 33, 0x03},	/* Register 33 */
			{ 34, 0x00},	/* Register 34 */
			{ 35, 0x00},	/* Register 35 */
			{ 36, 0x03},	/* Register 36 */
			{ 40, 0xc0},	/* Register 40 */
			{ 41, 0x92},	/* Register 41 */
			{ 42, 0x7B},	/* Register 42 */
			{ 43, 0x00},	/* Register 43 */
			{ 44, 0x1D},	/* Register 44 */
			{ 45, 0xC2},	/* Register 45 */
			{ 46, 0x00},	/* Register 46 */
			{ 47, 0x1D},	/* Register 47 */
			{ 48, 0xC2},	/* Register 48 */
			{ 55, 0x00},	/* Register 55 */
			{131, 0x1f},	/* Register 131 */
			{132, 0x02},	/* Register 132 */
			{137, 0x01},	/* Register 137 */
			{138, 0x0f},	/* Register 138 */
			{139, 0xff},	/* Register 139 */
			{142, 0x00},	/* Register 142 */
			{143, 0x00},	/* Register 143 */
			{136, 0x40}		/* Register 136 */
	};


#ifdef __DEBUG
	u8 tmpRead;
	u8 readBuffer[43];
#endif

	/*
	 * point the i2c to the si5324
	 */
	Status = i2cSetSI5324(pIIC);

	// Write data to registers
	for(index=0; index<(sizeof(si5324InitTable)/2); index++) {
		WriteBuffer[0] = si5324InitTable[index].regIndex;
		WriteBuffer[1] = si5324InitTable[index].value;

#ifdef __DEBUG
		xil_printf("wrote reg %d\t- 0x%02X\n",si5324InitTable[index].regIndex,si5324InitTable[index].value);
#endif

		Status = i2cWriteData(pIIC, &WriteBuffer[0], 2);
		if (Status != XST_SUCCESS) {
			xil_printf("Write to Reg 2 FAILED\r\n");
			return XST_FAILURE;
		}
	}

#ifdef __DEBUG

	for(index=0; index<(sizeof(si5324InitTable)/2); index++) {
		Status = i2cReadData(pIIC, si5324InitTable[index].regIndex, &tmpRead,1 );
		readBuffer[index] = tmpRead;
		if (Status != XST_SUCCESS) {
			xil_printf("i2c: Read failed\r\n");
			return XST_FAILURE;
		}
	}

	xil_printf("\n");
	for(index=0; index<(sizeof(si5324InitTable)/2); index++) {
		xil_printf("read reg %d\t- 0x%02X\n",si5324InitTable[index].regIndex,readBuffer[index]);
	}

#endif

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief set up the si570
* This function programs the SI570.
*
* @param	pIIC contains the address I2C controller
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*
******************************************************************************/
int setupSI570(XIic *pIIC) {

	int Status;
	u8 WriteBuffer[sizeof(AddressType) + PAGE_SIZE];
	int index;

	/*
	 *  for 50MHz
	 */
	//SI570Info si570InitTable[] = {
	//		{  7, 0x63},	/* Register 7 */
	//		{  8, 0x42},	/* Register 8 */
	//		{  9, 0xAD},	/* Register 9 */
	//		{  10, 0xAD},	/* Register 10 */
	//		{  11, 0x69},	/* Register 11 */
	//		{  12, 0x3D}	/* Register 12 */
	//};

	/*
	 *  for 100MHz
	 */
	SI570Info si570InitTable[] = {
			{  7, 0x22},	/* Register 7 */
			{  8, 0x42},	/* Register 8 */
			{  9, 0xBB},	/* Register 9 */
			{  10, 0xAB},	/* Register 10 */
			{  11, 0xB9},	/* Register 11 */
			{  12, 0xC1}	/* Register 12 */
	};


#ifdef __DEBUG
	u8 tmpRead;
	u8 readBuffer[43];
#endif

	/*
	 * point the i2c to the si570
	 */
	Status = i2cSetSI570(pIIC);

	// Freeze the DCO (register 137 - bit 4)
	WriteBuffer[0] = 137;
	WriteBuffer[1] = 0x10;

	Status = i2cWriteData(pIIC, &WriteBuffer[0], 2);
	if (Status != XST_SUCCESS) {
		xil_printf("i2c write failed\r\n");
		return XST_FAILURE;
	}

#ifdef __DEBUG
		xil_printf("\n");
#endif

	// Write data to registers
	for(index=0; index<(sizeof(si570InitTable)/2); index++) {
		WriteBuffer[0] = si570InitTable[index].regIndex;
		WriteBuffer[1] = si570InitTable[index].value;

		Status = i2cWriteData(pIIC, &WriteBuffer[0], 2);
		if (Status != XST_SUCCESS) {
			xil_printf("Write to Reg 2 FAILED\r\n");
			return XST_FAILURE;
		}

#ifdef __DEBUG
		xil_printf("wrote reg %d\t- 0x%02X\n",si570InitTable[index].regIndex,si570InitTable[index].value);
#endif

	}

	// unfreeze the DCO (register 137 - bit 4)
	WriteBuffer[0] = 137;
	WriteBuffer[1] = 0x00;

	Status = i2cWriteData(pIIC, &WriteBuffer[0], 2);
	if (Status != XST_SUCCESS) {
		xil_printf("i2c write failed\r\n");
		return XST_FAILURE;
	}

	// assert NewFreq (register 135 - bit 6)
	WriteBuffer[0] = 135;
	WriteBuffer[1] = 0x40;

	Status = i2cWriteData(pIIC, &WriteBuffer[0], 2);
	if (Status != XST_SUCCESS) {
		xil_printf("i2c write failed\r\n");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief write data to the i2c
* This function writes a buffer of data to the IIC.
*
* @param	pIIC contains the address I2C controller
* @param	buffer points the the data to be written
* @param	ByteCount contains the number of bytes in the buffer to be
*			written.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		The Byte count should not exceed the page size
*
******************************************************************************/
int i2cWriteData(XIic *pIIC, u8 *buffer, u16 ByteCount)
{
	int Status;

	/*
	 * Set the defaults.
	 */
	TransmitComplete = 1;
	pIIC->Stats.TxErrors = 0;


	/*
	 * Start the IIC device.
	 */
	Status = XIic_Start(pIIC);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Send the Data.
	 */
	Status = XIic_MasterSend(pIIC, buffer, ByteCount);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Wait till the transmission is completed.
	 */
	while ((TransmitComplete) || (XIic_IsIicBusy(pIIC) == TRUE)) {

		if (pIIC->Stats.TxErrors != 0) {

			/*
			 * Enable the IIC device.
			 */
			Status = XIic_Start(pIIC);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if (!XIic_IsIicBusy(pIIC)) {
				/*
				 * Send the Data.
				 */
				Status = XIic_MasterSend(pIIC,
							 buffer,
							 ByteCount);
				if (Status == XST_SUCCESS) {
					pIIC->Stats.TxErrors = 0;
				}
				else {
				}
			}
		}
	}

	/*
	 * Stop the IIC device.
	 */
	Status = XIic_Stop(pIIC);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
/*****************************************************************************/
/**
* @brief read the si5324 status
* This function reads the status from the IIC into a specified buffer.
*
* @param	pIIC contains the address I2C controller
* @param	BufferPtr contains the address of the data buffer to be filled.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int i2cReadSI5324Status(XIic *pIIC, u8 *BufferPtr) {

	int Status;
	u8 tmpRead;
	u8 WriteBuffer[2];


	Status = i2cSetSI5324(pIIC);

	/*
	 * clear flags
	 */

	WriteBuffer[0] = 132;
	WriteBuffer[1] = 0x00;

	Status = i2cWriteData(pIIC, &WriteBuffer[0], 2);
	if (Status != XST_SUCCESS) {
		xil_printf("si5324: clear flags FAILED\r\n");
		return XST_FAILURE;
	}

	/*
	 * read status
	 */
	Status = i2cReadData(pIIC, 132, &tmpRead, 1);

	*BufferPtr = tmpRead;

	if (Status != XST_SUCCESS) {
		xil_printf("i2c: Read status failed\r\n");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief read from an i2c bus device
* This function reads data from the IIC into a specified buffer.
*
* @param	pIIC contains the address I2C controller
* @param	addr contains the address I2C register to be read
* @param	BufferPtr contains the address of the data buffer to be filled.
* @param	ByteCount contains the number of bytes in the buffer to be read.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int i2cReadData(XIic *pIIC, AddressType addr, u8 *BufferPtr, u16 ByteCount)
{
	int Status;
	AddressType Address;
	Address = addr;
	u8 WriteBuffer[sizeof(AddressType) + PAGE_SIZE];

	/*
	 * Set the Defaults.
	 */
	ReceiveComplete = 1;

	/*
	 * Position the Pointer in EEPROM.
	 */
	if (sizeof(Address) == 1) {
		WriteBuffer[0] = (u8) (Address);
	}
	else {
		WriteBuffer[0] = (u8) (Address >> 8);
		WriteBuffer[1] = (u8) (Address);
	}

	Status = i2cWriteData(pIIC, &WriteBuffer[0], sizeof(Address));
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the IIC device.
	 */
	Status = XIic_Start(pIIC);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Receive the Data.
	 */
	Status = XIic_MasterRecv(pIIC, BufferPtr, ByteCount);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till all the data is received.
	 */
	while ((ReceiveComplete) || (XIic_IsIicBusy(pIIC) == TRUE)) {

	}

	/*
	 * Stop the IIC device.
	 */
	Status = XIic_Stop(pIIC);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief set I2c data path to si570
* This function sets up the data path to communicate with the SI570.
*
* @param	pIIC contains the address I2C controller
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int i2cSetSI570(XIic *pIIC) {

	int Status;
	u8 WriteBuffer[2];

	/*
	 * set address to IIC switch
	 */
	Status = XIic_SetAddress(pIIC, XII_ADDR_TO_SEND_TYPE,
				 IIC_SWITCH_ADDRESS);
	if (Status != XST_SUCCESS) {
		xil_printf("XIic_SetAddress to PCA9548 FAILED\r\n");
		return XST_FAILURE;
	}

	/*
	 * Write to the IIC Switch.
	 */
	WriteBuffer[0] = IIC_BUS_0; //Select Bus0 - Si570
	Status = i2cWriteData(pIIC, &WriteBuffer[0],1);
	if (Status != XST_SUCCESS) {
		xil_printf("PCA9548 FAILED to select Si570 IIC Bus\r\n");
		return XST_FAILURE;
	}

	/*
	 * Set the Slave address to the Si570
	 */
	Status = XIic_SetAddress(pIIC, XII_ADDR_TO_SEND_TYPE,
			IIC_SI570_ADDRESS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
* @brief set I2c data path to si5324
* This function sets up the data path to communicate with the SI5324.
*
* @param	pIIC contains the address I2C controller
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int i2cSetSI5324(XIic *pIIC) {

	int Status;
	u8 WriteBuffer[2];


	/*
	 * set address to IIC switch
	 */
	Status = XIic_SetAddress(pIIC, XII_ADDR_TO_SEND_TYPE,
				 IIC_SWITCH_ADDRESS);
	if (Status != XST_SUCCESS) {
		xil_printf("XIic_SetAddress to PCA9548 FAILED\r\n");
		return XST_FAILURE;
	}

	/*
	 * Write to the IIC Switch.
	 */
	WriteBuffer[0] = IIC_BUS_7; //Select Bus0 - Si5324
	Status = i2cWriteData(pIIC, &WriteBuffer[0],1);
	if (Status != XST_SUCCESS) {
		xil_printf("PCA9548 FAILED to select Si5324 IIC Bus\r\n");
		return XST_FAILURE;
	}

	/*
	 * Set the Slave address to the Si5324
	 */
	Status = XIic_SetAddress(pIIC, XII_ADDR_TO_SEND_TYPE,
			IIC_SI5324_ADDRESS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief I2c interrupt handler for status
* This Send handler is called asynchronously from an interrupt
* context and indicates that data in the specified buffer has been sent.
*
* @param	InstancePtr is not used, but contains a pointer to the IIC
*			device driver instance which the handler is being called for.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void iicSendHandler(XIic *InstancePtr)
{
	TransmitComplete = 0;

#ifdef __DEBUG
	xil_printf("i2c send interrupt\n");
#endif

}

/*****************************************************************************/
/**
* @brief I2c interrupt handler for status
* This Receive handler is called asynchronously from an interrupt
* context and indicates that data in the specified buffer has been Received.
*
* @param	InstancePtr is not used, but contains a pointer to the IIC
*			device driver instance which the handler is being called for.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void iicReceiveHandler(XIic *InstancePtr)
{
	ReceiveComplete = 0;

#ifdef __DEBUG
	xil_printf("i2c receive interrupt\n");
#endif

}

/*****************************************************************************/
/**
* @brief I2c interrupt handler for status
* This Status handler is called asynchronously from an interrupt
* context and indicates the events that have occurred.
*
* @param	InstancePtr is a pointer to the IIC driver instance for which
*			the handler is being called for.
* @param	Event indicates the condition that has occurred.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void iicStatusHandler(XIic * InstancePtr, int Event)
{

#ifdef __DEBUG
	xil_printf("I2C: Status Interrupt\n");
#endif

}
