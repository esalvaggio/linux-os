#include "pit.h"

#define PORT_0          0x40
#define PORT_1          0x41
#define PORT_2          0x42
#define CMMD_R          0x43        //Write only

#define OG_FREQ      1193180

#define TOP_SHIFT          8
#define LOW_BITMASK     0xFF

#define PIT_IRQ            0
#define PIT_INDEX         32
#define COMMAND         0x36

#define NEW_HZ            32

int ticks;
/*
 * Eventually this function will call context switching
 inside this function. Ticks is just a placeholder for
 counting seconds,
 */
void pit_handler(){
    sti();
    send_eoi(PIT_IRQ);
    ticks++;
    if (ticks == 32){
        //Every second-ish
        ticks = 0;
    }
}
/*
 * pit_init
 * INPUTS: None
 * OUTPUTS: None
 * Initializes the PIT. Sets idt entry to present and links handler.
 * Writes the new PIT frequency to what we want for interrupts.
 * Unmasks IRQ0 for interrupts.
 * SIDE EFFECTS: All mentioned above.
*/
void pit_init(){

    idt[PIT_INDEX].present = 1;
    SET_IDT_ENTRY(idt[PIT_INDEX], pit_handler); //index 40 is the RTC in the IDT
    set_pit_freq(NEW_HZ);

    ticks = 0;

    enable_irq(PIT_IRQ);
}
/*
 * set_pit_freq
 * INPUTS: New frequency (in HZ, not ms)
 * OUTPUTS: None
 * This function sets the pit to a new frequency
 * by first writing to its command port and then
 * writing to port A (which is connected to IRQ0)
 * the new frequency.
 * SIDE EFFECTS: changes frequenct of interrupts
*/
void set_pit_freq(uint32_t hz){
    pit_send_command((uint8_t)COMMAND);
    //Need to calculate what to write to PIT
    uint32_t divisor = OG_FREQ / hz;

    //Write divisor to PIT, low bits then high bits
    outb(divisor & LOW_BITMASK, PORT_0); //low
    outb((divisor >> TOP_SHIFT) & LOW_BITMASK, PORT_0); //high
}
/*
 * pit_send_command
 * INPUTS: 8bit command signal
 * OUTPUTS: None
 * Writes a command to the command port. Since it is one line,
 * it could be just included in pit_init, but the function was
 * written in case we need to call it elsewhere.
 * SIDE EFFECTS: writes to PIT's command port
*/
void pit_send_command(uint8_t cmd){
    outb(cmd, CMMD_R);
}