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
#include <sequencer_lib.h>


volatile unsigned long T_START = 40000;

volatile uint8_t slower_interval = 0;

volatile uint8_t current_song_index  = 0;

volatile unsigned long t = 40000; // long
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

uint8_t button_step_index = 0;
uint8_t start_sequencer = 0;


seq_instance SEQUENCER_1;

#define   SEQUENCER1_PATTERN_LENGTH 8
seq_pattern SEQUENCER_1_PATTERN_ARRAY[SEQUENCER1_PATTERN_LENGTH] =
{
    //sustain, freq
    { 6000, 200}, //1
    { 6000, 200}, //2
    { 6000, 200}, //3
    { 6000, 200}, //4
    { 6000, 200}, //5
    { 6000, 200}, //6
    { 6000, 200}, //7
    { 6000, 200}, //8
};


void setup_sequencer()
{
    //init with sequencer instance, sequencer stage array, sequencer stage length, default tempo
    seq_init(&SEQUENCER_1, SEQUENCER_1_PATTERN_ARRAY, SEQUENCER1_PATTERN_LENGTH, 10000);
}

void update_sound(uint8_t song_id)
{
    switch (song_id)
    {
        case 0:
        snd = (t*(5+(pot1/5))&t>>7)|(t*3&t>>(10-(seq_get_current_pattern(&SEQUENCER_1)->freq/5)));
        break;

        case 1:
        snd = (t|(t>>(9+(pot1/2))|t>>7))*t&(t>>(11+(seq_get_current_pattern(&SEQUENCER_1)->freq/2))|t>>9);
        break;

        case 2:
        snd = (t*9&t>>4|t*5&t>>(7+(pot1/2))|t*3&t/(1024-(seq_get_current_pattern(&SEQUENCER_1)->freq/2)))-1;
        break;

        case 3:
        snd = (t>>6|t|t>>(t>>(16-(pot1/2))))*10+((t>>11)&(7+(seq_get_current_pattern(&SEQUENCER_1)->freq/2)));
        break;

        case 4:
        snd = t*(((t>>(11-(seq_get_current_pattern(&SEQUENCER_1)->freq/2)))&(t>>8))&((123-pot1)&(t>>3)));
        break;

        case 5:
        snd = t*(t^t+(t>>15|1)^(t-(1280-(pot1/2))^t)>>(10-(seq_get_current_pattern(&SEQUENCER_1)->freq/5)));
        break;

        case 6:
        snd = t * ((pot1>>12|t>>8)&seq_get_current_pattern(&SEQUENCER_1)->freq&t>>4);
        //snd = (t*t/(256-pot1))&(t>>((t/(1024-seq_get_current_step(&sequencer1)->freq))%16))^t%64*(0xC0D3DE4D69>>(t>>9&30)&t%32)*t>>18;
        break;

        case 7:
        snd = (t&t>>(12+(pot1/2)))*(t>>4|t>>(8-(seq_get_current_pattern(&SEQUENCER_1)->freq/2)))^t>>6;
        break;

        case 8:
        snd = t*(((t>>9)^((t>>9)-(1+(pot1/2)))^1)%(13+(seq_get_current_pattern(&SEQUENCER_1)->freq/2)));
        break;

        case 9:
        snd = t*(((t>>(12+(pot1/2)))|(t>>8))&((63-(seq_get_current_pattern(&SEQUENCER_1)->freq/2))&(t>>4)));
        break;

    }

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
                    SEQUENCER_1_PATTERN_ARRAY[button_step_index].freq = pot1;
                    //sequencer1_sequence[button_step_index].duration = pot1 << 6;
                    //}
                }

            if(btn_button_down(BUTTON1_MASK))
                {
                    current_song_index = (current_song_index + 1) % 9;

                    //T_START += 1000;
                }


            if(btn_button_down(BUTTON2_MASK))
                {

                    if (start_sequencer)
                    {
                        T_START += 100;

                    }else{
                        //TCNT1 = 0;
    //                //set sequence mode
                        button_step_index++;

                        seq_goto_pattern(&SEQUENCER_1, button_step_index);

                        if(button_step_index > 7)
                            {
                                //play enable interrupt timer pwm
                                //TIMSK ^= _BV(OCIE1A);
                                start_sequencer = 1;
                            }
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
    //if(pot2_val< 10 ) pot2_val = 10;

    if(start_sequencer)
        {


            seq_set_tempo(&SEQUENCER_1,pot2_val << 6); //map 0-16320

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

            if(SEQUENCER_1.sound_generator_on > 0)
                {

                    update_sound(current_song_index);
                    //snd = t * ((10>>12|t>>8)& pot2 &t>>4);
                    //snd = t*(((t>>(11-(pot2/2)))&(t>>8))&((123-20)&(t>>3)));
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

            seq_update_time(&SEQUENCER_1);

        }
    else
        {

            if(t>(SEQUENCER_1.sequencer_pattern_array[button_step_index].sustain + T_START))
                {
                    t = T_START;
                }

            update_sound(current_song_index);

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
