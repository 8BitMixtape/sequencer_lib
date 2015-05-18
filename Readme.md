# sequencer_lib

Generic step sequencer library for AVR/C based synthesizer

![stepseq](https://raw.githubusercontent.com/8BitMixtape/8BitMixtapeDJ/seqlib/step_sequencer.png)

------

## Api 

``` c
typedef struct
{
  unsigned int sustain;
  unsigned int freq;
} seq_pattern;

typedef struct
{
  volatile unsigned char current_pattern_index;
  volatile unsigned char sound_generator_on;
  volatile unsigned int  tempo_time; //in tick
  volatile unsigned int  remaining_tempo_time;
  volatile unsigned int  remaining_sustain_time;
  seq_pattern *     sequencer_pattern_array;
  unsigned char   sequencer_pattern_array_length;
} seq_instance;

seq_pattern * seq_get_current_pattern(seq_instance * seq_instance)

void seq_goto_pattern(seq_instance * seq_instance, int index)

void seq_set_tempo(seq_instance * seq_instance, unsigned int tempo)

void seq_update_time(seq_instance * seq_instance)

void seq_init(seq_instance * seq_instance, seq_pattern * seq_sequence_p, unsigned char  sequencer_length, unsigned int tempo)
```

## Installation

1. Download repo
2. Extract the zip file
3. Move the "sequencer_lib" folder that has been extracted to your Arduino libraries directory.


## Example Usage

Sequencer running on 8 Khz timer

**1.include sequencer lib**

``` c
include <sequencer_lib.h>
```

**2.Declare sequencer instance variable**

``` c
seq_instance SEQUENCER_1;
```

**3.Declare sequencer stage length and array (max length 255)**

``` c
define   SEQUENCER1_PATTERN_LENGTH 8
  
seq_pattern SEQUENCER1PATTERN_ARRAY[SEQUENCER1_PATTERN_LENGTH] =
    {
        //sustain, freq, stage index
        { 6000, 200}, //1
        { 6000, 200}, //2
        { 6000, 200}, //3
        { 6000, 200}, //4
        { 6000, 200}, //5
        { 6000, 200}, //6
        { 6000, 200}, //7
        { 6000, 200}, //8
    };
```

**4. Call init sequencer**

``` c
seq_init(&SEQUENCER1, SEQUENCER1PATTERN_ARRAY, SEQUENCER1PATTERN_LENGTH, 10000);
```

**5. Play Sound and Update Sequencer time**

``` c
ISR(TIMER1_COMPA_vect) //e.g. 8 Khz Timer
{
    //play with tempo (map 0-255 to 0-16320)
    seq_set_tempo(&SEQUENCER1,pot2val << 6);
    if(SEQUENCER_1.sound_generator_on > 0)
        {
            uint8t freq = seq_get_current_pattern(&SEQUENCER1)->freq;
            snd = t * ((10>>12|t>>8)&freq&t>>4);
            OCR0A = snd;
            t++;
        }
    else
        {
            OCR0A = 0;
            t=0;
        }
    }

 seq_update_time(&SEQUENCER_1);
```

## Extra

Customize pattern data struct, put this before including sequencer_lib

``` c
#define CUSTOM_PATTERN_STRUCT

typedef struct
    {
        unsigned int sustain;
        unsigned int freq;
        unsigned int freq1;
        unsigned int freq2;
    } seq_pattern;

include <sequencer_lib.h>
```