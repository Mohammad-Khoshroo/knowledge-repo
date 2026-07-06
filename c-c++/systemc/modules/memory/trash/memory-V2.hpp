#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <cmath>
#include <systemc.h>
#include <sstream>
#include <cstring>
#include <cstdint>
#include <csv.hpp>
#include <sc_cast.hpp>

using namespace std;

template <int WIDTH>
class Memory : public sc_module
{
private:
    enum class State
    {
        IDLE,
        INIT,
        READ,
        DUMP,
        ISSUE_READY,
        WRITE,
    };

    int memoryCapacity; // how many address we can set on memory

    // Register of Memory
    // Instructions and Data
    sc_lv<WIDTH> *memory;
    sc_lv<WIDTH> *instructions;
    sc_lv<WIDTH> *data;

    vector<int> &dumpTimes; // times to dump memory

    // Read File Stream
    // Load Files
    csv::CSVReader instructionsIFile; // Instruction
    csv::CSVReader dataIFile;         // Data
    csv::CSVReader memoryIFile;       // Both

    // Write File Stream
    // Save Files
    ofstream instructionsOFile; // Instruction
    ofstream dataOFile;         // Data
    ofstream memoryOFile;       // Both
    ofstream dumpOFile;

    // Trace variables
    bool traceInit, traceRead, traceDump, traceIssueReady, traceWrite;

    /*Functions*/
    /////////////////////////////////////////////////////////////////////////

    void readCSV(csv::CSVReader &fileStream, sc_lv<WIDTH> *targetMemory)
    {
        for (csv::CSVRow &row : fileStream)
        {
            uint64_t addr = sc_cast::string_cast<uint64_t>(row["address"].get(), 16);

            if (addr < 0 || addr >= memoryCapacity)
                SC_REPORT_WARNING("Memory", "Address out of bounds in CSV");

            if (row["data"].get().length())
                SC_REPORT_WARNING("Memory", "Data field is empty in CSV");
            targetMemory[addr] = sc_cast::sc_lv_cast<sc_lv<WIDTH>>(row["data"].get(), 2);
        }
    }

    void writeCSV(std::ofstream &outFile, T *memoryArray)
    {
        outFile << "num,address,data\n";
        for (int i = 0; i < memoryCapacity; ++i)
        {
            stringstream hexAddress;
            hexAddress << std::hex << i; // << std::hex sets hex format
            outFile << i << "," << hexAddress.str() << "," << memoryArray[i].read() << "\n";
        }
    }

    // when you read from Memory one Data
    void readMem();
    // when you write on Memory one Data
    void writeMem();
    // when you save all Memory n dumpFile
    void dump();
    // issue ready signal for handShaking
    void setMemReady();
    // trace activities of memory
    void trace();
    // checking conflict of read and write
    void checkConflict();

public:
    sc_in<sc_logic> clk;

    // Select Module
    // somehow a start signal
    sc_in<sc_logic> chipSelect;

    // Select Mode
    sc_in<sc_logic> write;
    sc_in<sc_logic> read;

    // Signals for writing Data or Instruction on memory OR reading from it
    sc_in<sc_lv<WIDTH>> address;
    sc_in<sc_lv<WIDTH>> input;
    sc_out<sc_lv<WIDTH>> output;

    // ready signal
    sc_out<sc_logic> ready;

    // Trace signal
    sc_out<sc_lv<3>> traceState;

    SC_HAS_PROCESS(Memory);
    Memory(sc_module_name name, string dataPath, string instPath, string memPath, string dumpPath, vector<int> &dumpTimes)
        : sc_module(name),
          memoryCapacity(1 << WIDTH),
          instructionsIFile(instPath),
          dataIFile(dataPath),
          memoryIFile(memPath),
          currentState(State::INIT)
    {
        traceInit = 1;
        traceRead = 0;
        traceDump = 0;
        traceIssueReady = 0;
        traceWrite = 0;

        memory = new T[memoryCapacity];
        instructions = new T[memoryCapacity];
        Data = new T[memoryCapacity];

        // Output files
        instructionsOFile.open(instPath, std::ios::out);
        dataOFile.open(dataPath, std::ios::out);
        memoryOFile.open(memPath, std::ios::out);
        dumpOFile.open(dumpPath, std::ios::out | std::ios::app);

        readCSV(instructionsIFile, instructions);
        readCSV(dataIFile, Data);
        readCSV(memoryIFile, memory);

        currentState = State::IDLE;

        traceInit = 0;

        SC_METHOD(checkConflict);
        sensitive << read << write;

        // Asynchronous read
        SC_THREAD(readMem);
        sensitive << address << chipSelect << read;

        // synchronous write
        SC_THREAD(writeMem);
        sensitive << clk;

        SC_THREAD(setMemReady);
        sensitive << address << chipSelect << read << write;

        SC_THREAD(dump);

        SC_METHOD(trace);
        sensitive << traceInit << traceRead << traceDump << traceIssueReady << traceWrite;
    }

