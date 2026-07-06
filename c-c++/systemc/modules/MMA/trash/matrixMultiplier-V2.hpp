#include "basics.hpp"

using namespace std;
#define SAYAC_WIDTH 16

template <typename T>
class Matrix
{
private:
    // number of columns -> save it in one Register in MMA module
    int length;
    // number of rows -> save it in one Register in MMA module
    int width;
    // indices data -> each data one Register
    T *indices;

    void allocMem()
    {
        if (this->indices)
            delete[] indices;
        indices = new T[length * width];
    }

    void setWidth(int width) { this->width = width; }
    void setLength(int length) { this->length = length; }

public:
    Matrix() : length(0), width(0), indices(nullptr) {};
    Matrix(const Matrix &other)
        : length(other.length), width(other.width)
    {
        indices = new T[length * width];
        for (int i = 0; i < length * width; ++i)
            indices[i] = other.indices[i];
    }
    ~Matrix() { delete[] indices; }

    Matrix &operator=(const Matrix &other)
    {
        if (this == &other)
            return *this;
        delete[] indices;
        length = other.length;
        width = other.width;
        indices = new T[length * width];
        for (int i = 0; i < length * width; ++i)
            indices[i] = other.indices[i];
        return *this;
    }

    // set Methods
    void setShape(int width, int length)
    {
        setWidth(width);
        setLength(length);
        allocMem();
    }

    void setIndices(T *indices)
    {
        if (!this->indices)
            allocMem();
        for (int i = 0; i < width * length; i++)
            this->indices[i] = indices[i];
    }

    // get Methods
    vector<int> shape() { return {width, length}; }
    T *getIndices() { return indices; }
};

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
    Matrix<float> *A;

    // Registers of B
    Matrix<float> *B;

    // Registers of C
    Matrix<float> *C;
    // matrixC: A * B
    // matrixC: A_width * B_length

    void configParam();

    void configMatrix(Matrix<float> *M);
    void configData();

    void eval();

    void writeResult();

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

        A = new Matrix<float>();
        B = new Matrix<float>();
        C = new Matrix<float>();
        config = true;
        multiply = false;
        write = false;

        SC_THREAD(configParam);
        sensitive << rst << clk;

        SC_THREAD(configData);
        sensitive << rst << clk;

        SC_THREAD(eval);
        sensitive << rst << clk;

        SC_THREAD(writeResult);
        sensitive << rst << clk;
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

        int A_width = 0;
        int A_length = 0;
        int B_width = 0;
        int B_length = 0;

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
            A->setShape(A_width, A_length);
            B->setShape(B_width, B_length);
            C->setShape(A_width, B_length);
        }
        wait(clk.posedge_event());

        ready = SC_LOGIC_1;
    }
};

void MMA::configMatrix(Matrix<float> *M)
{
    int width = M->shape()[0];
    int length = M->shape()[1];
    float *tempIndices = new float[width * length];

    wait(startI.posedge());
    for (int row = 0; row < width; row++)
        for (int column = 0; column < length; column++)
        {
            if (startI.read() == SC_LOGIC_1)
            {
                tempIndices[row * length + column] = input.read().to_int();
                wait(clk.posedge_event());
                ready = SC_LOGIC_1;
            }
            wait(clk.posedge_event());
            ready = SC_LOGIC_0;
            if (!(row == (width - 1) && column == (length - 1)))
                wait(startI.posedge());
        }
    M->setIndices(tempIndices);
    delete[] tempIndices;
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
        configMatrix(A);
        configMatrix(B);

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
        
        vector<int> A_shape = A->shape();
        vector<int> B_shape = B->shape();
        vector<int> C_shape = C->shape();
        int A_width = A_shape[0];
        int A_length = A_shape[1];
        int B_width = B_shape[0];
        int B_length = B_shape[1];
        int C_width = C_shape[0];
        int C_length = C_shape[1];
        
        float *A_ = A->getIndices();
        float *B_ = B->getIndices();
        
        float *C_ = new float[C_width * C_length];
        
        for (int row = 0; row < A_width; row++)
        for (int column = 0; column < B_length; column++)
        {
                float result = 0;
                for (int index = 0; index < A_length; index++)
                    result += A_[row * A_length + index] * B_[index * B_width + column];
                    C_[row * B_length + column] = result;
                }
                
                
        C->setIndices(C_);

        delete[] C_;

        ready = SC_LOGIC_0;
        output = sc_lv<SAYAC_WIDTH>(0);

        wait(clk.posedge_event());

        multiply = false;
        ready = SC_LOGIC_1;
    }
};

void MMA::writeResult()
{
    while (true)
    {
        vector<int> C_shape = C->shape();

        int C_width = C_shape[0];
        int C_length = C_shape[1];
        float *C_ = C->getIndices();

        while (multiply || config)
            wait(clk.posedge_event());

        for (int row = 0; row < C_width; row++)
            for (int column = 0; column < C_length; column++)
            {
                if (startW.read() == SC_LOGIC_1)
                {
                    output.write(float_to_lv16(C_[row * C_length + column]));
                    wait(clk.posedge_event());
                    ready = SC_LOGIC_1;
                }
                wait(clk.posedge_event());
                ready = SC_LOGIC_0;
                if (!(row == (C_width - 1) && column == (C_length - 1)))
                    wait(startW.posedge());
            }

        ready = SC_LOGIC_0;
        output = sc_lv<SAYAC_WIDTH>(0);

        wait(clk.posedge_event());

        ready = SC_LOGIC_1;
    }
};