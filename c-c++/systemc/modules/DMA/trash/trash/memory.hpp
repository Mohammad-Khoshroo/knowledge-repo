#pragma once
#include "csv.hpp"
#include "sc_cast.hpp"

using namespace std;

template <typename T, int WIDTH>
class Memory : public sc_module
{
private:
    enum class State
    {
        INIT = 0,
        READ = 1,
        WRITE = 2,
        ISSUE_READY = 3,
        DUMP = 4,
        IDLE = 7,
    };

    int memoryCapacity; // how many address we can set on memory

    // Register of Memory
    // Instructions and Data
    std::vector<T> memory;

    std::vector<int> &dumpTimes; // times to dump memory

    // Write File Stream
    // Save Files
    std::ofstream memoryOFile;
    std::string memPath; // Path to memory file
    std::ofstream dumpOFile;
    std::string dumpPath; // Path to dump file

    // Trace variables
    sc_signal<bool> traceInit, traceRead, traceDump, traceIssueReady, traceWrite;

    /*Functions*/
    /////////////////////////////////////////////////////////////////////////

    void readCSV(csv::CSVReader &fileStream, std::vector<T> &targetMemory, bool fullMemory = true)
    {
        // cout << "HI" << endl;
        uint64_t i = 0;
        std::string emptyData(WIDTH, 'X'); // Default value for uninitialized memory
        T val;
        // cout << type_name(val) << endl;
        for (csv::CSVRow &row : fileStream)
        {
            // cout << "start" << endl;
            std::string addr_str(row["address"].get_sv());
            uint64_t addr = sc_cast::string_cast<uint64_t>(addr_str, "address", 16);
            // cout << addr << endl;
            if (addr >= memoryCapacity)
            {
                SC_REPORT_WARNING("Memory", "Address out of bounds in CSV");
                continue; // Skip this entry
            }
            std::string data_str(row["data"].get_sv());
            // cout << data_str << endl;
            if (data_str.empty())
            {
                SC_REPORT_WARNING("Memory", "Data field is empty in CSV!!!! Filling with default value X");
                if (sc_cast::is_sc_lv<T>::value)
                    val = sc_cast::string_cast<T>(emptyData, "data", 2);
                else
                    val = static_cast<T>(0);
                // val = sc_cast::string_cast<T>(emptyData); // Fill with default value
                // continue;
            }
            else
            {
                val = sc_cast::string_cast<T>(data_str, "data", 2);
            }
            // cout << val << endl;
            targetMemory[addr] = val;
            i++;
        }
        cout << "read data Done" << endl;
        if ((i != memoryCapacity) && fullMemory)
        {
            std::string msg = "CSV file does not match memory capacity. Expected: " + to_string(memoryCapacity) + ", Found: " + to_string(i);
            SC_REPORT_WARNING("Memory", msg.c_str());
            for (size_t j = i; j < memoryCapacity; j++)
            {
                val = sc_cast::string_cast<T>(emptyData, "data", 2); // Initialize remaining memory to zero
                targetMemory[j] = val;
            }
        }
    }

