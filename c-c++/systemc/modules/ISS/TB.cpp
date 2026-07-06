#include <iostream>
#include <systemc.h>
#include <bits/stdc++.h>
#include "System.h"

SC_MODULE(systemTest)
{
	sc_signal <sc_logic> clk;


	EmbeddedSystem<16> *systemModule;

	SC_CTOR(systemTest)
	{
		systemModule = new EmbeddedSystem<16>("systemModule");
		(*systemModule)(clk);

		SC_THREAD (clocking);
		sensitive << clk;
	}

	void clocking();
};
void systemTest::clocking()
{
	clk = SC_LOGIC_0;
	while (true)
	{
		wait(2.5,SC_NS);
		clk = SC_LOGIC_0;
		wait(2.5,SC_NS);
		clk = SC_LOGIC_1;
	}
}


int sc_main(int argc, char *argv[])
{
	
	sc_report_handler::set_actions (SC_ID_VECTOR_CONTAINS_LOGIC_VALUE_,
                                SC_DO_NOTHING);
	sc_report_handler::set_actions (SC_WARNING, SC_DO_NOTHING);


	systemTest * TOP = new systemTest ("systemTest_TB");
	sc_trace_file* VCDFile;
	VCDFile = sc_create_vcd_trace_file("wave");
	sc_trace(VCDFile, TOP -> clk, "clk");
	sc_trace(VCDFile, TOP->systemModule->mma->controlReg, "mmaControlReg");
	sc_trace(VCDFile, TOP->systemModule->mma->dmaOut, "mmaDMAOut");
	sc_trace(VCDFile, TOP->systemModule->mma->dmaIn, "mmaDMAIn");

	sc_trace(VCDFile, TOP->systemModule->bus->req, "busReq");

	sc_trace(VCDFile, TOP->systemModule->dma->s_cs, "dmaConfigPortCS");
	sc_trace(VCDFile, TOP->systemModule->dma->s_in, "dmaConfigPortIn");
	sc_trace(VCDFile, TOP->systemModule->dma->s_out, "dmaConfigPortOut");
	sc_trace(VCDFile, TOP->systemModule->dma->s_addr, " dmaConfigPortAddr");
	sc_trace(VCDFile, TOP->systemModule->dma->s_rd, "dmaConfigPortRD");
	sc_trace(VCDFile, TOP->systemModule->dma->s_wr, "dmaConfigPortWR");
	sc_trace(VCDFile, TOP->systemModule->dma->s_ready, "dmaConfigPortReady");


	sc_trace(VCDFile, TOP->systemModule->memory->s_cs, " memPortCS");
	sc_trace(VCDFile, TOP->systemModule->memory->s_in, " memPortIn");
	sc_trace(VCDFile, TOP->systemModule->memory->s_out, " memPortOut");
	sc_trace(VCDFile, TOP->systemModule->memory->s_addr, " memPortAddr");
	sc_trace(VCDFile, TOP->systemModule->memory->s_rd, " memPortRD");
	sc_trace(VCDFile, TOP->systemModule->memory->s_wr, " memPortWR");
	sc_trace(VCDFile, TOP->systemModule->memory->s_ready, " memPortReady");


	// sc_trace(VCDFile, TOP->systemModule->cpu->m_in, "cpuMasterIn");
	// sc_trace(VCDFile, TOP->systemModule->cpu->m_addr, "cpuMasterAddr");
	// sc_trace(VCDFile, TOP->systemModule->cpu->m_out, "cpuMasterOut");
	// sc_trace(VCDFile, TOP->systemModule->cpu->m_rd, "cpuMasterRD");
	// sc_trace(VCDFile, TOP->systemModule->cpu->m_wr, "cpuMasterWR");
	// sc_trace(VCDFile, TOP->systemModule->cpu->m_ready, "cpuMasterReady");

	sc_trace(VCDFile, TOP->systemModule->dp->m_in, "cpuMasterIn");
	sc_trace(VCDFile, TOP->systemModule->dp->m_addr, "cpuMasterAddr");
	sc_trace(VCDFile, TOP->systemModule->dp->m_out, "cpuMasterOut");
	sc_trace(VCDFile, TOP->systemModule->dp->m_rd, "cpuMasterRD");
	sc_trace(VCDFile, TOP->systemModule->dp->m_wr, "cpuMasterWR");
	sc_trace(VCDFile, TOP->systemModule->dp->m_ready, "cpuMasterReady");

	sc_trace(VCDFile, TOP->systemModule->dma->interrupt, "dmaInt");
	sc_trace(VCDFile, TOP->systemModule->dma->m_in, "dmaMasterIn");
	sc_trace(VCDFile, TOP->systemModule->dma->m_addr, "dmaMasterAddr");
	sc_trace(VCDFile, TOP->systemModule->dma->m_out, "dmaMasterOut");
	sc_trace(VCDFile, TOP->systemModule->dma->m_rd, "dmaMasterRD");
	sc_trace(VCDFile, TOP->systemModule->dma->m_wr, "dmaMasterWR");
	sc_trace(VCDFile, TOP->systemModule->dma->m_ready, "dmaMasterReady");

	sc_trace(VCDFile, TOP->systemModule->pic->INT, "picINT");
	sc_trace(VCDFile, TOP->systemModule->pic->PIC_output, "picOUT");
	sc_trace(VCDFile, TOP->systemModule->pic->IR, "picIR");

	for(int i = 0;i<2;i++){
		sc_trace(VCDFile, TOP->systemModule->bus->m_req[i], "busMasterReq"+std::to_string(i));
		sc_trace(VCDFile, TOP->systemModule->bus->m_in[i], "busMasterIn"+std::to_string(i));
		sc_trace(VCDFile, TOP->systemModule->bus->m_addr[i], "busMasterAddr"+std::to_string(i));
		sc_trace(VCDFile, TOP->systemModule->bus->m_out[i], "busMasterOut"+std::to_string(i));
		sc_trace(VCDFile, TOP->systemModule->bus->m_rd[i], "busMasterRD"+std::to_string(i));
		sc_trace(VCDFile, TOP->systemModule->bus->m_wr[i], "busMasterWR"+std::to_string(i));
		sc_trace(VCDFile, TOP->systemModule->bus->m_ready[i], "busMasterReady"+std::to_string(i));
	}

    sc_start(10000,SC_NS);
	
	return 0;
}