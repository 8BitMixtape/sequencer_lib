/* Name: main.c
 * Author: Budi Prakosa
 *
 * Ported from
 * http://interface.khm.de/index.php/lab/interfaces-advanced/arduino-dds-sinewave-generator/
 * to Attiny85 @ 16Mhz internal PLL
 *
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/pgmspace.h>
#include <math.h>


#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

#include "hardware_setup.h"



// table of 256 sine values / one sine period / stored in flash memory
PROGMEM  prog_uchar sine256[]  = {
  127,130,133,136,139,143,146,149,152,155,158,161,164,167,170,173,176,178,181,184,187,190,192,195,198,200,203,205,208,210,212,215,217,219,221,223,225,227,229,231,233,234,236,238,239,240,
  242,243,244,245,247,248,249,249,250,251,252,252,253,253,253,254,254,254,254,254,254,254,253,253,253,252,252,251,250,249,249,248,247,245,244,243,242,240,239,238,236,234,233,231,229,227,225,223,
  221,219,217,215,212,210,208,205,203,200,198,195,192,190,187,184,181,178,176,173,170,167,164,161,158,155,152,149,146,143,139,136,133,130,127,124,121,118,115,111,108,105,102,99,96,93,90,87,84,81,78,
  76,73,70,67,64,62,59,56,54,51,49,46,44,42,39,37,35,33,31,29,27,25,23,21,20,18,16,15,14,12,11,10,9,7,6,5,5,4,3,2,2,1,1,1,0,0,0,0,0,0,0,1,1,1,2,2,3,4,5,5,6,7,9,10,11,12,14,15,16,18,20,21,23,25,27,29,31,
  33,35,37,39,42,44,46,49,51,54,56,59,62,64,67,70,73,76,78,81,84,87,90,93,96,99,102,105,108,111,115,118,121,124

};


double dfreq;
// const double refclk=31372.549;  // =16MHz / 510  http://playground.arduino.cc/Main/TimerPWMCheatsheet
const double refclk=32258.064;      // measured

// variables used inside interrupt service declared as voilatile
volatile uint8_t icnt;              // var inside interrupt

volatile uint8_t icnt1;             // var inside interrupt
//volatile uint8_t c4ms;              // counter incremented all 4ms
volatile unsigned long phaccu;   // pahse accumulator
volatile unsigned long tword_m;  // dds tuning word m


/// StarT sequencer
///
#include <sequencer_lib.h>

seq_instance SEQUENCER_1;

#define   SEQUENCER1_PATTERN_LENGTH 8
seq_pattern SEQUENCER_1_PATTERN_ARRAY[SEQUENCER1_PATTERN_LENGTH] =
{
    //sustain, freq
    { 6000, 200}, //0
    { 6000, 300}, //1
    { 6000, 400}, //2
    { 6000, 500}, //3
    { 6000, 200}, //4
    { 6000, 400}, //5
    { 6000, 400}, //6
    { 6000, 200}, //7
};


static inline void setup_sequencer()
{
    //init with SEQUENCER_1 instance, pattern SEQUENCER_1 array, array length, default tempo = 10000 ticks
    seq_init(&SEQUENCER_1, SEQUENCER_1_PATTERN_ARRAY, SEQUENCER1_PATTERN_LENGTH, 10000);
}


/// end sequencer


int main(void)
{
    //setup avr hardware
    //pots, adc, pwm, timer
    hw_init();
    setup_sequencer();

    dfreq=600.0;                    // initial output frequency = 1000.o Hz
    tword_m=pow(2,32)*dfreq/refclk;  // calulate DDS new tuning word

    //based on https://www.tty1.net/blog/2008/avr-gcc-optimisations_en.html
    for(;;)
        {
           // if (c4ms > 250) {                 // timer / wait fou a full second
                 //c4ms=0;
            if(SEQUENCER_1.sound_generator_on > 0)
                {
                     dfreq=seq_get_current_pattern(&SEQUENCER_1)->freq; // read Poti to adjust output frequency from 0..1020 Hz
                     tword_m=pow(2,32)*dfreq/refclk;  // calulate DDS new tuning word
                }else{
                     tword_m = 0;
                }

                 //  }
        }

    return 0;   /* never reached */
}


////COMPB will be executed after COMPA
//ISR(TIMER1_COMPB_vect)
//{
//}


//PWM SUSTAIN TIMER
//freq = 32.258 Khz
ISR(TIMER1_COMPA_vect)
{    
    phaccu=phaccu+tword_m; // soft DDS, phase accu with 32 bits
    icnt=phaccu >> 24;     // use upper 8 bits for phase accu as frequency information
                           // read value fron ROM sine table and send to PWM DAC
    OCR0A=pgm_read_byte_near(sine256 + icnt);

    if(icnt1++ >= 4) {  // 32.258 Hz / 4 = 8.064.5 Hz
      seq_update_time(&SEQUENCER_1);
      icnt1=0;
     }
}

