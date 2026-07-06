#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <cmath>
#include <systemc.h>
#include <sstream>
#include <cstring>
#include <cstdint>

using namespace std;
#define SAYAC_WIDTH 16

template <typename T>
class Matrix
{
private:
    // name for printing
    string name;
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
        if (length <= 0 || width <= 0)
        {
            cerr << "Error: Matrix dimensions must be positive integer." << endl;
            exit(EXIT_FAILURE);
        }
        indices = new T[length * width];
    }

    void setWidth(int width) { this->width = width; }
    void setLength(int length) { this->length = length; }

public:
    Matrix(string name) : length(0), width(0), indices(nullptr), name(name) {};
    Matrix(const Matrix &other)
    {
        this->name = other.name;
        this->setShape(other.width, other.length);
        for (int i = 0; i < this->length * this->width; ++i)
            this->indices[i] = other.indices[i];
    }

    Matrix &operator=(const Matrix &other)
    {
        if (this == &other)
            return *this;

        this->name = other.name;
        this->setShape(other.width, other.length);
        for (int i = 0; i < this->length * this->width; ++i)
            this->indices[i] = other.indices[i];

        return *this;
    }

    T *operator[](int row)
    {
        if (row < 0 || row >= width)
            throw std::out_of_range("Row index out of range");
        return &indices[row * length];
    }
    const T *operator[](int row) const
    {
        if (row < 0 || row >= width)
            throw std::out_of_range("Row index out of range");
        return &indices[row * length];
    }

    ~Matrix() { delete[] indices; }

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
    vector<int> shape() const { return {width, length}; }

    // Returns a copy of the internal data
    T *getIndices() const
    {
        if (!indices)
            return nullptr;

        T *copy = new T[width * length];
        for (int i = 0; i < width * length; ++i)
            copy[i] = indices[i];

        return copy;
    }

    Matrix<T> transpose() const
    {
        Matrix<T> transposed(name + "_transposed");
        transposed.setShape(length, width); // swap width and length

        for (int row = 0; row < width; ++row)
            for (int column = 0; column < length; ++column)
                transposed.indices[column * width + row] = indices[row * length + column];

        return transposed;
    };

    void print() const
    {
        vector<size_t> col_width(length, 0);
        for (int col = 0; col < length; ++col)
        {
            for (int row = 0; row < width; ++row)
            {
                ostringstream oss;
                oss << indices[row * length + col];
                size_t len = oss.str().length();

                if (len > col_width[col])
                    col_width[col] = len;
            }
        }

        cout << this->name << " = [ ";
        int indent = static_cast<int>(this->name.length()) + 5; // 'name = [ '

        for (int row = 0; row < width; ++row)
        {
            if (row > 0)
                cout << string(indent, ' ');
            for (int col = 0; col < length; ++col)
            {
                cout << setw(col_width[col]) << indices[row * length + col];
                if (col != length - 1)
                    cout << ", ";
            }
            if (row != width - 1)
                cout << ",\n";
            else
                cout << " ]" << endl;
        }
    }
};

class MMA : public sc_module
{
private:
    // config data and parameters or not?
    bool configP;
    bool configD;
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

    void configShape();
    void configParam();

    void configMatrix(Matrix<float> *M);
    void configData();

    void multiplyMatrices();
    void eval();

    void writeMatrixResult();
    void writeResult();

    void ORreadySignals();

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

    // Inner Ready Signals which OR-ed together for Represent the Outer ready signal
    sc_signal<sc_logic> ready_configParam, ready_configData, ready_eval, ready_writeResult;
    sc_out<sc_logic> ready;

    SC_HAS_PROCESS(MMA);
    MMA(sc_module_name name) : sc_module(name)
    {

        A = new Matrix<float>("A");
        B = new Matrix<float>("B");
        C = new Matrix<float>("C");
        configP = true;
        configD = false;
        multiply = false;
        write = false;

        ready_configParam.write(SC_LOGIC_0);
        ready_configData.write(SC_LOGIC_0);
        ready_eval.write(SC_LOGIC_0);
        ready_writeResult.write(SC_LOGIC_0);

        SC_THREAD(configParam);
        sensitive << rst << clk;

        SC_THREAD(configData);
        sensitive << rst << clk;

        SC_THREAD(eval);
        sensitive << rst << clk;

        SC_THREAD(writeResult);
        sensitive << rst << clk;

        // I test with one Ready signal but systemC get Error
        // you should drive each signal just in one process core
        // I use ready in every single process cores which exist in MMA module :)
        SC_METHOD(ORreadySignals);
        sensitive << ready_configParam << ready_configData << ready_eval << ready_writeResult;
    }
    ~MMA()
    {
        delete A;
        delete B;
        delete C;
    }
};

void MMA::configShape()
{
    int A_width = 0, A_length = 0;
    int B_width = 0, B_length = 0;

    // cout << "in CongifShape Function!" << endl;
    for (int i = 0; i < 4; i++)
    {
        // cout << "in loop of CongifShape Function!" << endl;

        while (startP.read() != SC_LOGIC_1)
        {
            // cout << "waiting" << endl;
            wait(clk.posedge_event());
        }

        // cout << "end waiting for startP " << startP.read() << endl;
        wait(SC_ZERO_TIME);
        wait(SC_ZERO_TIME);

        switch (stateP.read().to_uint())
        {
        case 0: // Matrix A width Register sets
            if (i == 0)
                A_width = input.read().to_int();
            break;
        case 1: // Matrix A length Register sets
            if (i == 1)
                A_length = input.read().to_int();
            break;
        case 2: // Matrix B width Register sets
            if (i == 2)
                B_width = input.read().to_int();
            break;
        case 3: // Matrix B length Register sets
            if (i == 3)
                B_length = input.read().to_int();
            break;
        }
        // if setting parameters is done:

        if (i == 3)
        {
            // Allocate Memory for Matrix Data
            // which means how many Register we need in Accelerator
            // each indices; one Register 16bit
            A->setShape(A_width, A_length);
            B->setShape(B_width, B_length);
            C->setShape(A_width, B_length);
            cout << "A width sets: " << A->shape()[0] << " " << "A length sets: " << A->shape()[1] << endl;
            cout << "B width sets: " << B->shape()[0] << " " << "B length sets: " << B->shape()[1] << endl;
            cout << "C width sets: " << C->shape()[0] << " " << "C length sets: " << C->shape()[1] << endl;
        }
    }
}

