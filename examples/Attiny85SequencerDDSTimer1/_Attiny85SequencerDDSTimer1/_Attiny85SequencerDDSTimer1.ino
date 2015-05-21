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

volatile unsigned long t = 0; // long
volatile uint8_t snd; // 0...255


#include "hardware_setup.h"

//START DDS STUFF

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
//volatile uint8_t c4ms;              // counter incremented all 4ms
volatile unsigned long phaccu;   // pahse accumulator
volatile unsigned long tword_m;  // dds tuning word m

//END DDS STUFF


//START SEQUENCER

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

#include <sequencer_lib.h>

volatile uint8_t icnt1; // var inside interrupt for sequencer timing

seq_instance SEQUENCER_1;

#define   SEQUENCER1_PATTERN_LENGTH 12
seq_pattern SEQUENCER_1_PATTERN_ARRAY[SEQUENCER1_PATTERN_LENGTH] =
{
    //sustain, freq
    { 6000, NOTE_C1}, //0
    { 6000, NOTE_E7}, //1
    { 6000, 0}, //2
    { 6000, NOTE_E7}, //3
    { 6000, 0}, //4
    { 6000, NOTE_C7}, //5
    { 6000, NOTE_E7}, //6
    { 6000, 0}, //7
    { 6000, NOTE_G7}, //7
    { 6000, 0}, //7
    { 6000, 0}, //7
    { 6000, 0}, //7

};


static inline void setup_sequencer()
{
    //init with SEQUENCER_1 instance, pattern SEQUENCER_1 array, array length, default tempo = 10000 ticks
    seq_init(&SEQUENCER_1, SEQUENCER_1_PATTERN_ARRAY, SEQUENCER1_PATTERN_LENGTH, 10000);
}


//END SEQUENCER


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

            seq_set_tempo(&SEQUENCER_1,pot1 << 6); //map 0-16320 play with tempo

            if(SEQUENCER_1.sound_generator_on > 0)
                {
                       // dfreq=pot1<< 2; // set dfreq to SEQUENCER_1 current step FREQUENCY

                     dfreq=seq_get_current_pattern(&SEQUENCER_1)->freq; // set dfreq to SEQUENCER_1 current step FREQUENCY
                     tword_m=pow(2,32)*dfreq/refclk;  // calulate DDS new tuning word
                }else{
                     tword_m = 0;
                }

        }

    return 0;   /* never reached */
}


//PWM SUSTAIN TIMER
//freq = 32.258 Khz
ISR(TIMER0_COMPA_vect)
{    
    phaccu=phaccu+tword_m; // soft DDS, phase accu with 32 bits
    icnt=phaccu >> 24;     // use upper 8 bits for phase accu as frequency information
                           // read value fron ROM sine table and send to PWM DAC
                           
    //                                          bitcrusher effect... 
    OCR1A=pgm_read_byte_near(sine256 + icnt) >> (pot2 >> 5); //pot2 range 0..255 to 0..7

    if(icnt1++ >= 4) {  // 32.258 Hz / 4 = 8.064.5 Hz
      seq_update_time(&SEQUENCER_1);
      icnt1=0;
     }
}

