/*
 * @file tests.c
 *
 *  Created on: Feb 4, 2016
 *      Author: Howard Graves
 */

#include "tests.h"
#include "nwl_dma.h"
#include "interrupt.h"
#include "dma.h"

/*****************************************************************************/
/**
 * @brief PCI to Aurora loop back test.
 * This test will receive an RTSP frame packet from the PC over PCIe using
 * the NWL DMA controller.  Once a packet has been received, an interrupt will
 * be issued from the NWL DMA controller.  Once the interrupt has been received,
 * the AXI DMA controller will transfer the packet from DDR to the Aurora
 * controller.  The data will be looped back to the Aurora controller and the
 * AXI DMA controller will transfer the received packet to another section of
 * the DDR.  At the conclusion of the test, the packet received will be compared
 * the the looped-back packet to make sure the transfers were successful.
 *
 *
 *    PC -->  NWL DMA --> DDR --> AXI DMA --> Aurora ---
 *                                                      |
 *                                                      |
 *                        DDR <-- AXI DMA <-- Aurora <--
 *
 *
 * 		@param	p points to parameters structure
 *
 * 		@return	success/failure
 *
 * 		@note 	none
 *
******************************************************************************/
int PCIeAuroraLoopbackTest(params_struct *p) {

	int status;

	p->pTxBuffer = (u8 *)p->dataSourceLocation;
	p->pRxBuffer = (u8 *)p->dataDestinationLocation;


	nwlInterruptFlag = 0;
	resetAxiInterrupt(0);
	enableInterrupts(p, NWL_INTERRUPT);

	xil_printf("start PCIe transfer...\n");

	while(!nwlInterruptFlag) {
		/* NOP */
	}

	nwlInterruptFlag = 0;

	xil_printf("got NLW interrupt\n");

	p->testPacketSize = (p->ptr_RtspFrameHeader->dataSize + 4) * 4;

	xil_printf("packet size - %d\n",p->testPacketSize);

	status = receiveDMA(p);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	xil_printf("receive set up\n");

	status = sendDMA(p);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	xil_printf("sent\n");

	while (!RxDone) {
			/* NOP */
	}

	xil_printf("transfer complete\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief Run the aurura loopback test
 * This function performs a loopback test using the Aurora interface
 *
 * 		@param	p is a pointer to the parameters structure
 *
 * 		@return	success/failure
 *
 * 		@note 	none
 *
******************************************************************************/
int AuroraloopbackTest(params_struct *p) {

	int status;
	u8 value;
	int index;


/* Initialize flags before start transfer test  */
	TxDone = 0;
	RxDone = 0;
	Error = 0;

	value = 0x0C;

	/*
	 * clear tx and rx buffers
	 */
	for(index = 0; index < DMA_TEST_VALUES; index ++) {
		p->pTxBuffer[index] = 0x00;
		p->pRxBuffer[index] = 0x00;
	}

	/*
	 * fill transmit buffer
	 */
	for(index = 0; index < DMA_TEST_VALUES; index ++) {
		p->pTxBuffer[index] = value;

			value = (value + 1) & 0xFF;
	}

	/*
	 * clear tx and rx buffers
	 */

	XAxiDma_IntrEnable(p->pAxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

	XAxiDma_IntrEnable(p->pAxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);

	/* Flush the SrcBuffer before the DMA transfer, in case the Data Cache
	 * is enabled
	 */
	Xil_DCacheFlushRange((u32)p->pTxBuffer, DMA_TEST_VALUES);
	Xil_DCacheFlushRange((u32)p->pRxBuffer, DMA_TEST_VALUES);

	status = XAxiDma_SimpleTransfer(p->pAxiDma,(u32) p->pRxBuffer, DMA_TEST_VALUES, XAXIDMA_DEVICE_TO_DMA);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	status = XAxiDma_SimpleTransfer(p->pAxiDma,(u32) p->pTxBuffer, DMA_TEST_VALUES, XAXIDMA_DMA_TO_DEVICE);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait TX done and RX done
	 */
	while (!TxDone && !RxDone && !Error) {
			/* NOP */
	}

#ifdef __DEBUG
#ifdef __DEBUG_VERBOSE
	xil_printf("\nPosttest\n");
	for(index=0; index<DMA_TEST_VALUES; index++) {
		xil_printf("tx - 0x%02X\trx - 0x%02X\n",txBuffer[index],rxBuffer[index]);
	}
#endif
#endif

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

	for(index=0; index<DMA_TEST_VALUES; index++) {
		if(p->pTxBuffer[index] != p->pRxBuffer[index]) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}
