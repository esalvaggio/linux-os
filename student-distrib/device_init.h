#ifndef DEVICE_INIT_H
#define DEVICE_INIT_H

#include "x86_desc.h"
#include "i8259.h"
#include "lib.h"


//https://wiki.osdev.org/8259_PIC

#define CMOS_REG 0x70 /*CMOS register is located at port 0x70*/
#define PIC_REG  0x71 /*PIC register is at port 0x71*/

#define NMI_MASK 0x80 /*NMI is the high bit of */

#define REG_A    0x0A
#define REG_B    0x0B
#define REG_C    0x0C
#define SET_PIE  0x40

void RTC_Init();
void RTC_Handler();

#endif
