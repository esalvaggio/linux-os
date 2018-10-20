#ifndef DEVICE_INIT_H
#define DEVICE_INIT_H

#include "x86_desc.h"

//https://wiki.osdev.org/8259_PIC

#define CMOS_REG 0x70 /*CMOS register is located at port 0x70*/
#define PIC_REG  0x71 /*PIC register is at port 0x71*/

#define NMI_MASK 0x80 /*NMI is the high bit of */

#define REG_A    0x0A
#define REG_B    0x0B
#define SET_PIE  0x40

void RTC_Init();
void Keyboard_Init();

unsigned char * keybindings= ['\n','0','1','2','3','4','5','6','7','8','9','0','-','=']

#endif
