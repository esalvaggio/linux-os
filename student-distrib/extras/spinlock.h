#ifndef SPINLOCK_H
#define SPINLOCK_H
#define LOCKED   = 1
#define UNLOCKED = 0
extern void spin_lock(unsigned int * arg);
extern void spin_unlock(unsigned int * arg);
#endif
