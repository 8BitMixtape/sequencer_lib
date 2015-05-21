#ifndef HARDWARE_SETUP
#define HARDWARE_SETUP

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/iotn85.h>

//#include "debounce_lib.h"

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

volatile uint8_t hw_adc1 = _BV(ADLAR) | _BV(MUX0); //PB2-ADC1 pot2
volatile uint8_t hw_adc2 = _BV(ADLAR) | _BV(MUX1); //PB4-ADC2 pot1

volatile uint8_t pot1; // 0...255
volatile uint8_t pot2; // 0...255

// ATTINY85 pin
//                               +-\/-+
//                         PB5  1|    |8   VCC
//                         PB3  2|    |7   PB2  (D  2) pot2 (bitcrusher)
//          pot1 (tempo)   PB4  3|    |6   PB1  (D  1) pwm0 ====> OCR1A / sound output
//                         GND  4|    |5   PB0  (D  0)
//                               +----+

static inline void hw_set_output_pin()
{
    //DDRB |= 1<<DDB4; //set PB4 as output
    //PORTB &= ~(1 << PB4); //set PB4 output 0

    DDRB |= 1<<DDB1; //set PB1 as output
    PORTB &= ~(1 << PB1); //set PB1 output 0

//    DDRB |= 1<<DDB0; //set PB0 as output
//    PORTB &= ~(1 << PB0); //set PB0 output 0

    //btn_debounce_init();
}


static inline void hw_set_timer1()
{

    //TIMER SETUP [leave this settings]
    //When the CTC1 control bit is set (one), Timer/Counter1 is reset to $00 in the CPU clock cycle after a compare match with OCR1C register value.
    //If the control bit is cleared,
    //Timer/Counter1 continues counting and is
    //unaffected by a compare match.

    //TCCR1 |= _BV(CTC1); //clear timer on compare
    //TIMSK |= _BV(OCIE1A); //activate compare interruppt A
    //TIMSK |= _BV(OCIE1B); //activate compare interruppt B

    //TCNT1 = 0; //init count

    //TIMER PRESCALE FREQUENCY  [change this]
    //TCCR1 |= _BV(CS10); // prescale 1
    //TCCR1 |= _BV(CS11); // prescale 2
    //TCCR1 |= _BV(CS10)|_BV(CS12); // prescale 16
    //TCCR1 |= _BV(CS11)|_BV(CS12); // prescale 32
    //TCCR1 |= _BV(CS10)|_BV(CS11)|_BV(CS12); // prescale 64
    //TCCR1 |= _BV(CS13); // prescale 128
    //TCCR1 |= _BV(CS10) | _BV(CS13); // prescale 256

    //When the CTC1 control bit is set (one), Timer/Counter1 is reset to $00 in the CPU clock cycle after a compare match with OCR1C register value
    //OCR1C = 31; //(16000000/16)/31 = 32.258 Khz
    //OCR1B = 22;

    //OCR1C = 125;  // (16000000/16)/8000 = 125
    //OCR1C = 90;   // (16000000/16)/11025 = 90
    //OCR1C = 45;   // (16000000/16)/22050 = 45
    //OCR1C = 22;   // (16000000/16)/44100 = 22

    // Enable PLL and async PCK for high-speed PWM timer 1
    //PLLCSR |= (1 << PLLE) | (1 << PCKE);

//    //PWM1A, CLEAR ON COMPARE, NO PRESCALE
//    TCCR1 = 1<<PWM1A | 2<<COM1A0 | 1<<CS10; // PWM A, clear on match, 1:1 prescale
//    GTCCR = 1<<PWM1B | 2<<COM1B0;           // PWM B, clear on match
//    OCR1A = 128; OCR1B = 128;               // 50% duty at start


    // Enable PLL and async PCK for high-speed PWM
    PLLCSR |= (1 << PLLE) | (1 << PCKE);

    //set ctc mode
    TCCR1 |= _BV(CTC1); //pwm not affected if you disable this

    //http://www.technoblogy.com/show?LE0
    //no prescale
    TCCR1 |= _BV(CS10);

    //PWM1A
    TCCR1 |= _BV(PWM1A);

    //bug fix
    //http://electronics.stackexchange.com/questions/97596/attiny85-pwm-why-does-com1a0-need-to-be-set-before-pwm-b-will-work
    TCCR1 |= 3<<COM1A0;


}

static inline void hw_setup_timer0_pwm()
{
    //PWM SOUND OUTPUT MODE [change this]
    //TCCR0A |= (1<<WGM00)|(1<<WGM01); //Fast pwm
    //TCCR0A |= (1<<WGM00) ; //Phase correct pwm

    //PWM SETTINGS [leave this settings]
    //TCCR0A |= (1<<COM0A1); //Clear OC0A/OC0B on Compare Match when up-counting.
    //TCCR0B |= (1<<CS00);//no prescale

    //from ref website, this enable OCR0B
    //TCCR0A = 2<<COM0A0 | 2<<COM0B0 | 3<<WGM00;
    //TCCR0B = 0<<WGM02 | 1<<CS00;

    // Set up Timer/Counter0 for 8kHz interrupt to output samples.
    TCCR0A = 3<<WGM00;                      // Fast PWM
    TCCR0B = 1<<WGM02 | 2<<CS00;            // 1/8 prescale
    TIMSK = 1<<OCIE0A;                      // Enable compare match
    OCR0A = 61;
}

static inline void hw_adc_init()
{
    ADCSRA |= _BV(ADIE); //adc interrupt enable
    ADCSRA |= _BV(ADEN); //adc enable
    ADCSRA |= _BV(ADATE); //auto trigger
    ADCSRA |= _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); //prescale 128
    ADMUX  = hw_adc1;
    ADCSRB = 0;
}

static inline void hw_adc_start()
{
    ADCSRA |= _BV(ADSC); //start adc conversion
}

static inline void hw_init()
{
    //no prescale, enable 16 Mhz
    clock_prescale_set(clock_div_1);

    //timer 0 pwm - setup pwm / sound output
    hw_setup_timer0_pwm();

    //setup timer 1 - for sequence clock update
    hw_set_timer1();

    //set output pin - for led / counter
    hw_set_output_pin();

    //init adc - adc input for pots
    hw_adc_init();

    //enable global interrupt
    sei();

    //start adc conversion
    hw_adc_start();
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


#endif // HARDWARE_SETUP

