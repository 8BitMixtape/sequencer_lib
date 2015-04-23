/* Name: main.c
 * Author: <insert your name here>
 * Copyright: <insert your copyright message here>
 * License: <insert your license reference here>
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/power.h>

#include "hardware_setup.h"
#include "debounce_lib.h"
#include "sequencer_lib.h"


#define T_START 40000

volatile uint8_t slower_interval = 0;

volatile unsigned long t = T_START; // long
//volatile uint64_t t; // this will affect sound quality
//volatile uint16_t song_interval = 0;

//volatile unsigned long t_delay; // long
//volatile unsigned long u; // long

volatile uint8_t snd; // 0...255
volatile uint8_t snd2; // 0...255

volatile uint8_t pot1; // 0...255
volatile uint8_t pot2; // 0...255

volatile uint8_t pot1_val = 0;
volatile uint8_t pot2_val = 0;

#define SEQUENCER1_LENGTH 8

uint8_t button_step_index = 0;
uint8_t start_sequencer = 0;

seq_sequence sequencer1_sequence[SEQUENCER1_LENGTH] =
{
    //sustain, freq
    { 2000, 200},
    { 2000, 200},
    { 2000, 200},
    { 2000, 200},
    { 2000, 200},
    { 2000, 200},
    { 2000, 200},
    { 2000, 200},
};

seq_instance sequencer1;

void setup_sequencer()
{
    seq_init(&sequencer1, sequencer1_sequence, SEQUENCER1_LENGTH, 10000);
}


int main(void)
{
    //setup avr hardware
    //pots, adc, pwm, timer
    hw_init();

    //setup step sequencer
    setup_sequencer();

    //enter sequencing mode disable sequencer interrupt
    //TIMSK ^= _BV(OCIE1A);


    //based on https://www.tty1.net/blog/2008/avr-gcc-optimisations_en.html
    for(;;)
        {
            /*
            uint8_t btn1_now = button_is_pressed(PINB, PB1);
            if ( btn1_previous != btn1_now && btn1_now == 1 ) {
                songs++;
                if (songs > 3) songs = 0;
                btn1_previous = btn1_now;
            }else{
                btn1_previous = btn1_now;
            }
            */

            if(!start_sequencer)
                {
                    //ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
                    //{
                    sequencer1_sequence[button_step_index].freq = pot2;
                    sequencer1_sequence[button_step_index].duration = pot1 << 6;
                    //}
                }

            if(btn_button_down(BUTTON2_MASK))
                {


                    //TCNT1 = 0;
//                //set sequence mode
                    button_step_index++;

                    if(button_step_index > 7)
                        {
                            //play enable interrupt timer pwm
                            //TIMSK ^= _BV(OCIE1A);
                            start_sequencer = 1;
                        }
                }

        }

    return 0;   /* never reached */
}


//COMPB will be executed after COMPA
ISR(TIMER1_COMPB_vect)
{

    ++slower_interval;

    if (slower_interval> 100)
        {
            btn_debounce();
            slower_interval = 0;
        }



}


//PWM SUSTAIN TIMER freq = 8000 Hz
ISR(TIMER1_COMPA_vect)
{


    pot1_val = pot1;
    pot2_val = pot2;

    //if(pot1_val< 10 ) pot1_val = 10;
    if(pot2_val< 10 ) pot2_val = 10;

    if(start_sequencer)
        {


            seq_set_tempo(&sequencer1,pot1_val << 6);

            // if(button_down(BUTTON2_MASK))
            // {
            //     pot1_val += 10;
            //     seq_set_tempo(&sequencer1,pot1_val << 6);
            // }



//    if(pot2_val >= 127)
//    {
//        crossfade_l = pot2_val - 126;
//        crossfade_r = 256 - pot2_val;
//    }else{
//        crossfade_l = pot2_val;
//        crossfade_r = 127 - pot2_val;
//    }


//    if(slower_interval >= pot1_val)
//    {
//        //play song
//        //snd2 = (t*(5+(40/5))&t>>7)|(t*3&t>>(10-(8/5)));
//        //snd = t*(((t>>9)^((t>>9)-(1+(44/2)))^1)%(13+(12/2)));
//       snd = t*(((t>>(11-(pot2_val/2)))&(t>>8))&((123-20)&(t>>3)));
//   OCR0A = snd;
//   t++;

            if(sequencer1.sound_generator_on > 0)
                {

                    //snd = t * ((10>>12|t>>8)& pot2 &t>>4);
                    snd = t*(((t>>(11-(pot2/2)))&(t>>8))&((123-20)&(t>>3)));
                    //snd = (t*(5+(pot2_val/5))&t>>7)|(t*3&t>>(10-(8/5)));
                    //snd = t*(((t>>(11-(pot2_val/2)))&(t>>8))&((123-20)&(t>>3)));
                    OCR0A = snd;
                    t++;
                }
            else
                {
                    OCR0A = 0;
                    t = T_START;
                }

//        //OCR0B = (snd/2) + (snd2/2);
//        slower_interval = 0;
//    }

            seq_update(&sequencer1);

        }
    else
        {

            if(t>(sequencer1.sequencer[button_step_index].duration + T_START))
                {
                    t = T_START;
                }

            snd = t*(((t>>(11-(pot2/2)))&(t>>8))&((123-20)&(t>>3)));

            //snd = t * ((10>>12|t>>8)&pot2&t>>4);
            OCR0A = snd;
            t++;

        }

}


//handle adc conversion
//ATtiny25/45/85 [DATASHEET] page 48 vector 9
ISR(ADC_vect)
{
    //http://joehalpin.wordpress.com/2011/06/19/multi-channel-adc-with-an-attiny85/

    static uint8_t firstTime = 1;
    static uint8_t val;

    val = ADCH;

    if (firstTime == 1)
        firstTime = 0;
    else if (ADMUX  == hw_adc1)
        {
            pot1 = val;
            ADMUX = hw_adc2;
        }
    else if ( ADMUX == hw_adc2)
        {
            pot2  = val;
            ADMUX = hw_adc1;
        }

}
