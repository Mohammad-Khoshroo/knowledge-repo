#include <systemc.h>
#include <iostream>
#include "Bus.h"
#include "Memory.h"
#include "DMA.cpp"
#include "MatMulAcc.cpp"
#include "DummyProcessor.h"
#include "PICWrapper.h"
#include "SayacInterface.h"

#define NumberOfSlaves 4
#define NumberOfMasters 2
template <int N>
SC_MODULE(EmbeddedSystem)
{
public:
    sc_in<sc_logic> clk;

    Bus<N, NumberOfMasters, NumberOfSlaves> *bus;
    MatMulAcc<N> *mma;
    DMA *dma;
    Memory<N> *memory;
    DummyProcessor<N> *dp;
    PICWrapper* pic;
    SayacInterface* cpu;


    sc_signal<sc_lv<N>> m_addr[NumberOfMasters];
    sc_signal<sc_lv<N>> m_in[NumberOfMasters];
    sc_signal<sc_lv<N>> m_out[NumberOfMasters];
    sc_signal<sc_logic> m_wr[NumberOfMasters];
    sc_signal<sc_logic> m_rd[NumberOfMasters];
    sc_signal<sc_logic> m_ready[NumberOfMasters];

    sc_signal<sc_logic> s_cs[NumberOfSlaves];
    sc_signal<sc_lv<N>> s_addr[NumberOfSlaves];
    sc_signal<sc_lv<N>> s_in[NumberOfSlaves];
    sc_signal<sc_lv<N>> s_out[NumberOfSlaves];
    sc_signal<sc_logic> s_wr[NumberOfSlaves];
    sc_signal<sc_logic> s_rd[NumberOfSlaves];
    sc_signal<sc_logic> s_ready[NumberOfSlaves];

    // MMA to DMA signals
    sc_signal<sc_lv<N>> dmammaAddr;
    sc_signal<sc_lv<N>> dmammaIn;
    sc_signal<sc_lv<N>> dmammaOut;
    sc_signal<sc_logic> dmammaWR;
    sc_signal<sc_logic> dmammaRD;

    // PIC signals
    sc_signal<sc_logic> dmaInterrupt;
    sc_signal<sc_logic> mmaInterrupt;
    sc_signal<sc_logic> INTA_bar;
    sc_signal<sc_logic> INT;
   

    SC_CTOR(EmbeddedSystem)
    {
        bus = new Bus<N, NumberOfMasters, NumberOfSlaves>("bus");
        bus->clk(clk);

        // 0 -> cpu
        bus->m_addr[0](m_addr[0]);
        bus->m_in[0](m_in[0]);
        bus->m_out[0](m_out[0]);
        bus->m_wr[0](m_wr[0]);
        bus->m_rd[0](m_rd[0]);
        bus->m_ready[0](m_ready[0]);

        //1 -> dma
        bus->m_addr[1](m_addr[1]);
        bus->m_in[1](m_in[1]);
        bus->m_out[1](m_out[1]);
        bus->m_wr[1](m_wr[1]);
        bus->m_rd[1](m_rd[1]);
        bus->m_ready[1](m_ready[1]);


        dp = new DummyProcessor<N>("CPU");
        dp->clk(clk);
        dp->m_addr(m_addr[0]);
        dp->m_in(m_in[0]);
        dp->m_out(m_out[0]);
        dp->m_wr(m_wr[0]);
        dp->m_rd(m_rd[0]);
        dp->m_ready(m_ready[0]);
        

        // cpu = new SayacInterface("CPU");
        // cpu->clk(clk);
        // cpu->m_addr(m_addr[0]);
        // cpu->m_in(m_in[0]);
        // cpu->m_out(m_out[0]);
        // cpu->m_wr(m_wr[0]);
        // cpu->m_rd(m_rd[0]);
        // cpu->m_ready(m_ready[0]);
        // cpu->INT(INT);
        // cpu->INTA_bar(INTA_bar);

    

        memory = new Memory<N>("mem");
        memory->clk(clk);
        memory->s_cs(s_cs[2]);
        memory->s_addr(s_addr[2]);
        memory->s_in(s_in[2]);
        memory->s_out(s_out[2]);
        memory->s_wr(s_wr[2]);
        memory->s_rd(s_rd[2]);
        memory->s_ready(s_ready[2]);
        // Starting at 0x0000
        bus->s_cs[2](s_cs[2]);
        bus->s_addr[2](s_addr[2]);
        bus->s_in[2](s_in[2]);
        bus->s_out[2](s_out[2]);
        bus->s_wr[2](s_wr[2]);
        bus->s_rd[2](s_rd[2]);
        bus->s_ready[2](s_ready[2]);
        bus->startAddress[2] = 0x0000; //MMLocation
        bus->sizeAddress[2] = 0x8000; //MMSize

        // MMA
        mma = new MatMulAcc<N>("mma");
        mma->clk(clk);
        mma->dmaAddr(dmammaAddr);
        mma->dmaIn(dmammaIn);
        mma->dmaOut(dmammaOut);
        mma->dmaWR(dmammaWR);
        mma->dmaRD(dmammaRD);
        mma->interrupt(mmaInterrupt);

        mma->s_cs(s_cs[0]);
        mma->s_addr(s_addr[0]);
        mma->s_in(s_in[0]);
        mma->s_out(s_out[0]);
        mma->s_wr(s_wr[0]);
        mma->s_rd(s_rd[0]);
        mma->s_ready(s_ready[0]);
        // Starting at 0x8000
        bus->s_cs[0](s_cs[0]);
        bus->s_addr[0](s_addr[0]);
        bus->s_in[0](s_in[0]);
        bus->s_out[0](s_out[0]);
        bus->s_wr[0](s_wr[0]);
        bus->s_rd[0](s_rd[0]);
        bus->s_ready[0](s_ready[0]);

        bus->startAddress[0] = 0x8000;
        bus->sizeAddress[0] = 8; 

        // DMA
        dma = new DMA("dma");
        dma->clk(clk);
        dma->mmaAddr(dmammaAddr);
        dma->mmaIn(dmammaIn);
        dma->mmaOut(dmammaOut);
        dma->mmaWR(dmammaWR);
        dma->mmaRD(dmammaRD);
        dma->interrupt(dmaInterrupt);


        dma->m_addr(m_addr[1]);
        dma->m_in(m_in[1]);
        dma->m_out(m_out[1]);
        dma->m_wr(m_wr[1]);
        dma->m_rd(m_rd[1]);
        dma->m_ready(m_ready[1]);

        dma->s_cs(s_cs[1]);
        dma->s_addr(s_addr[1]);
        dma->s_in(s_in[1]);
        dma->s_out(s_out[1]);
        dma->s_wr(s_wr[1]);
        dma->s_rd(s_rd[1]);
        dma->s_ready(s_ready[1]);
        // Starting at 0x8008
        bus->s_cs[1](s_cs[1]);
        bus->s_addr[1](s_addr[1]);
        bus->s_in[1](s_in[1]);
        bus->s_out[1](s_out[1]);
        bus->s_wr[1](s_wr[1]);
        bus->s_rd[1](s_rd[1]);
        bus->s_ready[1](s_ready[1]);

        bus->startAddress[1] = 0x8008;
        bus->sizeAddress[1] = 8;

        pic = new PICWrapper("picWrapper");
        pic->clk(clk);
        pic->INTA_bar(INTA_bar);
        pic->INT(INT);
        pic->dmaInterrupt(dmaInterrupt);
        pic->mmaInterrupt(mmaInterrupt);

        pic->s_cs(s_cs[3]);
        pic->s_addr(s_addr[3]);
        pic->s_in(s_in[3]);
        pic->s_out(s_out[3]);
        pic->s_wr(s_wr[3]);
        pic->s_rd(s_rd[3]);
        pic->s_ready(s_ready[3]);
        // Starting at 0x8010
        bus->s_cs[3](s_cs[3]);
        bus->s_addr[3](s_addr[3]);
        bus->s_in[3](s_in[3]);
        bus->s_out[3](s_out[3]);
        bus->s_wr[3](s_wr[3]);
        bus->s_rd[3](s_rd[3]);
        bus->s_ready[3](s_ready[3]);

        bus->startAddress[3] = 0x8010;
        bus->sizeAddress[3] = 1;
    }
};
