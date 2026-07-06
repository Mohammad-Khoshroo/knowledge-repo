#ifndef MASTER_SLAVE_BUS_UTILS_HPP
#define MASTER_SLAVE_BUS_UTILS_HPP


#include <systemc.h>
#include <cstdint>


// Traits to extract width from sc_lv<W>
template <typename T>
struct is_sc_lv : std::false_type {};
// Special case: if T is sc_lv<W> for any int W, then it's TRUE
template <int W>
struct is_sc_lv<sc_lv<W>> : std::true_type {};

template <typename T>
struct is_sc_in : std::false_type {};

template <typename T>
struct is_sc_signal : std::false_type {};

template <typename T>
struct is_sc_in<sc_in<T>> : std::true_type {};

template <typename T>
struct is_sc_signal<sc_signal<T>> : std::true_type {};


// Memory Partition Settings
class MemoryPartition {
private:
    uint64_t startAddress = 0;
    uint64_t size = 0;
    uint64_t endAddress = 0;

    static uint64_t memoryTop;

public:
    MemoryPartition(int memoryStartAddr, int memorySize) {

        if (memorySize <= 0)
            SC_REPORT_FATAL("Memory Partition", "You should give a positive memory size.");

        if (memoryStartAddr != memoryTop)
            SC_REPORT_FATAL("Memory Partition", "You should start partition from first location which is empty.");

        startAddress = memoryStartAddr;
        size = memorySize;
        endAddress = startAddress + size;

        memoryTop = endAddress;
    }

    template<typename T>
    typename std::enable_if<std::is_integral<T>::value && !std::is_floating_point<T>::value && !std::is_same<T, bool>::value, bool>::type
        is_cover(T address) {
        if ((address < endAddress) && (address >= startAddress))
            return true;
        else
            return false;
    }

    template<typename T>
    typename std::enable_if<is_sc_lv<T>::value, bool>::type
        is_cover(T _address) {
        uint64_t address = _address.to_uint64();
        if ((address < endAddress) && (address >= startAddress))
            return true;
        else
            return false;
    }

    template<typename T>
    typename std::enable_if<(is_sc_signal<T>::value) || (is_sc_in<T>::value), bool>::type
        is_cover(T _address) {
        uint64_t address = _address.read().to_uint64();
        if ((address < endAddress) && (address >= startAddress))
            return true;
        else
            return false;
    }

    template<typename T>
    typename std::enable_if<std::is_integral<T>::value && !std::is_floating_point<T>::value && !std::is_same<T, bool>::value, T>::type
        local(T address) {
        return address - startAddress;
    }

    template<typename T>
    typename std::enable_if<is_sc_lv<T>::value, T>::type
        local(T _address) {
        uint64_t address = _address.to_uint64();
        return address - startAddress;
    }

    template<typename T>
    typename std::enable_if<(is_sc_signal<T>::value) || (is_sc_in<T>::value), bool>::type
        local(T _address) {
        uint64_t address = _address.read().to_uint64();
        return address - startAddress;
    }


    static uint64_t top() { return memoryTop; }
};

uint64_t MemoryPartition::memoryTop = 0;

#endif // MASTER_SLAVE_BUS_UTILS_HPP