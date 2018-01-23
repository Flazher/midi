//
// Created by flazher on 23.01.18.
//

#include <iostream>
#include <fstream>
#include <cstring>

#include "midi.h"

midi::midi(char* filename) {
    ifstream midi_file;
    midi_file.open(filename, ios::binary);

    if (!midi_file)
        die("Error during file reading");

    process_header_chunk(read_chunk(midi_file));
    for(int i = 0; i < track_chunks_count; i++) {
        process_track_chunk(read_chunk(midi_file));
    }
}

midi* midi::read(char* filename) {
    midi* instance = new midi(filename);
    ifstream midi_file;
    midi_file.open(filename, ios::binary);

    if (!midi_file)
        die("Error during file reading");

    int track_chunks_count = 0;
    int midi_type = 0;
    int delta_time_per_quarter = 0;

    // Header chunk processing
    chunk* header_chunk = read_chunk(midi_file);
    if (strcmp(header_chunk->type, "MThd") == 0) {
        // It's a header chunk
        cout << "Header chunk type is " << header_chunk->type << ", length of data = " << four_chars_to_int(
                header_chunk->length) << endl;
        header_chunk_data* header_data = (header_chunk_data*) header_chunk->data;

        midi_type = two_chars_to_int(header_data->format);
        track_chunks_count = two_chars_to_int(header_data->tracks);

        int firstDivisionBit = header_data->division[0] >> 7 && 0x1;
        if (firstDivisionBit == 0) {
            delta_time_per_quarter = ((header_data->division[0]) << 8);
        } else
            die("I didn't expect that!");

        // В исследовательских целях пока перевариваем только type-0 MIDI-файлы
        if (midi_type != 0)
            die("I expected MIDI file type 0!");

        instance->delta_per_quarter = delta_time_per_quarter;

        cout << "MIDI file type=" << midi_type << ", number of tracks=" << track_chunks_count << endl;
        cout << "Delta time per quarter note=" << delta_time_per_quarter << endl;
    } else
        // Header chunk type isn't MThd. It's not a header chunk then.
        die("First chunk isn't a header chunk! Something is terribly wrong!");

    chunk* track_chunk;

    // Track chunks processing
    for(int i = 0; i < track_chunks_count; i++) {
        track_chunk = read_chunk(midi_file);
        cout << "Reading Track chunk" << endl;

        // Проверяем, является ли чанк track-чанком
        if (strcmp(track_chunk->type, "MTrk") != 0) {
            cout << "Chunk is not a track-chunk, ignoring" << endl;
            continue;
        }

        bool end_of_track = false;
        int chunk_offset = 0;

        midi_event* event;
        while (!end_of_track && chunk_offset <= four_chars_to_int(track_chunk->length)) {
            unsigned int delta = 0;

            // Подсчет delta-time
            for (;;) {
                delta <<= 7;
                delta |= track_chunk->data[chunk_offset] & 0x7f;
                int firstBit = track_chunk->data[chunk_offset] >> 7 && 0x1;
                chunk_offset++;
                if (firstBit == 0)
                    break;
            }

            byte sign_byte = (byte)track_chunk->data[chunk_offset];

            if (sign_byte >= 0x80 && sign_byte <= 0xE0) {
                byte ttt = track_chunk->data[chunk_offset];
                if ((byte)(sign_byte & 0xf0) == 0x80)
                    cout << "[" << delta << "]" << "NOTE OFF" << endl;

                if ((byte)(sign_byte & 0xf0) == 0x90)
                    cout << "[" << delta << "]" << "NOTE ON" << endl;

                chunk_offset += (sign_byte >= 0xC0 && sign_byte <= 0xDF) ? 2 : 3;
            } else {
                switch ((byte)track_chunk->data[chunk_offset]) {
                    case 0xFF: {
                        if ((byte)track_chunk->data[chunk_offset + 1] == 0x2F) {
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
                    default: die("Unrecognized event type!");
                }
            }
        }
    }
    return instance;
}

void midi::process_header_chunk(chunk* header_chunk) {
    if (strcmp(header_chunk->type, "MThd") == 0) {
        // It's a header chunk
        cout << "Header chunk type is " << header_chunk->type << ", length of data = " << four_chars_to_int(
                header_chunk->length) << endl;
        header_chunk_data* header_data = (header_chunk_data*) header_chunk->data;

        midi_type = two_chars_to_int(header_data->format);
        track_chunks_count = two_chars_to_int(header_data->tracks);

        int firstDivisionBit = header_data->division[0] >> 7 && 0x1;
        if (firstDivisionBit == 0) {
            delta_per_quarter = ((header_data->division[0]) << 8);
        } else
            die("I didn't expect that!");

        // В исследовательских целях пока перевариваем только type-0 MIDI-файлы
        if (midi_type != 0)
            die("I expected MIDI file type 0!");

        cout << "MIDI file type=" << midi_type << ", number of tracks=" << track_chunks_count << endl;
        cout << "Delta time per quarter note=" << delta_per_quarter << endl;
    } else
        // Header chunk type isn't MThd. It's not a header chunk then.
        die("First chunk isn't a header chunk! Something is terribly wrong!");
}

void process_track_chunk(chunk* track_chunk) {
    //midi_event** events = (midi_event**)malloc(sizeof(midi_event**));

    int chunk_offset = 0;

    if (strcmp(track_chunk->type, "MTrk") != 0) {
        cout << "Chunk is not a track-chunk, ignoring" << endl;
        return;
    }

    bool end_of_track = false;

    while (!end_of_track && chunk_offset <= four_chars_to_int(track_chunk->length)) {
        unsigned int delta = 0;
        midi_event* event = (midi_event*)malloc(sizeof(midi_event*));

        // Подсчет delta-time
        for (;;) {
            delta <<= 7;
            delta |= track_chunk->data[chunk_offset] & 0x7f;
            int first_bit = track_chunk->data[chunk_offset] >> 7 && 0x1;
            chunk_offset++;
            if (first_bit == 0)
                break;
        }

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
                default: die("Unrecognized event type!");
            }
        }
    }
}

chunk* midi::read_chunk(ifstream& stream) {
    chunk* current_chunk = (chunk*)malloc(sizeof(chunk*));
    stream.read((char *)current_chunk, 8);
    unsigned int length_to_read = four_chars_to_int(current_chunk->length);
    current_chunk->data = (byte*)malloc(length_to_read);
    stream.read((char *)current_chunk->data, length_to_read);
    return current_chunk;
}