#include <iostream>
#include <fstream>
#include <cstring>
#include "midi.h"
#include "util.h"

using namespace std;

typedef unsigned char byte;

int main(int argc, char* argv[])
{
    midi* m = midi::read("/home/flazher/test.mid");
    cout << m->delta_per_quarter;
    cin.ignore();
	return 0;
}