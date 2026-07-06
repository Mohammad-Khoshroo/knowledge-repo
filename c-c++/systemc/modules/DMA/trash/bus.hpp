
#ifndef SLAVE_MODULE_PORTBUNDLE_HPP
#define SLAVE_MODULE_PORTBUNDLE_HPP

#include "slave_module.hpp"

template <int A_WIDTH, int D_WIDTH>
struct sc_slave_portBundle {

    using addr_t = sc_lv<A_WIDTH>;
    using data_t = sc_lv<D_WIDTH>;


    sc_signal<addr_t> address;
    sc_signal<data_t> input;
    sc_signal<data_t> out;
    sc_signal<sc_logic> read;
    sc_signal<sc_logic> write;
    sc_signal<sc_logic> chipSelect;
    sc_signal<sc_logic> ready;

    sc_slave_portBundle() = default;

    void bind(
        sc_in<addr_t>& _address,
        sc_in<data_t>& _input,
        sc_out<data_t>& _out,
        sc_in<sc_logic>& _read,
        sc_in<sc_logic>& _write,
        sc_in<sc_logic>& _chipSelect,
        sc_out<sc_logic>& _ready
    ) {
        _address(address);
        _input(input);
        _out(out);
        _read(read);
        _write(write);
        _chipSelect(chipSelect);
        _ready(ready);
    }

};

#endif

#ifndef MASTER_MODULE_PORTBUNDLE_HPP
#define MASTER_MODULE_PORTBUNDLE_HPP

#include "master_module.hpp"

template <int A_WIDTH, int D_WIDTH>
struct sc_master_portBundle {

    using addr_t = sc_lv<A_WIDTH>;
    using data_t = sc_lv<D_WIDTH>;


    sc_signal<addr_t> address;
    sc_signal<data_t> input;
    sc_signal<data_t> out;

    sc_signal<sc_logic> write;
    sc_signal<sc_logic> read;
    sc_signal<sc_logic> ready;

    sc_master_portBundle() {}

    void bind(
        sc_out<addr_t>& _address,
        sc_in<data_t>& _input,
        sc_out<data_t>& _out,
        sc_out<sc_logic>& _write,
        sc_out<sc_logic>& _read,
        sc_in<sc_logic>& _ready
    ) {
        _address(address);
        _input(input);
        _out(out);
        _write(write);
        _read(read);
        _ready(ready);

    }
};

#endif



#ifndef MASTER_SLAVE_SHARED_BUS_HPP
#define MASTER_SLAVE_SHARED_BUS_HPP



// Helper class for RAII-style mutex locking
class sc_mutex_lock {
    sc_mutex& mtx;
public:
    explicit sc_mutex_lock(sc_mutex& m) : mtx(m) { mtx.lock(); }
    ~sc_mutex_lock() { mtx.unlock(); }
};

template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
class sc_bus : public sc_module {

    using addr_t = sc_lv<A_WIDTH>;
    using data_t = sc_lv<D_WIDTH>;


private:

    sc_trace_file* tf;
    sc_mutex bus_mutex;  // Main bus mutex
    int priorities[mastersNum];

    MemoryPartition* memoryPartitions[slavesNum];

    sc_signal<sc_logic> is_there_request;
    sc_signal<sc_logic> requests[mastersNum];

    sc_slave_portBundle<A_WIDTH, D_WIDTH> slavePortBundles[slavesNum];
    sc_master_portBundle<A_WIDTH, D_WIDTH> masterPortBundles[mastersNum];

    sc_signal<addr_t> targetAddress;
    sc_signal<addr_t> localTargetAddress;

    int currentSlave = -1;
    int currentMaster = -1;

    int availSlaveNum = 0;
    int availMastersNum = 0;

    void addMaster_trace(sc_master_portBundle<A_WIDTH, D_WIDTH>& bundle, sc_module_name name);
    void addSlave_trace(sc_slave_portBundle<A_WIDTH, D_WIDTH>& bundle, sc_module_name name);
    void checkRequest();
    void arbiter();
    void requestProcess();

