/*
 * dma.h
 *
 *  Created on: Jan 11, 2016
 *      Author: Howard Graves
 */

#ifndef DMA_H_
#define DMA_H_

#include "xaxidma.h"
#include "common.h"

int receiveDMA( params_struct *);
int initDMA( XAxiDma *dmaController, u8 *rxBuffer, u8 *txBuffer);
int sendDMA( params_struct *);
void displayDmaRegisters(XAxiDma *dmaController);
unsigned int getDmaBytesReceived(XAxiDma *dmaController);

#endif /* DMA_H_ */
