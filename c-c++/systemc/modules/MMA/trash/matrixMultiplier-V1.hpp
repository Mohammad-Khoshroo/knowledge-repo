#include "basics.hpp"

#define SAYAC_WIDTH 16

class MMA : public sc_module
{
private:
    // config data and parameters or not?
    bool config;
    // do Operation or not?
    bool multiply;
    // do writing or not?
    bool write;

    // Registers of A
    float *matrixA;
    int A_width;
    int A_length;

    // Registers of B
    float *matrixB;
    int B_width;
    int B_length;

    // Registers of C
    float *matrixC;
    // matrixC: A * B
    // matrixC: A_width * B_length

    void configParam();

    void configMatrix(float *matrix, int length, int width);
    void configData();

    void eval();

    void getResult();

public:
    sc_in<sc_logic> clk;
    sc_in<sc_logic> rst;

    sc_in<sc_lv<SAYAC_WIDTH>> input;
    sc_out<sc_lv<SAYAC_WIDTH>> output;

    // Read and set Parameters
    sc_in<sc_logic> startP;
    sc_in<sc_uint<2>> stateP;

    // Read and set matrixes indices
    sc_in<sc_logic> startI;

    // Write Result Matrix C in Memory
    sc_in<sc_logic> startW;

    sc_out<sc_logic> ready;

    SC_HAS_PROCESS(MMA);
    MMA(sc_module_name name) : sc_module(name)
    {
        A_width = 0;
        A_length = 0;
        B_width = 0;
        B_length = 0;
        matrixA = nullptr;
        matrixB = nullptr;
        matrixC = nullptr;
        multiply = false;

        config = true;
        multiply = false;
        write = false;

        SC_THREAD(configParam);
        sensitive << rst << clk;

        SC_THREAD(configData);
        sensitive << rst << clk;

        SC_THREAD(eval);
        sensitive << rst << clk;

        SC_THREAD(getResult);
        sensitive << rst << clk;
    }

    ~MMA() {
        if (matrixA) delete[] matrixA;
        if (matrixB) delete[] matrixB;
        if (matrixC) delete[] matrixC;
    }
};

void MMA::configParam()
{
    while (true)
    {
        while (startP.read() != SC_LOGIC_1)
            wait(clk.posedge_event());

        config = true;
        ready = SC_LOGIC_0;
        output = sc_lv<SAYAC_WIDTH>(0);

        switch (stateP.read().to_uint())
        {
        case 0: // Matrix A width Register sets
            A_width = input.read().to_int();
            break;
        case 1: // Matrix A length Register sets
            A_length = input.read().to_int();
            break;
        case 2: // Matrix B width Register sets
            B_width = input.read().to_int();
            break;
        case 3: // Matrix B length Register sets
            B_length = input.read().to_int();
            break;
        }

        // if setting parameters is done:
        if (stateP.read().to_uint() == 3)
        {

            // Allocate Memory for Matrix Data
            // which means how many Register we need in Accelerator
            // each indices; one Register 16bit
            if (matrixA) delete[] matrixA;
            if (matrixB) delete[] matrixB;
            if (matrixC) delete[] matrixC;
            matrixA = new float[A_width * A_length];
            matrixB = new float[B_width * B_length];
            matrixC = new float[A_width * B_length];
        }
        wait(clk.posedge_event());

        ready = SC_LOGIC_1;
    }
};

void MMA::configMatrix(float *matrix, int length, int width)
{
    wait(startI.posedge());
    for (int row = 0; row < width; row++)
        for (int column = 0; column < length; column++)
        {
            if (startI.read() == SC_LOGIC_1)
            {
                matrix[row * length + column] = input.read().to_int();
                wait(clk.posedge_event());
                ready = SC_LOGIC_1;
            }
            wait(clk.posedge_event());
            ready = SC_LOGIC_0;
            if (!(row == (width - 1) && column == (length - 1)))
                wait(startI.posedge());
        }
};

void MMA::configData()
{
    while (true)
    {
        while (startI.read() != SC_LOGIC_1)
            wait(clk.posedge_event());

        ready = SC_LOGIC_0;
        output = sc_lv<SAYAC_WIDTH>(0);

        // Assume that First collect MatrixA data then MatrixB
        // row by row
        configMatrix(matrixA, A_length, A_width);
        configMatrix(matrixB, B_length, B_width);

        config = false;

        wait(clk.posedge_event());

        ready = SC_LOGIC_1;
    }
};

void MMA::eval()
{
    while (true)
    {
        while (config)
            wait(clk.posedge_event());

        multiply = true;
        for (int row = 0; row < A_width; row++)
            for (int column = 0; column < B_length; column++)
            {
                float result = 0;
                for (int index = 0; index < A_length; index++)
                    result += matrixA[row * A_length + index] * matrixB[index * B_width + column];
                wait(clk.posedge_event());
                wait(clk.posedge_event());
                wait(clk.posedge_event());
                matrixC[row * B_length + column] = result;
            }

        ready = SC_LOGIC_0;
        output = sc_lv<SAYAC_WIDTH>(0);

        wait(clk.posedge_event());

        multiply = false;
        ready = SC_LOGIC_1;
    }
};

void MMA::getResult()
{
    while (true)
    {
        while (multiply || config)
            wait(clk.posedge_event());

        for (int row = 0; row < A_width; row++)
            for (int column = 0; column < B_length; column++)
            {
                if (startW.read() == SC_LOGIC_1)
                {
                    output.write(float_to_lv16(matrixC[row * B_length + column]));
                    wait(clk.posedge_event());
                    ready = SC_LOGIC_1;
                }
                wait(clk.posedge_event());
                ready = SC_LOGIC_0;
                if (!(row == (A_width - 1) && column == (B_length - 1)))
                    wait(startW.posedge());
            }

        ready = SC_LOGIC_0;
        output = sc_lv<SAYAC_WIDTH>(0);

        wait(clk.posedge_event());

        ready = SC_LOGIC_1;
    }
};
