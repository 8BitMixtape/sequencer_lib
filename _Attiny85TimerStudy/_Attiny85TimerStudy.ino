/* Name: main.c
 * Author: <insert your name here>
 * Copyright: <insert your copyright message here>
 * License: <insert your license reference here>
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/power.h>

#define STEP_COUNT 8

//const uint8_t NUMBER_OF_STEPS = STEP_COUNT - 1;
//volatile uint8_t CURRENT_STEP_INDEX = 0;

//uint8_t stepFreqs[]    = {  10,  255, 10,255, 10,  255, 10,255};
//uint8_t stepSustains[] = {100,100,100,100,100,100,100,100};

//volatile uint16_t timer0_tempo_interval = 0;
//volatile uint16_t timer0_sustain_interval = 0;
//volatile uint16_t timer0_duration_millis = 0;

//uint8_t first_start = 1;


//uint8_t pwm_fade = 0;

//volatile uint64_t t; // this will affect sound quality
//volatile uint16_t song_interval = 0;
unsigned long slower_interval = 0;

volatile unsigned long t; // long
volatile unsigned long t_delay; // long

volatile unsigned long u; // long
volatile uint8_t snd; // 0...255
volatile uint8_t snd2; // 0...255

volatile uint8_t adc1 = _BV(ADLAR) | _BV(MUX0); //PB2-ADC1 pot2
volatile uint8_t adc2 = _BV(ADLAR) | _BV(MUX1); //PB4-ADC2 pot1
volatile uint8_t pot1; // 0...255
volatile uint8_t pot2; // 0...255

void setupOutputPin()
{
    DDRB |= 1<<DDB4; //set PB4 as output
    PORTB &= ~(1 << PB4); //set PB4 output 0

    DDRB |= 1<<DDB1; //set PB1 as output
    PORTB &= ~(1 << PB1); //set PB1 output 0

    DDRB |= 1<<DDB0; //set PB0 as output
    PORTB &= ~(1 << PB0); //set PB0 output 0

}

void setupTimer1()
{

    //TIMER SETUP [leave this settings]
    TCCR1 |= _BV(CTC1); //clear timer on compare
    TIMSK |= _BV(OCIE1A); //activate compare interruppt
    TIMSK |= _BV(OCIE1B); //activate compare interruppt

    TCNT1 = 0; //init count


    //TIMER PRESCALE FREQUENCY  [change this]
    //TCCR1 |= _BV(CS10); // prescale 1
    //TCCR1 |= _BV(CS11); // prescale 2
    TCCR1 |= _BV(CS10)|_BV(CS12); // prescale 16
    //TCCR1 |= _BV(CS11)|_BV(CS12); // prescale 32
    //TCCR1 |= _BV(CS10)|_BV(CS11)|_BV(CS12); // prescale 64
    //TCCR1 |= _BV(CS13); // prescale 128
    //TCCR1 |= _BV(CS10) | _BV(CS13); // prescale 256

    OCR1C = 100;
    //OCR1B = 22;

    //OCR1C = 125;  // (16000000/16)/8000 = 128
    //OCR1C = 90; // (16000000/16)/11025 = 90
    //OCR1C = 45; // (16000000/16)/22050 = 45
    //OCR1C = 22; // (16000000/16)/44100 = 22


}


void disableTimer0PWM()
{
    //Writing the TSM bit to one activates the Timer/Counter Synchronization Mode. In this mode,
    //the value written to PSR0 is kept, hence keeping the Prescaler Reset signal asserted
    GTCCR |= _BV(TSM) | _BV(PSR0);
}

void enableTimer0PWM()
{
    //When the TSM bit is written to zero, the PSR0 bit is cleared by hardware, and the timer/counter start counting.
    GTCCR &= ~_BV(TSM);
}

void setupTimer0PWM()
{
    //PWM SOUND OUTPUT MODE [change this]
    TCCR0A |= (1<<WGM00)|(1<<WGM01); //Fast pwm
    //TCCR0A |= (1<<WGM00) ; //Phase correct pwm
    //PWM SETTINGS [leave this settings]
//    TCCR0A |= (1<<COM0A1); //Clear OC0A/OC0B on Compare Match when up-counting.
//    TCCR0B |= (1<<CS00);//no prescale

     //from ref website, this enable OCR0B
     TCCR0A = 2<<COM0A0 | 2<<COM0B0 | 3<<WGM00;
     TCCR0B = 0<<WGM02 | 1<<CS00;
}

void adc_init()
{
    ADCSRA |= _BV(ADIE); //adc interrupt enable
    ADCSRA |= _BV(ADEN); //adc enable
    ADCSRA |= _BV(ADATE); //auto trigger
    ADCSRA |= _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); //prescale 128
    ADMUX  = adc1;
    ADCSRB = 0;
}

void adc_start()
{
    ADCSRA |= _BV(ADSC); //start adc conversion
}

int main(void)
{
    clock_prescale_set(clock_div_1);


    //cli(); //disable interrupt

    //disable timer 0
    //disableTimer0PWM();
    //timer 0 pwm
    setupTimer0PWM();
    //setup timer 1
    setupTimer1();
    //set PB1 output 1
    setupOutputPin();
    //enable timer 0
    //enableTimer0PWM();

    adc_init();

    sei(); //enable global interrupt

    adc_start();

//    /* endless loop */
//    for(;;)
//        {

