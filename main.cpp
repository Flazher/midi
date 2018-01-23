#include <iostream>
#include <fstream>
#include <cstring>
#include "MIDI.h"
#include "util.h"

using namespace std;

typedef unsigned char byte;

int main(int argc, char* argv[])
{
    MIDI* midi = MIDI::read("D:\\Projects\\test.mid");
    for (midi_event* event : midi->events) {
        if (event->status == 0x80) {
            cout << event-> delta_time << " " << get_note(event->data[0]) << endl;
        }
    }
    cin.ignore();
	return 0;
}