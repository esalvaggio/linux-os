#ifndef KEYBOARD
#define KEYBOARD

#include "x86_desc.h"
#include "i8259.h"
#include "lib.h"

#define STATUS_PORT     0x64
#define DATA_PORT       0x60

void Keyboard_Init();
void Keyboard_Handler();
unsigned char keysofthekeys[17] = {0,27,'1','2','3','4','5',
                                  '6','7','8','9','0','-','=',
                                  '\b', '\t'};

//https://github.com/arjun024/mkeykernel/blob/master/keyboard_map.h
//Obviously this is incomplete but every key is shown


#endif
