#include "wrapper8to32.hpp"

// Simple 8-bit memory module for testing
template<int A_WIDTH>
class simple_memory8 : public sc_slave_module<A_WIDTH, 8> {

    using addr_t = sc_lv<A_WIDTH>;
    using data_t = sc_lv<8>;

private:
    static const int MEMORY_SIZE = 1 << A_WIDTH;
    uint8_t memory_array[MEMORY_SIZE];

public:

    SC_HAS_PROCESS(simple_memory8);
    simple_memory8(sc_module_name name, sc_trace_file* tf)
        : sc_slave_module<A_WIDTH, 8>(name, tf) {

        // Initialize memory with test pattern
        for (int i = 0; i < MEMORY_SIZE; i++) {
            memory_array[i] = i & 0xFF;
            cout << "memory in loc " << i << " = " << int(memory_array[i]) << endl;
        }

        SC_THREAD(memory_process);
        this->sensitive << this->clk.pos();
    }

private:
    void memory_process() {

        bool wr = false;
        bool rd = false;
        bool cs = false;

        this->out = 0;

        while (true) {

            this->ready = SC_LOGIC_0;

            do wait();
            while (this->chipSelect.read() != SC_LOGIC_1);

            uint64_t addr = this->address.read().to_uint64();

            cs = (this->chipSelect.read() == SC_LOGIC_1);
            rd = (this->read.read() == SC_LOGIC_1);
            wr = (this->write.read() == SC_LOGIC_1);

            // cout << cs << rd << wr << endl;

            // Conflict check
            if (rd && wr)
                SC_REPORT_FATAL("Memory", "Conflict: both read and write are asserted.");

            // Write
            if (wr)
            {
                // cout << "input write is : " << this->input.read().to_int() << endl;
                memory_array[addr] = this->input.read().to_int();
                for (int i = 0; i < MEMORY_SIZE; i++) {
                    cout << "memory in loc " << i << " = " << int(memory_array[i]) << endl;
                }

            }// Read
            else if (rd)
            {
                this->out = memory_array[addr];

            }


            this->ready = SC_LOGIC_1;

            wait(this->clk.posedge_event()); // next clock

            // cout << "out setted to : " << this->out.read().to_int() << endl;

        }
    }
};

// Test bench module
template<int A_WIDTH>
class memory_wrapper_testBench : public sc_module {
public:
    // Clock and reset
    sc_clock clk;

    // DUT (Device Under Test)
    memory_wrapper8to32<A_WIDTH>* dut;
    simple_memory8<A_WIDTH>* memory_8bit;

    // Test signals
    sc_signal<sc_lv<A_WIDTH>> address;
    sc_signal<sc_lv<32>> input_data;
    sc_signal<sc_lv<32>> output_data;
    sc_signal<sc_logic> read_signal;
    sc_signal<sc_logic> write_signal;
    sc_signal<sc_logic> chip_select;
    sc_signal<sc_logic> ready;

    // Trace file
    sc_trace_file* tf;

    SC_HAS_PROCESS(memory_wrapper_testBench);

    memory_wrapper_testBench(sc_module_name name)
        : sc_module(name),
        clk("clk", 10, SC_NS) {

        // Create trace file
        tf = sc_create_vcd_trace_file("out");

        // Instantiate memory and wrapper
        memory_8bit = new simple_memory8<A_WIDTH>("memory_8bit", tf);
        dut = new memory_wrapper8to32<A_WIDTH>("memory_wrapper", tf, memory_8bit);

        // Connect signals
        dut->clk(clk);
        dut->address(address);
        dut->input(input_data);
        dut->out(output_data);
        dut->read(read_signal);
        dut->write(write_signal);
        dut->chipSelect(chip_select);
        dut->ready(ready);

        // Add traces
        sc_trace(tf, clk, "clk");
        sc_trace(tf, address, "address");
        sc_trace(tf, input_data, "input_data");
        sc_trace(tf, output_data, "output_data");
        sc_trace(tf, read_signal, "read_signal");
        sc_trace(tf, write_signal, "write_signal");
        sc_trace(tf, chip_select, "chip_select");
        sc_trace(tf, ready, "ready");

        SC_THREAD(test_process);

    }

