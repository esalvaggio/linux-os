#ifndef DEVICE_INIT_H
#define DEVICE_INIT_H

#include "../x86_desc.h"
#include "i8259.h"
#include "../lib.h"


//https://wiki.osdev.org/8259_PIC

#define CMOS_REG 0x70 /*CMOS register is located at port 0x70*/
#define PIC_REG  0x71 /*PIC register is at port 0x71*/

//MASKS
#define NMI_MASK 0x80 /*NMI is the high bit of */
#define TOP_4    0xF0
#define BOT_4    0x0F

//Registers
#define REG_A    0x0A
#define REG_B    0x0B
#define REG_C    0x0C
#define SET_PIE  0x40

//Error / Success
#define ERROR    -1
#define SUCCESS   0

//Bounds
#define LOW_RATE 1
#define HI_RATE 10
void RTC_Init();
void RTC_Handler();

int32_t RTC_open();
int32_t RTC_read(void* buf, int32_t nbytes);
int32_t RTC_write(void* buf, int32_t nbytes);
int32_t RTC_close();

int32_t square_root(int32_t input);


#endif
