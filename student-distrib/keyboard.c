#include "lib.h"
#include "keyboard.h"


void Keyboard_Handler() {
  printf("Set keyboard handler 3\n");

  unsigned char status, output_key;
  char scan_code;
  int caps_lock_flag=0;
  int caps_been_set = 0;
  int shift_flag=0;
//http://www.osdever.net/bkerndev/Docs/keyboard.htm
  unsigned char keyboard_map[3][60] ={{ /* regular keys */
                                          '\0','\0', '1', '2', '3', '4', '5', '6', '7', '8',
                                          '9', '0', '-', '=', '\b','\t','q', 'w', 'e', 'r',
                                          't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n','\0',
                                          'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
                                          '\'', '`','\0','\\', 'z', 'x', 'c', 'v', 'b', 'n',
                                          'm', ',', '.', '/',   '\0','*','\0',' ',
                                          '\0',	/* Caps lock key */
                                        },
                                        { /* SHIFT PRESSED */
                                          '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*',
                                          '(', ')', '_', '+', '\b',	'\t',	'Q', 'W', 'E', 'R',
                                          'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',	'\0',
                                          'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
                                          '\"', '~', '\0','\\', 'Z', 'X', 'C', 'V', 'B', 'N',
                                          'M', '<', '>', '?','\0','*','\0',' ',
                                          '\0',	/* Caps lock */
                                        },
                                        { /* CAPS LOCK PRESSED */
                                          '\0','\0', '1', '2', '3', '4', '5', '6', '7', '8',
                                          '9', '0', '-', '=', '\b','\t','Q', 'W', 'E', 'R',
                                          'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n','\0',
                                          'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',
                                          '\'', '`','\0','\\', 'Z', 'X', 'C', 'V', 'B', 'N',
                                          'M', ',', '.', '/',   '\0','*','\0',' ',
                                          '\0',	/* Caps lock key */
                                        }
                                    };

  while(1) {

    status = inb(STATUS_PORT);
    // printf("%d", status & 0x01);
    if (status & 0x01) {
          scan_code = inb(DATA_PORT);
          // printf(":%d\n", scan_code);
          // if (scan_code = 58){
          //     printf("\nCAPS LOCK");
          // }


          if (scan_code >= 0) {
            // if (scan_code == 58){
            //     printf("58");
            // }
            if(scan_code == 58){
              if(caps_been_set == 0){
                caps_lock_flag = 1;
                caps_been_set = 1;
              }else{
                caps_lock_flag = 0;
                caps_been_set = 0;
              }
            }
            // printf("%d", caps_lock_flag);
            if(caps_lock_flag == 1){
              output_key = keyboard_map[2][scan_code];
            }else{
              output_key = keyboard_map[0][scan_code];
            }

            // output_key = keyboard_map[0][scan_code];
            printf("%c", output_key);
          }
    }
  }

    send_eoi(1);
}

void Keyboard_Init() {
    idt[33].present = 1;
    SET_IDT_ENTRY(idt[33], Keyboard_Handler);
    enable_irq(1);
}