    void setMasterPortBundlesZero();
    void setSlavePortBundlesZero();

    int selectMaster();
    int findTargetSlave(addr_t address);

public:

    sc_in<bool> clk;

    void addSlave(sc_slave_module<A_WIDTH, D_WIDTH>* slave);
    void addMaster(sc_master_module<A_WIDTH, D_WIDTH>* master);

    SC_HAS_PROCESS(sc_bus);
    sc_bus(sc_module_name name, sc_trace_file* tf, sc_clock& _clk) : sc_module(name), tf(tf), clk(_clk) {



        for (int i = 0; i < slavesNum; ++i) {
            memoryPartitions[i] = nullptr;
        }

        for (int i = 0; i < mastersNum; ++i) {
            priorities[i] = i;
        }

        SC_THREAD(requestProcess);
        for (int i = 0; i < mastersNum; ++i)
            sensitive << masterPortBundles[i].write << masterPortBundles[i].read;


        SC_THREAD(checkRequest);
        for (int i = 0; i < mastersNum; ++i)
            sensitive << requests[i];

        SC_THREAD(arbiter);
        sensitive << clk;
    }
};

template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
void sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::setMasterPortBundlesZero()
{
    for (int i = 0; i < mastersNum; i++)
    {
        masterPortBundles[i].input = 0;
        masterPortBundles[i].ready = SC_LOGIC_1;
    }
}


template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
void sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::setSlavePortBundlesZero()
{
    for (int i = 0; i < slavesNum; i++)
    {
        slavePortBundles[i].input = 0;
        slavePortBundles[i].read = SC_LOGIC_0;
        slavePortBundles[i].write = SC_LOGIC_0;
        slavePortBundles[i].address = 0;
        slavePortBundles[i].chipSelect = SC_LOGIC_0;
    }
}

template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
void sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::addMaster_trace(sc_master_portBundle<A_WIDTH, D_WIDTH>& bundle, sc_module_name name)
{
    std::string prefix = std::string(name) + "/" + this->name() + "bundle";

    sc_trace(tf, bundle.address, (prefix + "/address").c_str());
    sc_trace(tf, bundle.input, (prefix + "/input").c_str());
    sc_trace(tf, bundle.out, (prefix + "/out").c_str());
    sc_trace(tf, bundle.read, (prefix + "/read").c_str());
    sc_trace(tf, bundle.write, (prefix + "/write").c_str());
    sc_trace(tf, bundle.ready, (prefix + "/ready").c_str());

}

template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
void sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::addSlave_trace(sc_slave_portBundle<A_WIDTH, D_WIDTH>& bundle, sc_module_name name)
{
    std::string prefix = std::string(name) + "/" + this->name() + "bundle";

    sc_trace(tf, bundle.address, (prefix + "/address").c_str());
    sc_trace(tf, bundle.input, (prefix + "/input").c_str());
    sc_trace(tf, bundle.out, (prefix + "/out").c_str());
    sc_trace(tf, bundle.read, (prefix + "/read").c_str());
    sc_trace(tf, bundle.write, (prefix + "/write").c_str());
    sc_trace(tf, bundle.ready, (prefix + "/ready").c_str());
    sc_trace(tf, bundle.chipSelect, (prefix + "/chipSelect").c_str());

}

// Implementation of addSlave (with mutex protection)
template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
void sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::addSlave(
    sc_slave_module<A_WIDTH, D_WIDTH>* slave)
{
    if (availSlaveNum >= slavesNum) {
        SC_REPORT_ERROR("sc_bus", "Too many slaves added");
        return;
    }

    memoryPartitions[availSlaveNum] = &slave->memory;

    slavePortBundles[availSlaveNum].bind(
        slave->address,
        slave->input,
        slave->out,
        slave->read,
        slave->write,
        slave->chipSelect,
        slave->ready
    );

    addSlave_trace(slavePortBundles[availSlaveNum], slave->name());
    ++availSlaveNum;
}

