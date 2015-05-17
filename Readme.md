sequencer_lib
===================


Generic step sequencer library for AVR/C based synthesizer

![stepseq](https://raw.githubusercontent.com/8BitMixtape/8BitMixtapeDJ/seqlib/step_sequencer.png)

----------

Api
-------------

    typedef struct
    {
        unsigned int sustain;
        unsigned int freq;  //add more variable here
    } seq_stage;

    seq_stage * seq_get_current_stage(seq_instance * seq_instance)

    void seq_goto_stage(seq_instance * seq_instance, int index)

    void seq_set_tempo(seq_instance * seq_instance, unsigned int tempo)

    void seq_update_time(seq_instance * seq_instance)

    void seq_init(seq_instance * seq_instance, seq_stage * seq_sequence_p, unsigned char  sequencer_length, unsigned int tempo)


Example Usage
-------------
Sequencer running on 8 Khz timer

**1.Declare sequencer instance variable**

    seq_instance SEQUENCER_1;
    
**2.Declare sequencer stage length and array (max length 255)**
    
    #define   SEQUENCER1_STAGE_LENGTH 8
    
    seq_stage SEQUENCER_1_STAGE_ARRAY[SEQUENCER1_STAGE_LENGTH] =
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

**3. Call init sequencer**

    seq_init(&SEQUENCER_1, SEQUENCER_1_STAGE_ARRAY, SEQUENCER1_STAGE_LENGTH, 10000);

**4. Play Sound and Update Sequencer time**

    ISR(TIMER1_COMPA_vect) //e.g. 8 Khz Timer
    {
		//play with tempo (map 0-255 to 0-16320)
    	seq_set_tempo(&SEQUENCER_1,pot2_val << 6);
    
        if(SEQUENCER_1.sound_generator_on > 0)
            {
		        uint8_t freq = seq_get_current_stage(&SEQUENCER_1)->freq;
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


