/*
 * @file interrupt.h
 *
 *  Created on: Dec 18, 2015
 *      Author: Howard Graves
 */

#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#include "XIic.h"
#include "xaxidma.h"
#include "common.h"

int setupInterruptController(params_struct *);
void nwlDMA_InterruptHandler(void *);
void dmaMM2S_InterruptHandler(void *);
void dmaS2MM_InterruptHandler(void *);
void enableInterrupts(params_struct *, unsigned int);
void disableInterrupts(params_struct *, unsigned int);
void clearInterruptFlags(void);

#endif /* INTERRUPT_H_ */
