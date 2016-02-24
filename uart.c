/*
 * @file read.c
 *
 *  Created on: Aug 21, 2015
 *      Author: Howard Graves
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "v7_rtsp_aurora.h"
#include "uart.h"

/*****************************************************************************/
/**
 * @brief gets an unsigned 32-bit value from the uart
 * This function performs a loopback test using the Aurora interface
 *
 * 		@param	p is a pointer to the parameters structure
 * 		@param	display 1-->show results
 * 		@param	BASE is the number base that the 32-bit value represents
 *
 * 		@return	success/failure
 *
 * 		@note 	none
 *
******************************************************************************/
unsigned int get_u32_value(params_struct *p, char display, int BASE) {

	char readBuffer[8], done, tempRead;
	unsigned int i, Status;
	unsigned int u32Value;

	for(i=0;i<8;i++)							// clear read buffer
		readBuffer[i] = 0;

	i = 0;
	done = 0;

	while (!done) {

		while(!(Status= p->pUART->status & 0x0001)); 	// wait for character

		tempRead = p->pUART->rx;				// get character

		if(tempRead != 0x20) {					// check for 'space'
			readBuffer[i] = tempRead;			// store character if not a 'space'

			if(display)
				xil_printf("%c",tempRead);

			i++;

			if(i==8) {
				done = 1;
			}
		}

		if( (tempRead == 0x0d) || (tempRead == 0x20) ) {		// 'enter' or space indicates the end input
			done = 1;
		}
	}

	u32Value = (unsigned int) strtoul(readBuffer,NULL,BASE);	// convert to unsigned long

	return u32Value;
}

/***************************************************************************************/
/**
 * @brief send a block of data out the UART
 * This function reads a block from memory as unsigned bytes and send them out of the UART
 *
 *		@param	uart		(volatile uart_struct *)	- pointer to uart
 *		@param	addr		(unsigned char *)			- pointer to starting address
 *		@param	numwords	(unsigned int) 				- number of words to dump
 *		@param	disp		(char) 						- display mode
 *
 *		@return		none
 *
 *		@note		none
 *
 ******************************************************************************************/
void read_u8(volatile uart_struct *uart, unsigned char *addr, unsigned int numwords, char disp) {

	unsigned int i, j, status;
	unsigned char readData;


	 for (i = 0; i < numwords; i++) {

		 readData = *addr;							// read data from ddr


		 if(disp) {

			 if(i % 16 == 0)
				 xil_printf("\n%08x - ",addr);

			 xil_printf("%02x ",readData);		// in ASCII mode, print out data
		 }
	 	 else {
 			 uart->tx = readData;
	 	 }

	 	 addr++;

	 	 while(!(status = uart->status & 0x0004));					// give the transmitter time to breathe

	 	 for(j=0;j<10;j++);

	 	 if((status=uart->status & 0x0001))							// quit if we receive a key stroke
	 	 	 break;
	 }
}

/***************************************************************************************/
/**
 * @brief send a block of data out the UART
 * This function reads a block from memory as unsigned shorts and send them out of the UART
 *
 *		@param	uart		- pointer to uart
 *		@param	addr		- pointer to starting address
 *		@param	numwords	- number of words to dump
 *		@param	disp		- display mode
 *
 *		@return		none
 *
 *		@note		none
 *
 ******************************************************************************************/
void read_u16(volatile uart_struct *uart, unsigned short *addr, unsigned int numwords, char disp) {

	unsigned int i, status;
	unsigned short readData;

	 for (i = 0; i < numwords; i++) {

		 readData = *addr;							// read data from ddr


		 if(disp) {

			 if(i % 8 == 0)
				 xil_printf("\n%08x - ",addr);

			 xil_printf("%04x ",readData);		// in ASCII mode, print out data
		 }
	 	 else {
 			 uart->tx = readData;
	 	 }

	 	 addr++;

	 	while(!(status = uart->status & 0x0004));					// give the transmitter time to breathe

	 	 if((status=uart->status & 0x0001))							// quit if we receive a key stroke
	 	 	 break;
	 }
}


/***************************************************************************************/
/**
 * @brief send a block of data out the UART
 * This function reads a block from memory as unsigned ints and send them out of the UART
 *
 *		@param	uart		- pointer to uart
 *		@param	addr		- pointer to starting address
 *		@param	numwords	- number of words to dump
 *		@param	disp		- display mode
 *
 *		@return		none
 *
 *		@note		none
 *
 ******************************************************************************************/
 void read_u32(volatile uart_struct *uart, unsigned int *addr, unsigned int numwords, char disp) {

	unsigned int i, status;
	unsigned int readData;

	 for (i = 0; i < numwords; i++) {

		 readData = *addr;							// read data from ddr

		 if(disp) {

			 if(i % 4 == 0)
				 xil_printf("\n%08x - ",addr);

			 xil_printf("%08x ",readData);		// in ASCII mode, print out data

		 }
	 	 else {
 			 uart->tx = readData;
	 	 }

	 	 addr++;

	 	while(!(status = uart->status & 0x0004));					// give the transmitter time to breathe

	 	 if((status=uart->status & 0x0001))							// quit if we receive a key stroke
	 	 	 break;
	 }
}

