/*
 * read.h
 *
 *  Created on: Aug 21, 2015
 *      Author: Howard Graves
 */

#include "common.h"

unsigned int get_u32_value(params_struct *, char, int);
void read_u8(volatile uart_struct *, unsigned char *, unsigned int, char);
void read_u16(volatile uart_struct *, unsigned short *, unsigned int, char);
void read_u32(volatile uart_struct *, unsigned int *, unsigned int, char);