    void writeCSV(std::ofstream &outFile, std::vector<T> &memoryArray)
    {
        outFile << "address,data\n";
        for (size_t i = 0; i < memoryArray.size(); ++i)
        {
            stringstream hexAddress;
            hexAddress << std::hex << std::uppercase << i; // << std::hex sets hex format
            std::string dataStr = memoryArray[i].to_string();
            if (dataStr.find('X') != std::string::npos || dataStr.find('x') != std::string::npos)
                dataStr = std::string(WIDTH, 'X');
            outFile << "0X"<<hexAddress.str() << "," << dataStr << "\n";
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
    Memory(sc_module_name name, string memPath_, string dumpPath_, vector<int> &dumpTimes_)
        : sc_module(name),
          memoryCapacity(1ULL << WIDTH), // 2^WIDTH
          memPath(memPath_),
          dumpPath(dumpPath_),
          dumpTimes(dumpTimes_)
    {
        cout << "Memory instance created with capacity: " << memoryCapacity << endl;
        traceInit.write(1);
        traceRead.write(0);
        traceDump.write(0);
        traceIssueReady.write(0);
        traceWrite.write(0);

        cout << "Memory instance created with path: " << memPath << endl;
        if (memPath.empty())
            SC_REPORT_FATAL("Memory", "File path cannot be empty");

        cout << "Memory instance created with dump path: " << dumpPath << endl;
        ///////////////////////////////////////////////////
        // Check if file exists
        std::fstream iFile(memPath, std::ios::in);
        size_t memLastSizeUsed = 0;
        if (!iFile.is_open())
            SC_REPORT_FATAL("Memory", "Failed to open memory input file");
        std::string line;
        // Skip the header line
        std::getline(iFile, line);
        // Count remaining data lines
        while (std::getline(iFile, line))
            if (!line.empty())
                ++memLastSizeUsed;
        iFile.close();
        ////////////////////////////////////////////////////
        cout << "Memory last size used: " << memLastSizeUsed << endl;
        // Resize memory
        memory.resize(memLastSizeUsed);

        cout << "memory resized: " << memory.size() << endl;
        csv::CSVReader memoryIFile(memPath.c_str());

        cout << "csv file opened!" << endl;
        readCSV(memoryIFile, memory, false);
        cout << "csv file closed!" << endl;

        if (!dumpPath.empty())
            dumpOFile.open(dumpPath, std::ios::out | std::ios::app);
        else
            SC_REPORT_WARNING("Memory", "Dump file path is empty");

        sort(dumpTimes.begin(), dumpTimes.end());
        if (!dumpTimes.empty())
            SC_THREAD(dump);

        traceInit.write(0);

        SC_METHOD(checkConflict);
        sensitive << read << write;

        // Asynchronous read
        SC_METHOD(readMem);
        sensitive << address << chipSelect << read;

        // synchronous write
        SC_THREAD(writeMem);
        sensitive << clk;

        SC_THREAD(setMemReady);
        sensitive << address << chipSelect << read << write;

        SC_METHOD(trace);
        sensitive << traceInit << traceRead << traceDump << traceIssueReady << traceWrite;
    }

    ~Memory()
    {
        // Output files
        memoryOFile.open(memPath, std::ios::out);
        if (memoryOFile.is_open())
            writeCSV(memoryOFile, memory);
        else
            SC_REPORT_FATAL("Memory", "Failed to open memory output file");

        memoryOFile.close();
        if (dumpOFile.is_open())
            dumpOFile.close();
    }
};

template <typename T, int WIDTH>
void Memory<T, WIDTH>::readMem()
{
    if ((chipSelect.read() == SC_LOGIC_1) && (read.read() == SC_LOGIC_1))
    {
        traceRead.write(1);
        sc_lv<WIDTH> address_lv = address.read();
        uint64_t addr = sc_cast::sc_lv_cast<uint64_t>(address_lv);
        if (addr >= memoryCapacity)
        {
            SC_REPORT_ERROR("Memory", "Read address out of bounds");
            std::string x = std::string(WIDTH, 'X');
            output.write(x.c_str());
            traceRead.write(0);
            return; // Skip this read
        }
        else if (addr < memory.size())
        {
            output.write(sc_cast::sc_lv_cast<sc_lv<WIDTH>>(memory[addr]));
        }
        else
        {
            SC_REPORT_WARNING("Memory", "Read address exceeds memory size, returning X");
            std::string x = std::string(WIDTH, 'X');
            output.write(x.c_str());
        }
        traceRead.write(0);
    }
}

template <typename T, int WIDTH>
void Memory<T, WIDTH>::writeMem()
{
    while (true)
    {

        if ((chipSelect.read() == SC_LOGIC_1) && (write.read() == SC_LOGIC_1))
        {
            traceWrite.write(1);

            sc_lv<WIDTH> address_lv = address.read();
            uint64_t addr = sc_cast::sc_lv_cast<uint64_t>(address_lv);
            if (addr >= memoryCapacity || addr >= memory.size())
            {
                SC_REPORT_WARNING("Memory", "Write address invalid");
                wait(); // avoid tight loop busy wait
                continue;
            }

            sc_lv<WIDTH> input_lv = input.read();
            uint64_t input = sc_cast::sc_lv_cast<uint64_t>(input_lv);
            memory[addr] = sc_cast::sc_lv_cast<T>(input);

            traceWrite.write(0);
        }

        wait();
    }
}

template <typename T, int WIDTH>
void Memory<T, WIDTH>::dump()
{
    for (double t : dumpTimes)
    {
        sc_time target = sc_time(t, SC_NS);

        sc_time now = sc_time_stamp();
        sc_time wait_time = target - now;
        if (wait_time > SC_ZERO_TIME)
            wait(wait_time);

        traceDump.write(1);

        dumpOFile << "Dump at " << sc_time_stamp().to_string() << " seconds:\n";
        writeCSV(dumpOFile, memory);
        dumpOFile << "\n";

        wait(SC_ZERO_TIME);

        traceDump.write(0);
    }
}

template <typename T, int WIDTH>
void Memory<T, WIDTH>::setMemReady()
{
    while (true)
    {
        ready.write(SC_LOGIC_0);

        if ((chipSelect.read() == SC_LOGIC_1) && ((write.read() == SC_LOGIC_1) || (read.read() == SC_LOGIC_1)))
        {
            traceIssueReady.write(1);

            sc_lv<WIDTH> address_lv = address.read();
            uint64_t addr = sc_cast::sc_lv_cast<uint64_t>(address_lv);
            if (addr < memoryCapacity)
                ready.write(SC_LOGIC_1);

            wait(SC_ZERO_TIME);
            wait(SC_ZERO_TIME);

            traceIssueReady.write(0);
        }
        wait(); // wait for sensitivity signals (address, chipSelect, read, write)
    }
}

template <typename T, int WIDTH>
void Memory<T, WIDTH>::trace()
{
    if (!traceInit.read() && !traceRead.read() && !traceDump.read() && !traceIssueReady.read() && !traceWrite.read())
        traceState.write(static_cast<int>(State::IDLE));

    else if (traceInit.read())
        traceState.write(static_cast<int>(State::INIT));

    else if (traceRead.read())
        traceState.write(static_cast<int>(State::READ));
    else if (traceWrite.read())
        traceState.write(static_cast<int>(State::WRITE));

    else if (traceIssueReady.read())
        traceState.write(static_cast<int>(State::ISSUE_READY));

    else if (traceDump.read())
        traceState.write(static_cast<int>(State::DUMP));
}

template <typename T, int WIDTH>
void Memory<T, WIDTH>::checkConflict()
{
    if ((read.read() == SC_LOGIC_1) &&
        (write.read() == SC_LOGIC_1))
        SC_REPORT_FATAL("Memory", "Conflict: both read and write signals are asserted simultaneously.");
}