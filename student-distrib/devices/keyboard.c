#include "keyboard.h"

 unsigned char status, output_key;
  char scan_code;
  int caps_lock_flag=0;
  int caps_been_set = 0;
  int shift_flag=0;
  int shift_pressed = 0;
  int shift_released = 0;
  int ctrl_flag = 0;
  int clear_flag = 0;

static unsigned char keyboard_map[KB_CAPS_CASES][KB_MAP_SIZE] ={{ /* regular keys */
                                          '\0','\0', '1', '2', '3', '4', '5', '6', '7', '8',
                                          '9', '0', '-', '=', '\b','\t','q', 'w', 'e', 'r',
                                          't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n','\0',
                                          'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
                                          '\'', '`','\0','\\', 'z', 'x', 'c', 'v', 'b', 'n',
                                          'm', ',', '.', '/',   '\0','*','\0',' ',
                                          '\0', /* Caps lock key */
                                        },
                                        { /* SHIFT PRESSED */
                                          '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*',
                                          '(', ')', '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R',
                                          'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', '\0',
                                          'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
                                          '\"', '~', '\0','\\', 'Z', 'X', 'C', 'V', 'B', 'N',
                                          'M', '<', '>', '?','\0','*','\0',' ',
                                          '\0', /* Caps lock */
                                        },
                                        { /* CAPS LOCK PRESSED */
                                          '\0','\0', '1', '2', '3', '4', '5', '6', '7', '8',
                                          '9', '0', '-', '=', '\b','\t','Q', 'W', 'E', 'R',
                                          'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n','\0',
                                          'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',
                                          '\'', '`','\0','\\', 'Z', 'X', 'C', 'V', 'B', 'N',
                                          'M', ',', '.', '/',   '\0','*','\0',' ',
                                          '\0', /* Caps lock key */
                                        },
                                        { /* CAPS LOCK AND SHIFT PRESSED */
                                          '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*',
                                          '(', ')', '_', '+', '\b', '\t', 'q', 'w', 'e', 'r',
                                          't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n', '\0',
                                          'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':',
                                          '\"', '~', '\0','\\', 'z', 'x', 'c', 'v', 'b', 'n',
                                          'm', '<', '>', '?','\0','*','\0',' ',
                                          '\0', /* Caps lock */
                                        }
                                    };
/* Keyboard_Handler
 * inputs: none
 * outputs: none
 * This function reads in data from the keyboard, maps the key to the ascii,
 * and then displays it on screen. It finishes by sending EOI
 */
void Keyboard_Handler() {
    cli();
    clear_flag = 0;
    status = inb(STATUS_PORT);
    if (status & LOW_BITMASK) { // get last bit value of status is the character to be displayed
          scan_code = inb(DATA_PORT);
          // printf("%d",scan_code);
          if(scan_code == CTRL_KEY_DOWN){
                ctrl_flag = 1;
          }
          if(scan_code == CTRL_KEY_UP){
            ctrl_flag = 0;
          }
          if(scan_code == L_KEY_DOWN && ctrl_flag == 1){
              clear();
              clear_flag = 1;
          }
          if(scan_code == SHIFT_LEFT_PRESS || scan_code == SHIFT_RIGHT_PRESS){
              shift_pressed = 1;
          }
          if(scan_code == SHIFT_LEFT_RELEASE || scan_code == SHIFT_RIGHT_RELEASE){
            shift_pressed = 0;
          }
          if (scan_code >= 0) {
            //checks if caps lock is set
            if(scan_code == CAPS_LOCK){

              if(caps_been_set == 0){
                caps_lock_flag = 1;
                caps_been_set = 1;
              }else{
                caps_lock_flag = 0;
                caps_been_set = 0;
              }
            }

            if(caps_lock_flag == 1 && shift_pressed == 0){
              output_key = keyboard_map[2][(unsigned char)scan_code];
            }else if(caps_lock_flag == 0 && shift_pressed == 1){
              output_key = keyboard_map[1][(unsigned char)scan_code];
            }else if(caps_lock_flag == 1 && shift_pressed == 1){
              output_key = keyboard_map[3][(unsigned char)scan_code];
            }else{
                output_key = keyboard_map[0][(unsigned char)scan_code];
            }
            if(clear_flag != 1 && output_key != '\0'){
              printf("%c", output_key); //print to screen
            }
          }
    }
    sti();
    send_eoi(KEYBOARD_IRQ); //keyboard port on master
}

/* Keyboard_Init
 * Inputs: none
 * Outputs: none
 * This function sets the appropriate entry in the IDT to enable the keyboard and also
 * enables the irq in the master PIC
 */
void Keyboard_Init() {
    idt[KEYBOARD_INDEX].present = 1;
    SET_IDT_ENTRY(idt[KEYBOARD_INDEX], Keyboard_Handler);
    enable_irq(KEYBOARD_IRQ);
}
