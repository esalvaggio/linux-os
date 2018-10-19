#ifndef DEVICE_INIT_H
#define DEVICE_INIT_H

#include "x86_desc.h"

//https://wiki.osdev.org/8259_PIC

#define PIC 0x20
#define PIC_EOI 0x20
#define SLAVE 0x28

void PIC_send_EOI(int irq);
void SET_PIC();










#endif
