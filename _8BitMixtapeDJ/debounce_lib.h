#ifndef DEBOUNCE_LIB_H
#define DEBOUNCE_LIB_H

//Software debouncing of buttons by snigelen
//from
//http://www.avrfreaks.net/sites/default/files/forum_attachments/debounce.pdf

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#define BUTTON_PORT PORTB
#define BUTTON_PIN PINB
#define BUTTON_DDR DDRB
#define BUTTON1_MASK (1<<PB1)
#define BUTTON2_MASK (1<<PB3)
#define BUTTON_MASK (BUTTON1_MASK | BUTTON2_MASK)

#define VC_DEC_OR_SET(high, low, mask)  \
    low = ~(low & mask);                \
    high = low ^ (high & mask);         \

volatile uint8_t buttons_down;

static inline void debounce (void){

    static uint8_t vcount_low = 0xFF, vcount_high = 0xFF;
    static uint8_t button_state = 0;

    uint8_t state_changed = ~BUTTON_PIN ^ button_state;
    VC_DEC_OR_SET(vcount_high, vcount_low, state_changed);

    state_changed &= vcount_low & vcount_high;
    button_state  ^= state_changed;

    buttons_down |= button_state & state_changed;

}

uint8_t button_down(uint8_t button_mask)
{
    //cli();
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        button_mask &= buttons_down;
        buttons_down ^= button_mask;
    }
    //sei();

    return button_mask;
}

static inline void debounce_init()
{
    BUTTON_DDR  &= ~(BUTTON_MASK);
    BUTTON_PORT |= BUTTON_MASK;
}


#endif // DEBOUNCE_LIB_H

