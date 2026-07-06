#ifndef MASTER_SLAVE_SHARED_BUS_HPP
#define MASTER_SLAVE_SHARED_BUS_HPP

#include "master_portBundle.hpp"
#include "slave_portBundle.hpp"

using namespace sc_dt;

#define MAX_ADDR (std::string(64, '1')).c_str()

// Helper class for RAII-style mutex locking
class sc_mutex_lock {
    sc_mutex& mtx;
public:
    explicit sc_mutex_lock(sc_mutex& m) : mtx(m) { mtx.lock(); }
    ~sc_mutex_lock() { mtx.unlock(); }
};


class sc_bus_identifier {
    static int busID;
public:
    sc_bus_identifier() = default;
    int gen() { return ++busID; } //generate

};


int sc_bus_identifier::busID = -1;


template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
class sc_bus : public sc_bus_identifier, public sc_module {

    using addr_t = sc_lv<A_WIDTH>;
    using data_t = sc_lv<D_WIDTH>;

private:
    uint16_t availSlaveNum = 0;
    uint16_t availMastersNum = 0;

    sc_mutex bus_mutex;  // Main bus mutex
    int priorities[mastersNum];

    AddressMap* addressMaps[slavesNum];

    sc_signal<sc_logic> is_there_request;
    sc_signal<sc_logic> requests[mastersNum];

    sc_slave_portBundle<A_WIDTH, D_WIDTH> slavePortBundles[slavesNum];
    sc_master_portBundle<A_WIDTH, D_WIDTH> masterPortBundles[mastersNum];

    sc_signal<addr_t> targetAddr;
    sc_signal<addr_t> mappedAddr;

    int selectedSlave = -1;
    int selectedMaster = -1;

    void checkRequest();

    void arbiter();

    void initialize(sc_trace_file* tf);

    void fetchMaster();
    void fetchSlave();

    int myID;

public:

    sc_in<bool> clk;

    void addSlave(sc_slave_module<A_WIDTH, D_WIDTH>* slave, sc_trace_file* tf);
    void addMaster(sc_master_module<A_WIDTH, D_WIDTH>* master, sc_trace_file* tf);

    SC_HAS_PROCESS(sc_bus);
    sc_bus(sc_module_name name, sc_trace_file* tf) :sc_bus_identifier(), sc_module(name) {

        initialize(tf);
        myID = sc_bus_identifier::gen();

        SC_THREAD(checkRequest);
        for (int i = 0; i < mastersNum; i++)
            sensitive << masterPortBundles[i].write << masterPortBundles[i].read;

        SC_THREAD(arbiter);
        sensitive << clk.pos();
    }
};



template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
void sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::initialize(sc_trace_file* tf)
{
    for (int i = 0; i < slavesNum; i++)
        addressMaps[i] = nullptr;


    for (int i = 0; i < mastersNum; i++)
    {
        priorities[i] = i;
        requests[i] = SC_LOGIC_0;
        sc_trace(tf, requests[i], (std::string(this->name()) + "_requests" + "(" + std::to_string(i) + ")").c_str());
    }

    is_there_request = SC_LOGIC_0;
    targetAddr = MAX_ADDR;
    mappedAddr = MAX_ADDR;


    // sc_trace(tf, targetAddr, (std::string(this->name()) + "_targetAddr").c_str());
    // sc_trace(tf, mappedAddr, (std::string(this->name()) + "_mappedAddr").c_str());
    sc_trace(tf, is_there_request, (std::string(this->name()) + "_request?").c_str());
}


template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
void sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::addSlave(
    sc_slave_module<A_WIDTH, D_WIDTH>* slave,
    sc_trace_file* tf
) {
    if (availSlaveNum >= slavesNum) {

        std::string msg = "Too many slaves added";
        std::string ctx = "sc_bus " + std::to_string(myID) + "/" + std::string(slave->name());
        SC_REPORT_FATAL(ctx.c_str(), msg.c_str());
        return;

    }

    addressMaps[availSlaveNum] = slave->memory;

    slavePortBundles[availSlaveNum].bind(
        slave->address,
        slave->input,
        slave->out,
        slave->read,
        slave->write,
        slave->chipSelect,
        slave->ready
    );

    std::string prefix = std::string(this->name()) + "/" + std::string(slave->name()) + "_bundle";
    slavePortBundles[availSlaveNum].trace(tf, prefix);

    slavePortBundles[availSlaveNum].initialize();

    availSlaveNum++;
}

template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
void sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::addMaster(
    sc_master_module<A_WIDTH, D_WIDTH>* master,
    sc_trace_file* tf
) {
    if (availMastersNum >= mastersNum) {
        std::string msg = "Too many masters added";
        std::string ctx = "sc_bus " + std::to_string(myID) + "/" + std::string(master->name());
        SC_REPORT_FATAL(ctx.c_str(), msg.c_str());
        return;
    }

    masterPortBundles[availMastersNum].bind(
        master->address,
        master->out,
        master->input,
        master->write,
        master->read,
        master->ready
    );

    std::string prefix = std::string(this->name()) + "/" + std::string(master->name()) + "_bundle";
    masterPortBundles[availMastersNum].trace(tf, prefix);


    masterPortBundles[availMastersNum].initialize();

    availMastersNum++;

}

