#include "memory.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;
using namespace sc_core;
using namespace sc_dt;

// TestBench class
template<int receiversNum, int A_WIDTH, int D_WIDTH>
class DMA_TestBench : public sc_module {
private:

    // Clock and reset signals
    sc_signal<sc_logic> clk;
    sc_signal<sc_logic> rst;
    
    // DMA instance
    DMA<receiversNum, A_WIDTH, D_WIDTH> dma;
    
    // Shared bus
    sc_bus<2, 3, A_WIDTH, D_WIDTH> shared_bus; // 2 masters (DMA + testBench), 3 slaves (2 memories + DMA config)
    
    // Memory modules
    Memory<sc_lv<D_WIDTH>, A_WIDTH> source_memory;
    Memory<sc_lv<D_WIDTH>, A_WIDTH> destination_memory;
    
    // TestBench master for configuration
    sc_master_module<A_WIDTH, D_WIDTH> testBench_master;
    
    // Dump times for memory
    vector<int> dump_times;
    
    // Test data
    const int SOURCE_MEM_START = 0x0000;
    const int SOURCE_MEM_SIZE = 1024;
    const int DEST_MEM_START = 0x1000;
    const int DEST_MEM_SIZE = 1024;
    const int DMA_CONFIG_START = 0x2000;
    const int DMA_CONFIG_SIZE = 16;
    
    void generate_clock();
    void test_sequence();
    void create_test_data();
    
public:
    SC_HAS_PROCESS(DMA_TestBench);
    
    DMA_TestBench(sc_module_name name) : 
        sc_module(name),
        dma("dma", nullptr),
        shared_bus("shared_bus", nullptr),
        source_memory("source_mem", "source_memory.csv", "source_dump.csv", dump_times),
        destination_memory("dest_mem", "dest_memory.csv", "dest_dump.csv", dump_times),
        testBench_master("testBench_master", nullptr)
    {
        // Initialize dump times
        dump_times = {100, 500, 1000, 2000};
        
        // Create test data files
        create_test_data();
        
        // Connect clock and reset
        dma.clk(clk);
        shared_bus.clk(clk);
        shared_bus.rst(rst);
        source_memory.clk(clk);
        destination_memory.clk(clk);
        testBench_master.clk(clk);
        
        // Connect DMA to bus
        shared_bus.addMaster(&dma);
        
        // Connect memories to bus
        shared_bus.addSlave(&source_memory);
        shared_bus.addSlave(&destination_memory);
        // Note: DMA config module is already connected through DMA master
        
        // Connect testBench master to bus
        shared_bus.addMaster(&testBench_master);
        
        // SC_THREAD processes
        SC_THREAD(generate_clock);
        SC_THREAD(test_sequence);
        
        // Initialize signals
        rst.write(SC_LOGIC_0);
    }
};

template<int receiversNum, int A_WIDTH, int D_WIDTH>
void DMA_TestBench<receiversNum, A_WIDTH, D_WIDTH>::generate_clock() {
    while (true) {
        clk.write(SC_LOGIC_0);
        wait(5, SC_NS);
        clk.write(SC_LOGIC_1);
        wait(5, SC_NS);
    }
}

template<int receiversNum, int A_WIDTH, int D_WIDTH>
void DMA_TestBench<receiversNum, A_WIDTH, D_WIDTH>::create_test_data() {
    // Create source memory data
    ofstream source_file("source_memory.csv");
    source_file << "address,data\n";
    for (int i = 0; i < 256; i++) {
        source_file << "0x" << hex << i << ",0x" << hex << (i * 2 + 1) << "\n";
    }
    source_file.close();
    
    // Create destination memory data (empty)
    ofstream dest_file("dest_memory.csv");
    dest_file << "address,data\n";
    for (int i = 0; i < 256; i++) {
        dest_file << "0x" << hex << i << ",0x0000\n";
    }
    dest_file.close();
    
    cout << "Test data files created successfully!" << endl;
}

