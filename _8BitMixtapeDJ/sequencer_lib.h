#ifndef SEQUENCER_LIB
#define SEQUENCER_LIB


//todo : in ms
typedef struct
{
    unsigned int duration;
    unsigned char freq;
    unsigned char last;
} seq_sequence;

#define SEQUENCE_LENGTH 4

seq_sequence SEQUENCER[SEQUENCE_LENGTH] =
{
    //sustain, freq, last?
    { 5000, 100, 0},
    { 5000, 200, 0},
    { 5000, 100, 0},
    { 5000, 200, 1},
};

volatile unsigned int tempo_time = 0; //in ms
volatile unsigned char next_step_index = 0;

volatile unsigned int  remaining_tempo_time = 0;
volatile unsigned int  remaining_sequence_time = 0;

volatile unsigned char sound_generator_on = 0;


void seq_set_current_step(int index)
{
    next_step_index  = index;
    remaining_sequence_time = SEQUENCER[index].duration;
    remaining_tempo_time = tempo_time; // restart tempo //SEQUENCER[next_step_index].duration;
    sound_generator_on = 1;
    //printf("--new sequence\n");
}

void seq_set_tempo(unsigned int tempo)
{
    tempo_time = tempo;
}

void seq_update_state()
{

    //seq_sequence curr_seq = SEQUENCER[next_step_index];
    //printf("time %i sequence %i remaining_tempo_time %i remaining sequence time %i frequency %i\n", time, next_step_index, remaining_tempo_time, remaining_sequence_time, curr_seq.freq);

    if (remaining_tempo_time>0)
        {
            remaining_tempo_time--;

            if(remaining_sequence_time > 0)
                {
                    remaining_sequence_time--;
                }else if(sound_generator_on){
                    //printf("stop freq\n");
                    sound_generator_on = 0;
                }
        }
    else
        {

            if (remaining_sequence_time > 0)
            {
                sound_generator_on = 0;
                //printf("force stop freq");
            }

            seq_set_current_step((next_step_index+1) % SEQUENCE_LENGTH);
            remaining_tempo_time--;
        }

}


#endif // SEQUENCER_LIB