template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
void sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::checkRequest()
{
    bool OR_requests = false;

    while (true)
    {
        is_there_request = SC_LOGIC_0;

        cout << "request check bus " << myID << " : " << endl;

        for (int i = 0; i < mastersNum; i++) {

            bool readBool = (masterPortBundles[i].read.read() == SC_LOGIC_1);
            bool writeBool = (masterPortBundles[i].write.read() == SC_LOGIC_1);
            bool orBool = (readBool || writeBool);
            requests[i] = orBool ? SC_LOGIC_1 : SC_LOGIC_0;
            cout << "master" << i << " read: " << readBool << ", write: " << writeBool << " request? " << orBool << endl;
            OR_requests = (OR_requests || orBool);
        }

        is_there_request = OR_requests ? SC_LOGIC_1 : SC_LOGIC_0;

        wait(SC_ZERO_TIME);



        if (is_there_request == SC_LOGIC_1) {
            std::string msg = "request received";
            std::string ctx = "sc_bus " + std::to_string(myID);
            SC_REPORT_INFO(ctx.c_str(), msg.c_str());
        }

        wait();

        OR_requests = false;
    }
}


template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
void sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::fetchMaster() {

    std::string ctx = "sc_bus " + std::to_string(myID);

    bool successful = false;
    sc_mutex_lock lock(bus_mutex);

    for (int i = 0; i < mastersNum; i++) {

        // In this implementation, masters that are connected earlier to the bus
        // (i.e., with lower indices in the priorities array) are given higher priority.
        // The priority list is initialized in the constructor and updated dynamically
        // after each successful arbitration.
        int priorMaster = priorities[i];

        if ((priorMaster < 0) || (priorMaster >= mastersNum)) {
            std::string msg = "invalid masterId(" + std::to_string(priorMaster) + ") in priority " + std::to_string(i);
            SC_REPORT_FATAL(ctx.c_str(), msg.c_str());
        }

        if (requests[priorMaster] == SC_LOGIC_1) {

            // Move priorities[i] to the end of the array by shifting all elements
            // after it one position to the left. This effectively rotates the priority
            // list so that the granted master gets the lowest priority in the next round.
            std::rotate(priorities + i, priorities + i + 1, priorities + mastersNum);
            selectedMaster = priorMaster;
            successful = true;
            break;
        }
    }

    if (!successful)
        SC_REPORT_FATAL(ctx.c_str(), "No master has request!");
    else
    {
        std::string msg = "Master successfully fetched" + std::string(" Master ") + std::to_string(selectedMaster);
        SC_REPORT_INFO(ctx.c_str(), msg.c_str());
    }
}

template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
void sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::fetchSlave()
{
    bool successful = false;

    std::string ctx = "sc_bus " + std::to_string(myID);
    // fetch address
    uint64_t _targetAddr = masterPortBundles[selectedMaster].address.read().to_uint64();
    cout << "targetAddr in bus " << myID << ": " << _targetAddr << endl;

    // Decode address and find the corresponding slave
    for (int i = 0; i < slavesNum; i++) {
        if (addressMaps[i]->is_cover(_targetAddr))
        {
            targetAddr = _targetAddr;

            uint64_t _mappedAddr = addressMaps[i]->local(_targetAddr);
            mappedAddr = _mappedAddr;

            cout << "mappedAddr in bus " << myID << ": " << _mappedAddr << endl;
            selectedSlave = i;
            successful = true;
            break;
        }
    }

    if (!successful) {
        std::string msg = "there is an invalid address to access the modules from master " + std::to_string(selectedMaster);
        SC_REPORT_FATAL(ctx.c_str(), msg.c_str());
    }
    else {
        wait(SC_ZERO_TIME);
        std::string msg = "Slave successfully fetched" + std::string(" Slave ") + std::to_string(selectedSlave);
        SC_REPORT_INFO(ctx.c_str(), msg.c_str());
    }
}


template <int mastersNum, int slavesNum, int A_WIDTH, int D_WIDTH>
void sc_bus<mastersNum, slavesNum, A_WIDTH, D_WIDTH>::arbiter() {
    while (true) {

        // Wait until any master issues a request
        do { wait(clk.posedge_event()); } while (is_there_request.read() != SC_LOGIC_1);

        cout << "is_there_request? " << is_there_request.read() << endl;

        // Select the requesting master and decode the target slave address
        fetchMaster();
        fetchSlave();

        sc_slave_portBundle<A_WIDTH, D_WIDTH>* slave = &slavePortBundles[selectedSlave];
        sc_master_portBundle<A_WIDTH, D_WIDTH>* master = &masterPortBundles[selectedMaster];

        // master_slave_ Handshaking
        {
            // Initiate the transaction by enabling chip select and resetting master's ready
            slave->chipSelect = SC_LOGIC_1;
            master->ready = SC_LOGIC_0;

            // Forward read/write control signals from master to slave
            slave->read = master->read;
            slave->write = master->write;

            // Provide the translated address and master output data to the slave
            slave->address = mappedAddr;
            slave->input = master->out;

            cout << "master out in bus" << myID << " : " << master->out.read() << endl;

            // Wait for the slave to assert ready indicating it has completed the transaction
            do { wait(clk.posedge_event()); } while (slave->ready.read() != SC_LOGIC_1);

            cout << "slave in in bus" << myID << " : " << slave->input.read() << endl;
            cout << "slave out in bus" << myID << " : " << slave->out.read() << endl;

            // Notify the master that the transaction is complete
            master->ready = SC_LOGIC_1;

            // Reset slave control signals to default inactive states
            slave->chipSelect = SC_LOGIC_0;
            slave->read = SC_LOGIC_0;
            slave->write = SC_LOGIC_0;

            // Transfer slave output data back to master input port
            master->input = slave->out;


            // Wait for the next clock edge before starting the next arbitration cycle
            wait(clk.posedge_event());

            cout << "master in in bus" << myID << " : " << master->input.read() << endl;


            SC_REPORT_INFO("sc_bus", "Handshaking successfully done");
        }

    }
}

#endif