#ifndef IDT_SETUP_H
#define IDT_SETUP_H

#include "x86_desc.h"

void create_IDT_entry();

void divide_by_zero();
void debug();
void not_used();
void breakpoint();
void overflow();
void bounds_check();
void invalid_opcode();
void device_not_available();
void double_fault();
void cop_seg_overrun();
void invalid_tss();
void seg_not_present();
void stack_seg_fault();
void general_protection();
void page_fault();
void reserved();
void floating_point_err();
void aligment_check_fault();
void machine_check();
void simd_exception();

void slave_handler();

#endif
