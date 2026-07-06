#include <systemc.h>
#include "DMA.hpp"
#include "slave_module.hpp"
#include "master_module.hpp"

// حافظه ساده به صورت اسلیو (خواندن و نوشتن با آماده بودن)
template<int A_WIDTH, int D_WIDTH>
struct SimpleMemory : public sc_slave_module<A_WIDTH, D_WIDTH> {
    std::vector<sc_lv<D_WIDTH>> mem;

    SC_CTOR(SimpleMemory) : sc_slave_module<A_WIDTH, D_WIDTH>("SimpleMemory", nullptr, 0, 64) {
        mem.resize(64, sc_lv<D_WIDTH>("0"));

        SC_THREAD(process);
        sensitive << clk.pos();
    }

    void process() {
        while (true) {
            wait(); // منتظر لبه کلاک مثبت
            if (this->chipSelect == SC_LOGIC_1) {
                if (this->write == SC_LOGIC_1) {
                    uint16_t addr = sc_cast::sc_lv_cast<uint16_t>(this->address);
                    if (addr < mem.size()) {
                        mem[addr] = this->input.read();
                        this->ready = SC_LOGIC_1;
                    }
                }
                else if (this->read == SC_LOGIC_1) {
                    uint16_t addr = sc_cast::sc_lv_cast<uint16_t>(this->address);
                    if (addr < mem.size()) {
                        this->out = mem[addr];
                        this->ready = SC_LOGIC_1;
                    }
                }
                else
                    this->ready = SC_LOGIC_0;
            }
            else {
                this->ready = SC_LOGIC_0;
            }
        }
    }
};

// مصرف‌کننده ساده که داده رو دریافت می‌کنه و می‌نویسه (شبیه یک slave مصرف‌کننده)
template<int A_WIDTH, int D_WIDTH>
struct SimpleConsumer : public sc_slave_module<A_WIDTH, D_WIDTH> {
    std::vector<sc_lv<D_WIDTH>> buffer;

    SC_CTOR(SimpleConsumer) : sc_slave_module<A_WIDTH, D_WIDTH>("SimpleConsumer", nullptr, 0, 64) {
        buffer.resize(64, sc_lv<D_WIDTH>("0"));

        SC_THREAD(process);
        sensitive << clk.pos();
    }

    void process() {
        while (true) {
            wait();
            if (this->chipSelect == SC_LOGIC_1 && this->write == SC_LOGIC_1) {
                uint16_t addr = sc_cast::sc_lv_cast<uint16_t>(this->address);
                if (addr < buffer.size()) {
                    buffer[addr] = this->input.read();
                    this->ready = SC_LOGIC_1;
                }
            }
            else
                this->ready = SC_LOGIC_0;
        }
    }
};

SC_MODULE(Testbench) {
    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<sc_logic> interrupt;

    DMA<1, 16, 8> dma{"dma", nullptr};

    SimpleMemory<16, 8> memory{"memory"};
    SimpleConsumer<16, 8> consumer{"consumer"};

    // سیگنال‌های وصل برای حافظه
    sc_signal<sc_logic> mem_cs, mem_read, mem_write, mem_ready;
    sc_signal<sc_lv<16>> mem_address;
    sc_signal<sc_lv<8>> mem_data_in, mem_data_out;

    // سیگنال‌های وصل برای مصرف‌کننده
    sc_signal<sc_logic> cons_cs, cons_read, cons_write, cons_ready;
    sc_signal<sc_lv<16>> cons_address;
    sc_signal<sc_lv<8>> cons_data_in, cons_data_out;

    void bind_signals() {
        // حافظه
        memory.clk(clk);
        memory.chipSelect(mem_cs);
        memory.read(mem_read);
        memory.write(mem_write);
        memory.address(mem_address);
        memory.input(mem_data_in);
        memory.out(mem_data_out);
        memory.ready(mem_ready);

        // مصرف‌کننده
        consumer.clk(clk);
        consumer.chipSelect(cons_cs);
        consumer.read(cons_read);
        consumer.write(cons_write);
        consumer.address(cons_address);
        consumer.input(cons_data_in);
        consumer.out(cons_data_out);
        consumer.ready(cons_ready);

        // DMA کانفیگ
        dma.clk(clk);
        dma.configModule.clk(clk);
        dma.configModule.chipSelect(cs);
        dma.configModule.read(read);
        dma.configModule.write(write);
        dma.configModule.address(address);
        dma.configModule.input(data_in);
        dma.configModule.out(data_out);
        dma.configModule.ready(ready);

        // DMA توزیع کننده (distributerModule)
        dma.distributerModule.clk(clk);
        dma.interrupt(interrupt);

        // وصل کردن master DMA به حافظه یا مصرف کننده
        // این بستگی به مد دارد (Mem->MMA یا بالعکس) باید به درستی کانکت کنی.
        // فرض کنیم dma.master مربوط به حافظه است:
        dma.distributerModule.address(mem_address);
        dma.distributerModule.input(mem_data_in);
        dma.distributerModule.out(mem_data_out);
        dma.distributerModule.ready(mem_ready);
        dma.distributerModule.write(mem_write);
        dma.distributerModule.read(mem_read);

        // مصرف‌کننده هم همین سیگنال‌ها را بگیرند (مگر اینکه بخواهی جداگانه)
        dma.distributerModule.receiverAddress(cons_address);
        dma.distributerModule.receiverInput(cons_data_in);
        dma.distributerModule.receiverOut(cons_data_out);
        dma.distributerModule.receiverReady(cons_ready);
        dma.distributerModule.receiverWrite(cons_write);
        dma.distributerModule.receiverRead(cons_read);
    }

    // کانفیگ DMA و راه‌اندازی تست
    void config_dma() {
        wait(50, SC_NS);

        // نوشتن کانفیگ (مثال انتقال 5 کلمه از حافظه 0x0 به مصرف‌کننده 0x0)
        cs.write(SC_LOGIC_1);
        write.write(SC_LOGIC_1);
        address.write(0);
        data_in.write(5);
        wait(clk.posedge_event());

        address.write(1);
        data_in.write(0);
        wait(clk.posedge_event());

        address.write(2);
        data_in.write(0);
        wait(clk.posedge_event());

        address.write(3);
        data_in.write(0x2); // فرض کنید bit1 = WR
        wait(clk.posedge_event());

        cs.write(SC_LOGIC_0);
        write.write(SC_LOGIC_0);

        // منتظر interrupt بمان
        do {
            wait(interrupt.value_changed_event());
        } while (interrupt.read() != SC_LOGIC_1);

        cout << "DMA transfer (Mem->Consumer) done at " << sc_time_stamp() << endl;

        sc_stop();
    }

    // سیگنال‌های DMA config
    sc_signal<sc_logic> cs, read, write;
    sc_signal<sc_lv<16>> address;
    sc_signal<sc_lv<8>> data_in, data_out;
    sc_signal<sc_logic> ready;

    SC_CTOR(Testbench) : dma("dma", nullptr), memory("memory"), consumer("consumer") {
        bind_signals();

        SC_THREAD(config_dma);
        sensitive << clk.pos();
    }
};

int sc_main(int argc, char* argv[]) {
    Testbench tb{"tb"};
    sc_start();
    return 0;
}
 