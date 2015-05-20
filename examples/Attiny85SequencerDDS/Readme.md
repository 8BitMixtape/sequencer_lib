# Attiny85 DDS Pwm

Port from http://interface.khm.de/index.php/lab/interfaces-advanced/arduino-dds-sinewave-generator/  to Attiny85


![stepseq](https://raw.githubusercontent.com/8BitMixtape/sequencer_lib/master/examples/8BitMixtapeDJ/breadboard_bb.png)



## Install

4. Install Arduino-tiny core from https://code.google.com/p/arduino-tiny or https://github.com/TCWORLD/ATTinyCore
   
5. From Arduino Ide choose Tools > Boards > **ATtiny85 @ 16Mhz (internal PLL; 4.3 V BOD)**
   
6. Tools > Burn Bootloader
   
7. Upload Sketch
   
   ​






## Pin Configuration

``` c
// ATTINY85 _Attiny85Timer0DDSPwm pin
//
//                               +-\/-+
//  Ain0           (D  5)  PB5  1|    |8   VCC
//  Ain3           (D  3)  PB3  2|    |7   PB2  (D  2)  INT0  Ain1 ====> pot2
// pot1 ===> Ain2  (D  4)  PB4  3|    |6   PB1  (D  1)        pwm1
//                         GND  4|    |5   PB0  (D  0)        pwm0 ====> OCR0A / sound output
//                      +----+
```

------