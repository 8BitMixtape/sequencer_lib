#ifndef SEQUENCER_LIB
#define SEQUENCER_LIB

//todo : in ms
typedef struct
{
    unsigned int duration;
    unsigned int freq;
} seq_sequence;

typedef struct
{
    volatile unsigned int tempo_time; //in ms
    volatile unsigned char next_step_index;
    volatile unsigned int  remaining_tempo_time;
    volatile unsigned int  remaining_sequence_time;
    volatile unsigned char sound_generator_on;

    seq_sequence * sequencer;
    unsigned char  sequencer_length;

} seq_instance;



void seq_set_current_step(seq_instance * seq_instance, int index)
{
    seq_instance->next_step_index  = index;
    seq_instance->remaining_sequence_time = seq_instance->sequencer[index].duration;
    seq_instance->remaining_tempo_time = seq_instance->tempo_time; // restart tempo //SEQUENCER[next_step_index].duration;
    seq_instance->sound_generator_on = 1;
    //printf("--new sequence\n");
}

void seq_set_tempo(seq_instance * seq_instance, unsigned int tempo)
{
    seq_instance->tempo_time = tempo;
}

void seq_update(seq_instance * seq_instance)
{

    //seq_sequence curr_seq = SEQUENCER[next_step_index];
    //printf("time %i sequence %i remaining_tempo_time %i remaining sequence time %i frequency %i\n", time, next_step_index, remaining_tempo_time, remaining_sequence_time, curr_seq.freq);

    if (seq_instance->remaining_tempo_time>0)
        {
            seq_instance->remaining_tempo_time--;

            if(seq_instance->remaining_sequence_time > 0)
                {
                    seq_instance->remaining_sequence_time--;
                }
            else if(seq_instance->sound_generator_on > 0)
                {
                    //printf("stop freq\n");
                    seq_instance->sound_generator_on = 0;
                }
        }
    else
        {

            if (seq_instance->remaining_sequence_time > 0)
                {
                    seq_instance->sound_generator_on = 0;
                    //printf("force stop freq");
                }

            seq_set_current_step(seq_instance, (seq_instance->next_step_index+1) % seq_instance->sequencer_length);
            seq_instance->remaining_tempo_time--;
        }

}

void seq_init(seq_instance * seq_instance, seq_sequence * seq_sequence_p, unsigned char  sequencer_length, unsigned int tempo)
{
    seq_instance->tempo_time = 0;
    seq_instance->next_step_index = 0;
    seq_instance->remaining_tempo_time = 0;
    seq_instance->remaining_sequence_time = 0;
    seq_instance->sound_generator_on = 0;
    seq_instance->sequencer = seq_sequence_p;
    seq_instance->sequencer_length = sequencer_length;

    seq_set_tempo(seq_instance, tempo);
    seq_set_current_step(seq_instance, 0);
}

#endif // SEQUENCER_LIB

