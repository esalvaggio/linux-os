#define ASM
.globl pit_setup, rtc_setup

pit_setup:
  pushal
  call pit_handler
  popal
  iret

rtc_setup:
  pushal
  call RTC_Handler
  popal
  iret
