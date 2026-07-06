#include <iostream>
#include <systemc.h>
#include <string>
#include <IssPowerV2.h>

SC_MODULE(SayacInterface)
{

    sc_in<sc_logic> clk;

    // Master port for accessing the bus
    sc_out<sc_lv<16>> m_addr;
    sc_in<sc_lv<16>> m_in;
    sc_out<sc_lv<16>> m_out;
    sc_out<sc_logic> m_wr;
    sc_out<sc_logic> m_rd;
    sc_in<sc_logic> m_ready;

    sc_in<sc_logic> INT;
    sc_out<sc_logic> INTA_bar;

    sayacInstruction<16, 4, 16, 3> *sayac;

    sc_signal <sc_logic> memReady;


	sc_signal <sc_lv<16>> dataBus;
	sc_signal <sc_lv<16>> dataBusOut;

	sc_signal <sc_logic> readMem, writeMem;
	sc_signal <sc_logic> readIO, writeIO;
	sc_signal <sc_lv<16>> addrBus;

    SC_CTOR(SayacInterface)
    {
        sayac = new sayacInstruction<16, 4, 16, 3>("sayac");
        sayac->clk(clk);
        sayac->memReady(m_ready);
        sayac->interrupt(INT);
        sayac->dataBus(m_in);
        sayac->dataBusOut(m_out);
        sayac->readMem(m_rd);
        sayac->writeMem(m_wr);
        sayac->addrBus(m_addr);
        sayac->INTA_bar(INTA_bar);

        sayac->readIO(readIO);
        sayac->writeIO(writeIO);
    }

};