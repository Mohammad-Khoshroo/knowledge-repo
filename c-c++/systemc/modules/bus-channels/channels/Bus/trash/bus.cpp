#include "head.hpp"

template <int WIDTH, int mastersNum, int slavesNum>
class sc_bus : public sc_module {

private:

    int priorities[mastersNum];
    int priority();

    sc_lv<WIDTH> channelAddress;

public:

    sc_signal<sc_logic> req;
    sc_in<sc_logic> clk;

    sc_lv<WIDTH> startAddress[slavesNum]; // Address space start
    sc_lv<WIDTH> sizeAddress[slavesNum];  // Address space size

    // Slave ports
    sc_out<sc_logic> slaveChipSelect[slavesNum]; // Chip Select, slave is selected
    sc_out<sc_lv<WIDTH>> slaveAddress[slavesNum]; // Address in slave's local address space
    sc_out<sc_lv<WIDTH>> slaveInput[slavesNum];   // Data going to slave
    sc_in<sc_lv<WIDTH>> slaveOut[slavesNum];   // Data coming from slave
    sc_out<sc_logic> slaveWrite[slavesNum];       // Write operation
    sc_out<sc_logic> slaveRead[slavesNum];       // Read operation
    sc_in<sc_logic> slaveReady[slavesNum];     // Operation is done

    // Master ports
    sc_signal<sc_logic> masterRequest[mastersNum]; // Request for bus access
    sc_in<sc_lv<WIDTH>> masterAddress[mastersNum]; // Address
    sc_out<sc_lv<WIDTH>> masterInput[mastersNum];  // Data going to master
    sc_in<sc_lv<WIDTH>> masterOut[mastersNum];  // Data coming from master
    sc_in<sc_logic> masterWrite[mastersNum];       // Write operation
    sc_in<sc_logic> masterRead[mastersNum];       // Read operation
    sc_out<sc_logic> masterReady[mastersNum];   // Operation is done

    SC_HAS_PROCESS(sc_bus);
    sc_bus(sc_module_name name) :
        sc_module(name)
    {
        // Set the initial priorities
        for (int i = 0; i < mastersNum; i++)
            priorities[i] = i;

        SC_THREAD(rdwrProc);
        for (int i = 0; i < mastersNum; i++)
            sensitive << masterWrite[i] << masterRead[i];
        

        SC_THREAD(reqProc);
        for (int i = 0; i < mastersNum; i++)
            sensitive << masterRequest[i];
        
        SC_THREAD(arbiter);
        sensitive << clk;
    }

    void rdwrProc()
    {
        while (1)
        {
            wait();
            for (int i = 0; i < mastersNum; i++)
            {
                masterRequest[i] = masterWrite[i] | masterRead[i];
            }
        }
    }

    void reqProc()
    {
        req = sc_logic_0;
        while (1)
        {
            wait();
            sc_logic _req = sc_logic_0;

            for (int i = 0; i < mastersNum; i++)
            {
                _req = _req | masterRequest[i];
            }

            req = _req;
        }
    }

    void arbiter()
    {
        while (1)
        {
            for (int i = 0; i < slavesNum; i++)
            {
                s_cs[i] = sc_logic_0;
                s_in[i] = 0;
                s_rd[i] = sc_logic_0;
                s_wr[i] = sc_logic_0;
                s_addr[i] = 0;
            }
            for (int i = 0; i < mastersNum; i++)
            {
                masterInput[i] = 0;
                masterReady[i] = sc_logic_1;
            }
            // Wait for a request
            do
            {
                wait(clk->posedge_event());
            } while (req != '1');
            cout << "Arbiting" << endl;
            // Find the requesting master with the most priority
            int currentMaster = 0;
            for (int i = 0; i < mastersNum; i++)
            {
                cout << "prior[" << i << "]  : " << priorities[i] << endl;
                if (masterRequest[priorities[i]] == '1')
                {
                    currentMaster = priorities[i];
                    cout << "cand : " << currentMaster << endl;
                    for (int j = i; j < mastersNum - 1; j++)
                        priorities[j] = priorities[j + 1];
                    priorities[mastersNum - 1] = currentMaster;
                    break;
                }
            }
            cout << "Req granted for : " << currentMaster << endl;

            // Save accessing address
            channelAddress = masterAddress[currentMaster];
            cout << masterAddress[currentMaster].read().to_uint() << endl;

            // Find the slave that master is addressing
            int currentSlave = 0;
            for (int i = 0; i < slavesNum; i++)
            {
                cout << channelAddress.to_uint() << endl;
                if (channelAddress.to_uint() >= startAddress[i].to_uint() && channelAddress.to_uint() < startAddress[i].to_uint() + sizeAddress[i].to_uint())
                {

                    currentSlave = i;
                    break;
                }
            }
            cout << "Selected slave : " << currentSlave << endl;

            // Set the slave's address
            s_addr[currentSlave] = channelAddress.to_uint() - startAddress[currentSlave].to_uint();
            // Enable cs, selecting the slave
            s_cs[currentSlave] = sc_logic_1;
            // Pass the read and write signals
            s_rd[currentSlave] = masterRead[currentMaster];
            s_wr[currentSlave] = masterWrite[currentMaster];
            // Tell the master to wait
            masterReady[currentMaster] = sc_logic_0;

            // Pass the master's output to slave's input in case of a write operation
            s_in[currentSlave] = masterOut[currentMaster];

            // Wait until the slave is ready
            do
            {
                wait(clk->posedge_event());
            } while (s_ready[currentSlave] != '1');

            // Pass the slave's output to the master's input in case of a read operation
            masterInput[currentMaster] = s_out[currentSlave];

            // Deselect the slave
            s_cs[currentSlave] = sc_logic_0;

            // Tell the master the operation is done
            masterReady[currentMaster] = sc_logic_1;
        }
    }
};
