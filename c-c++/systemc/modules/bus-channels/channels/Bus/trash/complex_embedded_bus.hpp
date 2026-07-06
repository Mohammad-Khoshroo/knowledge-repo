#pragma once
#include "slave_module.hpp"
#include "master_module.hpp"

// A == address 
// D == Data 
template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
class sc_bus : public sc_module, public PortTypes {
private:

    int priorities[mastersNum];

    MemoryPartition* memoryPartitions[slavesNum];

    sc_signal<sc_logic> is_there_request;
    sc_signal<sc_logic> requests[mastersNum];

    sc_slave_portBundle<A_WIDTH, D_WIDTH> slavePortBundles[slavesNum];
    sc_master_portBundle<A_WIDTH, D_WIDTH> masterPortBundles[mastersNum];

    // global Address which partition between modules
    addr_t targetAddress;
    // local Address in internal slave bus 
    addr_t localTargetAddress;
    
    int currentSlave = -1;
    int currentMaster = -1;
    
    int availSlaveNum = 0;
    int availMastersNum = 0;
    
    
    void checkRequest();
    void arbiter();
    void requestProcess();
    
    void setMasterPortBundlesZero();
    void setSlavePortBundlesZero();
    
    int selectMaster();
    int findTargetSlave(typename sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::addr_t address);
    
    
    public:
    
    sc_in<sc_logic> clk;
    sc_in<sc_logic> rst;
    
    void addSlave(sc_slave_module<A_WIDTH, D_WIDTH>* slave);
    void addMaster(sc_master_module<A_WIDTH, D_WIDTH>* master);
    
    SC_HAS_PROCESS(sc_bus);
    sc_bus(sc_module_name name, sc_trace_file* tf) : sc_module(name) {
        
        for (int i = 0; i < mastersNum; ++i)
            priorities[i] = i;
            
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
void sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::addSlave(sc_slave_module<A_WIDTH, D_WIDTH>* slave) {

    if (availSlaveNum >= slavesNum) {
        SC_REPORT_ERROR("sc_bus", "Too many slaves added");
        return;
    }

    memoryPartitions[availSlaveNum] = slave->memory;

    slavePortBundles[availSlaveNum] = new sc_slave_portBundle<A_WIDTH, D_WIDTH>(
        slave->address,
        slave->input,
        slave->out,
        slave->read,
        slave->write,
        slave->cheapSelect,
        slave->ready
    );

    ++availSlaveNum;
}


template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
void sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::addMaster(sc_master_module< A_WIDTH, D_WIDTH>* master) {

    if (availMastersNum >= mastersNum) {
        SC_REPORT_ERROR("sc_bus", "Too many masters added");
        return;
    }

    masterPortBundles[availMastersNum] = new sc_master_portBundle<A_WIDTH, D_WIDTH>(
        master->address,
        master->input,
        master->out,
        master->write,
        master->read,
        master->ready
    );

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

        for (int i = 0; i < M; i++)
            OR_requests = OR_requests | requests[i];

        is_there_request = OR_requests;
    }
}

template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
void sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::setMasterPortBundlesZero()
{
    for (int i = 0; i < mastersNum; i++)
    {
        masterPortBundles[i].input.write(0);
        masterPortBundles[i].ready.write(SC_LOGIC_1);

    }
}


template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
void sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::setSlavePortBundlesZero()
{
    for (int i = 0; i < slavesNum; i++)
    {
        slavePortBundles[i].input.write(0);
        slavePortBundles[i].read.write(SC_LOGIC_0);
        slavePortBundles[i].write.write(SC_LOGIC_0);
        slavePortBundles[i].address.write(0);
        slavePortBundles[i].cheapSelect.write(SC_LOGIC_0);

    }
}


template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
int sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::selectMaster()
{
    for (int i = 0; i < mastersNum; i++)
    {
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
}

template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
int sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::findTargetSlave(typename sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::addr_t address)
{
    for (int i = 0; i < slavesNum; i++)
        if (memoryPartitions[i].is_cover(address)) {
            return i;
        }
    SC_REPORT_ERROR("sc_bus", "there is a unvalid address to access the modules!");
}



template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
void sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::arbiter()
{
    while (true)
    {
        setMasterPortBundlesZero();
        setSlavePortBundlesZero();

        // Wait for a request
        do { wait(); } while (is_there_request.read() != SC_LOGIC_1);

        // Find the requesting master with the most priority
        currentMaster = selectMaster();

        // Save accessing address
        targetAddress = masterPortBundles[currentMaster].address;

        // Find the slave that master is addressing
        currentSlave = findTargetSlave(targetAddress);
        localTargetAddress = targetAddress - memoryPartitions[currentSlave].startAddress;

        ////////////////////////////////////////////////////////

        // communication between master and Slave

        masterPortBundles[currentMaster].ready = SC_LOGIC_0;

        slavePortBundles[currentSlave].address = localTargetAddress;
        slavePortBundles[currentSlave].cheapSelect = SC_LOGIC_1;
        slavePortBundles[currentSlave].read = masterPortBundles[currentMaster].read;
        slavePortBundles[currentSlave].write = masterPortBundles[currentMaster].write;
        slavePortBundles[currentSlave].input = masterPortBundles[currentMaster].out;

        // wait for slave to done its job
        do { wait(clk.posedge_event()); } while (slavePortBundles[currentSlave].ready != SC_LOGIC_1);

        slavePortBundles[currentSlave].cheapSelect = SC_LOGIC_0;

        slavePortBundles[currentMaster].input = masterPortBundles[currentSlave].out;
        masterPortBundles[currentMaster].ready = SC_LOGIC_0;
    }
}