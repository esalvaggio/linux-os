#include "keyboard.h"

#define BUFFER_LENGTH   128
#define SUCCESS         0
#define FAILURE         -1

 unsigned char status, output_key;
  char scan_code;
  int caps_lock_flag=0;
  int caps_been_set = 0;
  int shift_flag=0;
  int shift_pressed = 0;
  int shift_released = 0;
  int ctrl_flag = 0;
  int clear_flag = 0;
  char old_buffer[BUFFER_LENGTH];
  char new_buffer[BUFFER_LENGTH];
  int old_index;
  int new_index;
  int enter_flag;

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
    enter_flag = 0;
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
              update_cursor(0,0);
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
            if(clear_flag != 1 && output_key != '\0')
            {

              printf("%c", output_key); //print to screen
              new_buffer[new_index] = output_key;
              new_index++;

              if(output_key == '\n')
              {
                int x;
                for(x = 0; x < BUFFER_LENGTH; x++)
                {
                  old_buffer[x] = new_buffer[x]; //copy new_buffer into old_buffer;
                  new_buffer[x] = ' '; //clear new_buffer
                }
                //Keyboard_Write(old_buffer);
              }

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
    update_cursor(0,0);

    int x;
    for(x = 0; x < BUFFER_LENGTH; x++)
    {
      old_buffer[x] = ' ';
      new_buffer[x] = ' ';
    }
    old_index = 0;
    new_index = 0;
    enter_flag = 0;
}

int32_t Keyboard_Open(const void * buf, int32_t nbytes)
{
  return SUCCESS;
}

int32_t Keyboard_Close(const void * buf, int32_t nbytes)
{
  return SUCCESS;
}

int32_t Keyboard_Read(const void * buf, int32_t nbytes)
{
  int32_t num_chars = 0;
  int x;

  for(x = 0; x < BUFFER_LENGTH;x++)
  {
    if(old_buffer[x] == '\n')
    {
      break;
    }
    num_chars++;
  }
  return num_chars;
}


int32_t Keyboard_Write(const void * buf, int32_t nbytes)
{
  int x;
  for(x = 0; x < BUFFER_LENGTH; x++)
  {
    printf("%c", old_buffer[x]);
  }
  return SUCCESS;
}
