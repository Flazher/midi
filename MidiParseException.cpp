//
// Created by Flazher on 24.01.2018.
//

#include "MidiParseException.h"

MidiParseException::MidiParseException(const char* err) {
    this->member_err = err;
}

MidiParseException::~MidiParseException() throw () { }