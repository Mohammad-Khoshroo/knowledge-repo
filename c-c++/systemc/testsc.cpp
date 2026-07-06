#include <systemc.h>
#include <iostream>

int sc_main(int argc, char* argv[]) {
    std::cout << sc_version() << std::endl;
    return 0;
}


// FOR NOW IT'S HOW TO COMPILE ON WSL (MY MACHINE)
// SYSTEMC_HOME=/usr/local/systemc g++ -std=c++20 -g -O0 \
//   -I$SYSTEMC_HOME/include \
//   -L$SYSTEMC_HOME/lib \
//   -Wl,-rpath,$SYSTEMC_HOME/lib \
//   testsc.cpp -lsystemc -lm -lpthread -o testsc
