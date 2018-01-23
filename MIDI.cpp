//
// Created by flazher on 23.01.18.
//

#include <iostream>
#include <fstream>
#include <cstring>

#include "MIDI.h"
#include "MidiParseException.h"

MIDI::MIDI(const char *filename) {
    ifstream midi_file;
    midi_file.open(filename, ios::binary);

    if (!midi_file)
        throw MidiParseException("Error during file reading");

    process_header_chunk(read_chunk(midi_file));
    for(int i = 0; i < track_chunks_count; i++) {
        process_track_chunk(read_chunk(midi_file));
    }
}

MIDI* MIDI::read(const char *filename) {
    return new MIDI(filename);
}

void MIDI::process_header_chunk(chunk* header_chunk) {
    if (strcmp(header_chunk->type, "MThd") == 0) {
        header_chunk_data* header_data = (header_chunk_data*) header_chunk->data;

        midi_type = two_chars_to_int(header_data->format);
        track_chunks_count = two_chars_to_int(header_data->tracks);

        bool first_division_bit = header_data->division[0] >> 7 != 0;
        if (!first_division_bit) {
            delta_per_quarter = ((header_data->division[0]) << 8);
        } else
            throw MidiParseException("Not implemented yet, working with PPRQ only");

        if (midi_type != 0)
            throw MidiParseException("Currently supporting MIDI type-0 only");

    } else
        throw MidiParseException("First chunk isn't a header chunk");
}

void MIDI::process_track_chunk(chunk* track_chunk) {
    int chunk_offset = 0;

    if (strcmp(track_chunk->type, "MTrk") != 0)
        return;

    bool end_of_track = false;

    while (!end_of_track && chunk_offset <= four_chars_to_int(track_chunk->length)) {
        midi_event* event = (midi_event*)malloc(sizeof(midi_event));

        event->delta_time = get_delta_time(&track_chunk->data[chunk_offset], &chunk_offset);
        event->status = track_chunk->data[chunk_offset];
        event->data = &track_chunk->data[chunk_offset + 1];

        if (event->status >= 0x80 && event->status <= 0xE0) {
            chunk_offset += (event->status >= 0xC0 && event->status <= 0xDF) ? 2 : 3;
        } else {
            switch (track_chunk->data[chunk_offset]) {
                case 0xFF: {
                    if (track_chunk->data[chunk_offset + 1] == 0x2F) {
                        end_of_track = true;
                        break;
                    }
                    chunk_offset += track_chunk->data[chunk_offset + 2] + 3;
                    break;
                }
                case 0xF0:
                case 0xF7: {
                    chunk_offset += track_chunk->data[chunk_offset + 1] + 2;
                    break;
                }
                default: throw MidiParseException("Unrecognized event type!");
            }
        }
        this->events.push_back(event);
    }
}

chunk* MIDI::read_chunk(ifstream& stream) {
    chunk* current_chunk = (chunk*)malloc(sizeof(chunk*));
    stream.read((char *)current_chunk, 8);
    unsigned int length_to_read = four_chars_to_int(current_chunk->length);
    current_chunk->data = (byte*)malloc(length_to_read);
    stream.read((char *)current_chunk->data, length_to_read);
    return current_chunk;
}

unsigned int MIDI::get_delta_time(const byte* first_byte, int* carriage) {
    unsigned int delta = 0;
    int offset = 0;
    for (;;) {
        delta <<= 7;
        delta |= *(first_byte + offset) & 0x7f;
        bool first_bit = *(first_byte + offset) >> 7 != 0;
        offset++;
        if (!first_bit)
            break;
    }
    *carriage += offset;
    return delta;
}