/*
 * @file utilities.c
 * @brief utility functions
 *
 *  Created on: Aug 24, 2015
 *      Author: Howard Graves
 */

#include "utilities.h"
#include "interrupt.h"
#include "i2c.h"
#include "dma.h"

/*****************************************************************************/
/**
 * @brief initialize controllers.
 * This function initializes all the external controllers
 *
 * This function changes endian from big to little, or little to big
 *
 * 		@param	p points the the parameters structure
 *
 * 		@return	XST_SUCCESS/XST_FAILURE
 *
 * 		@note 	none
 *
******************************************************************************/
unsigned int initControllers(params_struct * p) {

	unsigned int status;

		xil_printf("starting initialization....\n");

		/*
		 * setup the interrupt controller
		 */
		xil_printf("setting up interrupt controller....");

		/* set up interrupt controller */
		status = setupInterruptController(p);
		if (status != XST_SUCCESS) {

			xil_printf( "Failed to initialize interrupt controller\r\n");
			return XST_FAILURE;
		} else
			xil_printf("done\n");

		/*
		 * Set the Handlers for i2c.
		 */
		xil_printf("setting up I2C......");

		status = IicInit((XIic *)p->pIicInstance);
		if (status != XST_SUCCESS) {
			xil_printf( "I2C Init: Failed\r\n");
			return XST_FAILURE;
		} else
			xil_printf("done\n");

		/*
		 * Setup the DMA controller
		 */
		xil_printf("setting up AXI DMA controller......");

		status = initDMA(p->pAxiDma, p->pRxBuffer, p->pTxBuffer);
		if (status != XST_SUCCESS) {
			xil_printf("DMA: Init failure\n");
			return XST_FAILURE;
		} else
			xil_printf("done\n");

		/*
		 * setup the si5324
		 */
		xil_printf("setting up SI5324......");

		status = setupSI5324((XIic *)p->pIicInstance);
		if (status != XST_SUCCESS) {
			xil_printf( "SI5324 Init: Failed\r\n");
			return XST_FAILURE;
		} else
			xil_printf("done\n");

		/*
		 * setup the si570
		 */
		xil_printf("setting up SI570......");

		status = setupSI570((XIic *)p->pIicInstance);
		if (status != XST_SUCCESS) {
			xil_printf( "SI570 Setup: Failed\r\n");
			return XST_FAILURE;
		} else
		 xil_printf("done\n");

		return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief swap endian.
 * This function changes endian from big to little, or little to big
 *
 * 		@param	data holds an unsigned int to be converter
 *
 * 		@return	swapped data
 *
 * 		@note 	none
 *
******************************************************************************/
unsigned int swap_endian(unsigned int data) {

	unsigned int b1,b2,b3,b4;

	b1 = (data & 0x000000ff) << 24;
	b2 = (data & 0x0000ff00) << 8;
	b3 = (data & 0x00ff0000) >> 8;
	b4 = (data & 0xff000000) >> 24;

	return (b1 | b2 | b3 | b4);

}

/*****************************************************************************/
/**
 * @brief check data boundary.
 * This function checks that data is on a 1, 2, or 4 byte boundary
 *
 * 		@param	address holds value to be checked
 *
 * 		@return	returns boundary
 *
 * 		@note 	none
 *
******************************************************************************/
int boundary_check(unsigned long address) {

	if ((address & 0x03) == 0)					// check for int(long) boundry
		return 4;
	else if ((address & 0x01) == 0)				// check for short boundry
		return 2;
	else
		return 1;
}

/*****************************************************************************/
/**
 * @brief clear a section of the DDR.
 *
 * 		@param	startAddr holds the start address
 *		@param	endAddr holds the end address
 *		@param	uVal holds the value for the memory to be cleared to
 *		@param 	silent 0-don't print "done"
 *
 * 		@return	none
 *
 * 		@note 	none
 *
******************************************************************************/
void clear_ddr( unsigned int *startAddr, unsigned int *endAddr, unsigned int uVal, unsigned char silent) {

	unsigned int i=0;

	while (startAddr <= endAddr) {

		*startAddr = uVal;

		 startAddr++;

		 if(!silent) {
			 if (i % (unsigned int) MOD == 0) {
				 xil_printf(".");
			 }
		 }

		 i++;

	}

	if(!silent)
		xil_printf("\ndone\n");

	return;
}

/*****************************************************************************/
/**
 * @brief display the menu
 *
 * 		@param	p points to the parameter structure
 *
 * 		@return	none
 *
 * 		@note 	none
 *
******************************************************************************/
void display_menu(params_struct *p) {
	xil_printf("\n******************************************************\n");
	xil_printf("VC709 RTSP (Slave DMA) (V. %d)\n",p->software_version);
	xil_printf("(Firmware - V.%d)\n",p->firmware_version);

	xil_printf("1 - Set Region\t\t\t5 - Write to Memory\n");
	xil_printf("2 - Scroll DDR\t\t\t6 - Read From Memory\n");
	xil_printf("3 - Clear DDR\t\t\t7 - Read DMA Registers\n");
	xil_printf("4 - Run PCI Loopback Test\t8 - Display Status\n");
	xil_printf("L - Aurora Loopback test\t9 - Clear AXI Interrupt\n");
	xil_printf("M - Display Menu\t\tW - Wait for Interrupt\n");
	xil_printf("S - Send Aurora Pkt\t\tR - Run RTSP\n");
	xil_printf("******************************************************\n\n");

	xil_printf("Region - ");

	switch (p->ptr_sSidebandRegister->s_arregion) {
		case 0 :
			xil_printf("(PCIe)\n");
			break;
		case 2 :
			xil_printf("(AXI)\n");
			break;
		default :
			xil_printf("(UNDEF)\n");
	}

	xil_printf("\nBAR0     - 0x80de0000\n");
	xil_printf("BAR1     - 0x80400000\n");
	xil_printf("BAR2     - 0x80800000\n");
	xil_printf("DMA Regs - 0x%08X\n",p->nwlDmaSlaveRegisterBase);

	xil_printf("\n>");

	return;
}

/*****************************************************************************/
/**
 * @brief display the NWL DMA registers
 *
 * 		@param	p points to the parameter structure
 * 		@param	ch holds the channel number of the register set to read
 *
 * 		@return	none
 *
 * 		@note 	none
 *
******************************************************************************/
void display_registers(params_struct *p, unsigned int ch) {

	xil_printf("\nDMA Channel %d\n", ch);
	xil_printf("(00) SRC_Q_PTR_LO - 0x%08x\n",p->pDmaChannelRegisters[ch]->SRC_Q_PTR_LO);
	xil_printf("(04) SCR_Q_PTR_HI - 0x%08x\n",p->pDmaChannelRegisters[ch]->SCR_Q_PTR_HI);
	xil_printf("(08) SRC_Q_SIZE   - 0x%08x\n",p->pDmaChannelRegisters[ch]->SRC_Q_SIZE);
	xil_printf("(0C) SRC_Q_LIMIT  - 0x%08x\n",p->pDmaChannelRegisters[ch]->SRC_Q_LIMIT);
	xil_printf("(10) DST_Q_PTR_LO - 0x%08x\n",p->pDmaChannelRegisters[ch]->DST_Q_PTR_LO);
	xil_printf("(14) DST_Q_PTR_HI - 0x%08x\n",p->pDmaChannelRegisters[ch]->DST_Q_PTR_HI);
	xil_printf("(18) DST_Q_SIZE   - 0x%08x\n",p->pDmaChannelRegisters[ch]->DST_Q_SIZE);
	xil_printf("(1C) DST_Q_LIMIT  - 0x%08x\n",p->pDmaChannelRegisters[ch]->DST_Q_LIMIT);
	xil_printf("(20) STA_Q_PTR_LO - 0x%08x\n",p->pDmaChannelRegisters[ch]->STA_Q_PTR_LO);
	xil_printf("(24) STA_Q_PTR_HI - 0x%08x\n",p->pDmaChannelRegisters[ch]->STA_Q_PTR_HI);
	xil_printf("(28) STA_Q_SIZE   - 0x%08x\n",p->pDmaChannelRegisters[ch]->STA_Q_SIZE);
	xil_printf("(2C) STA_Q_LIMIT  - 0x%08x\n",p->pDmaChannelRegisters[ch]->STA_Q_LIMIT);
	xil_printf("(30) SRC_Q_NEXT   - 0x%08x\n",p->pDmaChannelRegisters[ch]->SRC_Q_NEXT);
	xil_printf("(34) DST_Q_NEXT   - 0x%08x\n",p->pDmaChannelRegisters[ch]->DST_Q_NEXT);
	xil_printf("(38) STA_Q_NEXT   - 0x%08x\n",p->pDmaChannelRegisters[ch]->STA_Q_NEXT);
	xil_printf("(3C) DMA_CONTROL  - 0x%04x\n",p->pDmaChannelRegisters[ch]->DMA_CONTROL);
	xil_printf("(3E) PCIE_STATUS  - 0x%02x\n",p->pDmaChannelRegisters[ch]->PCIE_STATUS);
	xil_printf("(3F) AXI_STATUS   - 0x%02x\n\n",p->pDmaChannelRegisters[ch]->AXI_STATUS);

	return;
}

/*****************************************************************************/
/**
 * @brief write to a register
 *
 * 		@param	reg points to the register to be written
 * 		@param	val is the value to be written
 *
 * 		@return	none
 *
 * 		@note 	none
 *
******************************************************************************/

 void write_register(unsigned int *reg, unsigned int val) {

	*reg = val;

}

 /*****************************************************************************/
 /**
  * @brief display all status and settings
  *
  * 		@param	p points to the parameter structure
  *
  * 		@return	none
  *
  * 		@note 	none
  *
 ******************************************************************************/
 void display_all (params_struct *p) {

 	unsigned int tmp_intr_rx =  *p->ptr_GpioIntrRxReg;
 	unsigned int tmp_status_register = *p->ptr_GpioStatusReg;
 	unsigned int tmp_sSidebandReg = *p->ptr_GpioSidebandReg;

 	xil_printf("\n\nRegisters\n");
 	xil_printf("Sideband \t- %08X\n",tmp_sSidebandReg);
 	xil_printf("Status \t\t- %08X\n",tmp_status_register);

 	xil_printf("\ns_arregion \t\t-%01X\n",p->ptr_sSidebandRegister->s_arregion);
 	xil_printf("s_arid \t\t\t-%01X\n",p->ptr_sSidebandRegister->s_arid);
 	xil_printf("s_arpciecmd \t\t-%01X\n",p->ptr_sSidebandRegister->s_arpciecmd);
 	xil_printf("s_arerror \t\t-%01X\n",p->ptr_sSidebandRegister->s_arerror);

 	xil_printf("\ns_awregion \t\t-%01X\n",p->ptr_sSidebandRegister->s_awregion);
 	xil_printf("s_awid \t\t\t-%01X\n",p->ptr_sSidebandRegister->s_awid);
 	xil_printf("s_awpciecmd \t\t-%01X\n",p->ptr_sSidebandRegister->s_awpciecmd);
 	xil_printf("s_awerror \t\t-%01X\n",p->ptr_sSidebandRegister->s_awerror);

 	xil_printf("\ns_wid \t\t\t-%01X\n",p->ptr_sSidebandRegister->s_wid);
 	xil_printf("s_werror \t\t-%01X\n",p->ptr_sSidebandRegister->s_werror);

 	xil_printf("\ns_init_rx_edge_level_n \t-%01X\n",p->ptr_sSidebandRegister->s_init_rx_edge_level_n);

 	xil_printf("\ns_int_tx_edge_level_n \t-%01X\n",p->ptr_sStatusRegister->s_int_tx_edge_level_n);
 	xil_printf("s_link_up \t\t-%01X\n",p->ptr_sStatusRegister->s_link_up);
 	xil_printf("m_arerror \t\t-%01X\n",p->ptr_sStatusRegister->m_arerror);
 	xil_printf("m_awerror \t\t-%01X\n",p->ptr_sStatusRegister->m_awerror);
 	xil_printf("m_werror \t\t-%01X\n",p->ptr_sStatusRegister->m_werror);
 	xil_printf("mmcm_locked \t\t-%01X\n",p->ptr_sStatusRegister->mmcm_locked);
 	xil_printf("init_calib_complete \t-%01X\n",p->ptr_sStatusRegister->init_calib_complete);
 	xil_printf("tx_lock \t\t-%01X\n",p->ptr_sStatusRegister->tx_lock);
 	xil_printf("lane_up \t\t-%01X\n",p->ptr_sStatusRegister->lane_up);
 	xil_printf("channel_up \t\t-%01X\n",p->ptr_sStatusRegister->channel_up);
 	xil_printf("frame_err \t\t-%01X\n",p->ptr_sStatusRegister->frame_err);
 	xil_printf("hard_err \t\t-%01X\n",p->ptr_sStatusRegister->hard_err);
 	xil_printf("soft_err \t\t-%01X\n",p->ptr_sStatusRegister->soft_err);
 	xil_printf("pll_not_locked \t\t-%01X\n",p->ptr_sStatusRegister->pll_not_locked);
 	xil_printf("rx_resetdone_out \t-%01X\n",p->ptr_sStatusRegister->rx_resetdone_out);
 	xil_printf("tx_resetdone_out \t-%01X\n",p->ptr_sStatusRegister->tx_resetdone_out);
 	xil_printf("sfp1_tx_fault \t\t-%01X\n",p->ptr_sStatusRegister->sfp1_tx_fault);
 	xil_printf("sfp1_mod_detect \t-%01X\n",p->ptr_sStatusRegister->sfp1_mod_detect);


 	xil_printf("\ns_intr_rx \t\t-%08X\n",tmp_intr_rx);

 	xil_printf("\n>");

 }
