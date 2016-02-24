/*
 * @file dma.c
 * AXI DMA functions
 *
 *  Created on: Jan 11, 2016
 *      Author: Howard Graves
 */

#include "dma.h"
#include "xil_cache.h"

/*****************************************************************************/
/**
 * @brief Initialize AXI DMA controller
 * This function initializes the DMA controller
 *
 * @param	dmaController holds a pointer to the DMA controller instance
 * @param	rxBuffer holds a pointer to the receive data buffer
 * @param	txBuffer holds a pointer to the transmit data buffer
 *
 * @return	success/failure
 *
 * @note 	none
 *
******************************************************************************/
int initDMA( XAxiDma *dmaController, u8 *rxBuffer, u8 *txBuffer) {

	int status;

	XAxiDma_Config *Config;

	Config = XAxiDma_LookupConfig(DMA_DEV_ID);
	if (!Config) {
		xil_printf("DMA: No config found for %d\r\n", DMA_DEV_ID);

		return XST_FAILURE;
	}

#ifdef __DEBUG
	xil_printf("\nDeviceId - %d\n",Config->DeviceId);
	xil_printf("BaseAddr - 0x%08X\n",Config->BaseAddr);
	xil_printf("HasStsCntrlStrm - %d\n",Config->HasStsCntrlStrm);
	xil_printf("HasMm2S - %d\n",Config->HasMm2S);
	xil_printf("HasMm2SDRE - %d\n",Config->HasMm2SDRE);
	xil_printf("Mm2SDataWidth - %d\n",Config->Mm2SDataWidth);
	xil_printf("HasS2Mm - %d\n",Config->HasS2Mm);
	xil_printf("HasS2MmDRE - %d\n",Config->HasS2MmDRE);
	xil_printf("S2MmDataWidth - %d\n",Config->S2MmDataWidth);
	xil_printf("HasSg - %d\n",Config->HasSg);
	xil_printf("Mm2sNumChannels - %d\n",Config->Mm2sNumChannels);
	xil_printf("S2MmNumChannels - %d\n",Config->S2MmNumChannels);
	xil_printf("Mm2SBurstSize - %d\n",Config->Mm2SBurstSize);
	xil_printf("S2MmBurstSize - %d\n",Config->S2MmBurstSize);
	xil_printf("MicroDmaMode - %d\n",Config->MicroDmaMode);
#endif

	/* Initialize DMA engine */
	status = XAxiDma_CfgInitialize(dmaController, Config);
	if (status != XST_SUCCESS) {
		xil_printf("DMA: Initialization failed %d\r\n", status);
		return XST_FAILURE;
	}

	if(XAxiDma_HasSg(dmaController)){
		xil_printf("DMA: Device configured as SG mode \r\n");
		return XST_FAILURE;
	}

	if(!dmaController->Initialized) {
		xil_printf("DMA: Not Initialized\n");
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}



/*****************************************************************************/
/**
 * @brief DMA Send.
 * This function DMA's a block of data to the aurora
 *
 * @param	p is a pointer to the parameters structure
 *
 * @return	success/failure
 *
 * @note 	none
 *
******************************************************************************/
int sendDMA( params_struct *p) {

	int status;

/* Initialize flags before start transfer test  */
	TxDone = 0;
	Error = 0;

	/*
	 * enable tx interrupt
	 */

	XAxiDma_IntrEnable(p->pAxiDma, XAXIDMA_IRQ_ALL_MASK,	XAXIDMA_DMA_TO_DEVICE);


	/* Flush the SrcBuffer before the DMA transfer, in case the Data Cache
	 * is enabled
	 */
	Xil_DCacheFlushRange((u32)p->pTxBuffer, p->testPacketSize);

	status = XAxiDma_SimpleTransfer(p->pAxiDma, (u32) p->pTxBuffer, p->testPacketSize, XAXIDMA_DMA_TO_DEVICE);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait TX done and RX done
	 */
	while (!TxDone && !Error) {
			/* NOP */
	}

	if (Error) {
		xil_printf("DMA ERROR -  transmit%s done, "
		"receive%s done\r\n", TxDone? "":" not",
						RxDone? "":" not");

#ifdef __DEBUG
	unsigned int *tmpAddr;
	unsigned int val;

	tmpAddr = (unsigned int *)(dmaController->RegBase+0x00);val = *(tmpAddr); xil_printf("\n0x%08X\t0x%08X\n",tmpAddr,val);
	tmpAddr = (unsigned int *)(dmaController->RegBase+0x04);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
	tmpAddr = (unsigned int *)(dmaController->RegBase+0x18);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
	tmpAddr = (unsigned int *)(dmaController->RegBase+0x1c);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
	tmpAddr = (unsigned int *)(dmaController->RegBase+0x28);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
	tmpAddr = (unsigned int *)(dmaController->RegBase+0x30);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
	tmpAddr = (unsigned int *)(dmaController->RegBase+0x34);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
	tmpAddr = (unsigned int *)(dmaController->RegBase+0x48);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
	tmpAddr = (unsigned int *)(dmaController->RegBase+0x4c);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
	tmpAddr = (unsigned int *)(dmaController->RegBase+0x58);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
#endif

	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief DMA Receive
 * This function receives a block of data from the aurora
 *
 * @param	p is a pointer to the parameters structure
 *
 * @return	success/failure
 *
 * @note 	none
 *
******************************************************************************/
int receiveDMA( params_struct *p) {

	int status;

/* Initialize flags before start transfer test  */
	RxDone = 0;
	Error = 0;

	/*
	 * enable rx interrupt
	 */

	XAxiDma_IntrEnable(p->pAxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);

	/* Flush the SrcBuffer before the DMA transfer, in case the Data Cache
	 * is enabled
	 */
	Xil_DCacheFlushRange((u32)p->pRxBuffer, p->testPacketSize + 1);

	status = XAxiDma_SimpleTransfer(p->pAxiDma,(u32) p->pRxBuffer, p->testPacketSize + 1, XAXIDMA_DEVICE_TO_DMA);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * return to calling function to wait for data
	 */
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief display the AXI DMA registers
 * This function displays the current values in the DMA registers
 *
 * @param	dmaController holds a pointer to the DMA controller instance
 *
 * @return	none
 *
 * @note 	none
 *
******************************************************************************/
void displayDmaRegisters(XAxiDma *dmaController) {

	unsigned int *tmpAddr;
	unsigned int val;

	tmpAddr = (unsigned int *)(dmaController->RegBase+0x00);val = *(tmpAddr); xil_printf("\n0x%08X\t0x%08X\n",tmpAddr,val);
	tmpAddr = (unsigned int *)(dmaController->RegBase+0x04);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
	tmpAddr = (unsigned int *)(dmaController->RegBase+0x18);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
	tmpAddr = (unsigned int *)(dmaController->RegBase+0x1c);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
	tmpAddr = (unsigned int *)(dmaController->RegBase+0x28);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
	tmpAddr = (unsigned int *)(dmaController->RegBase+0x30);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
	tmpAddr = (unsigned int *)(dmaController->RegBase+0x34);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
	tmpAddr = (unsigned int *)(dmaController->RegBase+0x48);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
	tmpAddr = (unsigned int *)(dmaController->RegBase+0x4c);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
	tmpAddr = (unsigned int *)(dmaController->RegBase+0x58);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);

}

/*****************************************************************************/
/**
 * @brief get the number of bytes received
 * This function return the number of bytes received from the DMA controller
 *
 * @param	dmaController holds a pointer to the DMA controller instance
 *
 * @return	none
 *
 * @note 	none
 *
******************************************************************************/
unsigned int getDmaBytesReceived(XAxiDma *dmaController) {
	unsigned int *tmpAddr;
	unsigned int val;

	tmpAddr = (unsigned int *)(dmaController->RegBase+0x58);
	val = *(tmpAddr);
	return val;

}