////        for (pwm_fade =0; pwm_fade<255; ++pwm_fade)
////            {
////                OCR1A = 255 - pwm_fade;
////                _delay_ms(2);
////            }
////        _delay_ms(10);
////        for (pwm_fade =255; pwm_fade>0; --pwm_fade)
////            {
////                OCR1A = 255 - pwm_fade;
////                _delay_ms(2);
////            }
////        _delay_ms(100);
////        PORTB ^= (1 << PB4);


////            if(TIFR & _BV(OCF0A)) //TIMER0_COMPA_vect
////                {
////                    timer0_tempo_interval++;

////                    if(CURRENT_STEP_INDEX == 0)
////                        {
////                            OCR1A = 255 - stepFreqs[CURRENT_STEP_INDEX];
////                        }

////                    //on end of bar
////                    if (timer0_tempo_interval >= 10000)
////                        {
////                            timer0_tempo_interval = 0;
////                            timer0_duration_millis = 0;
////                            CURRENT_STEP_INDEX++;
////                            if(CURRENT_STEP_INDEX > NUMBER_OF_STEPS) CURRENT_STEP_INDEX = 0;
////                        }


////                    TIFR = _BV(OCF0A); // will need this if no timer interrupt not activated
////                    //OCF0A is cleared by writing a logic one to the flag.
////                    //OCF0A is cleared by hardware when executing the corresponding interrupt handling vector
////                }





//        }

    while(1)
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


    }

    return 0;   /* never reached */
}


//COMPB will be executed after COMPA

//TEMPO TIMER 7843
ISR(TIMER1_COMPB_vect)
{
//    timer0_tempo_interval++;

////    if(first_start)
////    {
////        OCR1A = 255 - stepFreqs[CURRENT_STEP_INDEX];
////    }


////    if(CURRENT_STEP_INDEX == 0)
////        {
////        }



//    //on end of bar
//    if (timer0_tempo_interval >= 10000)
//        {
//            //PORTB ^= (1 << PB4);
//            timer0_tempo_interval = 0;
//            //timer0_duration_millis = 0;

//            CURRENT_STEP_INDEX++;
//            if(CURRENT_STEP_INDEX > NUMBER_OF_STEPS) CURRENT_STEP_INDEX = 0;
//        }

//    if(first_start) first_start = 0;

}


//PWM SUSTAIN TIMER freq = 8000 Hz
ISR(TIMER1_COMPA_vect)
{

    slower_interval++;

    cli();
    uint8_t rate = pot1;
    uint8_t pot2_val = pot2;

    if(pot2_val< 10 ) pot2_val = 10;

    //OCR1C = pot2_val;
    sei();



//    if(pot2_val >= 127)
//    {
//        crossfade_l = pot2_val - 126;
//        crossfade_r = 256 - pot2_val;
//    }else{
//        crossfade_l = pot2_val;
//        crossfade_r = 127 - pot2_val;
//    }


    if(slower_interval >= rate)
    {
        //play song
        snd = (t|(t>>(9+(10/2))|t>>7))*t&(t>>(11+(20/2))|t>>9);

        snd2 = (t*(5+(40/5))&t>>7)|(t*3&t>>(10-(8/5)));
        //snd = t*(((t>>9)^((t>>9)-(1+(44/2)))^1)%(13+(12/2)));


        OCR0A = snd;
        OCR0B = (snd/2) + (snd2/2);

        t++;
        slower_interval = 0;
    }

//    //15625 = 1 sec
//    //7812 = 0.5 sec
//    timer0_duration_millis++;
////    song_interval++;
////    //timer0_duration_millis += 16;

////        t++;
////        snd = (t*(5+(12/5))&t>>7)|(t*3&t>>(10-(8/5)));
////        song_interval = 0;
////        OCR1A = 255 - snd;


//        //        snd = t*(((t>>(12+(120/2)))|(t>>8))&((63-(60/2))&(t>>4)));

//        //        snd = t*(((t>>(11-(4/2)))&(t>>8))&((123-2)&(t>>3)));


//    if(timer0_duration_millis >= 2500)
//    {
//        PORTB ^= (1 << PB4);
//        timer0_duration_millis = 0;
//    }

}



ISR(ADC_vect)
{
    //http://joehalpin.wordpress.com/2011/06/19/multi-channel-adc-with-an-attiny85/

    static uint8_t firstTime = 1;
    static uint8_t val;

    val = ADCH;

    if (firstTime == 1)
        firstTime = 0;
    else if (ADMUX  == adc1) {
        pot1 = val;
        ADMUX = adc2;
    }
    else if ( ADMUX == adc2) {
        pot2  = val;
        ADMUX = adc1;
    }

}
