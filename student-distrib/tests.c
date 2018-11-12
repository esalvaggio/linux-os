#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "./devices/rtc.h"
#include "./devices/i8259.h"
#include "fs_setup.h"
#include "./devices/keyboard.h"
#include "sys_calls.h"
#include "paging.h"


#define PASS 1
#define FAIL 0
#define RTC_READ_LOOPS 6
#define AMT_FREQ 13

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

	page_dir_init(0x08000000, 0x800000);
	int * validAddress1 = (int *)0x08000000;
	int * validAddress2 = (int *)0x08048000;
	printf("Inside valid Video memory address 1: %d\n", *validAddress1);
	printf("Inside valid Video memory address 2: %d\n", *validAddress2);
	// int * invalidAddress = (int *)0xB8FFF;
	// printf("Inside invalid address: ");
	// printf("%d\n", *invalidAddress);
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
 */
int change_frequency_test(){
	TEST_HEADER;

	int no_interrupts;
	int counter;
	int timer;
	int frequencies[AMT_FREQ] = {64, 8, 13, 128, 2, 1024, 12, 16, 512, 4, 128, 2048, 32};

	uint8_t fname[3] = "RTC";
	for (counter = 0; counter < AMT_FREQ; counter++){
		RTC_open(fname);
		printf("Testing Frequency: %d", frequencies[counter]);
		for (timer = 0; timer < 4; timer++){
				RTC_read(0, NULL,0);
				printf(".");

		}


		if (RTC_write(0, &frequencies[counter], 4) == 0){ //returns success
			no_interrupts = frequencies[counter] * 2;
			for (;no_interrupts>=0;no_interrupts--){
				RTC_read(0, NULL ,0);
				clear();
				update_cursor(0,0);
				printf("interrupt: %d", no_interrupts);
			}
		}else{
				clear();
				update_cursor(0,0);
				printf("Bad Input!");
				for (timer = 0; timer < 4; timer ++){
						RTC_read(0, NULL, 0);
				}
		}

	clear();
	update_cursor(0,0);
	}
	RTC_open(fname);
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
	//RTC_READ_LOOPS is just an arbitary number,
	//enough to show it actually works
	// (about 3 seconds for 2hz)
	int interrupts = RTC_READ_LOOPS;
	while (interrupts > 0){
			RTC_read(0, NULL, 0);
			printf("Interrupt!\n");
			interrupts --;
	}
	return PASS;
}


//char b = 'b';
/*
* Terminal Read/Write Test
* This test reads the desired number of characters from
* keyboard into variable b, gets its length (including the enter key)
* and then writes the desired number of characters to string
* Inputs: None
* Outputs: PASS
* Side Effects: Returns PASS by default, user should verify that expected number
* of chars is read/written, do not assume PASS means that it works
*/
int terminal_test()
{
	TEST_HEADER;
	char b[128] = ""; //buffer with space for 128 chars as specified,this value needs to be 128 in order to have a buffer with length 128 as specified
	int readResult = Terminal_Read(0,b,128);
	printf("%d \n", readResult);
	Terminal_Write(0,b,128);
	//int writeResult = Terminal_Write(b,5); //if desired you can see the number of chars written as well
		//printf("%d \n", writeResult);
	return PASS; //text written and Terminal_Write need to be compared directly to see if correct or not
}

/* file_system_test_1
 *
 * Tests if file open and file read works with a simple
 * 	text file.
 * Inputs: None
 * Outputs: PASS on success, FAIL if the file can't open
 * Side Effects: None
 *
 */
int file_system_test_1() {
		TEST_HEADER;
		uint8_t file[34] = "frame0.txt";
		if (file_open(file) < 0)
				return FAIL;

		int8_t buf[10000];
		file_read(2, buf, 187);
		int i;
		for (i = 0; i < 187; i++) {
				putc(buf[i]);
		}

		return PASS;
}

/* file_system_test_2
 *
 * Tests if file open and file read works with a non-
 * 	text file. It prints the beginning and the end of
 *	the file.
 * Inputs: None
 * Outputs: PASS on success, FAIL if the file can't open
 * Side Effects: None
 *
 */
