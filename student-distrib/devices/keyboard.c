#include "keyboard.h"
#include "../terminals.h"
#include "../scheduler.h"


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
  int alt_flag = 0;
  int clear_flag = 0;
  char old_buffer[BUFFER_LENGTH]; //"old buffer" saves the "new buffer" when enter is pressed and the "new buffer is cleared"
  char new_buffer[BUFFER_LENGTH]; //contains the current typing of the user

  char new_text_buffer_list[NUM_OF_TERMINALS][BUFFER_LENGTH];
  char old_text_buffer_list[NUM_OF_TERMINALS][BUFFER_LENGTH];
  int new_index_list[NUM_OF_TERMINALS];
  int old_index_list[NUM_OF_TERMINALS];



  int old_index; //contains the index of the enter key
  int new_index; //contains the current index to write to while typing
  int enter_flag; //checks if enter is pressed for Terminal_Read()
  int changed_terminals;

  int enter_flag_list[NUM_OF_TERMINALS];

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
//    enter_flag = 0;

    // enter_flag_list[0] = 0;
    // enter_flag_list[1] = 0;
    // enter_flag_list[2] = 0;
    term_t * curr_term = get_curr_terminal();
    int terminal_num = curr_term->term_index;

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
          if(scan_code == ALT_KEY_DOWN){
            alt_flag = 1;
          }
          if(scan_code == ALT_KEY_UP){
            alt_flag = 0;
          }
          if(ctrl_flag == 1){
            if(scan_code == L_KEY_DOWN){
              clear();
              clear_flag = 1;
              update_cursor(0,0);
              int x;

              //new_index = 0;
              new_index_list[terminal_num] = 0;
              for(x = 0; x < BUFFER_LENGTH; x++)
              {
                //new_buffer[x] = '\0';
                new_text_buffer_list[terminal_num][x] = '\0';
              }
            }

            if(scan_code == C_KEY_DOWN)
            {
              pcb_t * curr_pcb = get_curr_pcb();
                sti();
                send_eoi(KEYBOARD_IRQ);
                halt(curr_pcb->process_num);

            }
          }
          if(alt_flag == 1){
            if(scan_code >= F1_KEY_DOWN && scan_code <= F3_KEY_DOWN){
              terminal_fn_key = scan_code - F1_KEY_DOWN;
              // printf("%d",terminal_fn_key);
              changed_terminals = 1; //flag to indicate that we changed terminals
              sti();
              send_eoi(KEYBOARD_IRQ); //keyboard port on master
              switch_terminal(terminal_num, terminal_fn_key);
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
              output_key = keyboard_map[2][(unsigned char)scan_code]; //CAPS, not shift
            }else if(caps_lock_flag == 0 && shift_pressed == 1){
              output_key = keyboard_map[1][(unsigned char)scan_code]; //Shift, not CAPS
            }else if(caps_lock_flag == 1 && shift_pressed == 1){
              output_key = keyboard_map[3][(unsigned char)scan_code]; //Shift and CAPS
            }else{
                output_key = keyboard_map[0][(unsigned char)scan_code]; //No shift, no CAPS
            }
            if(clear_flag != 1 && output_key != '\0') //print key if clear flag is not set and key to print is not NULL
            {
              print_to_screen(output_key); //call helper function to handle all the cases to print char

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

    int x, a;
    for(a = 0; a < NUM_OF_TERMINALS; a++)
    {
      enter_flag_list[a] = 0;

    for(x = 0; x < BUFFER_LENGTH; x++)
    {
      old_buffer[x] = '\0';
      new_buffer[x] = '\0';
      new_text_buffer_list[a][x] = '\0';
      old_text_buffer_list[a][x] = '\0';
    }
  }
    old_index = 0;
    new_index = 0;
    enter_flag = 0;
    changed_terminals = 0;
    terminal_fn_key = 0;
}

/*Terminal_Open()
* Inputs: buf, nbytes both do nothing currently
* Outputs: returns 0
* This function currently does nothing but may be given functionality when system calls are implemented
*/
int32_t Terminal_Open(const uint8_t * filename)
{
  return SUCCESS; //open always works
}

/*Terminal_Close()
* Inputs: buf, nbytes both do nothing currently
* Outputs: returns 0
* This function currently does nothing but may be given functionality when system calls are implemented
*/
int32_t Terminal_Close(int32_t fd)
{
  return SUCCESS; //close always works
}

/*
* Terminal_Read()
* Inputs: buf -> buffer to read data from keyboard into, nbytes -> number of chars to read from input
* Outputs: number of chars read from text buffer
* This function waits for enter to be pressed, then reads the desired number of chars from the text buffer
* into the provided buffer. It returns the number of bytes read
*/
int32_t Terminal_Read(int32_t fd, void * buf, int32_t nbytes)
{

if(buf == NULL || nbytes < 0) //invalid input
{
  return FAILURE;
}

term_t * curr_term = get_curr_terminal();

if(curr_term == NULL)
{
  return FAILURE;
}

int terminal_num;

terminal_num = curr_term->term_index;

while(!enter_flag_list[terminal_num])
{
  if(changed_terminals == 1) //flag to indicate that we need to change the terminal num
  {
    curr_term = get_curr_terminal(); //get terminal
    terminal_num = curr_term->term_index; //change terminal num
    changed_terminals = 0; //reset flag
  }
}
//while(!enter_flag); //wait for enter to be pressed to do anything


  int x;

  // if(nbytes < old_index) //if we want to read less a smaller portion of buffer, change to smaller amount
  // {
  //   old_index = nbytes;
  // }

  if(nbytes < old_index_list[terminal_num])
  {
    old_index_list[terminal_num] = nbytes;
  }
  //if nbytes > old_index, keep old_index the same to avoid problems of reading more than possible

  // for(x = 0; x < old_index;x++) //old index = num_chars in old_buffer
  // {
  // ((char*)buf)[x] = old_buffer[x]; //copy in to given buffer
  // }

  for(x = 0; x < old_index_list[terminal_num]; x++)
  {
    ((char*)buf)[x] = old_text_buffer_list[terminal_num][x]; //copy in to given buffer
  }

enter_flag_list[terminal_num] = 0;
//enter_flag = 0;
//  return old_index; //return num chars read into buffer argument which is either the number of chars in the
                    //buffer if nbytes > old_index or the number of chars desired if nbytes < old_index

return old_index_list[terminal_num];
}

/*
* Terminal_Write()
* Inputs: buf -> text buffer to print to screen, nbytes -> number of chars to print to screen
* Outputs: number of chars printed to screen
* This function prints the provided buffer to the screen, but prints only the provided number of
* chars. If nbytes is bigger than the size of the buffer, the entire buffer will be printed but cut off
* after enter
*/
int32_t Terminal_Write(int32_t fd, const void * buf, int32_t nbytes)
{
    if(buf == NULL || nbytes < 0)
    {
      return FAILURE;
    }
    // cli();
    process_t* p = get_curr_process();
    term_t* t = get_curr_terminal();

    if(t->term_index == p->index)
    {

      int x;

        for(x = 0; x < nbytes; x++)
        {
          putc(((char *)buf)[x]);
        }

      // sti();
      return nbytes; //return number of bytes printed
    }
    else
    {
      int x;

        for(x = 0; x < nbytes; x++)
        {
          putc_dif_term(p, ((char *)buf)[x]);
        }

      // sti();
      return nbytes; //return number of bytes printed
    }

}


/*
* print_to_screen()
* Inputs: output_key, key to print to screen based on keyboard input
* Outputs: none
* This function is a helper function for the handler in order to cut down its size.
* This function has 3 main cases: backspace, enter, 'normal' chars which are detailed below
*/
void print_to_screen(char output_key)
{
  // process_t*
  term_t * curr_term = get_curr_terminal();
  int terminal_num;

  if(curr_term == NULL)
  {
    return;
  }

  terminal_num = curr_term->term_index;

  if(output_key == '\b') //backspace case
  {
    if(new_index_list[terminal_num] != 0)
    {
        // new_index--; //move back an index and fill with NULL
        // new_buffer[new_index] = '\0';
        new_index_list[terminal_num]--;
        new_text_buffer_list[terminal_num][new_index_list[terminal_num]] = '\0';
        printf("%c", output_key);
    }
  }
  //else if(new_index == ENTER_BUFFER_INDEX) //Buffer is full, fill last entry with enter to set up next if condition
  else if(new_index_list[terminal_num] == ENTER_BUFFER_INDEX)
  {
    if(output_key == '\n')
    {
      //new_buffer[new_index] = output_key;
      new_text_buffer_list[terminal_num][new_index_list[terminal_num]] = output_key;
      printf("%c", output_key);
      //new_index++;
      new_index_list[terminal_num]++;
      enter_flag_list[terminal_num] = 1;
      //enter_flag = 1
    }
  }
  else //any other chars besides backspace
  {
      printf("%c", output_key); //print to screen
      // new_buffer[new_index] = output_key; //fill in buffer
      //  new_index++;
      new_text_buffer_list[terminal_num][new_index_list[terminal_num]] = output_key;
      new_index_list[terminal_num]++;
  }



  if(output_key == '\n') //key is enter, end of buffer
  {
    //enter_flag = 1; //set enter flag to alert Terminal_Read that enter has been pressed
    enter_flag_list[terminal_num] = 1;

    int x;

     // old_index = new_index; //save location of enter key
     // new_index = 0;
    old_index_list[terminal_num] = new_index_list[terminal_num];
    new_index_list[terminal_num] = 0;



    for(x = 0; x < BUFFER_LENGTH; x++) //copy over new_buffer into old_buffer and clear new_buffer
    {
       // old_buffer[x] = new_buffer[x];
       // new_buffer[x] = '\0';
      old_text_buffer_list[terminal_num][x] = new_text_buffer_list[terminal_num][x];
      new_text_buffer_list[terminal_num][x] = '\0';
    }

  }
}
