//
// Created by Flazher on 23.01.2018.
//

#include <exception>

class MidiParseException : public std::exception
{
public:
    MidiParseException(const char* err);

    virtual ~MidiParseException() throw ();

    virtual const char* what() const throw() {
        return "Error during MIDI parsing";
    }

protected:
    const char* member_err;

};