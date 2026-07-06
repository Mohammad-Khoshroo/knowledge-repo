#include <systemc.h>
#include <iostream>

#ifndef BUS_INCLUDE
#define BUS_INCLUDE

template <int N, int M, int S>
SC_MODULE(Bus)
{
public:
    int priorities[M];
    sc_lv<N> addressBus;
    sc_signal<sc_logic> req;

    sc_in<sc_logic> clk;

    // Slave ports
    // sc_port<SlavePort<N>> slaves[S];
    sc_out<sc_logic> s_cs[S];   // Chip Select, slave is selected
    sc_out<sc_lv<N>> s_addr[S]; // Address in slave's local address space
    sc_out<sc_lv<N>> s_in[S];   // Data going to slave
    sc_in<sc_lv<N>> s_out[S];   // Data coming from slave
    sc_out<sc_logic> s_wr[S];   // Write operation
    sc_out<sc_logic> s_rd[S];   // Read operation
    sc_in<sc_logic> s_ready[S]; // Operation is done

    sc_lv<N> startAddress[S]; // Address space start
    sc_lv<N> sizeAddress[S];  // Address space size

    // Master ports

    
    sc_signal<sc_logic> m_req[M];    // Request for bus access

    //sc_out<sc_logic> m_gnt[M];   // Bus access granted
    sc_in<sc_lv<N>> m_addr[M];   // Address
    sc_out<sc_lv<N>> m_in[M];    // Data going to master
    sc_in<sc_lv<N>> m_out[M];    // Data coming from master
    sc_in<sc_logic> m_wr[M];     // Write operation
    sc_in<sc_logic> m_rd[M];     // Read operation
    sc_out<sc_logic> m_ready[M]; // Operation is done

    SC_CTOR(Bus)
    {
        // Set the initial priorities
        for (int i = 0; i < M; i++)
            priorities[i] = i;

        SC_THREAD(rdwrProc);
        for (int i = 0; i < M; i++)
        {
            sensitive << m_wr[i] << m_rd[i];
        }

        SC_THREAD(reqProc);
        for (int i = 0; i < M; i++)
        {
            sensitive << m_req[i];
        }

        SC_THREAD(arbiter);
        sensitive << clk;

    }

    void rdwrProc()
    {
        while(1)
        {
            wait();
            for (int i = 0; i < M; i++)
            {
                m_req[i] = m_wr[i] | m_rd[i];
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

            for (int i = 0; i < M; i++)
            {
                _req = _req | m_req[i];
            }

            req = _req;
        }
    }

    void arbiter()
    {
        while (1)
        {
            for (int i = 0; i < S; i++)
            {
                s_cs[i] = sc_logic_0;
                s_in[i] = 0;
                s_rd[i] = sc_logic_0;
                s_wr[i] = sc_logic_0;
                s_addr[i] = 0;
            }
            for (int i = 0; i < M; i++)
            {
                m_in[i] = 0;
                m_ready[i] = sc_logic_1;
            }
            // Wait for a request
            do
            {
                wait(clk->posedge_event());
            } while (req != '1');
            cout << "Arbiting" << endl;
            // Find the requesting master with the most priority
            int currentMaster = 0;
            for (int i = 0; i < M; i++)
            {
                cout << "prior[" << i << "]  : " << priorities[i] << endl;
                if (m_req[priorities[i]] == '1')
                {
                    currentMaster = priorities[i];
                    cout << "cand : "<< currentMaster<< endl;
                    for (int j = i; j < M - 1; j++)
                        priorities[j] = priorities[j + 1];
                    priorities[M - 1] = currentMaster;
                    break;
                }
            }
            cout << "Req granted for : " << currentMaster << endl;



            // Save accessing address
            addressBus = m_addr[currentMaster];
            cout <<  m_addr[currentMaster].read().to_uint() << endl;

            // Find the slave that master is addressing
            int currentSlave = 0;
            for (int i = 0; i < S; i++)
            {
                cout << addressBus.to_uint() << endl;
                if (addressBus.to_uint() >= startAddress[i].to_uint() && addressBus.to_uint() < startAddress[i].to_uint() + sizeAddress[i].to_uint())
                {
                    
                    currentSlave = i;
                    break;
                }
            }
             cout << "Selected slave : " << currentSlave << endl;

            // Set the slave's address
            s_addr[currentSlave] = addressBus.to_uint() - startAddress[currentSlave].to_uint();
            // Enable cs, selecting the slave
            s_cs[currentSlave] = sc_logic_1;
            // Pass the read and write signals
            s_rd[currentSlave] = m_rd[currentMaster];
            s_wr[currentSlave] = m_wr[currentMaster];
            // Tell the master to wait
            m_ready[currentMaster] = sc_logic_0;

            // Pass the master's output to slave's input in case of a write operation
            s_in[currentSlave] = m_out[currentMaster];

            // Wait until the slave is ready
            do
            {
                wait(clk->posedge_event());
            } while (s_ready[currentSlave] != '1');

            // Pass the slave's output to the master's input in case of a read operation
            m_in[currentMaster] = s_out[currentSlave];

            // Deselect the slave
            s_cs[currentSlave] = sc_logic_0;

            // Tell the master the operation is done
            m_ready[currentMaster] = sc_logic_1;
        }
    }
};

#endif
