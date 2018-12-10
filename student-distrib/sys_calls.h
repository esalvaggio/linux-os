#ifndef _SYS_CALLS_H
#define _SYS_CALLS_H

#include "types.h"

#define FILE_ARRAY_SIZE     8
#define NUM_OF_PROCESSES    6
#define VIRTUAL_ADDRESS     0x8000000
#define STACK_PAGE_SIZE     0x400000
#define FOUR_BYTE_ADDR      4
#define ARGS_LEN            1024
#define VIDMEM_ADDR         0x8400000

#define ADDR_8MB      0x800000
#define ADDR_4MB      0x400000
#define ADDR_8KB      0x002000
#define ADDR_4KB      0x001000

//pointer is four bytes so you want address of the pointer

#ifndef ASM

int32_t halt(uint8_t status);
int32_t execute(const uint8_t* command);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
int32_t open(const uint8_t* filename);
int32_t close(int32_t fd);
int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn(void);

/* File Operations Table Pointer */
typedef struct fotp {
    int32_t (*open)(const uint8_t*);
    int32_t (*close)(int32_t);
    int32_t (*read)(int32_t, void*, int32_t);
    int32_t (*write)(int32_t, const void*, int32_t);
} fotp_t;

/* File Descriptor */
typedef struct fd {
    fotp_t file_ops_table_ptr; // jump table pointer
    int32_t inode;
    int32_t file_pos;
    int32_t flags;
} fd_t;

/* Process Control Block */
typedef struct pcb {
    fd_t file_array[FILE_ARRAY_SIZE];
    int32_t mem_addr_start;
    // int32_t parent_pcb;
    struct pcb* parent_pcb;
    uint8_t args[ARGS_LEN];
    int8_t process_num;
    int8_t term_index;
    int8_t in_use;
    int32_t esp;
    int32_t ebp;
    int32_t esp0;
    int32_t ss0;
    // add more good stuff
} pcb_t;


/* Initializer function for PCB */
int32_t find_new_process();
pcb_t* create_new_pcb(int32_t process_num);
pcb_t* get_curr_pcb();
pcb_t* get_pcb_ptr();

#endif
#endif
