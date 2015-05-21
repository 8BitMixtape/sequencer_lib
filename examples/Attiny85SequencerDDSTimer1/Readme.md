# Attiny85 Sequencer DDS Timer1

Port from http://interface.khm.de/index.php/lab/interfaces-advanced/arduino-dds-sinewave-generator/  to Attiny85 + sequencer_lib 



> Using Attiny85 Timer1 as PWM with 65 Mhz carrier frequency http://www.technoblogy.com/show?QBB



![stepseq](https://raw.githubusercontent.com/8BitMixtape/sequencer_lib/master/examples/Attiny85SequencerDDSTimer1/breadboard_bb.png)



## Install

4. Install Arduino-tiny core from https://code.google.com/p/arduino-tiny or https://github.com/TCWORLD/ATTinyCore
   
5. From Arduino Ide choose Tools > Boards > **ATtiny85 @ 16Mhz (internal PLL; 4.3 V BOD)**
   
6. Tools > Burn Bootloader
   
7. Upload Sketch
   
   ​


## Pin Configuration

``` c
// ATTINY85 pin
//                               +-\/-+
//                         PB5  1|    |8   VCC
//                         PB3  2|    |7   PB2  (D  2) pot2 (bitcrusher)
//          pot1 (tempo)   PB4  3|    |6   PB1  (D  1) pwm0 ====> OCR1A / sound output        
//                         GND  4|    |5   PB0  (D  0) 
//                               +----+
```

------