    ~Memory()
    {
        delete[] memory;
        delete[] instructions;
        delete[] Data;

        instructionsOFile.close();
        dataOFile.close();
        memoryOFile.close();
        dumpOFile.close();
    }
};

template <int WIDTH>
void Memory<WIDTH>::readMem()
{
    if ((chipSelect.read() == SC_LOGIC_1) && (read.read() == SC_LOGIC_1))
    {
        traceRead = 1;
        int addr = address.read().to_uint();

        if (addr < 0 || addr >= memoryCapacity)
            SC_REPORT_FATAL("Memory", "Address out of bounds");

        else
            output.write(memory[addr]);

        wait(SC_ZERO_TIME);
        wait(SC_ZERO_TIME);

        traceRead = 0;
    }
}

template <int WIDTH>
void Memory<WIDTH>::writeMem()
{
    while (true)
    {
        wait();

        if ((chipSelect.read() == SC_LOGIC_1) && (write.read() == SC_LOGIC_1))
        {
            traceWrite = 1;
            int addr = address.read().to_uint();

            if (addr < 0 || addr >= memoryCapacity)
                SC_REPORT_FATAL("Memory", "Address out of bounds");
            else
                memory[addr] = static_cast<T>(input.read().to_int());

            traceWrite = 0;
        }
    }
}

template <int WIDTH>
void Memory<WIDTH>::dump()
{
    if (!dumpTimes.empty())
    {

        sort(dumpTimes.begin(), dumpTimes.end());

        for (double t : dumpTimes)
        {
            sc_time target = sc_time(t, SC_NS);
            sc_time now = sc_time_stamp();

            sc_time wait_time = target - sc_time_stamp();
            if (wait_time > SC_ZERO_TIME)
                wait(wait_time);

            traceDump = 1;

            dumpOFile << "Dump at " << sc_time_stamp().to_default_time_units() << " seconds:\n";
            writeCSV(dumpOFile, memory);
            dumpOFile << "\n";

            wait(SC_ZERO_TIME);

            traceDump = 0;
        }
    }
}

template <int WIDTH>
void Memory<WIDTH>::setMemReady()
{
    ready.write(SC_LOGIC_0);

    if ((chipSelect.read() == SC_LOGIC_1) && ((write.read() == SC_LOGIC_1) || (read.read() == SC_LOGIC_1)))
    {
        traceIssueReady = 1;

        int addr = address.read().to_int();

        if (addr < 0 || addr >= memoryCapacity)
            SC_REPORT_FATAL("Memory", "Address out of bounds");
        else
            ready.write(SC_LOGIC_1);

        wait(SC_ZERO_TIME);
        wait(SC_ZERO_TIME);

        traceIssueReady = 0;
    }
}

template <int WIDTH>
void Memory<WIDTH>::trace()
{
    if (!traceInit && !traceRead && !traceDump && !traceIssueReady && !traceWrite)
        traceState.write(static_cast<sc_lv<3>>(static_cast<int>(State::IDLE)));

    else if (traceInit)
        traceState.write(static_cast<sc_lv<3>>(static_cast<int>(State::INIT)));

    else if (traceRead)
        traceState.write(static_cast<sc_lv<3>>(static_cast<int>(State::READ)));
    else if (traceWrite)
        traceState.write(static_cast<sc_lv<3>>(static_cast<int>(State::WRITE)));

    if (traceIssueReady)
        traceState.write(static_cast<sc_lv<3>>(static_cast<int>(State::ISSUE_READY)));

    if (traceDump)
        traceState.write(static_cast<sc_lv<3>>(static_cast<int>(State::DUMP)));

    wait(SC_ZERO_TIME);
    wait(SC_ZERO_TIME);
}

template <int WIDTH>
void Memory<WIDTH>::checkConflict()
{
    if ((read.read() == SC_LOGIC_1) &&
        (write.read() == SC_LOGIC_1))
        SC_REPORT_FATAL("Memory", "Conflict read and write signals.Both are High Impedance.");
}