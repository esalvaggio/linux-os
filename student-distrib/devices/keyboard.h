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
#define C_KEY_DOWN        46
#define KEY_OUT_OF_BOUNDS 59
#define ENTER_BUFFER_INDEX 127

#define F1_KEY_DOWN       59
#define F3_KEY_DOWN       61

#define ALT_KEY_DOWN      56
#define ALT_KEY_UP        -72


#define TAB_DOWN          15
#define TAB_UP          -113


void Keyboard_Init();
void Keyboard_Handler();
int32_t Terminal_Write(int32_t fd, const void * buf, int32_t nbytes);
int32_t Terminal_Read(int32_t fd, void * buf, int32_t nbytes);
int32_t Terminal_Open(const uint8_t * filename);
int32_t Terminal_Close(int32_t fd);
void print_to_screen(char output_key);

int terminal_fn_key;

#endif
