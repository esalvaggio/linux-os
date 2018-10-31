#ifndef _SYS_CALLS_H
#define _SYS_CALLS_H

#include "types.h"
#include "fs_setup.h"
#include "devices/rtc.h"
#include "devices/keyboard.h"

#define FILE_ARRAY_SIZE     8

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
/* Initializer function for PCB */
void pcb_init();

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
    // add more good stuff
} pcb_t;

/* Global PCB file array */
pcb_t* pcb;

fotp_t file_funcs = {file_open, file_close, file_read, file_write};
fotp_t rtc_funcs = {RTC_open, RTC_close, RTC_read, RTC_write};
fotp_t dir_funcs = {dir_open, dir_close, dir_read, dir_write};

#endif
