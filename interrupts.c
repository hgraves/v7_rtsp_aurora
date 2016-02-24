/*
 * @file interrupts.c
 * @brief interrupt functions
 *
 *  Created on: Dec 18, 2015
 *      Author: Howard Graves
 */

#include "xintc.h"
#include "interrupt.h"
#include "XIic.h"
#include "xaxidma.h"
#include "common.h"
#include "xparameters.h"
#include "nwl_dma.h"

/*****************************************************************************/
/**
 * @brief set up the interrupt controller.
 * This function sets up the interrupt controller and registers all the
 * handlers
 *
 * @param	pParams is a pointer the the parameter structure
 *
 * @return	success/failure
 *
 * @note 	none
 *
******************************************************************************/
int setupInterruptController(params_struct *pParams) {

	int Status;

	/* Initialize the interrupt controller and connect the ISRs */
	Status = XIntc_Initialize(pParams->pInterruptController, XPAR_INTC_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {

		xil_printf( "Intc: Failed init\r\n");
		return XST_FAILURE;
	}

#ifdef __DEBUG
	xil_printf("Intc: initialized\n");
#endif

	/* Run self test on interrupt controller */
	Status = XIntc_SelfTest(pParams->pInterruptController);
	if (Status != XST_SUCCESS) {

		xil_printf( "Intc: Failed selftest\r\n");
		return XST_FAILURE;
	}

#ifdef __DEBUG
	xil_printf("Intc: self test complete\n");
#endif

	/* connect INTR pin to interrupt handler for NWLDMA*/
	Status = XIntc_Connect(pParams->pInterruptController, EXTERNAL_INTR_0_ID, (XInterruptHandler)nwlDMA_InterruptHandler, (void *)0);
	if (Status != XST_SUCCESS) {

		xil_printf( "Intc: Failed connect\r\n");
		return XST_FAILURE;
	}

#ifdef __DEBUG
	xil_printf("Intc: INTR connected\n");
#endif

	/* connect i2c controller to interrupt handler */
	Status = XIntc_Connect(pParams->pInterruptController, I2C_INTR_ID, (XInterruptHandler)XIic_InterruptHandler, pParams->pIicInstance);
	if (Status != XST_SUCCESS) {

		xil_printf( "Intc: Failed connect\r\n");
		return XST_FAILURE;
	}

#ifdef __DEBUG
	xil_printf("Intc: i2c connected\n");
#endif

	/* connect DMA-MM2S Interrupt to interrupt handler */
	Status = XIntc_Connect(pParams->pInterruptController, DMA_TX_INTR_ID, (XInterruptHandler)dmaMM2S_InterruptHandler, pParams->pAxiDma);
	if (Status != XST_SUCCESS) {

		xil_printf( "Intc: Failed connect\r\n");
		return XST_FAILURE;
	}

#ifdef __DEBUG
	xil_printf("Intc: MM2S Interrupt connected\n");
#endif

	/* connect DMA-S2MM Interrupt to interrupt handler */
	Status = XIntc_Connect(pParams->pInterruptController, DMA_RX_INTR_ID, (XInterruptHandler)dmaS2MM_InterruptHandler, pParams->pAxiDma);
	if (Status != XST_SUCCESS) {

		xil_printf( "Intc: Failed connect\r\n");
		return XST_FAILURE;
	}

#ifdef __DEBUG
	xil_printf("Intc: S2MM Interrupt connected\n");
#endif

	/* start the interrupt controller */
	Status = XIntc_Start(pParams->pInterruptController, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {

		xil_printf( "Intc: Failed start\r\n");
		return XST_FAILURE;
	}

#ifdef __DEBUG
	xil_printf("Intc: started\n");
#endif

	/* enable interrupts */
	enableInterrupts(pParams,ALL_INTERRUPTS);
//	XIntc_Enable(pParams->pInterruptController, EXTERNAL_INTR_0_ID);
//	XIntc_Enable(pParams->pInterruptController, I2C_INTR_ID);
//	XIntc_Enable(pParams->pInterruptController, DMA_TX_INTR_ID);
//	XIntc_Enable(pParams->pInterruptController, DMA_RX_INTR_ID);

//	microblaze_register_handler((XInterruptHandler)mbIntHandler,(void *)0);

	microblaze_enable_interrupts();

#ifdef __DEBUG
	xil_printf("Intc: return\n");
#endif

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief external interrupt handler
 * This function sets up the interrupt handler from the NWL DMA controller
 *
 * @param	CallbackRef is the pointer to the caller
 *
 * @return	none
 *
 * @note 	none
 *
******************************************************************************/
void nwlDMA_InterruptHandler(void *CallbackRef) {

#ifdef __DEBUG
	xil_printf("\nNWL Interrupt\n");
#endif

	nwlInterruptFlag = 1;
	resetAxiInterrupt(0);		// ack interrupt

}

#if 0
/*****************************************************************************/
/**
 * @brief microblaze interrupt handler
 *
 * This function setup the microblaze interrupt handler
 *
 * 		@param	none
 *
 * 		@return	none
 *
 * 		@note 	none
 *
******************************************************************************/
void mbIntHandler(void){

	/* disable interrupts */
	XIntc_Disable(pParams->pInterruptController, XPAR_INTC_0_DEVICE_ID);

	xil_printf("\nAXI Interrupt\n");

	/* clear interrupt controller interrupt */
	XIntc_Acknowledge((XIntc *)pParams->pInterruptController,XPAR_INTC_0_DEVICE_ID);

	/* enable interrupts */
	XIntc_Enable(pParams->pInterruptController, XPAR_INTC_0_DEVICE_ID);

}
#endif

/*****************************************************************************/
/**
 * @brief axi dma controller transmit interrupt handler
 * This function setup the transmit interrupt handler for the DMA controller
 * for transfers from memory mapped to stream
 *
 * @param	CallbackRef is the pointer to the caller
 *
 * @return	none
 *
 * @note 	none
 *
******************************************************************************/
void dmaMM2S_InterruptHandler(void *CallbackRef) {

	u32 IrqStatus = 0x00;
	int TimeOut;
	XAxiDma *AxiDmaInst = (XAxiDma *)CallbackRef;

#ifdef __DEBUG
	xil_printf("\nMM2S interrupt\n");
#endif

	/* Read pending interrupts */
	IrqStatus = XAxiDma_IntrGetIrq(AxiDmaInst, XAXIDMA_DMA_TO_DEVICE);

	/* Acknowledge pending interrupts */
	XAxiDma_IntrAckIrq(AxiDmaInst, IrqStatus, XAXIDMA_DMA_TO_DEVICE);

	/*
	 * If no interrupt is asserted, we do not do anything
	 */
	if (!(IrqStatus & XAXIDMA_IRQ_ALL_MASK)) {
		return;
	}

	/*
	 * If error interrupt is asserted, raise error flag, reset the
	 * hardware to recover from the error, and return with no further
	 * processing.
	 */
	if ((IrqStatus & XAXIDMA_IRQ_ERROR_MASK)) {

#ifdef __DEBUG
		unsigned int *tmpAddr;
		unsigned int val;

		xil_printf("\nTX Error (0x%08X)\n",IrqStatus);

		tmpAddr = (unsigned int *)(AxiDmaInst->RegBase+0x00);val = *(tmpAddr); xil_printf("\n0x%08X\t0x%08X\n",tmpAddr,val);
		tmpAddr = (unsigned int *)(AxiDmaInst->RegBase+0x04);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
		tmpAddr = (unsigned int *)(AxiDmaInst->RegBase+0x18);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
		tmpAddr = (unsigned int *)(AxiDmaInst->RegBase+0x1c);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
		tmpAddr = (unsigned int *)(AxiDmaInst->RegBase+0x28);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
		tmpAddr = (unsigned int *)(AxiDmaInst->RegBase+0x30);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
		tmpAddr = (unsigned int *)(AxiDmaInst->RegBase+0x34);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
		tmpAddr = (unsigned int *)(AxiDmaInst->RegBase+0x48);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
		tmpAddr = (unsigned int *)(AxiDmaInst->RegBase+0x4c);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
		tmpAddr = (unsigned int *)(AxiDmaInst->RegBase+0x58);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
#endif
		Error = 1;

		/*
		 * Reset should never fail for transmit channel
		 */

		XAxiDma_Reset(AxiDmaInst);
		TimeOut = RESET_TIMEOUT_COUNTER;


		while (TimeOut) {
			if (XAxiDma_ResetIsDone(AxiDmaInst)) {
				break;
			}

			TimeOut -= 1;
		}

		xil_printf("timeout\n");
		return;
	}

	/*
	 * If Completion interrupt is asserted, then set the TxDone flag
	 */
	if ((IrqStatus & XAXIDMA_IRQ_IOC_MASK)) {

#ifdef __DEBUG
		xil_printf("\nTX Done\n");
#endif

		TxDone = 1;
	}


}

/*****************************************************************************/
/**
 * @brief axi dma controller receive interrupt handler
 * This function setup the transmit interrupt handler for the DMA controller
 * for transfers from stream to memory mapped
 *
 * @param	CallbackRef is the pointer to the caller
 *
 * @return	none
 *
 * @note 	none
 *
******************************************************************************/
void dmaS2MM_InterruptHandler(void *CallbackRef) {

	u32 IrqStatus;
	int TimeOut;
	XAxiDma *AxiDmaInst = (XAxiDma *)CallbackRef;

#ifdef __DEBUG
	xil_printf("\nS2MM interrupt\n");
#endif

	/* Read pending interrupts */
	IrqStatus = XAxiDma_IntrGetIrq(AxiDmaInst, XAXIDMA_DEVICE_TO_DMA);

	/* Acknowledge pending interrupts */
	XAxiDma_IntrAckIrq(AxiDmaInst, IrqStatus, XAXIDMA_DEVICE_TO_DMA);

	/*
	 * If no interrupt is asserted, we do not do anything
	 */
	if (!(IrqStatus & XAXIDMA_IRQ_ALL_MASK)) {
		return;
	}

	/*
	 * If error interrupt is asserted, raise error flag, reset the
	 * hardware to recover from the error, and return with no further
	 * processing.
	 */
	if ((IrqStatus & XAXIDMA_IRQ_ERROR_MASK)) {

		xil_printf("\nRX Error (0x%08X)\n",IrqStatus);


#ifdef __DEBUG
		unsigned int *tmpAddr;
		unsigned int val;

		tmpAddr = (unsigned int *)(AxiDmaInst->RegBase+0x00);val = *(tmpAddr); xil_printf("\n0x%08X\t0x%08X\n",tmpAddr,val);
		tmpAddr = (unsigned int *)(AxiDmaInst->RegBase+0x04);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
		tmpAddr = (unsigned int *)(AxiDmaInst->RegBase+0x18);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
		tmpAddr = (unsigned int *)(AxiDmaInst->RegBase+0x1c);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
		tmpAddr = (unsigned int *)(AxiDmaInst->RegBase+0x28);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
		tmpAddr = (unsigned int *)(AxiDmaInst->RegBase+0x30);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
		tmpAddr = (unsigned int *)(AxiDmaInst->RegBase+0x34);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
		tmpAddr = (unsigned int *)(AxiDmaInst->RegBase+0x48);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
		tmpAddr = (unsigned int *)(AxiDmaInst->RegBase+0x4c);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);
		tmpAddr = (unsigned int *)(AxiDmaInst->RegBase+0x58);val = *(tmpAddr); xil_printf("0x%08X\t0x%08X\n",tmpAddr,val);

#endif

		Error = 1;

		/* Reset could fail and hang
		 * NEED a way to handle this or do not call it??
		 */
		XAxiDma_Reset(AxiDmaInst);
		TimeOut = RESET_TIMEOUT_COUNTER;

		while (TimeOut) {
			if(XAxiDma_ResetIsDone(AxiDmaInst)) {
				break;
			}
			TimeOut -= 1;
		}
		return;
	}

	/*
	 * If completion interrupt is asserted, then set RxDone flag
	 */
	if ((IrqStatus & XAXIDMA_IRQ_IOC_MASK)) {

#ifdef __DEBUG
		xil_printf("\nRX Done\n");
#endif

		RxDone = 1;
	}

}

/*****************************************************************************/
/**
 * @brief disable individual AXI interrupts
 * This function accepts an OR'ed list of devices interrupts to disable
 *
 * @param	p pointer to the params structure
 * @param	dev is an OR'ed list containing the desired devices to disable
 *
 * @return	none
 *
 * @note 	none
 *
******************************************************************************/
void disableInterrupts(params_struct *p, unsigned int dev) {

	if(dev && NWL_INTERRUPT) {
		XIntc_Disable(p->pInterruptController, EXTERNAL_INTR_0_ID);
#ifdef __DEBUG
		xil_printf("NWL_INTERRUPT disable\n");
#endif
	}

	if(dev && AXIDMA_TX_INTERRUPT) {
		XIntc_Disable(p->pInterruptController, I2C_INTR_ID);
#ifdef __DEBUG
		xil_printf("AXIDMA_TX_INTERRUPT disable\n");
#endif
	}

	if(dev && AXIDMA_RX_INTERRUPT) {
		XIntc_Disable(p->pInterruptController, DMA_TX_INTR_ID);
#ifdef __DEBUG
		xil_printf("AXIDMA_RX_INTERRUPT disable\n");
#endif
	}

	if(dev && I2C_INTERRUPT) {
		XIntc_Disable(p->pInterruptController, DMA_RX_INTR_ID);
#ifdef __DEBUG
		xil_printf("I2C_INTERRUPT disable\n");
#endif
	}


}

/*****************************************************************************/
/**
 * @brief enable individual AXI interrupts
 * This function accepts an OR'ed list of devices interrupts to enable
 *
 * @param	p pointer to the params structure
 * @param	dev is an OR'ed list containing the desired devices to enable
 *
 * @return	none
 *
 * @note 	none
 *
******************************************************************************/
void enableInterrupts(params_struct *p, unsigned int dev) {

	if(dev && NWL_INTERRUPT) {
		XIntc_Enable(p->pInterruptController, EXTERNAL_INTR_0_ID);
#ifdef __DEBUG
		xil_printf("NWL_INTERRUPT enable\n");
#endif
	}

	if(dev && AXIDMA_TX_INTERRUPT) {
		XIntc_Enable(p->pInterruptController, I2C_INTR_ID);
#ifdef __DEBUG
		xil_printf("AXIDMA_TX_INTERRUPT enable\n");
#endif
	}

	if(dev && AXIDMA_RX_INTERRUPT) {
		XIntc_Enable(p->pInterruptController, DMA_TX_INTR_ID);
#ifdef __DEBUG
		xil_printf("AXIDMA_RX_INTERRUPT enable\n");
#endif
	}

	if(dev && I2C_INTERRUPT) {
		XIntc_Enable(p->pInterruptController, DMA_RX_INTR_ID);
#ifdef __DEBUG
		xil_printf("I2C_INTERRUPT enable\n");
#endif
	}


}

void clearInterruptFlags(void) {
	TxDone=0;
	RxDone=0;
	Error=0;
	nwlInterruptFlag=0;
}