void MMA::configParam()
{
    while (true)
    {

        // cout << configD << write << multiply<< endl;
        while (write || configD || multiply)
        {
            // cout << configD << write << multiply<< endl;
            // cout << "clocking in fist Param while" << endl;
            wait(clk.posedge_event());
        }

        cout << "START PHASE 1" << endl;

        ready_configParam.write(SC_LOGIC_0);
        configP = true;

        configShape();

        ready_configParam.write(SC_LOGIC_1);

        cout << "DONE PHASE 1" << endl;

        wait(clk.posedge_event());

        ready_configParam.write(SC_LOGIC_0);
        configP = false;
        configD = true;
    }
};

void MMA::configData()
{
    while (true)
    {

        while (configP || multiply || write)
        {
            // cout << "clocking in fist Data while" << endl;
            wait(clk.posedge_event());
        }

        cout << "START PHASE 2" << endl;
        configD = true;
        // Assume that First collect MatrixA data then MatrixB
        // row by row
        configMatrix(A);
        A->print();
        configMatrix(B);
        B->print();

        cout << "DONE PHASE 2" << endl;

        configD = false;
        multiply = true;
    }
};

void MMA::configMatrix(Matrix<float> *M)
{
    int width = M->shape()[0];
    int length = M->shape()[1];
    float *tempIndices = new float[width * length];

    for (int row = 0; row < width; row++)
        for (int column = 0; column < length; column++)
        {
            while (startI.read() != SC_LOGIC_1)
                wait(clk.posedge_event());

            wait(SC_ZERO_TIME);
            wait(SC_ZERO_TIME);
            if (startI.read() == SC_LOGIC_1)
            {
                tempIndices[row * length + column] = input.read().to_int();
                wait(clk.posedge_event());
                ready_configData.write(SC_LOGIC_1);
            }
            wait(clk.posedge_event());
            ready_configData.write(SC_LOGIC_0);
            // cout << "Data in Row " << row << " and column " << column << " collected" << endl;
        }

    M->setIndices(tempIndices);
    delete[] tempIndices;
};

void MMA::eval()
{
    while (true)
    {
        while (configD || configP || write)
        {
            // cout << "clocking in fist eval while" << endl;
            wait(clk.posedge_event());
        }
        cout << "START PHASE 3" << endl;

        multiply = true;

        multiplyMatrices();

        ready_eval.write(SC_LOGIC_1);

        C->print();
        wait(clk.posedge_event());

        ready_eval.write(SC_LOGIC_0);

        cout << "DONE PHASE 3" << endl;
        multiply = false;
        write = true;
    }
};

void MMA::multiplyMatrices()
{
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

    Matrix<float> B_T = B->transpose();
    B_T.print();
    float *B_T_ = B_T.getIndices();

    int commonDim = A_length;

    for (int row = 0; row < A_width; row++)
        for (int column = 0; column < B_length; column++)
        {
            float result = 0;
            for (int index = 0; index < commonDim; index++)
            {
                result += A_[row * commonDim + index] * B_T_[column * commonDim + index];
                wait(clk.posedge_event());
                wait(clk.posedge_event());
            }
            C_[row * B_length + column] = result;
        }

    C->setIndices(C_);
    delete[] C_;
}

void MMA::writeResult()
{
    while (true)
    {
        while (multiply || configD || configP)
        {
            wait(clk.posedge_event());
            ready_writeResult.write(SC_LOGIC_0);
            output = sc_lv<SAYAC_WIDTH>(0);
        }
        write = true;
        writeMatrixResult();
        write = false;
    }
};
void MMA::writeMatrixResult()
{
    vector<int> C_shape = C->shape();
    int C_width = C_shape[0];
    int C_length = C_shape[1];
    float *C_ = C->getIndices();
    for (int row = 0; row < C_width; row++)
        for (int column = 0; column < C_length; column++)
        {
            while (startW.read() != SC_LOGIC_1)
            {
                wait(clk.posedge_event());
                output = sc_lv<SAYAC_WIDTH>(0);
                ready_writeResult.write(SC_LOGIC_0);
            }

            if (startW.read() == SC_LOGIC_1)
            {
                output.write(float_to_lv16(C_[row * C_length + column]));
                wait(clk.posedge_event());
                ready_writeResult.write(SC_LOGIC_1);
            }

            wait(clk.posedge_event());
            output = sc_lv<SAYAC_WIDTH>(0);
            ready_writeResult.write(SC_LOGIC_0);
        }
}

void MMA::ORreadySignals()
{
    // ready is high if any internal ready is high
    sc_logic status = (ready_configParam.read() == SC_LOGIC_1) |
                              (ready_configData.read() == SC_LOGIC_1) |
                              (ready_eval.read() == SC_LOGIC_1) |
                              (ready_writeResult.read() == SC_LOGIC_1)
                          ? SC_LOGIC_1
                          : SC_LOGIC_0;
    ready.write(status);
}
