#include "fs_setup.h"

void fs_init(uint32_t module_addr) {
    bb_startaddr = module_addr;
}

int32_t read_dentry_by_name(const uint8_t fname, dentry_t* dentry) {
    return 0;
}

int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry) {
    return 0;
}

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    return 0;
}
