#ifndef PIT_H_
#define PIT_H_

#include "../x86_desc.h"
#include "i8259.h"
#include "../lib.h"


void pit_handler();
void pit_init();
void set_pit_freq(uint32_t hz);
void pit_send_command(uint8_t cmd);
#endif