int file_system_test_2() {
		TEST_HEADER;
		uint8_t file[34] = "cat";
		if (file_open(file) < 0)
				return FAIL;

		int8_t buf[10000];
		file_read(2, buf, 5445);

		printf("Beginning of non-text file:\n\n");
		int i;
		for (i = 0; i < 500; i++) {
				putc(buf[i]);
		}

		printf("\n\nEnd of non-text file:\n\n");
		for (i = 5000; i < 5445; i++) {
				putc(buf[i]);
		}

		putc('\n');

		return PASS;
}

/* file_system_test_3
 *
 * Tests if file open and file read works with a large
 * 	text file. It prints the beginning and the end of
 *	the file.
 * Inputs: None
 * Outputs: PASS on success, FAIL if the file can't open
 * Side Effects: None
 *
 */
int file_system_test_3() {
		TEST_HEADER;
		uint8_t file[34] = "verylargetextwithverylongname.txt";
		if (file_open(file) < 0)
				return FAIL;

		int8_t buf[10000];
		file_read(2, buf, 5277);

		printf("Beginning of non-text file:\n\n");
		int i;
		for (i = 0; i < 300; i++) {
				putc(buf[i]);
		}

		printf("\n\nEnd of non-text file:\n\n");
		for (i = 4700; i < 5277; i++) {
				putc(buf[i]);
		}

		return PASS;
}

/* file_system_test_4
 *
 * Tests if dir_open and dir_read works. Since we only
 *	have one directory, it just prints out the files in
 *	the file system.
 * Inputs: None
 * Outputs: PASS on success, FAIL if the file can't open
 * Side Effects: None
 *
 */
int file_system_test_4() {
		TEST_HEADER;
		char buf[boot_block->dir_count];
		if (dir_read(1, &buf, 32) != 0)
				return FAIL;
		else
				return PASS;
}

/* Checkpoint 3 tests */

void linkage_test() {

	printf("we here\n");
	int fail;
	int sys_call = 2;
	asm volatile (
								"movl %0, %%eax\n\t"
								"int $0x80"
								: "=a"(fail)
								:	"r"(sys_call)
							);
}

int execute_test_print_test(){
	//uint8_t shell[6] = "shell ";
	TEST_HEADER;
	uint8_t * testPrint = (uint8_t *)"testprint";


	execute(testPrint);

/*
	asm volatile ("								\n\
								movl $2, %%eax	\n\
							  movl %0, %%ebx  \n\
								int $0x80				\n\
								"
								:
								:	"r"(testPrint)
								: "eax" , "ebx"
							);
*/
 return PASS;
}

int execute_hello_test()
{
	TEST_HEADER;
	uint8_t * hello = (uint8_t *)"hello";
	execute(hello);

/*
	asm volatile ("								\n\
								movl $2, %%eax	\n\
								movl %0, %%ebx  \n\
								int $0x80				\n\
								"
								:
								:	"r"(hello)
								: "eax" , "ebx"
							);
*/							
			return PASS;

}




/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// TEST_OUTPUT("idt_test", idt_test());
	//TEST_OUTPUT("paging_test", paging_test());
	//TEST_OUTPUT("divide by zero test ", divide_by_zero_test());
	/* to test RTC, go to rtc.c */

	/* Checkpoint 2 tests */
	TEST_OUTPUT("Change frequency", change_frequency_test());
	//TEST_OUTPUT("Test RTC Read", rtc_read());
	//TEST_OUTPUT("TEST_Terminal", terminal_test());
	// TEST_OUTPUT("File System: Text File test", file_system_test_1());
	// TEST_OUTPUT("File System:	Non-Text File Test", file_system_test_2());
	// TEST_OUTPUT("File System: Large File Test", file_system_test_3());
	// TEST_OUTPUT("File System: Directory Test", file_system_test_4());

	/*Checkpoint 2 regade tests*/

	/* Checkpoint 3 tests */
	// linkage_test();
	 //execute_test_print_test();
	 //execute_hello_test();

}
