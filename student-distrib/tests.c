#include "tests.h"
#include "x86_desc.h"
#include "lib.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* Divide By Zero Test 
 *
 * Divides by 0 and triggers a divide by zero exception
 * Inputs: None
 * Outputs: PASS
 * Side Effects: Program is stuck in Divide by zero exception
 */
int divide_by_zero_test(){
	TEST_HEADER;

	int i;
	int j = 0;
	int result = PASS;
	i = 5/j;

	return result;
}

/* Paging Test 
 *
 * Prints contents of addresses in video memory which has
 * been marked as valid by Paging.c, which correctly works,
 * then attempts to print content outside of video memory
 * and triggers a page fault, as expected 
 * Inputs: None
 * Outputs: PASS
 * Side Effects: Program is stuck in Divide by zero exception
 */
int paging_test(){

	int * validAddress1 = (int *)0xB8000;
	int * validAddress2 = (int *)0xB8FFC;
	printf("Inside valid VGA memory address 1: %d\n", *validAddress1);
	printf("Inside valid VGA memory address 2: %d\n", *validAddress2);
	int * invalidAddress = (int *)0xB8FFF;
	printf("Inside invalid address: ");
	printf("%d\n", *invalidAddress);
	return PASS;
}
/*
int RTC_Test(){
		TEST_HEADER;
		SET_IDT_ENTRY(40, RTC_Test_Handler);
		int i;
		for (i = 0; i < 100000; i++)

		SET_IDT_ENTRY(40, RTC_Handler())
		return PASS;
}

void RTC_Test_Handler(){
		printf("RTC Interrupt");
    outb(0x0C, 0x0C);	// select register C
    inb(PIC_REG);		// just throw away contents
    sti();
    send_eoi(8);
}*/

// add more tests here

/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	TEST_OUTPUT("idt_test", idt_test());
	TEST_OUTPUT("paging_test",paging_test())
	// launch your tests here
	TEST_OUTPUT("divide by zero test ", divide_by_zero_test());

}
