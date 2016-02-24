/*
 * @file v7_rtsp_aurora.c
 * @brief top level file containing main entry
 *
 *  Created on: Dec 18, 2015
 *      Author: Howard Graves
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "xil_cache.h"
#include "xstatus.h"
#include "xil_types.h"
#include "xil_testmem.h"

#include "xintc.h"
#include "xintc.h"
#include "xaxidma.h"
#include "xiic.h"
#include "mb_interface.h"

#include "platform.h"
#include "v7_rtsp_aurora.h"
#include "uart.h"
#include "utilities.h"
#include "interrupt.h"
#include "nwl_dma.h"
#include "tests.h"

//GPIO
//0  	LED#6 on VC709
//1-4	LEDs 1-4 on extension card
//5-7	not used
//8		DMA slave region (0-PCIe, 1-AXI)

/*****************************************************************************/
/**
* @brief v7_rtsp_aurora main routine.
* This the the main entry point the the v7_rtsp_aurora application
*
* @return	XST_SUCCESS/XST_FAILURE
*
* @note
*
******************************************************************************/
int main()
{

	params_struct Params;
	params_struct *pParams;
	pParams = &Params;

	unsigned int *hwGPIO;
    unsigned int *fwVersionReg;

    unsigned int channel;

    unsigned char *uc_writeAddress;
    unsigned short *us_writeAddress;
    unsigned int *ul_writeAddress;

    /*
     * variables to hold GPIO contents
     */
    unsigned int s_sideband_register = 0x00000202;
    unsigned int s_status_register = 0;
    unsigned int s_intr_rx = 0;
    unsigned int gpioRegister = 0;

    static XIic IicInstance;	/* The instance of the IIC device. */
	static XAxiDma AxiDma;		/* Instance of the XAxiDma */
	static XIntc InterruptController;

	hwGPIO = (unsigned int *)XPAR_GPIO_0_BASEADDR;
    fwVersionReg = (unsigned int *)XPAR_VERSION_REGISTER_0_S00_AXI_BASEADDR;

	unsigned int i, status, tmpAddr, testFailed;
	unsigned long startingAddress, wordsToRead, wordToWrite;
	unsigned char err, writeBytes;

	unsigned int startAddr, endAddr, clearValue;

	char readBuffer[8], done, tempRead, display;
	char ok2read = 0;
	char readSize;

	unsigned int k;

	unsigned int frameCount;
	unsigned char auroraFrameCount=0;

	pParams->software_version = SW_VERSION;
	pParams->firmware_version = *fwVersionReg;
	pParams->pAxiDma = &AxiDma;
	pParams->pIicInstance = &IicInstance;
	pParams->pInterruptController = &InterruptController;
	pParams->pUART = (uart_struct *)XPAR_UARTLITE_0_BASEADDR;
	pParams->dataSourceLocation = 		0x80000000;
	pParams->dataDestinationLocation = 	0x90000000;
	pParams->testPacketSize = 4096;
    pParams->nwlDmaSlaveRegisterBase = (unsigned int *)XPAR_M07_AXI_BASEADDR;
	pParams->pDmaChannelRegisters[0] = 	(dma_reg_struct *) (XPAR_M07_AXI_BASEADDR + 0x40);
	pParams->pDmaChannelRegisters[1] = 	(dma_reg_struct *) (XPAR_M07_AXI_BASEADDR + 0x80);
	pParams->pDmaChannelRegisters[2] = 	(dma_reg_struct *) (XPAR_M07_AXI_BASEADDR + 0xC0);
	pParams->pRxBuffer = (u8 *)DMA_RX_BUFFER_BASE;
	pParams->pTxBuffer = (u8 *)DMA_TX_BUFFER_BASE;
    pParams->ptr_sSidebandRegister = (struct strSSideband *)&s_sideband_register;
    pParams->ptr_sStatusRegister = (struct strSStatus *)&s_status_register;
    pParams->ptr_GpioSidebandReg = (unsigned int *)XPAR_S_SIDEBAND_REG_BASEADDR;
    pParams->ptr_GpioIntrRxReg = (unsigned int *)XPAR_S_INTR_RX_REG_BASEADDR;
    pParams->ptr_GpioStatusReg = (unsigned int *)XPAR_AXI_STATUS_REG_BASEADDR;
    pParams->ptr_GPIORegister = (gpio_reg_struct *)&gpioRegister;
    pParams->ptr_RtspFrameHeader = (strRtspFrameHeader *)0x80000004;

	init_platform();

	/* set up controllers */
	status = initControllers(pParams);
	if (status != XST_SUCCESS) {

		xil_printf( "Failed to initialize controllers\r\n");
		return XST_FAILURE;
	}

	// set default slave register values
    write_register(pParams->ptr_GpioSidebandReg, s_sideband_register);

	display = 1;

	printf("reseting aurora........");
	pParams->ptr_GPIORegister->auroraResetn = 1;		// release aurora_reset
	*hwGPIO = gpioRegister;
	for(i=0;i<5;i++);
	pParams->ptr_GPIORegister->gtResetn = 1;			// release gt_reset
	*hwGPIO = gpioRegister;
	printf("done\n");

	for(i=0;i<10000000;i++);

	display_menu(pParams);

	status = 0;
	startingAddress = 0;
	wordsToRead = 0;

	/*
	 * start of main loop
	 */
	while(1) {

		/*
		 * check for keystroke
		 */
		if((status = pParams->pUART->status & 0x0001)){

			switch(pParams->pUART->rx) {

				case '1':										// change memory region

					if(pParams->ptr_sSidebandRegister->s_arregion == 0x00) {
						pParams->ptr_sSidebandRegister->s_arregion = 0x02;
						pParams->ptr_sSidebandRegister->s_awregion = 0x02;
					} else {
						pParams->ptr_sSidebandRegister->s_arregion = 0x00;
						pParams->ptr_sSidebandRegister->s_awregion = 0x00;
					}

					pParams->ptr_sSidebandRegister->s_arid = 0x01;
					pParams->ptr_sSidebandRegister->s_awid = 0x01;

					pParams->ptr_sSidebandRegister->s_arpciecmd = _MEM_RD;
					pParams->ptr_sSidebandRegister->s_awpciecmd = _MEM_WR;

					pParams->ptr_sSidebandRegister->s_wid = 0x01;

					write_register(pParams->ptr_GpioSidebandReg, s_sideband_register);

					display_menu(pParams);

					break;
					display_menu(pParams);

					break;

				case '2':										// scroll read

					i=0;
					done = 0;
					ok2read = 0;

					for(i=0;i<8;i++)							// clear read buffer
						readBuffer[i] = 0;

					i=0;

		// first get starting address

					if(display)
						xil_printf("Display word size (1-byte, 2-16bit, 4-32bit) - ");

					while (!done) {

						while(!(status= pParams->pUART->status & 0x0001)); 	// wait for character

						readSize = pParams->pUART->rx;						// get character

						done = 1;

						xil_printf("%c\n",readSize);

					}

					if(display)
							xil_printf("Starting Address - 0x");

					startingAddress = get_u32_value(pParams, display, (int) 16);

					done = 0;

					while(!done) {
						switch (readSize) {
						case '1' :
							xil_printf("\n           0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
							xil_printf("\n----------------------------------------------------------");
							read_u8(pParams->pUART, (unsigned char *)startingAddress, 512, display);	// execute dump
							break;

						case '2' :
							xil_printf("\n           0    2    4    6    8    A    C    E");
							xil_printf("\n----------------------------------------------------------");
							read_u16(pParams->pUART, (unsigned short *)startingAddress, 256, display);	// execute dump
							break;

						case '4' :
							xil_printf("\n           0        4        8        C");
							xil_printf("\n----------------------------------------------------------");
							read_u32(pParams->pUART, (unsigned int *)startingAddress, 128, display);	// execute dump
							break;

						}

						xil_printf("\n<space> to continue, 'b' for back, 'q' to quit\n");

						while(!(status= pParams->pUART->status & 0x0001)); 	// wait for character

						tempRead = pParams->pUART->rx;						// get character

						switch (tempRead) {
							case 0x20 :
								startingAddress = startingAddress + 512;
								break;
							case 'b' :
								startingAddress = startingAddress - 512;
								break;
							case '1' :
								readSize = tempRead;
								break;
							case '2' :
								readSize = tempRead;
								break;
							case '4' :
								readSize = tempRead;
								break;
							case 'q' :
								done = 1;
								break;
							default:
								break;
						}

					}

					display_menu(pParams);

					break;

				case '3':										// clear all memory to 0x00
					if(display)
							xil_printf("\nStart Address - 0x");

					startAddr = get_u32_value(pParams, display, (int) 16);			// get starting address from uart

					if(display)
							xil_printf("\nEnd Address - 0x");

					endAddr = get_u32_value(pParams, display, (int) 16);			// get end address from uart

					if(display)
						xil_printf("\nClear Value (4byte) - 0x");

					clearValue = get_u32_value(pParams, display, (int) 16);		// get value to clear to from uart
					xil_printf("\nFilling DDR from 0x%08X to 0x%08X with 0x%08X\n",startAddr, endAddr, clearValue);

					clear_ddr((unsigned int *) startAddr, (unsigned int *) endAddr, clearValue, 0);
					xil_printf("\n>");
					break;
				case '4':
					s_status_register = *pParams->ptr_GpioStatusReg;

					if(pParams->ptr_sStatusRegister->channel_up == 1) {
						xil_printf("Running DMA/Aurora test.....\n");

						/*
						 * clear aurora rx and tx buffers
						 */
						for(i=0;i<AURORA_BUFFER_SIZE;i++) {
							pParams->pRxBuffer[i] = 0x00;
							pParams->pTxBuffer[i] = 0x00;
						}

						xil_printf("buffers cleared\n");

						status = PCIeAuroraLoopbackTest(pParams);
						if (status != XST_SUCCESS) {
							xil_printf("DMA: Initialization failed %d\r\n", status);
						}

						testFailed=0;
						for(i=0;i<pParams->testPacketSize;i++) {
							if(pParams->pTxBuffer[i] != pParams->pRxBuffer[i]){
								testFailed=1;
								break;
							}
						}

						if(testFailed)
							xil_printf("loopback failed\n>");
						else
							xil_printf("test passed\n>");
					} else xil_printf("Aurora channel not UP\n>");

					break;
				case '5':
					i = 0;
					done = 0;

					if(display)
						xil_printf("\nData - 0x");

					while (!done) {
						while(!(status= pParams->pUART->status & 0x0001)); 	// wait for character
						tempRead = pParams->pUART->rx;					// get character
						if(tempRead != 0x20) {						// ignore is 'space'
							readBuffer[i] = tempRead;				// store character
							if(display)
								xil_printf("%c",tempRead);
							i++;
						}
						if((tempRead == 0x20) || (tempRead == 0x0d)) {						// 'enter' or 'space' indicates end of command
							done = 1;
						}
					}
					writeBytes = (i-1)/2;

					if((writeBytes != 1) & (writeBytes != 2) & (writeBytes != 4)) {
						xil_printf("ERROR - Data must be 1, 2, or 4 bytes\n>");
						break;
					} else
						wordToWrite = strtoul(readBuffer,NULL,16);		// convert to unsigned long

					if(display)
							xil_printf("\nWrite Address - 0x");

					tmpAddr = get_u32_value(pParams, display, (int) 16);					// get  address from uart

					if ( (err = boundary_check(tmpAddr)) < writeBytes) {
						xil_printf("\nERROR - Address alignment doesn't match data\n>");
						break;
					}

					switch (writeBytes) {
					case 1 :
						uc_writeAddress = (unsigned char *) tmpAddr;
						*uc_writeAddress = wordToWrite;
						xil_printf("\nWrote 0x%02x ",wordToWrite);
						break;

					case 2:
						us_writeAddress = (unsigned short *) tmpAddr;
						*us_writeAddress = wordToWrite;
						xil_printf("\nWrote 0x%04x ",wordToWrite);
						break;

					case 4:
						ul_writeAddress = (unsigned int *) tmpAddr;
						*ul_writeAddress = wordToWrite;
						xil_printf("\nWrote 0x%08x ",wordToWrite);
						break;

					}
					xil_printf("to 0x%08x (%d)\n\n>", tmpAddr, pParams->ptr_sSidebandRegister->s_awregion);
					break;

				case '6':										// read DDR contents
					i=0;
					done = 0;
					ok2read = 0;

					for(i=0;i<8;i++)							// clear read buffer
						readBuffer[i] = 0;

					i=0;

		// first get starting address

					if(display)
						xil_printf("Enter word size (1-byte, 2-16bit, 4-32bit) - ");

					while (!done) {

						while(!(status= pParams->pUART->status & 0x0001)); 	// wait for character
						readSize = pParams->pUART->rx;						// get character
						done = 1;
						xil_printf("%c\n",readSize);
					}

					if(display)
							xil_printf("Starting Address - 0x");

					startingAddress = get_u32_value(pParams, display, (int) 16);

		// now get number of words to read

					if(display)
						xil_printf("\nWords to Read - ");

					wordsToRead = get_u32_value(pParams, display, (int) 10);

					if (wordsToRead == 0)							// if wordsToRead is 0, set to max
						wordsToRead = MAX_WORDS;

					if(display) {
						xil_printf("\nReading %d words from 0x%08x\n", wordsToRead, startingAddress );
					}

		// now get the go/stop command


					if(display) {
						for(i=0;i<8;i++)							// clear read buffer
							readBuffer[i] = 0;

						i=0;
						done = 0;

						xil_printf("Press <ENTER> to go, 'x' to abort\n");

						while (!done) {

							while(!(status= pParams->pUART->status & 0x0001)); 	// wait for character

							tempRead = pParams->pUART->rx;					// get character

							if(tempRead == 'x') {						// ignore is 'space'
								done = 1;
								ok2read = 0;
							}

							if(tempRead == 0x0d) {						// 'enter' indicates end of command
								ok2read = 1;
								done = 1;
							}

						}

					}

					if((ok2read == 1) | (display == 0)) {
						switch (readSize) {
						case '1' :
							xil_printf("\n           0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
							xil_printf("\n----------------------------------------------------------");
							read_u8(pParams->pUART, (unsigned char *)startingAddress, wordsToRead, display);	// execute dump
							break;

						case '2' :
							xil_printf("\n           0    2    4    6    8    A    C    E");
							xil_printf("\n----------------------------------------------------------");
							read_u16(pParams->pUART, (unsigned short *)startingAddress, wordsToRead, display);	// execute dump
							break;

						case '4' :
							xil_printf("\n           0        4        8        C");
							xil_printf("\n----------------------------------------------------------");
							read_u32(pParams->pUART, (unsigned int *)startingAddress, wordsToRead, display);	// execute dump
							break;

						}
					}
					else {
						xil_printf("Read Aborted %c\n",ok2read);

					}
					xil_printf("\n\n>");

					break;


				case '7':
					xil_printf("\nEnter Channel - ");

					done = 0;

					while (!done) {

						while(!(status= pParams->pUART->status & 0x0001)); 	// wait for character

						tempRead = pParams->pUART->rx;					// get received character

						if(display)
							xil_printf("%c\n",tempRead);

						channel = tempRead - 0x30;

						done = 1;
					}

					startingAddress = (unsigned long)( pParams->nwlDmaSlaveRegisterBase + (0x40 * channel));

					xil_printf("\n           0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
					xil_printf("\n----------------------------------------------------------");
					read_u8(pParams->pUART, (unsigned char *)startingAddress, 64, 1);	// execute dump

					xil_printf("\n");

					display_registers(pParams, channel);

					break;


				case '8':

					s_status_register = *pParams->ptr_GpioStatusReg;

					display_all(pParams);

					break;

				case '9':

					resetAxiInterrupt(0);

					xil_printf("AXI Interrput Reset\n>");

					break;

				case 'L':
				case 'l':
					s_status_register = *pParams->ptr_GpioStatusReg;

					if(pParams->ptr_sStatusRegister->channel_up == 1) {
						xil_printf("performing loopback test....");
						status = AuroraloopbackTest(pParams);

						if(status==XST_SUCCESS)
							xil_printf("Passed\n\n>");
						else
							xil_printf("FAILED\n\n>");
					} else xil_printf("Aurora channel not UP\n>");

					break;

				case 'V':
				case 'v':

					/* reset DMA engine */
					us_writeAddress = (unsigned short *)(0x4800003c); *us_writeAddress = 0x0004;			// DMA_CONTROL 		0x3c

					for(k=0;k<10000;k++);

					us_writeAddress = (unsigned short *)(0x4800003c); *us_writeAddress = 0x0000;		// DMA_CONTROL 		0x3c
					uc_writeAddress = (unsigned char *)(0x80500000); *uc_writeAddress = 0x00;			// reset SGL status eror

					xil_printf("DMA Engine Reset\n\n>");

					break;

				case 'R':
				case 'r':

					done = 0;

					/*
					 * enable AXI DMA tx interrupt
					 */
					XAxiDma_IntrEnable(pParams->pAxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

					/*
					 * clear NWL DMA interrupt
					 *
					 */
					resetAxiInterrupt(0);

					/*
					 * enable interrupts
					 */
					enableInterrupts(pParams, NWL_INTERRUPT || AXIDMA_TX_INTERRUPT);

					frameCount=0;


					/* Load first location of memory with a header */
					pParams->pTxBuffer = (u8 *)0x80000000;
					pParams->pTxBuffer[0] = 0xAA;
					pParams->pTxBuffer[1] = 0xBB;
					pParams->pTxBuffer[2] = 0xEB;
					pParams->pTxBuffer[3] = 0x90;

					xil_printf("Running (Press any key to quit)\n");

					while(!(pParams->pUART->status & 0x00000001)) {			// check for key press

						if(nwlInterruptFlag) {

							pParams->testPacketSize = ((pParams->ptr_RtspFrameHeader->dataSize + 4) * 4) + 4;

							xil_printf("packet size - %d\n",pParams->testPacketSize);

							status = XAxiDma_SimpleTransfer(pParams->pAxiDma, (u32) pParams->pTxBuffer, pParams->testPacketSize, XAXIDMA_DMA_TO_DEVICE);
							if (status != XST_SUCCESS) {
								return XST_FAILURE;
							}

							if((frameCount % 100) == 0)
								xil_printf(".");

							nwlInterruptFlag = 0;
							frameCount++;
						}

					}

					xil_printf("\n%d frames processed\n\n>",frameCount);

					disableInterrupts(pParams, ALL_INTERRUPTS);

					pParams->pTxBuffer = (u8 *)DMA_TX_BUFFER_BASE;

					break;

				case 'S':
				case 's':

					/*
					 * enable AXI DMA tx interrupt
					 */
					s_status_register = *pParams->ptr_GpioStatusReg;

					if(pParams->ptr_sStatusRegister->channel_up == 1) {
						xil_printf("\nNumber of bytes to send - ");
						pParams->testPacketSize = get_u32_value(pParams, display, (int) 10);			// get starting address from uart

						clearInterruptFlags();

						XAxiDma_IntrEnable(pParams->pAxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

						enableInterrupts(pParams, AXIDMA_TX_INTERRUPT);

						for(i=0;i<pParams->testPacketSize;i++) {
							pParams->pTxBuffer[i] = i;
						}
						pParams->pTxBuffer[0]=0xeb;
						pParams->pTxBuffer[1]=0x90;
						pParams->pTxBuffer[2]=auroraFrameCount;
						pParams->pTxBuffer[3]=auroraFrameCount;

						auroraFrameCount++;

//						for(i=0;i<pParams->testPacketSize;i++) {
//							if((i%16) == 0)
//								xil_printf("\n%08X\t%02X ",i,pParams->pTxBuffer[i]);
//							else
//								xil_printf("%02X ",pParams->pTxBuffer[i]);
//						}
//						xil_printf("\n");

						status = XAxiDma_SimpleTransfer(pParams->pAxiDma, (u32) pParams->pTxBuffer, pParams->testPacketSize, XAXIDMA_DMA_TO_DEVICE);
						if (status != XST_SUCCESS) {
							return XST_FAILURE;
						}

						disableInterrupts(pParams, ALL_INTERRUPTS);
						xil_printf("Sent %d bytes\n\n>",pParams->testPacketSize);
					} else xil_printf("Aurora channel not UP\n>");

					break;

				case 'M':										// display menu
				case 'm':
					display_menu(pParams);
					break;

				case '+':										// stream out data //

					/*make sure interrupt is clear */
					resetAxiInterrupt(0);

					/*Clear DDR*/
					clear_ddr((unsigned int *) 0x80000000, (unsigned int *) 0x80010000, 0x00, 1);

					uc_writeAddress = (unsigned char *)0x80000000;

					/* wait for interrupt */
					while ( !(s_intr_rx & 0x00000001) ) {

						s_intr_rx =  *pParams->ptr_GpioIntrRxReg;

					}

					/*got interrupt, clear it and pump out data */
					resetAxiInterrupt(0);

					for (i=0; i<4096; i++) {

						pParams->pUART->tx = *uc_writeAddress;

						uc_writeAddress++;

						while(!(err = pParams->pUART->status & 0x0004));					// give the transmitter time to breathe
					}

					xil_printf("done\n");

					break;

				case '-':										// test stream

					for (i=0; i<4096; i++) {

						pParams->pUART->tx = (unsigned int) (i & 0x000000ff);

						while((err = pParams->pUART->status & 0x0008));					// wait if fifo is full

					}

					break;

				case 0x0d:
					xil_printf("\n>");
					break;

				default:
					break;
			}
		} // end of if

		/*
		 * check for interrupts
		 */


	} // end of while

    return 0;

} // end of main






