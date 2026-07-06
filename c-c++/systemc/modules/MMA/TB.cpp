#pragma once
#include <systemc.h>
#include <iostream>
#include "matrixMultiplier-V3.hpp"
#include "basics.hpp"

using namespace std;

SC_MODULE(MMA_TB)
{
    sc_signal<sc_logic> clk;
    sc_signal<sc_logic> rst;
    sc_signal<sc_lv<SAYAC_WIDTH>> input;
    sc_signal<sc_lv<SAYAC_WIDTH>> output;
    sc_signal<sc_logic> startP;
    sc_signal<sc_uint<2>> stateP;
    sc_signal<sc_logic> startI;
    sc_signal<sc_logic> startW;
    sc_signal<sc_logic> ready;

    MMA *mma;

    void clock_gen()
    {
        while (true)
        {
            clk.write(SC_LOGIC_0);
            wait(2, SC_NS);
            clk.write(SC_LOGIC_1);
            wait(2, SC_NS);
        }
    }

    void test()
    {
        // Reset
        // rst.write(SC_LOGIC_1);
        // wait(10, SC_NS);
        // rst.write(SC_LOGIC_0);
        // wait(10, SC_NS);

        // Example: Multiply 2x3 by 3x2
        // A = [1 2 3; 4 5 6]
        // B = [7 8; 9 10; 11 12]
        // C = A * B = [58 64; 139 154]
        int A_width = 2, A_length = 3;
        int B_width = 3, B_length = 2;
        float A_vals[6] = {1, 2, 3, 4, 5, 6};
        float B_vals[6] = {7, 8, 9, 10, 11, 12};
        float expected[4] = {58, 64, 139, 154};

        // cout << "Hmmmmmmmmmmmmmmm" << endl;
        // Set matrix dimensions via configParam
        for (int i = 0; i < 4; ++i)
        {
            // cout << "loop number " << i << " in TB" << endl;

            startP.write(SC_LOGIC_1);
            wait(SC_ZERO_TIME);
            wait(SC_ZERO_TIME);
            // cout << "now startP is issues " << startP.read() << endl;

            stateP.write(i);
            wait(SC_ZERO_TIME);
            wait(SC_ZERO_TIME);
            // cout << "now stateP is issues " << stateP.read() << endl;

            switch (i)
            {
            case 0:
                input.write(A_width);
                break;
            case 1:
                input.write(A_length);
                break;
            case 2:
                input.write(B_width);
                break;
            case 3:
                input.write(B_length);
                break;
            }
            wait(SC_ZERO_TIME);
            wait(SC_ZERO_TIME);
            // cout << "now Input is issues " << input.read() << endl;

            wait(clk.posedge_event());
            startP.write(SC_LOGIC_0);
            wait(SC_ZERO_TIME);
            wait(SC_ZERO_TIME);
            // cout << "duration on cycle startP was 1 now: " << startP.read() << endl;
            // startP.write(SC_LOGIC_0);
        }

        // for (int i = 0; i < 4; ++i)
        // {
        //     cout << "loop number " << i << " in TB" << endl;

        //     startP.write(SC_LOGIC_1);
        //     wait(SC_ZERO_TIME);
        //     wait(SC_ZERO_TIME);
        //     cout << "now startP is issues " << startP.read() << endl;

        //     stateP.write(i);
        //     wait(SC_ZERO_TIME);
        //     wait(SC_ZERO_TIME);
        //     cout << "now stateP is issues " << stateP.read() << endl;

        //     switch (i)
        //     {
        //     case 0:
        //         input.write(5);
        //         break;
        //     case 1:
        //         input.write(6);
        //         break;
        //     case 2:
        //         input.write(4);
        //         break;
        //     case 3:
        //         input.write(12);
        //         break;
        //     }
        //     wait(SC_ZERO_TIME);
        //     wait(SC_ZERO_TIME);
        //     cout << "now Input is issues " << input.read() << endl;

        //     wait(clk.posedge_event());
        //     startP.write(SC_LOGIC_0);
        //     wait(SC_ZERO_TIME);
        //     wait(SC_ZERO_TIME);
        //     cout << "duration on cycle startP was 1 now: " << startP.read() << endl;
        // }

        // for (int i = 0; i < 4; ++i)
        // {
        //     cout << "loop number " << i << " in TB" << endl;

        //     startP.write(SC_LOGIC_1);
        //     wait(SC_ZERO_TIME);
        //     wait(SC_ZERO_TIME);
        //     cout << "now startP is issues " << startP.read() << endl;

        //     stateP.write(i);
        //     wait(SC_ZERO_TIME);
        //     wait(SC_ZERO_TIME);
        //     cout << "now stateP is issues " << stateP.read() << endl;

        //     switch (i)
        //     {
        //     case 0:
        //         input.write(32);
        //         break;
        //     case 1:
        //         input.write(20);
        //         break;
        //     case 2:
        //         input.write(50);
        //         break;
        //     case 3:
        //         input.write(14);
        //         break;
        //     }

        //     wait(SC_ZERO_TIME);
        //     wait(SC_ZERO_TIME);
        //     cout << "now Input is issues " << input.read() << endl;

        //     wait(clk.posedge_event());
        //     startP.write(SC_LOGIC_0);
        //     wait(SC_ZERO_TIME);
        //     wait(SC_ZERO_TIME);
        //     cout << "duration on cycle startP was 1 now: " << startP.read() << endl;
        // }

        // Set matrix A values
        for (int i = 0; i < A_width * A_length; ++i)
        {
            startI.write(SC_LOGIC_1);
            wait(SC_ZERO_TIME);
            wait(SC_ZERO_TIME);

            input.write(float_to_lv16(A_vals[i]));
            wait(SC_ZERO_TIME);
            wait(SC_ZERO_TIME);
            wait(clk.posedge_event());
            wait(clk.posedge_event());
            // wait(SC_ZERO_TIME);
            // wait(SC_ZERO_TIME);
            startI.write(SC_LOGIC_0);
            wait(SC_ZERO_TIME);
            wait(SC_ZERO_TIME);
            wait(clk.posedge_event());
        }

        // Set matrix B values
        for (int i = 0; i < B_width * B_length; ++i)
        {
            startI.write(SC_LOGIC_1);
            wait(SC_ZERO_TIME);
            wait(SC_ZERO_TIME);

            input.write(float_to_lv16(B_vals[i]));
            wait(SC_ZERO_TIME);
            wait(SC_ZERO_TIME);

            wait(clk.posedge_event());
            wait(clk.posedge_event());

            // wait(SC_ZERO_TIME);
            // wait(SC_ZERO_TIME);
            startI.write(SC_LOGIC_0);
            wait(SC_ZERO_TIME);
            wait(SC_ZERO_TIME);
            wait(clk.posedge_event());
        }

        // Wait for MMA to be ready
        wait(200, SC_NS);

        // // Start multiplication (handled by MMA automatically after config)
        // // Wait for result to be ready
        // wait(100, SC_NS);

        // Read result matrix C
        float result[4];
        for (int i = 0; i < A_width * B_length; ++i)
        {
            startW.write(SC_LOGIC_1);
            wait(SC_ZERO_TIME);
            wait(SC_ZERO_TIME);
            wait(clk.posedge_event());
            wait(clk.posedge_event());
            result[i] = output.read().to_int();

            wait(SC_ZERO_TIME);
            wait(SC_ZERO_TIME);
            cout << "C[" << i << "] = " << result[i] << endl;

            startW.write(SC_LOGIC_0);
            wait(SC_ZERO_TIME);
            wait(SC_ZERO_TIME);
        }
        // Check result
        bool pass = true;
        for (int i = 0; i < 4; ++i)
        {
            if (abs(result[i] - expected[i]) > 1e-3)
                pass = false;
        }
        if (pass)
            cout << "Test PASSED!" << endl;
        else
            cout << "Test FAILED!" << endl;

        sc_stop();
    }

    SC_CTOR(MMA_TB)
    {

        rst.write(SC_LOGIC_0);
        input.write(sc_lv<SAYAC_WIDTH>(0));

        // cout<<"Hi"<<endl;
        mma = new MMA("mma");
        // cout<<"Hi"<<endl;
        mma->clk(clk);
        // cout<<"Hi"<<endl;
        mma->rst(rst);
        // cout<<"Hi"<<endl;
        mma->input(input);
        // cout<<"Hi"<<endl;
        mma->output(output);
        // cout<<"Hi"<<endl;
        mma->startP(startP);
        // cout<<"Hi"<<endl;
        mma->stateP(stateP);
        // cout<<"Hi"<<endl;
        mma->startI(startI);
        // cout<<"Hi"<<endl;
        mma->startW(startW);
        // cout<<"Hi"<<endl;
        mma->ready(ready);
        // cout<<"Hi"<<endl;

        SC_THREAD(clock_gen);
        SC_THREAD(test);
    }
};

int sc_main(int argc, char *argv[])
{
    MMA_TB tb("tb");
    // VCD tracing
    sc_trace_file *tf = sc_create_vcd_trace_file("mma_tb");
    tf->set_time_unit(1, SC_NS); // Explicitly set timescale to 1 nanosecond

    sc_trace(tf, tb.clk, "clk");
    sc_trace(tf, tb.rst, "rst");
    sc_trace(tf, tb.input, "input");
    sc_trace(tf, tb.output, "output");
    sc_trace(tf, tb.startP, "startP");
    sc_trace(tf, tb.stateP, "stateP");
    sc_trace(tf, tb.startI, "startI");
    sc_trace(tf, tb.startW, "startW");
    sc_trace(tf, tb.ready, "ready");
    sc_start();
    sc_close_vcd_trace_file(tf);
    return 0;
}
