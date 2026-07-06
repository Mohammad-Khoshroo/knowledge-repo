#ifndef MASTER_SLAVE_BUS_ADDRESS_MAPPING_HPP
#define MASTER_SLAVE_BUS_ADDRESS_MAPPING_HPP

#include <systemc.h>
#include <cstdint>
#include <type_traits>
#include <string>
#include "include/sc_cast/sc_cast.hpp"

using namespace sc_cast;
using namespace sc_dt;

class sc_address_space {
private:
    uint64_t addrTop = 0;
    uint16_t moduleID = 0;

public:

    sc_address_space() = default;

    void allocate(uint64_t size) {
        addrTop += size;
        moduleID++;
    }

    uint64_t top() const { return addrTop; }
    uint16_t id() const { return moduleID; }

};


// Address Mapping Setting Class
class AddressMap {
private:
    uint64_t startAddr = 0;
    uint64_t _size = 0;
    uint64_t endAddr = 0;

public:
    explicit AddressMap(uint64_t memorySize, sc_address_space* space) {

        if (memorySize <= 0) {
            std::string errMsg = "You should give a positive memory size.";
            std::string ctx = "AddressMap Constructor (space id " + std::to_string(space->id()) + ")";
            SC_REPORT_FATAL(ctx.c_str(), errMsg.c_str());
        }

        _size = memorySize;
        startAddr = space->top();
        space->allocate(memorySize);
        endAddr = startAddr + _size;
    }

    // -------------------------
    // Address Coverage Check
    // -------------------------

    // Case 1: Integral types
    template<typename T>
    typename std::enable_if<
        std::is_integral<T>::value &&
        !std::is_floating_point<T>::value &&
        !std::is_same<T, bool>::value,
        bool >::type
        is_cover(T address) const {
        return (address >= startAddr && address < endAddr);
    }

    // Case 2: sc_lv<W>
    template<typename T>
    typename std::enable_if<is_sc_lv<T>::value, bool>::type
        is_cover(T& _address) const {
        uint64_t address = _address.to_uint64();
        return (address >= startAddr && address < endAddr);
    }

    // Case 3: sc_in<T> or sc_signal<T>
    template<typename T>
    typename std::enable_if<
        is_sc_in<T>::value || is_sc_signal<T>::value,
        bool >::type
        is_cover(T& _address) const {
        uint64_t address = _address.read().to_uint64();
        return (address >= startAddr && address < endAddr);
    }

    // -------------------------
    // Local Address Translation
    // -------------------------

    // Case 1: Integral types
    template<typename T>
    typename std::enable_if<
        std::is_integral<T>::value &&
        !std::is_floating_point<T>::value &&
        !std::is_same<T, bool>::value,
        uint64_t>::type
        local(T address) const {
        return address - startAddr;
    }

    // Case 2: sc_lv<W>
    template<typename T>
    typename std::enable_if<is_sc_lv<T>::value, uint64_t>::type
        local(T& _address) const {
        uint64_t address = _address.to_uint64();
        return address - startAddr;
    }

    // Case 3: sc_in<T> or sc_signal<T>
    template<typename T>
    typename std::enable_if<
        is_sc_in<T>::value || is_sc_signal<T>::value,
        uint64_t>::type
        local(T& _address) const {
        uint64_t address = _address.read().to_uint64();
        return address - startAddr;
    }

    uint64_t size() { return _size; }
};

#endif // MASTER_SLAVE_BUS_ADDRESS_MAPPING_HPP