// Implementation of addMaster (with mutex protection)
template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
void sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::addMaster(
    sc_master_module<A_WIDTH, D_WIDTH>* master)
{
    if (availMastersNum >= mastersNum) {
        SC_REPORT_ERROR("sc_bus", "Too many masters added");
        return;
    }

    masterPortBundles[availMastersNum].bind(
        master->address,
        master->input,
        master->out,
        master->write,
        master->read,
        master->ready
    );

    addMaster_trace(masterPortBundles[availMastersNum], master->name());

    ++availMastersNum;

}

template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
void sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::requestProcess()
{
    while (true)
    {
        wait();
        for (int i = 0; i < mastersNum; i++)
            requests[i] = masterPortBundles[i].read | masterPortBundles[i].write;
    }
}

template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
void sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::checkRequest()
{
    is_there_request = SC_LOGIC_0;
    while (1)
    {
        wait();
        sc_logic OR_requests = SC_LOGIC_0;

        for (int i = 0; i < mastersNum; i++)
            OR_requests = OR_requests | requests[i];

        is_there_request = OR_requests;
    }
}

// Optimized selectMaster with mutex
template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
int sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::selectMaster() {

    sc_mutex_lock lock(bus_mutex);

    for (int i = 0; i < mastersNum; i++) {

        // in this implementation,
        // each module append earlier to bus has a highest priority
        // in the constructor
        int priorMaster = priorities[i];

        if (requests[priorMaster] == SC_LOGIC_1) {

            // Move priorities[i] to the end, shifting all elements after it one step to the left
           //  from priorities[i], to priorities[i + 1], goes after priorities[M]
            std::rotate(priorities + i, priorities + i + 1, priorities + mastersNum);

            return priorMaster;
        }
    }
    SC_REPORT_ERROR("sc_bus", "No master has request!");
    return -1;
}

template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
int sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::findTargetSlave(addr_t address)
{
    for (int i = 0; i < slavesNum; i++)
        if (memoryPartitions[i]->is_cover(address))
        {
            localTargetAddress = memoryPartitions[i]->local(address);
            return i;
        }

    SC_REPORT_ERROR("sc_bus", "there is a unvalid address to access the modules!");
    return -1;
}


// Thread-safe arbiter
template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
void sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::arbiter() {
    while (true) {

        setMasterPortBundlesZero();
        setSlavePortBundlesZero();

        // Wait for request (no mutex needed here)
        do { wait(); } while (is_there_request.read() != SC_LOGIC_1);

        // Critical section
        {
            currentMaster = selectMaster();
            if (currentMaster == -1)
                SC_REPORT_FATAL("bus", "WTF :/");


            targetAddress = masterPortBundles[currentMaster].address.read();

            wait(SC_ZERO_TIME);

            currentSlave = findTargetSlave(targetAddress);

            if (currentSlave == -1)
                SC_REPORT_FATAL("bus", "WTF 2 :/");
        }

        // Data transfer (protected by slave's ready signal)
        masterPortBundles[currentMaster].ready = SC_LOGIC_0;

        slavePortBundles[currentSlave].address = localTargetAddress;
        slavePortBundles[currentSlave].chipSelect = SC_LOGIC_1;
        slavePortBundles[currentSlave].read = masterPortBundles[currentMaster].read;
        slavePortBundles[currentSlave].write = masterPortBundles[currentMaster].write;
        slavePortBundles[currentSlave].input = masterPortBundles[currentMaster].out;

        // Wait for slave
        do { wait(clk.posedge_event()); } while (slavePortBundles[currentSlave].ready != SC_LOGIC_1);

        // Cleanup
        slavePortBundles[currentSlave].chipSelect = SC_LOGIC_0;

        slavePortBundles[currentSlave].read = SC_LOGIC_0;
        slavePortBundles[currentSlave].write = SC_LOGIC_0;

        masterPortBundles[currentMaster].input = slavePortBundles[currentSlave].out;
        masterPortBundles[currentMaster].ready = SC_LOGIC_1;

        wait();

    }
}

#endif