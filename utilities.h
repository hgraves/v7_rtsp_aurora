/*
 * @file utilities.h
 *
 *  Created on: Aug 24, 2015
 *      Author: Howard Graves
 */

#include "common.h"

unsigned int initControllers(params_struct *);
unsigned int swap_endian(unsigned int);
int boundary_check(unsigned long);
void display_menu(params_struct *);
void clear_ddr(unsigned int *, unsigned int *, unsigned int, unsigned char);
void display_registers(params_struct *, unsigned int);
void write_register(unsigned int *,unsigned int);
void display_all(params_struct *);
