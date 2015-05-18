# 8BitMixtapeSequencer

**Sequencer version of 8BitMixtape and example of sequencer_lib**



**test**



`test`



``` c
//
//                               +-\/-+
//  Ain0           (D  5)  PB5  1|    |8   VCC
//  BUTTON_2  ===> (D  3)  PB3  2|    |7   PB2  (D  2)  INT0  Ain1 ====> pot2
// pot1 ===> Ain2  (D  4)  PB4  3|    |6   PB1  (D  1)   <=== BUTTON_1
//                         GND  4|    |5   PB0  (D  0)        pwm0 ====> OCR0A /speaker
//                               +----+
```

sdsad





![stepseq](https://raw.githubusercontent.com/8BitMixtape/sequencer_lib/master/examples/8BitMixtapeDJ/breadboard_bb.png)



------

## Pin Configuration

    // ATTINY85 8BitMixtapeDj pin

    //

    //                               +-\/-+

    //  Ain0           (D  5)  PB5  1|    |8   VCC

    //  BUTTON_2  ===> (D  3)  PB3  2|    |7   PB2  (D  2)  INT0  Ain1 ====> pot2

    // pot1 ===> Ain2  (D  4)  PB4  3|    |6   PB1  (D  1)   <=== BUTTON_1

    //                         GND  4|    |5   PB0  (D  0)        pwm0 ====> OCR0A / speaker

    //                               +----+

------