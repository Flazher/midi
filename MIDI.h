//
// Created by flazher on 23.01.18.
//

#include <cstdlib>
#include <list>
#include "util.h"

struct chunk {
    char type[4];
    byte length[4];
    byte *data;
};

struct header_chunk_data {
    byte format[2];
    byte tracks[2];
    char division[2];
};

struct midi_event {
    unsigned int delta_time;
    byte status;
    byte* data;
};

class MIDI
{
public:
    int delta_per_quarter;
    int midi_type;
    int track_chunks_count;
    std::list<midi_event*> events;

    static MIDI* read(const char *filename);
    MIDI(const char *filename);
private:
    static chunk* read_chunk(ifstream& stream);
    void process_header_chunk(chunk* chunk);
    void process_track_chunk(chunk* chunk);
    unsigned int get_delta_time(const byte* first_byte, int* carriage);
};