template<int receiversNum, int A_WIDTH, int D_WIDTH>
void DMA_TestBench<receiversNum, A_WIDTH, D_WIDTH>::test_sequence() {
    // Wait for system initialization
    wait(100, SC_NS);
    
    cout << "Starting DMA test sequence..." << endl;
    
    // Test 1: Memory to Memory transfer (WR operation)
    cout << "Test 1: Memory to Memory transfer (WR operation)" << endl;
    
    // Configure DMA for WR operation
    // Control register: 0x02 (WR=1, RD=0, Start=0)
    testBench_master.address.write(0x2000);
    testBench_master.out.write(0x02);
    testBench_master.write.write(SC_LOGIC_1);
    testBench_master.read.write(SC_LOGIC_0);
    wait(10, SC_NS);
    testBench_master.write.write(SC_LOGIC_0);
    
    // Set source address
    testBench_master.address.write(0x2001);
    testBench_master.out.write(0x0000); // Start from address 0
    testBench_master.write.write(SC_LOGIC_1);
    wait(10, SC_NS);
    testBench_master.write.write(SC_LOGIC_0);
    
    // Set destination address
    testBench_master.address.write(0x2002);
    testBench_master.out.write(0x1000); // Destination at 0x1000
    testBench_master.write.write(SC_LOGIC_1);
    wait(10, SC_NS);
    testBench_master.write.write(SC_LOGIC_0);
    
    // Set number of words to transfer
    testBench_master.address.write(0x2003);
    testBench_master.out.write(0x0010); // Transfer 16 words
    testBench_master.write.write(SC_LOGIC_1);
    wait(10, SC_NS);
    testBench_master.write.write(SC_LOGIC_0);
    
    // Start the transfer
    testBench_master.address.write(0x2000);
    testBench_master.out.write(0x03); // WR=1, RD=0, Start=1
    testBench_master.write.write(SC_LOGIC_1);
    wait(10, SC_NS);
    testBench_master.write.write(SC_LOGIC_0);
    
    // Wait for transfer to complete
    wait(1000, SC_NS);
    
    // Check if transfer is done
    testBench_master.address.write(0x2004);
    testBench_master.read.write(SC_LOGIC_1);
    testBench_master.write.write(SC_LOGIC_0);
    wait(10, SC_NS);
    sc_lv<D_WIDTH> done_status = testBench_master.input.read();
    testBench_master.read.write(SC_LOGIC_0);
    
    cout << "WR Transfer done status: " << done_status.to_string() << endl;
    
    // Test 2: Memory to Memory transfer (RD operation)
    cout << "Test 2: Memory to Memory transfer (RD operation)" << endl;
    
    // Configure DMA for RD operation
    // Control register: 0x04 (WR=0, RD=1, Start=0)
    testBench_master.address.write(0x2000);
    testBench_master.out.write(0x04);
    testBench_master.write.write(SC_LOGIC_1);
    testBench_master.read.write(SC_LOGIC_0);
    wait(10, SC_NS);
    testBench_master.write.write(SC_LOGIC_0);
    
    // Set source address (from destination memory)
    testBench_master.address.write(0x2001);
    testBench_master.out.write(0x1000); // Read from destination memory
    testBench_master.write.write(SC_LOGIC_1);
    wait(10, SC_NS);
    testBench_master.write.write(SC_LOGIC_0);
    
    // Set destination address (to source memory)
    testBench_master.address.write(0x2002);
    testBench_master.out.write(0x0200); // Write to source memory at offset 0x200
    testBench_master.write.write(SC_LOGIC_1);
    wait(10, SC_NS);
    testBench_master.write.write(SC_LOGIC_0);
    
    // Set number of words to transfer
    testBench_master.address.write(0x2003);
    testBench_master.out.write(0x0008); // Transfer 8 words
    testBench_master.write.write(SC_LOGIC_1);
    wait(10, SC_NS);
    testBench_master.write.write(SC_LOGIC_0);
    
    // Start the transfer
    testBench_master.address.write(0x2000);
    testBench_master.out.write(0x05); // WR=0, RD=1, Start=1
    testBench_master.write.write(SC_LOGIC_1);
    wait(10, SC_NS);
    testBench_master.write.write(SC_LOGIC_0);
    
    // Wait for transfer to complete
    wait(1000, SC_NS);
    
    // Check if transfer is done
    testBench_master.address.write(0x2004);
    testBench_master.read.write(SC_LOGIC_1);
    testBench_master.write.write(SC_LOGIC_0);
    wait(10, SC_NS);
    done_status = testBench_master.input.read();
    testBench_master.read.write(SC_LOGIC_0);
    
    cout << "RD Transfer done status: " << done_status.to_string() << endl;
    
    // Test 3: Verify data transfer
    cout << "Test 3: Verifying data transfer" << endl;
    
    // Read some data from destination memory to verify
    for (int i = 0; i < 5; i++) {
        testBench_master.address.write(0x1000 + i);
        testBench_master.read.write(SC_LOGIC_1);
        testBench_master.write.write(SC_LOGIC_0);
        wait(10, SC_NS);
        sc_lv<D_WIDTH> data = testBench_master.input.read();
        testBench_master.read.write(SC_LOGIC_0);
        
        cout << "Destination memory[" << hex << (0x1000 + i) << "] = " << data.to_string() << endl;
    }
    
    // Read some data from source memory (RD operation result)
    for (int i = 0; i < 5; i++) {
        testBench_master.address.write(0x0200 + i);
        testBench_master.read.write(SC_LOGIC_1);
        testBench_master.write.write(SC_LOGIC_0);
        wait(10, SC_NS);
        sc_lv<D_WIDTH> data = testBench_master.input.read();
        testBench_master.read.write(SC_LOGIC_0);
        
        cout << "Source memory[" << hex << (0x0200 + i) << "] = " << data.to_string() << endl;
    }
    
    cout << "DMA testBench completed successfully!" << endl;
    
    // Run simulation for a bit longer to allow memory dumps
    wait(3000, SC_NS);
    
    sc_stop();
}

// Main function
int sc_main(int argc, char* argv[]) {
    cout << "Starting DMA TestBench..." << endl;
    
    // Create testBench instance
    DMA_TestBench<2, 16, 16> testBench("dma_testBench");
    
    // Start simulation
    sc_start();
    
    cout << "Simulation completed!" << endl;
    
    return 0;
} 