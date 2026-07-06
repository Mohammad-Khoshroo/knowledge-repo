#include <systemc.h>
#include <iostream>
#include "Bus.h"
template <int N>
SC_MODULE(MatMulAcc)
{
    const int MAX_SIZE = (1 << N) * (1 << N);
    // mat1 : n*k
    // mat2 : k*m
    int *mat1;   // Offset : 0
    int *mat2;   // Offset : MAX_SIZE
    int *outMat; // Offset : 2*MAX_SIZE

    sc_in<sc_logic> clk;

    // Config port
    sc_in<sc_logic> s_cs;    
    sc_in<sc_lv<N>> s_addr; 
    sc_in<sc_lv<N>> s_in; 
    sc_out<sc_lv<N>> s_out;
    sc_in<sc_logic> s_wr;
    sc_in<sc_logic> s_rd;
    sc_out<sc_logic> s_ready;

    // Interrupt to PIC
    sc_out<sc_logic> interrupt;

    // Signals between DMA and MMA
    sc_in<sc_lv<N>> dmaAddr;
    sc_in<sc_lv<N>> dmaIn;
    sc_out<sc_lv<N>> dmaOut;
    sc_in<sc_logic> dmaWR;
    sc_in<sc_logic> dmaRD;

    // Start
    sc_lv<16> controlReg;
    // Done
    sc_lv<16> statusReg;
    // Matrix size
    sc_lv<16> n, k, m;

    SC_CTOR(MatMulAcc)
    {
        controlReg = 0;
        statusReg = 0;

       
    
        mat1 = new int[MAX_SIZE];
        mat2 = new int[MAX_SIZE];
        outMat = new int[MAX_SIZE];
        memset(mat1, 0, sizeof(int) * MAX_SIZE);
        memset(mat2, 0, sizeof(int) * MAX_SIZE);
        memset(outMat, 0, sizeof(int) * MAX_SIZE);
 
        SC_THREAD(evalConfigReg);
        sensitive << clk;
        SC_THREAD(eval);
        sensitive << clk;
        SC_THREAD(evalMem);
        sensitive << clk;
    }


    void evalConfigReg()
    {
        while (true)
        {
            
            s_ready = sc_logic_1;
            // Wait for CS
            do
            {
                wait(clk->posedge_event());
            } while (s_cs != '1');
            s_out = 0;
            s_ready = sc_logic_0;
            if (s_wr == '1') // Write
            {
                if (s_addr.read().to_uint() == 0) // Address 0 => control register
                {
                    controlReg = s_in;
                }
                else if (s_addr.read().to_uint() == 1) // Address 1 => n register
                {
                    n = s_in;
                }
                else if (s_addr.read().to_uint() == 2) // Address 2 => k register
                {
                    k = s_in;
                }
                else if (s_addr.read().to_uint() == 3) // Address 3 => m register
                {
                    m = s_in;
                }
            }
            else if (s_rd == '1')
            {
                if (s_addr.read().to_uint() == 4) // Address 4 => status register
                {
                    s_out = statusReg;
                }
            }
            s_ready = sc_logic_1;
        }
    }

    void evalMem()
    {
        while (true)
        {
            dmaOut = 0;
            // Wait for clock
            wait(clk->posedge_event());
            int localAddress = dmaAddr.read().to_uint();
            if (dmaRD == '1') // Read operation pending
            {
                // If outMat is addressed, return the data
                if (localAddress >= 2 * MAX_SIZE && localAddress < 3 * MAX_SIZE)
                    dmaOut = outMat[localAddress - 2 * MAX_SIZE];
            }
            else if (dmaWR == '1') // Write operation pending
            {

                if (localAddress < MAX_SIZE) // Mat1 being addressed
                    mat1[localAddress] = dmaIn.read().to_int();
                else if (localAddress >= MAX_SIZE && localAddress < 2 * MAX_SIZE) // Mat2 being addressed
                    mat2[localAddress - MAX_SIZE] = dmaIn.read().to_int();
            }
        }
    }

    void eval()
    {
        while (true)
        {
            interrupt = sc_logic_0;
            // Wait for start
            do
            {
                wait(clk->posedge_event());
                //cout << controlReg.to_uint() << endl;
            } while (controlReg.to_uint() == 0);

            // Set start,done and interrupt to zero
            controlReg = 0;
            statusReg = 0;
            interrupt = sc_logic_0;
            for (int x = 0; x < n.to_uint(); x++)
            {
                for (int y = 0; y < m.to_uint(); y++)
                {
                    int sum = 0;
                    for (int z = 0; z < k.to_uint(); z++)
                    {
                        // MAC operation
                        sum += mat1[x * k.to_uint() + z] * mat2[m.to_uint() * z + y];
                        // Wait a clock, assuming each MAC operation takes one clock to complete.
                        wait(clk->posedge_event());
                    }
                    // Store the result
                    outMat[x * m.to_uint() + y] = sum;
                }
            }
            // Set done to one
            statusReg = 1;

            // Issue interrupt for one clock
            interrupt = sc_logic_1;
            wait(clk->posedge_event());
            interrupt = sc_logic_0;
        }
    }
};