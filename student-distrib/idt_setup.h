#ifndef IDT_SETUP_H
#define IDT_SETUP_H

#include "x86_desc.h"

void create_IDT_entry();
int handlerFunction(int vectorNumber);


#endif
