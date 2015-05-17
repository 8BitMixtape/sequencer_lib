#ifndef SEQUENCER_LIB
#define SEQUENCER_LIB


#ifndef CUSTOM_PATTERN_STRUCT

typedef struct
{
    unsigned int sustain;
    unsigned int freq;
} seq_pattern;

#endif

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
{
    return &seq_instance->sequencer_pattern_array[seq_instance->current_pattern_index];
}

void seq_goto_pattern(seq_instance * seq_instance, int index)
{
    seq_instance->current_pattern_index  = index;
    seq_instance->remaining_sustain_time = seq_instance->sequencer_pattern_array[index].sustain;
    seq_instance->remaining_tempo_time = seq_instance->tempo_time; // restart tempo //SEQUENCER[next_step_index].duration;
    //seq_instance->sound_generator_on = 1;
    //printf("--new sequence\n");
}

void seq_set_tempo(seq_instance * seq_instance, unsigned int tempo)
{
    seq_instance->tempo_time = tempo;
}

void seq_update_time(seq_instance * seq_instance)
{

    //seq_sequence curr_seq = SEQUENCER[next_step_index];
    //printf("time %i sequence %i remaining_tempo_time %i remaining sequence time %i frequency %i\n", time, next_step_index, remaining_tempo_time, remaining_sequence_time, curr_seq.freq);

    if (seq_instance->remaining_tempo_time>0)
        {
            seq_instance->remaining_tempo_time--;

            if(seq_instance->remaining_sustain_time > 0)
                {
                    seq_instance->sound_generator_on = 1;
                    seq_instance->remaining_sustain_time--;
                }
            else
                {
                    //sustain time ended
                    seq_instance->sound_generator_on = 0;
                }
        }
    else
        {
            //if sustain > tempo force sound off
            seq_instance->sound_generator_on = 0;

            //end of stage, set next stage
            seq_goto_pattern(seq_instance, (seq_instance->current_pattern_index+1) % seq_instance->sequencer_pattern_array_length);
        }

}

void seq_init(seq_instance * seq_instance, seq_pattern * seq_sequence_p, unsigned char  sequencer_length, unsigned int tempo)
{
    seq_instance->tempo_time = 0;
    seq_instance->current_pattern_index = 0;
    seq_instance->remaining_tempo_time = 0;
    seq_instance->remaining_sustain_time = 0;
    seq_instance->sound_generator_on = 0;
    seq_instance->sequencer_pattern_array = seq_sequence_p;
    seq_instance->sequencer_pattern_array_length = sequencer_length;

    seq_set_tempo(seq_instance, tempo);
    seq_goto_pattern(seq_instance, 0);
}

#endif // SEQUENCER_LIB

