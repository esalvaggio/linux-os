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
  char old_buffer[BUFFER_LENGTH]; //"old buffer" saves the "new buffer" when enter is pressed and the "new buffer is cleared"
  char new_buffer[BUFFER_LENGTH]; //contains the current typing of the user
  int old_index; //contains the index of the enter key
  int new_index; //contains the current index to write to while typing
  int enter_flag; //checks if enter is pressed for Terminal_Read()

static unsigned char keyboard_map[KB_CAPS_CASES][KB_MAP_SIZE] ={{ /* regular keys */
                                          '\0','\0', '1', '2', '3', '4', '5', '6', '7', '8',
                                          '9', '0', '-', '=', '\b','\0','q', 'w', 'e', 'r',
                                          't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n','\0',
                                          'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
                                          '\'', '`','\0','\\', 'z', 'x', 'c', 'v', 'b', 'n',
                                          'm', ',', '.', '/',   '\0','\0','\0',' ',
                                          '\0', /* Caps lock key */
                                        },
                                        { /* SHIFT PRESSED */
                                          '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*',
                                          '(', ')', '_', '+', '\b', '\0', 'Q', 'W', 'E', 'R',
                                          'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', '\0',
                                          'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
                                          '\"', '~', '\0','\\', 'Z', 'X', 'C', 'V', 'B', 'N',
                                          'M', '<', '>', '?','\0','\0','\0',' ',
                                          '\0', /* Caps lock */
                                        },
                                        { /* CAPS LOCK PRESSED */
                                          '\0','\0', '1', '2', '3', '4', '5', '6', '7', '8',
                                          '9', '0', '-', '=', '\b','\0','Q', 'W', 'E', 'R',
                                          'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n','\0',
                                          'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',
                                          '\'', '`','\0','\\', 'Z', 'X', 'C', 'V', 'B', 'N',
                                          'M', ',', '.', '/',   '\0','\0','\0',' ',
                                          '\0', /* Caps lock key */
                                        },
                                        { /* CAPS LOCK AND SHIFT PRESSED */
                                          '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*',
                                          '(', ')', '_', '+', '\b', '\0', 'q', 'w', 'e', 'r',
                                          't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n', '\0',
                                          'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':',
                                          '\"', '~', '\0','\\', 'z', 'x', 'c', 'v', 'b', 'n',
                                          'm', '<', '>', '?','\0','\0','\0',' ',
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
              int x;
              new_index = 0;
              for(x = 0; x < BUFFER_LENGTH; x++)
              {
                new_buffer[x] = '\0';
              }
          }
          if(scan_code == SHIFT_LEFT_PRESS || scan_code == SHIFT_RIGHT_PRESS){
              shift_pressed = 1;
          }
          if(scan_code == SHIFT_LEFT_RELEASE || scan_code == SHIFT_RIGHT_RELEASE){
            shift_pressed = 0;
          }

          if(scan_code >= KEY_OUT_OF_BOUNDS)
          {
            //scan_code = 1; //escape key, if key is outside of used scope, set to escape key to print null
            clear_flag = 1; //ignore bad key
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
            if(clear_flag != 1 && output_key != '\0') //print key if clear flag is not set and key to print is not NULL
            {
              //Terminal_Write(new_buffer, BUFFER_LENGTH);

              if(output_key == '\b')
              {
                if(new_index == 0)
                {
                  //if buffer is empty, ignore backspace
                }
                else
                {
                new_index--;
                new_buffer[new_index] = '\0';
                printf("%c", output_key);
                }
              }

              else
              {
              printf("%c", output_key); //print to screen
              new_buffer[new_index] = output_key;
              new_index++;
              }

              if(new_index == ENTER_BUFFER_INDEX) //Buffer is full, fill last entry with enter to set up next if condition
              {
                output_key = '\n'; //put enter in buffer since it is full, print buffer
                new_buffer[new_index] = output_key;
                printf("%c", output_key);
                new_index++;
              }


              if(output_key == '\n') //key is enter, end of buffer
              {
                enter_flag = 1;
                int x;

                old_index = new_index;
                new_index = 0;
                for(x = 0; x < BUFFER_LENGTH; x++)
                {
                  old_buffer[x] = new_buffer[x];
                  new_buffer[x] = '\0'; //clear new_buffer
                }

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
      old_buffer[x] = '\0';
      new_buffer[x] = '\0';
    }
    old_index = 0;
    new_index = 0;
    enter_flag = 0;
}

/*Terminal_Open()
* Inputs: buf, nbytes both do nothing currently
* Outputs: returns 0
* This function currently does nothing but may be given functionality when system calls are implemented
*/
int32_t Terminal_Open(const void * buf, int32_t nbytes)
{
  if(buf == NULL || nbytes < 0) //invalid input
  {
    return FAILURE;
  }

  return SUCCESS; //open always works
}

/*Terminal_Close()
* Inputs: buf, nbytes both do nothing currently
* Outputs: returns 0
* This function currently does nothing but may be given functionality when system calls are implemented
*/
int32_t Terminal_Close(const void * buf, int32_t nbytes)
{
  if(buf == NULL || nbytes < 0) //invalid input
  {
    return FAILURE;
  }

  return SUCCESS; //close always works
}

/*
* Terminal_Read()
* Inputs: buf -> buffer to read data from keyboard into, nbytes -> number of chars to read from input
* Outputs: number of chars read from text buffer
* This function waits for enter to be pressed, then reads the desired number of chars from the text buffer
* into the provided buffer. It returns the number of bytes read
*/
int32_t Terminal_Read(const void * buf, int32_t nbytes)
{

if(buf == NULL || nbytes < 0) //invalid input
{
  return FAILURE;
}

while(!enter_flag); //wait for enter to be pressed to do anything

  int x;

  if(nbytes < old_index) //if we want to read less a smaller portion of buffer, change to smaller amount
  {
    old_index = nbytes;
  }
  //if nbytes > old_index, keep old_index the same to avoid problems of reading more than possible

  for(x = 0; x < old_index;x++) //old index = num_chars in old_buffer
  {
  ((char*)buf)[x] = old_buffer[x]; //copy in to given buffer
  }


enter_flag = 0;
  return old_index; //return num chars read into buffer argument which is either the number of chars in the
                    //buffer if nbytes > old_index or the number of chars desired if nbytes < old_index
}

/*
* Terminal_Write()
* Inputs: buf -> text buffer to print to screen, nbytes -> number of chars to print to screen
* Outputs: number of chars printed to screen
* This function prints the provided buffer to the screen, but prints only the provided number of
* chars. If nbytes is bigger than the size of the buffer, the entire buffer will be printed but cut off
* after enter
*/
int32_t Terminal_Write(const void * buf, int32_t nbytes)
{

  if(buf == NULL || nbytes < 0)
  {
    return FAILURE;
  }

  if(nbytes > BUFFER_LENGTH)
  {
    nbytes = old_index;
  }

  else if(nbytes > old_index)
  {
    nbytes = old_index;
  }

  int x = 0;
  for(x = 0; x < nbytes; x++)
  {
    printf("%c", ((char *)buf)[x]);
  }
  return nbytes;

}
