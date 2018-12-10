#define ASM
.globl pit_setup, rtc_setup, keyboard_setup

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

keyboard_setup:
  pushal
  call Keyboard_Handler
  popal
  iret
