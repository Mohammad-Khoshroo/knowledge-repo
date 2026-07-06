#include <iostream>
#include <systemc.h>
#include <string>
#include <PIC8259.h>

SC_MODULE(PICWrapper)
{
    sc_in<sc_logic> clk;
    sc_in<sc_logic> dmaInterrupt;
    sc_in<sc_logic> mmaInterrupt;
    sc_in<sc_logic> INTA_bar;
    sc_out<sc_logic> INT;


    sc_in<sc_logic> s_cs;    
    sc_in<sc_lv<16>> s_addr; 
    sc_in<sc_lv<16>> s_in; 
    sc_out<sc_lv<16>> s_out;
    sc_in<sc_logic> s_wr;
    sc_in<sc_logic> s_rd;
    sc_out<sc_logic> s_ready;


    sc_signal<sc_lv<8>> IR;
    sc_signal<sc_logic> RD_bar, WR_bar, CS_bar;
    sc_signal<sc_lv<16>> PIC_input;
   
    sc_signal<sc_lv<16>> PIC_output;

    PIC8259* pic;
    SC_CTOR(PICWrapper)
    {
        pic = new PIC8259("pic");
        pic->clk(clk);
        pic->IR(IR);
        pic->INTA_bar(INTA_bar);
        pic->RD_bar(RD_bar);
        pic->WR_bar(WR_bar);
        pic->CS_bar(CS_bar);
        pic->PIC_input(PIC_input);
        pic->INT(INT);
        pic->PIC_output(PIC_output);

        SC_THREAD(eval);
        sensitive << dmaInterrupt << mmaInterrupt << s_cs << s_addr << PIC_output << s_in << s_wr << s_rd;
    }
    void eval()
    {
        while(1)
        {
            IR = (sc_lv<6>("000000"),dmaInterrupt,mmaInterrupt);
            RD_bar = ~s_rd.read();
            WR_bar = ~s_wr.read();
            CS_bar = ~s_cs.read();
            PIC_input = s_in;
            s_out = PIC_output;
            s_ready = sc_logic_1;
            wait();
        }
    }
};