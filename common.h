/*
 * @file common.h
 *
 *  Created on: Dec 18, 2015
 *      Author: Howard Graves
 */

#ifndef COMMON_H_
#define COMMON_H_

#include "xparameters.h"
#include "xil_types.h"
#include "xintc.h"
#include "xintc.h"
#include "xaxidma.h"
#include "xiic.h"
#include "mb_interface.h"


/**
 * ************************ Constant Definitions ****************************
 */

#define SW_VERSION 150108

#define printf xil_printf	/* A smaller footprint printf */

#define BYTE_SEARCH 0
#define WORD_SEARCH 1

#define AURORA_BUFFER_SIZE		4096
#define AURORA_RXBUFFER_SIZE	AURORA_BUFFER_SIZE
#define AURORA_TXBUFFER_SIZE	AURORA_BUFFER_SIZE
#define AURORA_TEST_PKT_SIZE	1024

#define _PCIE_REGION	0x0
#define _AXI_REGION		0x2

#define _TYPE0_CFG_RD	0x0
#define _TYPE1_CFG_RD	0x1
#define _IO_RD			0x2
#define _MEM_RD 		0x3

#define _TYPE0_CFG_WR	0x0
#define _TYPE1_CFG_WR	0x1
#define _IO_WR			0x2
#define _MEM_WR 		0x3
#define _MSG_W_DATA		0x4
#define _MSG_WO_DATA	0x5

#define MOD 1048576

//#define MAX_WORDS	536870911		// 2GB x 32
//#define MAX_WORDS	268435455		// 1GB x 32
#define MAX_WORDS 134217728
//#define MAX_WORDS 65535
//#define MAX_WORDS 64

#define DDR_BASE	0x90000000
#define DDR_HIGH	0x9FFFFFFF

// PCA9548 8-port IIC Switch
#define IIC_SWITCH_ADDRESS 0x74
// Connected to IIC Buses
#define IIC_SI570_ADDRESS  0x5D			// Bus 0
#define IIC_FMC_HPC_ADDRESS 0x70		// Bus 1
#define IIC_FMC_LPC_ADDRESS 0x70		// Bus 2
#define IIC_EEPROM_ADDRESS 0x54			// Bus 3
#define IIC_SFP_ADDRESS 0x50			// Bus 4
#define IIC_ADV7511_ADDRESS 0x39		// Bus 5
#define IIC_DDR3_SPD_ADDRESS 0x50		// Bus 6
#define IIC_DDR3_TEMP_ADDRESS 0x18		// Bus 6
#define IIC_SI5326_ADDRESS 0x68			// Bus 7

#define IIC_BUS_0 0x01
#define IIC_BUS_1 0x02
#define IIC_BUS_2 0x04
#define IIC_BUS_3 0x08
#define IIC_BUS_4 0x10
#define IIC_BUS_5 0x20
#define IIC_BUS_6 0x40
#define IIC_BUS_7 0x80

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define INTC_DEVICE_ID              XPAR_INTC_0_DEVICE_ID
#define IIC_INTR_ID                 XPAR_INTC_0_IIC_0_VEC_ID

/*
 * The following constant defines the address of the IIC Slave device on the
 * IIC bus. Note that since the address is only 7 bits, this constant is the
 * address divided by 2.
 */
#define EEPROM_ADDRESS 0x54	/* 0xA0 as an 8 bit number. */

/*
 * The page size determines how much data should be written at a time.
 * The ML310/ML300 board supports a page size of 32 and 16.
 * The write function should be called with this as a maximum byte count.
 */
//#define PAGE_SIZE   16
#define PAGE_SIZE   8

/*
 * The following define the AXI DMA controller parameters
 */

#define DMA_DEV_ID			XPAR_AXIDMA_0_DEVICE_ID

#define DMA_TX_BUFFER_BASE	0xA0000000
#define DMA_TX_BUFFER_HIGH	0xA01FFFFF
#define DMA_RX_BUFFER_BASE	0xA0300000
#define DMA_RX_BUFFER_HIGH	0xA04FFFFF

#define DMA_RX_INTR_ID		XPAR_MICROBLAZE_0_AXI_INTC_AXI_DMA_0_S2MM_INTROUT_INTR
#define DMA_TX_INTR_ID		XPAR_MICROBLAZE_0_AXI_INTC_AXI_DMA_0_MM2S_INTROUT_INTR

#define RESET_TIMEOUT_COUNTER	10000

