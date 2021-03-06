#ifndef RTC_H
#define RTC_H

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

#define RTC_INDEX         40
#define RTC_IRQ            8
#define SLAVE_IRQ          2

void RTC_Init();
void RTC_Handler();

int32_t RTC_open(const uint8_t* filename);
int32_t RTC_read(int32_t fd, void* buf, int32_t nbytes);
int32_t RTC_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t RTC_close(int32_t fd);

//Helper functions
int32_t power_of_two(int32_t input);
void new_rtc_idt();



#endif
