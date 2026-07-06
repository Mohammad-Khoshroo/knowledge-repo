#include "memory.hpp"

int sc_main(int argc, char *argv[])
{
    std::vector<int> dump; // dummy
    Memory<sc_dt::sc_lv<8>, 8> mem("mem", "memory.csv", "", dump);

    // Just call readCSV again if needed for debugging:
    // csv::CSVReader reader("memory.csv");
    // std::vector<sc_dt::sc_lv<8>> temp(8);
    // mem.readCSV(reader, temp);

    return 0;
}