#define DMA_TEST_VALUES 0x100

#define TIMER_INTR_ID		XPAR_MICROBLAZE_0_AXI_INTC_AXI_TIMER_0_INTERRUPT_INTR
#define EXTERNAL_INTR_0_ID	XPAR_MICROBLAZE_0_AXI_INTC_SYSTEM_INTR_0_INTR
#define UART_INTR_ID		XPAR_MICROBLAZE_0_AXI_INTC_AXI_UARTLITE_0_INTERRUPT_INTR
#define I2C_INTR_ID			XPAR_MICROBLAZE_0_AXI_INTC_AXI_IIC_0_IIC2INTC_IRPT_INTR

#define NWL_INTERRUPT			XPAR_SYSTEM_INTR_0_MASK
#define UART_INTERRUPT			XPAR_AXI_UARTLITE_0_INTERRUPT_MASK
#define AXIDMA_TX_INTERRUPT		XPAR_AXI_DMA_0_MM2S_INTROUT_MASK
#define AXIDMA_RX_INTERRUPT		XPAR_AXI_DMA_0_S2MM_INTROUT_MASK
#define I2C_INTERRUPT			XPAR_AXI_IIC_0_IIC2INTC_IRPT_MASK
#define ALL_INTERRUPTS			0xFFFFFFFF


typedef unsigned int u32_t;
typedef u8 AddressType;

volatile u8 TransmitComplete;	/* Flag to check completion of Transmission */
volatile u8 ReceiveComplete;	/* Flag to check completion of Reception */

volatile int TxDone;
volatile int RxDone;
volatile int Error;
volatile int nwlInterruptFlag;

struct sgElement {
	unsigned int		addressLo			: 32;
	unsigned int		addressHi			: 32;
	unsigned int		byteCount			: 24;
	unsigned int		flags				: 8;
	unsigned int		reserved			: 32; 	// reserved
};

struct sgStatusElement {
		unsigned int	completed			: 1;
		unsigned int	sourceError			: 1;
		unsigned int	destinationError	: 1;
		unsigned int	internalError		: 1;
		unsigned int	reserved			: 28;	// reserved
};


struct strSSideband {
	unsigned int s_arregion 				: 4;
	unsigned int s_arid 					: 4;
	unsigned int s_awregion 				: 4;
	unsigned int s_awid 					: 4;
	unsigned int s_wid						: 4;
	unsigned int s_werror					: 1;
	unsigned int							: 2;
	unsigned int s_init_rx_edge_level_n		: 1;
	unsigned int s_arpciecmd				: 2;
	unsigned int s_arerror					: 1;
	unsigned int							: 1;
	unsigned int s_awpciecmd				: 3;
	unsigned int s_awerror					: 1;
};

struct strSStatus {
	unsigned int s_int_tx_edge_level_n		: 1;
	unsigned int							: 3;
	unsigned int s_link_up					: 1;
	unsigned int m_awerror					: 1;
	unsigned int m_werror					: 1;
	unsigned int m_arerror					: 1;
	unsigned int mmcm_locked				: 1;
	unsigned int init_calib_complete		: 1;
	unsigned int							: 2;
	unsigned int tx_lock					: 1;
	unsigned int lane_up					: 1;
	unsigned int channel_up					: 1;
	unsigned int							: 1;
	unsigned int frame_err					: 1;
	unsigned int hard_err					: 1;
	unsigned int soft_err					: 1;
	unsigned int pll_not_locked				: 1;
	unsigned int rx_resetdone_out			: 1;
	unsigned int tx_resetdone_out			: 1;
	unsigned int sfp1_tx_fault				: 1;
	unsigned int sfp1_mod_detect			: 1;
	unsigned int 							: 8;
};

typedef struct uart_struct_type {
	volatile unsigned int rx;
	volatile unsigned int tx;
	volatile unsigned int status;
	volatile unsigned int control;
}uart_struct;

typedef struct gpio_reg_type {
	unsigned int auroraResetn		: 1;
	unsigned int gtResetn			: 1;
	unsigned int					: 2;
	unsigned int intrRxEdgeLevel	: 1;
	unsigned int					: 4;
	unsigned int fmcLED1			: 1;
	unsigned int fmcLED2			: 1;
	unsigned int fmcLED3			: 1;
	unsigned int fmcLED4			: 1;
	unsigned int 					: 4;
	unsigned int fmcLSB				: 8;	// J20[8:1]
	unsigned int fmsMSB				: 8;	// J20[16:9]
} gpio_reg_struct;

