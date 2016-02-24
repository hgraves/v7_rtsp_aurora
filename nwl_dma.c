/*
 * @file nwl_dma.c
 *
 *  Created on: Feb 2, 2016
 *      Author: Howard Graves
 */

#include "nwl_dma.h"
#include "common.h"
#include "uart.h"

/*****************************************************************************/
/**
 * @brief reset the NWL AXI interrupt
 * This function resets and enables the AXI interrupt from slave port on the
 * NWL DMA controller
 *
 * @param	channel selects the DMA channel to reset
 *
 * @return	success/failure
 *
 * @note 	none
 *
******************************************************************************/
void resetAxiInterrupt(unsigned int channel) {

    unsigned short *us_Address;

	us_Address = (unsigned short *)0x4800003E;

	*us_Address = 0x0701;				// write status register

}

#if 0
/*****************************************************************************/
/**
 * @brief initiate a DMA transfer from the slave port
 *
 * @param
 *
 * @return
 *
 * @note 	none
 *
******************************************************************************/
int startDMA(params_struct *p, unsigned int channel) {

	/* Pseudo code
	 *
	 * 		Source 		 	- 0x80100000
	 * 		Destination		- 0x80200000
	 * 		Source SGL		- 0x80300000
	 * 		Destination SGL	- 0x80400000
	 * 		Status SGL		- 0x80500000
	 *
	 * 		- Load data at source address
	 * 		- clear destination address
	 * 		- load source SGL with source information
	 * 		- load destination SGL with destination information
	 * 		- disable DMA channel register
	 * 		- load DMA channel register
	 * 		- initiate transfer
	 *
	 */

	unsigned int *pSourceAddress;							/* pointer to source data location */
	unsigned int *pDestinationAddress;						/* pointer to destination data location */
	struct sgElement *source_sge;							/* pointer to source SGL element structure */
	struct sgElement *destination_sge;						/* pointer to destination SGL element structure */
	struct sgStatusElement *status_sge;						/* pointer to status SGL element structure */

	unsigned int *dmaRegister;
	unsigned short *dmaRegister16;
	unsigned short *us_writeAddress;
	unsigned char *uc_writeAddress;

	unsigned int tmpRegBase = 0x48000000;

	unsigned int i, k;
	unsigned int size = 128;

	pSourceAddress = (unsigned int *)p->dataSourceLocation;
	pDestinationAddress = (unsigned int *)p->dataDestinationLocation;

	source_sge = (struct sgElement *)p->sourceSglAddress;
	destination_sge = (struct sgElement *)p->destinationSglAddress;
	status_sge = (struct sgStatusElement *)p->statusSglAddress;

	for(i=0; i<size; i++) {
		*pSourceAddress = i;
		*pDestinationAddress = 0;

		pSourceAddress++;
		pDestinationAddress++;
	}

	/* assign Scatter-Gather Elements */
	/* first source element */
	source_sge->addressHi = 0x00;
	source_sge->addressLo = p->dataSourceLocation;
	source_sge->byteCount = 0x100;			// 256
	source_sge->flags = 0x05;
	source_sge->reserved = 0x00;

	/* second source element */
	source_sge = (struct sgElement *)(p->sourceSglAddress | 0x10);
	source_sge->addressHi = 0x00;
	source_sge->addressLo = p->dataSourceLocation | 0x100;
	source_sge->byteCount = 0x100;			// 256
	source_sge->flags = 0x07;
	source_sge->reserved = 0x00;


	/* first destination element */
	destination_sge->addressHi = 0x00;
	destination_sge->addressLo = p->dataDestinationLocation;
	destination_sge->byteCount = 0x100;
	destination_sge->flags = 0x01;
	destination_sge->reserved = 0x00;

	/* second destination element */
	destination_sge = (struct sgElement *)(p->destinationSglAddress | 0x10);
	destination_sge->addressHi = 0x00;
	destination_sge->addressLo = p->dataDestinationLocation | 0x100;
	destination_sge->byteCount = 0x100;
	destination_sge->flags = 0x01;
	destination_sge->reserved = 0x00;

	/* reset AXI Interrupts */
	resetAxiInterrupt(0);

	xil_printf("AXI Interrput Reset\n>");

	/* reset DMA engine */
	us_writeAddress = (unsigned short *)(0x4800003c); *us_writeAddress = 0x0004;			// DMA_CONTROL 		0x3c

	for(k=0;k<10000;k++);

	us_writeAddress = (unsigned short *)(0x4800003c); *us_writeAddress = 0x0000;		// DMA_CONTROL 		0x3c
	uc_writeAddress = (unsigned char *)(0x80500000); *uc_writeAddress = 0x00;			// reset SGL status eror

	xil_printf("DMA Engine Reset\n\n>");


	/* reset DMA engine */
	dmaRegister16 = (unsigned short *)(tmpRegBase + 0x3c); *dmaRegister16 = 0x0004;							// DMA_CONTROL 		0x3c

	/* Disable DMA Engine */
	dmaRegister16 = (unsigned short *)(tmpRegBase + 0x3c); *dmaRegister16 = 0x0000;							// DMA_CONTROL 		0x3c

	/* Init DMA registers */
	dmaRegister = (unsigned int *)tmpRegBase; *dmaRegister = (p->sourceSglAddress | 0x01);					// SRC_Q_PTR_LO 	0x00
	dmaRegister = (unsigned int *)(tmpRegBase + 0x04); *dmaRegister = 0x00000000;						// SCR_Q_PTR_HI 	0x04
	dmaRegister = (unsigned int *)(tmpRegBase + 0x08); *dmaRegister = 0x00000010;						// SRC_Q_SIZE 		0x08
	dmaRegister = (unsigned int *)(tmpRegBase + 0x0c); *dmaRegister = 0x00000000;						// SRC_Q_LIMIT 		0x0c
	dmaRegister = (unsigned int *)(tmpRegBase + 0x30); *dmaRegister = 0x00000000;						// SRC_Q_NEXT 		0x30

	dmaRegister = (unsigned int *)(tmpRegBase + 0x10); *dmaRegister = (p->destinationSglAddress | 0x01);	// DST_Q_PTR_LO 	0x10
	dmaRegister = (unsigned int *)(tmpRegBase + 0x14); *dmaRegister = 0x00000000;						// DST_Q_PTR_HI 	0x14
	dmaRegister = (unsigned int *)(tmpRegBase + 0x18); *dmaRegister = 0x00000010;						// DST_Q_SIZE 		0x18
	dmaRegister = (unsigned int *)(tmpRegBase + 0x1c); *dmaRegister = 0x00000000;						// DST_Q_LIMIT 		0x1c
	dmaRegister = (unsigned int *)(tmpRegBase + 0x34); *dmaRegister = 0x00000000;						// DST_Q_NEXT 		0x34

	dmaRegister = (unsigned int *)(tmpRegBase + 0x20); *dmaRegister = (p->statusSglAddress | 0x01);		// STA_Q_PTR_LO 	0x20
	dmaRegister = (unsigned int *)(tmpRegBase + 0x24); *dmaRegister = 0x00000000;						// STA_Q_PTR_HI 	0x24
	dmaRegister = (unsigned int *)(tmpRegBase + 0x28); *dmaRegister = 0x00000010;						// STA_Q_SIZE 		0x28
	dmaRegister = (unsigned int *)(tmpRegBase + 0x2c); *dmaRegister = 0x0000000f;						// STA_Q_LIMIT 		0x2c
	dmaRegister = (unsigned int *)(tmpRegBase + 0x38); *dmaRegister = 0x00000000;						// STA_Q_NEXT 		0x38

	/* Enable interrupts */
	dmaRegister16 = (unsigned short *)(tmpRegBase + 0x3e); *dmaRegister16 = 0x0101;						// STATUS 			0x3e

	xil_printf("DMA Setup Complete\n\n>");


	/* Enable DMA Engine */
	us_writeAddress = (unsigned short *)(0x4800003c); *us_writeAddress = 0x0001;						// DMA_CONTROL 		0x3c
	xil_printf("DMA Engine Enabled\n");

	xil_printf("bump\n");
	dmaRegister = (unsigned int *)(tmpRegBase + 0x0c); *dmaRegister = 0x00000001;						// SRC_Q_LIMIT 		0x0c
	dmaRegister = (unsigned int *)(tmpRegBase + 0x1c); *dmaRegister = 0x00000001;						// DST_Q_LIMIT 		0x1c

	xil_printf("bump\n");
	dmaRegister = (unsigned int *)(tmpRegBase + 0x0c); *dmaRegister = 0x00000002;						// SRC_Q_LIMIT 		0x0c
	dmaRegister = (unsigned int *)(tmpRegBase + 0x1c); *dmaRegister = 0x00000002;						// DST_Q_LIMIT 		0x1c

	dmaResults(p);

	xil_printf("\n\n");

	if(status_sge->completed)
		xil_printf("DMA Complete\n");
	else
		xil_printf("DMA Not Complete\n");


	if(status_sge->destinationError)
		xil_printf("Destination Error\n");

	if(status_sge->sourceError)
		xil_printf("Source Error\n");

	if(status_sge->internalError)
		xil_printf("Internal Error\n");

	xil_printf("\n>");

	return (0);

}
#endif

/*****************************************************************************/
/**
 * @brief display the DMA results
 * This function displays the memory locations associated with a DMA transfer
 *
 * @param	pParams is a pointer to the parameters structure
 *
 * @return	success/failure
 *
 * @note 	none
 *
******************************************************************************/
void dmaResults(params_struct *pParams) {

	xil_printf("Source memory");
	xil_printf("\n           0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
	xil_printf("\n----------------------------------------------------------");
	read_u8(pParams->pUART, (unsigned char *)pParams->dataSourceLocation, 128, 1);	// execute dump

	xil_printf("\n\nDestination Memory\n");
	read_u8(pParams->pUART, (unsigned char *)pParams->dataDestinationLocation, 128, 1);	// execute dump

	xil_printf("\n\n>");
}

