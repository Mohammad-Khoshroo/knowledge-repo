#include "head.hpp"

// Memory Partition Settings
class MemoryPartition {
private:
    uint64_t startAddress = 0;
    uint64_t size = 0;
    uint64_t endAddress = 0;

public:
    MemoryPartition(int memoryStartAddr, int memorySize) {
        
        startAddress = memoryStartAddr;
        size = memorySize;
        endAddress = startAddress + size;
    }

    bool is_cover(int address) {
        if ((address < endAddress) && (address >= startAddress))
            return true;
        else
            return false;
    }
};

template <int address_WIDTH, int data_WIDTH>
struct PortTypes {
    using addr_t = sc_lv<address_WIDTH>;
    using data_t = sc_lv<data_WIDTH>;
};