#ifndef MEMORY_WRAPPER_HPP
#define MEMORY_WRAPPER_HPP

#include "bus/slave_module.hpp"
#include "bus/slave_portBundle.hpp"

template <int A_WIDTH>
class memory_wrapper8to32 : public sc_slave_module<A_WIDTH, 32> {
    using addr_t = sc_lv<A_WIDTH>;
    using data8_t = sc_lv<8>;
    using data32_t = sc_lv<32>;

private:

    sc_slave_portBundle<A_WIDTH, 8> memory_port;

    enum State {
        IDLE,
        READING_BYTES,
        WRITING_BYTES,
        DONE
    } current_state;

    addr_t base_address;
    data32_t word_buffer;

    int byte_counter;
    bool is_read_operation;

public:

    SC_HAS_PROCESS(memory_wrapper8to32);
    memory_wrapper8to32(sc_module_name name, sc_trace_file* tf, sc_slave_module<A_WIDTH, 8>* memory_8bit)
        : sc_slave_module<A_WIDTH, 32>(name, tf),
        current_state(IDLE),
        byte_counter(0),
        is_read_operation(false) {

        this->memory = memory_8bit->memory;

        memory_8bit->clk(this->clk);

        memory_port.bind(
            memory_8bit->address,
            memory_8bit->input,
            memory_8bit->out,
            memory_8bit->read,
            memory_8bit->write,
            memory_8bit->chipSelect,
            memory_8bit->ready
        );

        // Initialize memory port
        memory_port.initialize();

        // Add tracing
        std::string prefix = std::string(name) + "_memory";
        sc_trace(tf, current_state, (prefix + "/state").c_str());
        sc_trace(tf, byte_counter, (prefix + "/byte_counter").c_str());
        sc_trace(tf, word_buffer, (prefix + "/word_buffer").c_str());

        SC_THREAD(memory_wrapper_process);
        this->sensitive << this->clk.pos();

        cout << "wrapper instantiate done" << endl;
    }

private:

    void memory_wrapper_process() {
        while (true) {

            switch (current_state) {
            case IDLE:
                handle_idle_state();
                break;
            case READING_BYTES:
                // cout << "READ HANDLE" << endl;
                handle_reading_bytes_state();
                break;
            case WRITING_BYTES:
                handle_writing_bytes_state();
                // cout << "WRITE HANDLE" << endl;
                break;
            case DONE:
                handle_done_state();
                // cout << "READY HANDLE" << endl;
                break;
            }
            wait();
        }
    }

    void handle_idle_state() {

        this->ready = SC_LOGIC_0;

        // cout << "ke?" << endl;
        // check request
        if (this->chipSelect.read() == SC_LOGIC_1 &&
            (this->read.read() == SC_LOGIC_1 || this->write.read() == SC_LOGIC_1)) {

            // cout << "ke." << endl;
            // Word aligned (set 0 tow right bit of address) 
            addr_t requested_addr = this->address.read();
            base_address = get_word_aligned_address(requested_addr);

            byte_counter = 0;
            word_buffer = 0xFFFFFFFF;
            is_read_operation = (this->read.read() == SC_LOGIC_1);

            if (is_read_operation) {
                // cout << "module is selected for read" << endl;

                current_state = READING_BYTES;
                // cout << "go for read" << endl;
            }
            else { // is_write_operation
                // cout << "module is selected for write" << endl;
                data32_t write_data = this->input.read();
                word_buffer = write_data;

                current_state = WRITING_BYTES;
            }
        }
    }

    void handle_reading_bytes_state() {

        for (int i = 0; i < 4; i++) {

            memory_port.chipSelect = SC_LOGIC_1;
            memory_port.read = SC_LOGIC_1;
            memory_port.write = SC_LOGIC_0;
            addr_t byte_addr = base_address.to_uint64() + byte_counter;
            memory_port.address = byte_addr;
            // cout << "address is setted to: " << byte_addr.to_int() << endl;


            // cout << "byte " << i << " :";

            do { wait(); } while (memory_port.ready.read() == SC_LOGIC_1);
            wait(SC_ZERO_TIME);

            data8_t byte_data = memory_port.out.read();

            // cout << memory_port.out.read() << endl;

            insert_byte_to_word(byte_data, byte_counter);

            byte_counter++;

        }

        if (byte_counter == 4)
        {
            memory_port.chipSelect.write(SC_LOGIC_0);
            memory_port.read.write(SC_LOGIC_0);
            current_state = DONE;
        }
    }

    void handle_writing_bytes_state() {

        for (int i = 0; i < 4; i++) {

            data8_t byte_to_write = extract_byte_from_word(byte_counter);
            memory_port.chipSelect = SC_LOGIC_1;
            memory_port.read = SC_LOGIC_0;
            memory_port.write = SC_LOGIC_1;
            memory_port.input = byte_to_write;
            addr_t byte_addr = base_address.to_uint64() + byte_counter;
            memory_port.address.write(byte_addr);
            
            do { wait(); } while (memory_port.ready.read() == SC_LOGIC_1);
            

            byte_counter++;

        }

        if (byte_counter == 4) {
            memory_port.chipSelect.write(SC_LOGIC_0);
            memory_port.write.write(SC_LOGIC_0);
            current_state = DONE;
        }
    }

    void handle_done_state() {

        if (is_read_operation)
            this->out = word_buffer;

        this->ready = SC_LOGIC_1;

        do { wait(); } while (this->chipSelect.read() != SC_LOGIC_0);

        if (this->read.read() == SC_LOGIC_0 && this->write.read() == SC_LOGIC_0)
            current_state = IDLE;
    }

    // Helper functions
    addr_t get_word_aligned_address(const addr_t& byte_addr) {

        addr_t aligned_addr = byte_addr;
        aligned_addr[1] = SC_LOGIC_0;
        aligned_addr[0] = SC_LOGIC_0;
        return aligned_addr;
    }

    void insert_byte_to_word(const data8_t& byte_data, int byte_offset) {

        // cout << "\nthis is what insert to word: " << byte_data.to_int() << endl;
        int start_bit = byte_offset * 8;

        for (int i = 0; i < 8; i++)
            word_buffer[start_bit + i] = byte_data[i];

    }

    data8_t extract_byte_from_word(int byte_offset) {

        data8_t byte_data;
        int start_bit = byte_offset * 8;

        for (int i = 0; i < 8; i++)
            byte_data[i] = word_buffer[start_bit + i];

        return byte_data;
    }
};

#endif // MEMORY_WRAPPER_HPP