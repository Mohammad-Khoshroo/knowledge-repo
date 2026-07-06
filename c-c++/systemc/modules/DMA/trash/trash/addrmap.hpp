#ifndef MASTER_SLAVE_BUS_ADDRESS_MAPPING_HPP
#define MASTER_SLAVE_BUS_ADDRESS_MAPPING_HPP

#include <systemc.h>
#include <cstdint>

// Special case: if T is sc_lv<W> for any int W, then it's TRUE
template <typename T>
struct is_sc_lv : std::false_type {};
template <int W>
struct is_sc_lv<sc_lv<W>> : std::true_type {};

template <typename T>
struct is_sc_in : std::false_type {};
template <typename T>
struct is_sc_in<sc_in<T>> : std::true_type {};

template <typename T>
struct is_sc_signal : std::false_type {};
template <typename T>
struct is_sc_signal<sc_signal<T>> : std::true_type {};


// Address Mapping Settings 
class AddressMap {
private:
    uint64_t startAddr = 0;
    uint64_t size = 0;
    uint64_t endAddr = 0;

    static uint64_t addrTop;
    static uint64_t moduleID;

public:

    AddressMap(int memorySize) {

        if (memorySize <= 0)
            SC_REPORT_FATAL(("Address Map Constructor" + std::to_string(moduleID)).c_str(), "You should give a positive memory size.");

        size = memorySize;
        startAddr = addrTop;

        endAddr = startAddr + size;

        addrTop = endAddr;
    }

    template<typename T>
    typename std::enable_if<std::is_integral<T>::value && !std::is_floating_point<T>::value && !std::is_same<T, bool>::value, bool>::type
        is_cover(T address) {
        if ((address < endAddr) && (address >= startAddr))
            return true;
        else
            return false;
    }

    template<typename T>
    typename std::enable_if<is_sc_lv<T>::value, bool>::type
        is_cover(T _address) {
        uint64_t address = _address.to_uint64();
        if ((address < endAddr) && (address >= startAddr))
            return true;
        else
            return false;
    }

    template<typename T>
    typename std::enable_if<(is_sc_signal<T>::value) || (is_sc_in<T>::value), bool>::type
        is_cover(T _address) {
        uint64_t address = _address.read().to_uint64();
        if ((address < endAddr) && (address >= startAddr))
            return true;
        else
            return false;
    }


    template<typename T>
    typename std::enable_if<std::is_integral<T>::value && !std::is_floating_point<T>::value && !std::is_same<T, bool>::value, T>::type
        local(T address) {
        return address - startAddr;
    }

    template<typename T>
    typename std::enable_if<is_sc_lv<T>::value, T>::type
        local(T _address) {
        uint64_t address = _address.to_uint64();
        return address - startAddr;
    }

    template<typename T>
    typename std::enable_if<(is_sc_signal<T>::value) || (is_sc_in<T>::value), bool>::type
        local(T _address) {
        uint64_t address = _address.read().to_uint64();
        return address - startAddr;
    }


    static uint64_t top() { return addrTop; }
};

uint64_t AddressMap::addrTop = 0;
uint64_t AddressMap::moduleID = 0;



#endif //MASTER_SLAVE_BUS_ADDRESS_MAPPING_HPP