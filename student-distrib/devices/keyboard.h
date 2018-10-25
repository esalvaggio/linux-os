#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../x86_desc.h"
#include "i8259.h"
#include "../lib.h"

#define STATUS_PORT      0x64
#define DATA_PORT        0x60
#define CAPS_LOCK          58
#define SHIFT_LEFT_PRESS  42
#define SHIFT_LEFT_RELEASE -86
#define SHIFT_RIGHT_PRESS   54
#define SHIFT_RIGHT_RELEASE -74
#define KB_MAP_SIZE         60
#define KB_CAPS_CASES       4
#define LOW_BITMASK     0x01
#define KEYBOARD_INDEX    33
#define KEYBOARD_IRQ       1
#define CTRL_KEY_DOWN     29
#define CTRL_KEY_UP       -99
#define L_KEY_DOWN        38

void Keyboard_Init();
void Keyboard_Handler();
int32_t Keyboard_Write(const void * buf, int32_t nbytes);
int32_t Keyboard_Read(const void * buf, int32_t nbytes);
int32_t Keyboard_Open(const void * buf, int32_t nbytes);
int32_t Keyboard_Close(const void * buf, int32_t nbytes);

#endif
