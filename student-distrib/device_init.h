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
void Keyboard_Init();
void Keyboard_Handler();
void RTC_Handler();

//https://github.com/arjun024/mkeykernel/blob/master/keyboard_map.h
//Obviously this is incomplete but every key is shown
static unsigned char keyboard_map[128] ={
                                    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
                                    '9', '0', '-', '=', '\b',	/* Backspace */
                                    '\t',			/* Tab */
                                    'q', 'w', 'e', 'r',	/* 19 */
                                    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
                                      0,			/* 29   - Control */
                                    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
                                   '\'', '`',   0,		/* Left shift */
                                   '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
                                    'm', ',', '.', '/',   0,				/* Right shift */
                                    '*',
                                      0,	/* Alt */
                                    ' ',	/* Space bar */
                                      0,	/* Caps lock */
                                      0,	/* 59 - F1 key ... > */
                                      0,   0,   0,   0,   0,   0,   0,   0,
                                      0,	/* < ... F10 */
                                      0,	/* 69 - Num lock*/
                                      0,	/* Scroll Lock */
                                      0,	/* Home key */
                                      0,	/* Up Arrow */
                                      0,	/* Page Up */
                                    '-',
                                      0,	/* Left Arrow */
                                      0,
                                      0,	/* Right Arrow */
                                    '+',
                                      0,	/* 79 - End key*/
                                      0,	/* Down Arrow */
                                      0,	/* Page Down */
                                      0,	/* Insert Key */
                                      0,	/* Delete Key */
                                      0,   0,   0,
                                      0,	/* F11 Key */
                                      0,	/* F12 Key */
                                      0,	/* All other keys are undefined */
                                  };

#endif