    ~memory_wrapper_testBench() {
        sc_close_vcd_trace_file(tf);
        delete dut;
        delete memory_8bit;
    }

private:
    void test_process() {
        cout << "\n=== Memory Wrapper Test Bench Started ===" << endl;

        // Initialize signals
        initialize_signals();

        // Wait for a few clock cycles
        wait(50, SC_NS);

        // Test 1: Read Operation
        cout << "\n--- Test 1: Read Operation ---" << endl;
        test_read_operation();

        // Wait for a few clock cycles
        wait(50, SC_NS);

        // Test 2: Write Operation
        cout << "\n--- Test 2: Write Operation ---" << endl;
        test_write_operation();

        // Wait for a few clock cycles
        wait(50, SC_NS);

        // Test 3: Multiple Read/Write Operations
        cout << "\n--- Test 3: Multiple Operations ---" << endl;
        test_multiple_operations();
        // Wait for a few clock cycles
        wait(50, SC_NS);

        // Test 4: Address Alignment Test
        cout << "\n--- Test 4: Address Alignment ---" << endl;
        test_address_alignment();
        // Wait for a few clock cycles
        wait(50, SC_NS);

        cout << "\n=== All Tests Completed ===" << endl;

        // Wait some more time to see final state
        wait(100, SC_NS);

        sc_stop();
    }

    void initialize_signals() {
        address.write(sc_lv<A_WIDTH>(0));
        input_data.write(sc_lv<32>(0));
        read_signal.write(SC_LOGIC_0);
        write_signal.write(SC_LOGIC_0);
        chip_select.write(SC_LOGIC_0);
    }

    void test_read_operation() {
        // Test reading from address 0x00 (should read 4 bytes: 0x00, 0x01, 0x02, 0x03)
        sc_lv<A_WIDTH> test_addr(0x04);

        cout << "Reading from address: 0x" << hex << test_addr.to_uint64() << endl;

        // Setup read operation
        address = test_addr;
        read_signal = SC_LOGIC_1;
        write_signal.write(SC_LOGIC_0);
        chip_select = SC_LOGIC_1;

        cout << "wait for setup done" << endl;
        // wait(SC_ZERO_TIME);
        // wait(clk.posedge_event()); // Wait for operation to start
        wait(10, SC_NS); // Wait for operation to start

        chip_select = SC_LOGIC_0;
        read_signal = SC_LOGIC_0;
        write_signal = SC_LOGIC_0;

        cout << "setup done" << endl;

        // Wait for ready signal
        int i = 0;
        while (ready.read() != SC_LOGIC_1) {
            wait(10, SC_NS);
            i++;
        }

        sc_lv<32> read_data = output_data.read();
        cout << "Read data: 0x" << hex << read_data.to_uint64() << endl;

        // Expected: 0x03020100 (little endian: byte0=0x00, byte1=0x01, byte2=0x02, byte3=0x03)
        uint32_t expected = 0x07060504;
        if (read_data.to_uint64() == expected) {
            cout << "✓ Read test PASSED" << endl;
        }
        else {
            cout << "✗ Read test FAILED. Expected: 0x" << hex << expected << endl;
        }

        // End operation
        chip_select.write(SC_LOGIC_0);
        read_signal.write(SC_LOGIC_0);
        wait(20, SC_NS);
    }

