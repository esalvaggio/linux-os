.globl spin_lock
spin_lock:
    pushl %ebp
    movl %esp, %ebp
    movl 8(%ebp), %ebx
spin_lock_loop:
    movl     $1, %eax          # Set the EAX register to 1.

    xchgl   (%ebx), %eax   # Atomically swap the EAX register with
                           #  the lock variable.
                           # This will always store 1 to the lock, leaving
                           #  the previous value in the EAX register.

    test    %eax, %eax     # Test EAX with itself. Among other things, this will
                           #  set the processor's Zero Flag if EAX is 0.
                           # If EAX is 0, then the lock was unlocked and
                           #  we just locked it.
                           # Otherwise, EAX is 1 and we didn't acquire the lock.

    jnz     spin_lock_loop  # Jump back to the MOV instruction if the Zero Flag is
                           #  not set; the lock was previously locked, and so
                           # we need to spin until it becomes unlocked.
    leave
    ret                     # The lock has been acquired, return to the calling
                           #  function.


.globl spin_unlock
spin_unlock:
    pushl %ebp
    movl %esp, %ebp
    movl 8(%ebp), %ebx
    #  movl %
    xorl     %eax, %eax        # Set the EAX register to 0.

    xchgl    %eax, (%ebx)   # Atomically swap the EAX register with
                           #  the lock variable.
    leave
    ret                     # The lock has been released.