/**
 * @struct dma_reg_struct
 * @brief NWL DMA registers
 */
typedef struct dma_reg_struct_type {
	unsigned int SRC_Q_PTR_LO;		/**< default -  0x00 */
	unsigned int SCR_Q_PTR_HI;		/**< default -  0x04 */
	unsigned int SRC_Q_SIZE;		/**< default -  0x08 */
	unsigned int SRC_Q_LIMIT;		/**< default -  0x0c */
	unsigned int DST_Q_PTR_LO;		/**< default -  0x10 */
	unsigned int DST_Q_PTR_HI;		/**< default -  0x14 */
	unsigned int DST_Q_SIZE;		/**< default -  0x18 */
	unsigned int DST_Q_LIMIT;		/**< default -  0x1c */
	unsigned int STA_Q_PTR_LO;		/**< default -  0x20 */
	unsigned int STA_Q_PTR_HI;		/**< default -  0x24 */
	unsigned int STA_Q_SIZE;		/**< default -  0x28 */
	unsigned int STA_Q_LIMIT;		/**< default -  0x2c */
	unsigned int SRC_Q_NEXT;		/**< default -  0x30 */
	unsigned int DST_Q_NEXT;		/**< default -  0x34 */
	unsigned int STA_Q_NEXT;		/**< default -  0x38 */
	unsigned short DMA_CONTROL;		/**< default -  0x3c */
	unsigned char PCIE_STATUS;		/**< default -  0x3e */
	unsigned char AXI_STATUS;		/**< default -  0x3f */
} dma_reg_struct;

typedef struct RTSP_FrameHeader_type {
	unsigned int	headerID;
	unsigned int	shelfID;
	unsigned int	dataSize;
} strRtspFrameHeader;

typedef struct RTSP_ChannelHeader_type {
	unsigned int channelNumber; //!< channel number
	unsigned int channelSize;	//!< number of elements from here to the end of the channel
	unsigned int W;				//!< number of ranges
	unsigned int D[8];			//!< ranges
	unsigned N[8];				//!< samples per range
}strRtspChannelHeader;

typedef struct params_type {
	unsigned int			software_version;					//!< current software version
	unsigned int			firmware_version;					//!< current firmware version
	uart_struct *			pUART;								//!< pointer to UART registers
	XIntc *					pInterruptController;				//!< pointer to interrupt controller
	XIic *					pIicInstance;						//!< pointer to i2c controller
	XAxiDma *				pAxiDma;							//!< pointer to AXI DMA controller
	dma_reg_struct *		pDmaChannelRegisters[3];			//!< pointer to NWL DMA channel registers
	unsigned int *			nwlDmaSlaveRegisterBase;			//!< pointer to NWL slave port
	unsigned int 			dataSourceLocation;					//!< address of source data
	unsigned int 			dataDestinationLocation;			//!< address of destination data
	u8 *					pRxBuffer;							//!< pointer to aurora rx buffer in memory
	u8 *					pTxBuffer;							//!< pointer to aurora tx buffer in memory
	unsigned int			testPacketSize;						//!< size of test packet from PC
	unsigned int *			ptr_GpioSidebandReg;				//!< pointer to hw address for the sideband register
	unsigned int *			ptr_GpioIntrRxReg;					//!< pointer to hw address for the interrupt register
	unsigned int *			ptr_GpioStatusReg;					//!< pointer to hw address for the status register
	struct strSSideband *	ptr_sSidebandRegister;				//!< pointer to the sw sideband register structure
	struct strSStatus *		ptr_sStatusRegister;				//!< pointer to the sw status register structure
	gpio_reg_struct *		ptr_GPIORegister;					//!< pointer to sw GPIO structure
	strRtspFrameHeader *	ptr_RtspFrameHeader			;		//!< pointer to an RTSP header
	strRtspChannelHeader *	ptr_RtspChannelHeader;

	unsigned int *			dmaSourceLocation;					//!< pointer to data location on host
	unsigned int *			dmaDestinationLocation;				//!< pointer to destination on card

	struct sgElement *		sourceSglAddress;					//!< pointer to source SGL structure
	struct sgElement *		destinationSglAddress;				//!< pointer to destination SGL structure
	struct sgStatusElement *statusSglAddress;					//!< pointer to SGL status structure
}params_struct;



#endif /* COMMON_H_ */
