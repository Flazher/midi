//
// Created by flazher on 23.01.18.
//

#include <iostream>
#include <cstring>
#include "util.h"

using namespace std;

char* notes [] = {"C", "C#", "D", "D#",	"E", "F", "F#", "G", "G#", "A", "A#", "B"};

void die(char* msg) {
    cout << msg << endl;
    cin.ignore();
    exit(-1);
}

unsigned int two_chars_to_int(byte *src) {
    return src[1] | src[0] << 8;
}

unsigned int four_chars_to_int(byte *src) {
    return src[3] | src[2]<<8 | src[1]<<16 | src[0]<<24;
}

char* get_note(byte code) {
    char* representation = (char*)malloc(3);
    strcpy(representation, notes[code % 12]);
    sprintf(representation, "%s%i", representation, code / 12);
    return representation;
}