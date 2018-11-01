#ifndef IDT_SETUP_H
#define IDT_SETUP_H

#include "x86_desc.h"

/* Constants for IDT indexes */
#define DIVIDE_BY_ZERO_IDT      0
#define DEBUG_IDT               1
#define NOT_USED_IDT            2
#define BREAKPOINT_IDT          3
#define OVERFLOW_IDT            4
#define BOUNDS_CHECK_IDT        5
#define INVALID_OPCODE_IDT      6
#define DEV_NOT_AVAILABLE_IDT   7
#define DOUBLE_FAULT_IDT        8
#define COP_SEG_OVERRUN_IDT     9
#define INVALID_TSS_IDT        10
#define SEG_NOT_PRESENT_IDT    11
#define STACK_SEG_FAULT_IDT    12
#define GEN_PROTECTION_IDT     13
#define PAGE_FAULT_IDT         14
#define RESERVED_IDT           15
#define FLT_POINT_ERROR_IDT    16
#define ALIGN_FAULT_IDT_IDT    17
#define MACHINE_CHECK_IDT      18
#define SIMD_EXCEPTION_IDT     19
#define INTEL_DEFINED_IDX      32
#define SYSTEM_CALL_IDT       127

/* IDT initializer function */
void create_IDT_entry();

/* Exception and interrupt handlers */
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

#endif
