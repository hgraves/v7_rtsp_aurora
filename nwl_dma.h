/*
 * @file nwl_dma.h
 *
 *  Created on: Feb 2, 2016
 *      Author: Howard Graves
 */

#ifndef NWL_DMA_H_
#define NWL_DMA_H_

#include "common.h"


int init_DMA(params_struct *, unsigned int);

void resetAxiInterrupt(unsigned int);
void dmaResults(params_struct *);

#endif /* NWL_DMA_H_ */
