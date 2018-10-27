#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "./devices/rtc.h"
#include "./devices/i8259.h"
#include "./devices/keyboard.h"


#define PASS 1
#define FAIL 0

volatile int freq_flag = 0;

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
	int result = FAIL;
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
 * Side Effects: Program is stuck in Page Fault Exception
 */
int paging_test(){
	TEST_HEADER;

	int * validAddress1 = (int *)0xB8000;
	int * validAddress2 = (int *)0xB8FFC;
	printf("Inside valid Video memory address 1: %d\n", *validAddress1);
	printf("Inside valid Video memory address 2: %d\n", *validAddress2);
	int * invalidAddress = (int *)0xB8FFF;
	printf("Inside invalid address: ");
	printf("%d\n", *invalidAddress);
	return PASS;
}

// add more tests here

/* Checkpoint 2 tests */

/* Change Frequency Test
 *
 * Changes the frequency of the rtc to a few different values and keeps
 * it there for a few seconds each.
 * (Right now it is set to 4 Hz, or double the usual)
 * Inputs: None
 * Outputs: PASS
 * Side Effects: Cursor is placed somewhre on screen (ususally about halfway down.)
 * TODO: Remove some magic numbers, add comments through function
 */
int change_frequency_test(){
	TEST_HEADER;
	cli();
	SET_IDT_ENTRY(idt[RTC_INDEX], new_rtc_idt);
	sti();
	int no_interrupts = 1;
	int counter;
	int frequencies[5] = {64, 8, 13, 128, 2};
	for (counter = 0; counter < 5; counter++){
		if (counter > 0) no_interrupts = (frequencies[counter-1] / 10) + 1;
		for (;no_interrupts>0;no_interrupts--){
			TEST_OUTPUT("Test RTC Read", rtc_read());
			clear();
		}
		RTC_write(NULL, frequencies[counter]);
	}
	SET_IDT_ENTRY(idt[RTC_INDEX], RTC_Handler);
	clear();
	return PASS;
}



/* RTC read test
 *
 * Sees if RTC_read returns. That is it
 * Inputs: None
 * Outputs: PASS
 * Side Effects: Gets stuck if RTC_read is a bad function,
 *							 otherwise just prints "Interrupt!" 10 times
 */
int rtc_read(){
	TEST_HEADER;
	int interrupts = 10;
	while (interrupts > 0){
			RTC_read(NULL, 0);
			printf("Interrupt!\n");
			interrupts --;
	}
	return PASS;
}


// int terminal_test()
// {
// 	TEST_HEADER;
// 	char b[129] = "";
// 	int readResult = Terminal_Read(b, 128);
// 	printf("%d \n", readResult);
// 	Terminal_Write(b, 128);
// 	return PASS;
// }



// int file_system_test() {
// 		int8_t* file = (int8_t*)"frame0.txt";
// 		int8_t* string = file_read(file);
// 		printf(string);
// 		return PASS;
// }




/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("paging_test", paging_test());
	//TEST_OUTPUT("divide by zero test ", divide_by_zero_test());
	/* to test RTC, go to rtc.c */
	/* Checkpoint 2 tests */
	//TEST_OUTPUT("Change frequency", change_frequency_test());
	//TEST_OUTPUT("Test RTC Read", rtc_read());
	//TEST_OUTPUT("Test Terminal", terminal_test());
	//file_system_test();
}
