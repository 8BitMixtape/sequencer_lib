#ifndef HARDWARE_SETUP
#define HARDWARE_SETUP

//#include "debounce_lib.h"


volatile uint8_t hw_adc1 = _BV(ADLAR) | _BV(MUX0); //PB2-ADC1 pot2
volatile uint8_t hw_adc2 = _BV(ADLAR) | _BV(MUX1); //PB4-ADC2 pot1

volatile uint8_t pot1; // 0...255
volatile uint8_t pot2; // 0...255

// ATTINY85 _Attiny85Timer0DDSPwm pin
//
//                               +-\/-+
//  Ain0           (D  5)  PB5  1|    |8   VCC
//  Ain3           (D  3)  PB3  2|    |7   PB2  (D  2)  INT0  Ain1 ====> pot2
// pot1 ===> Ain2  (D  4)  PB4  3|    |6   PB1  (D  1)        pwm1
//                         GND  4|    |5   PB0  (D  0)        pwm0 ====> OCR0A / sound output
//                               +----+

static inline void hw_set_output_pin()
{
    //DDRB |= 1<<DDB4; //set PB4 as output
    //PORTB &= ~(1 << PB4); //set PB4 output 0

    //DDRB |= 1<<DDB1; //set PB1 as output
    //PORTB &= ~(1 << PB1); //set PB1 output 0

    DDRB |= 1<<DDB0; //set PB0 as output
    PORTB &= ~(1 << PB0); //set PB0 output 0


    //btn_debounce_init();

}

void hw_enable_timer1()
{
    TCNT1 = 0; //reset counter
    TCCR1 |= _BV(CS10)|_BV(CS12); // prescale 16
}

void hw_disable_timer1()
{
    TCCR1 ^= (_BV(CS10)|_BV(CS12)); // prescale 16
}

static inline void hw_set_timer1()
{

    //TIMER SETUP [leave this settings]
    //When the CTC1 control bit is set (one), Timer/Counter1 is reset to $00 in the CPU clock cycle after a compare match with OCR1C register value.
    //If the control bit is cleared,
    //Timer/Counter1 continues counting and is
    //unaffected by a compare match.

    TCCR1 |= _BV(CTC1); //clear timer on compare
    TIMSK |= _BV(OCIE1A); //activate compare interruppt A
    //TIMSK |= _BV(OCIE1B); //activate compare interruppt B

    TCNT1 = 0; //init count


    //TIMER PRESCALE FREQUENCY  [change this]
    //TCCR1 |= _BV(CS10); // prescale 1
    //TCCR1 |= _BV(CS11); // prescale 2
    TCCR1 |= _BV(CS10)|_BV(CS12); // prescale 16
    //TCCR1 |= _BV(CS11)|_BV(CS12); // prescale 32
    //TCCR1 |= _BV(CS10)|_BV(CS11)|_BV(CS12); // prescale 64
    //TCCR1 |= _BV(CS13); // prescale 128
    //TCCR1 |= _BV(CS10) | _BV(CS13); // prescale 256

    //When the CTC1 control bit is set (one), Timer/Counter1 is reset to $00 in the CPU clock cycle after a compare match with OCR1C register value
    OCR1C = 31; //(16000000/16)/31 = 32.258 Khz
    //OCR1B = 22;

    //OCR1C = 125;  // (16000000/16)/8000 = 125
    //OCR1C = 90;   // (16000000/16)/11025 = 90
    //OCR1C = 45;   // (16000000/16)/22050 = 45
    //OCR1C = 22;   // (16000000/16)/44100 = 22


}


static inline void hw_disable_timer0()
{
    //Writing the TSM bit to one activates the Timer/Counter Synchronization Mode. In this mode,
    //the value written to PSR0 is kept, hence keeping the Prescaler Reset signal asserted
    GTCCR |= _BV(TSM) | _BV(PSR0);
}

static inline void hw_enable_timer0()
{
    //When the TSM bit is written to zero, the PSR0 bit is cleared by hardware, and the timer/counter start counting.
    GTCCR &= ~_BV(TSM);
}

static inline void hw_setup_timer0_pwm()
{
    //PWM SOUND OUTPUT MODE [change this]
    TCCR0A |= (1<<WGM00)|(1<<WGM01); //Fast pwm
    //TCCR0A |= (1<<WGM00) ; //Phase correct pwm
    //PWM SETTINGS [leave this settings]
    //TCCR0A |= (1<<COM0A1); //Clear OC0A/OC0B on Compare Match when up-counting.
    //TCCR0B |= (1<<CS00);//no prescale

    //from ref website, this enable OCR0B
    TCCR0A = 2<<COM0A0 | 2<<COM0B0 | 3<<WGM00;
    TCCR0B = 0<<WGM02 | 1<<CS00;
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