    void test_write_operation() {
        // Test writing 0xDEADBEEF to address 0x10
        sc_lv<A_WIDTH> test_addr(0x10);
        sc_lv<32> test_data(0xDEADBEEF);

        cout << "Writing 0x" << hex << test_data.to_uint64() << " to address: 0x" << test_addr.to_uint64() << endl;

        // Setup write operation
        address.write(test_addr);
        input_data.write(test_data);
        chip_select.write(SC_LOGIC_1);
        read_signal.write(SC_LOGIC_0);
        write_signal.write(SC_LOGIC_1);

        wait(10, SC_NS); // Wait for operation to start

        chip_select.write(SC_LOGIC_0);
        read_signal.write(SC_LOGIC_0);
        write_signal.write(SC_LOGIC_0);


        // Wait for ready signal
        while (ready.read() != SC_LOGIC_1) {
            wait(10, SC_NS);
        }

        cout << "✓ Write operation completed" << endl;

        // End operation
        chip_select.write(SC_LOGIC_0);
        write_signal.write(SC_LOGIC_0);
        wait(20, SC_NS);

        // Verify write by reading back
        cout << "Verifying write by reading back..." << endl;
        address.write(test_addr);

        chip_select.write(SC_LOGIC_1);
        read_signal.write(SC_LOGIC_1);

        wait(10, SC_NS);

        chip_select.write(SC_LOGIC_0);
        read_signal.write(SC_LOGIC_0);


        while (ready.read() != SC_LOGIC_1) {
            wait(10, SC_NS);
        }

        sc_lv<32> read_back = output_data.read();
        cout << "Read back: 0x" << hex << read_back.to_uint64() << endl;

        if (read_back.to_uint64() == test_data.to_uint64()) {
            cout << "✓ Write verification PASSED" << endl;
        }
        else {
            cout << "✗ Write verification FAILED" << endl;
        }

        chip_select.write(SC_LOGIC_0);
        read_signal.write(SC_LOGIC_0);
        wait(20, SC_NS);
    }

    void test_multiple_operations() {
        // Test multiple consecutive operations
        for (int i = 0; i < 3; i++) {
            sc_lv<A_WIDTH> addr(0x20 + i * 4);
            sc_lv<32> data(0x1000 + i);

            cout << "Operation " << i << ": Writing 0x" << hex << data.to_uint64()
                << " to address 0x" << addr.to_uint64() << endl;

            // Write
            address.write(addr);
            input_data.write(data);
            chip_select.write(SC_LOGIC_1);
            write_signal.write(SC_LOGIC_1);

            wait(10, SC_NS);
            chip_select.write(SC_LOGIC_0);
            write_signal.write(SC_LOGIC_0);
            while (ready.read() != SC_LOGIC_1) wait(10, SC_NS);

            chip_select.write(SC_LOGIC_0);
            write_signal.write(SC_LOGIC_0);
            wait(10, SC_NS);

            // Read back
            chip_select.write(SC_LOGIC_1);
            read_signal.write(SC_LOGIC_1);

            wait(10, SC_NS);

            chip_select.write(SC_LOGIC_0);
            read_signal.write(SC_LOGIC_0);
            while (ready.read() != SC_LOGIC_1) wait(10, SC_NS);

            sc_lv<32> read_data = output_data.read();
            cout << "Read back: 0x" << hex << read_data.to_uint64() << endl;

            chip_select.write(SC_LOGIC_0);
            read_signal.write(SC_LOGIC_0);
            wait(10, SC_NS);
        }
    }

    void test_address_alignment() {
        // Test that unaligned addresses are properly aligned
        cout << "Testing address alignment..." << endl;

        // Try to read from address 0x33 (should be aligned to 0x30)
        sc_lv<A_WIDTH> unaligned_addr(0x33);

        cout << "Requesting unaligned address: 0x" << hex << unaligned_addr.to_uint64() << endl;

        address.write(unaligned_addr);
        chip_select.write(SC_LOGIC_1);
        read_signal.write(SC_LOGIC_1);

        wait(10, SC_NS);

        chip_select.write(SC_LOGIC_0);
        read_signal.write(SC_LOGIC_0);
        while (ready.read() != SC_LOGIC_1) wait(10, SC_NS);

        sc_lv<32> aligned_data = output_data.read();
        cout << "Data from aligned address (0x30): 0x" << hex << aligned_data.to_uint64() << endl;

        chip_select.write(SC_LOGIC_0);
        read_signal.write(SC_LOGIC_0);
        wait(20, SC_NS);

        cout << "✓ Address alignment test completed" << endl;
    }
};

// Main function
int sc_main(int argc, char* argv[]) {
    // Create testbench with 8-bit address width

    memory_wrapper_testBench<5> tb("testbench");

    // Run simulation
    sc_start();

    return 0;
}