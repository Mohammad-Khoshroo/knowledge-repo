#include <systemc.h>
#include <iostream>
#include <fstream>
#include <string>
#include "Bus.h"

template <int N>
SC_MODULE(Memory)
{
	int DebugON;
	int Loading;
	int StartingLocation;
	int PuttingData;
	std::string file;

	sc_in<sc_logic> clk;

	sc_in<sc_logic> s_cs;
	sc_in<sc_lv<N>> s_addr;
	sc_in<sc_lv<N>> s_in;
	sc_out<sc_lv<N>> s_out;
	sc_in<sc_logic> s_wr;
	sc_in<sc_logic> s_rd;
	sc_out<sc_logic> s_ready;

	int memRange;
	sc_lv<N> *mem;

	SC_CTOR(Memory)
	{

		memRange = int(pow(2, N));
		mem = new sc_lv<N>[memRange];

		SC_THREAD(init);
		SC_METHOD(readMem);
		sensitive << s_addr << s_cs << s_rd;
		SC_METHOD(writeMem);
		sensitive << clk.pos();
		SC_THREAD(dump);
		SC_METHOD(setMemReady);
		sensitive << s_addr << s_cs << s_rd << s_wr;
	}
	void init()
	{
		int i = 0;
		sc_lv<N> data;
		ifstream initFile;
		ifstream initFileLoad;
		ifstream PutAddr;
		ifstream PutData;
		initFile.open("PasswordBinaryV2.txt");
		initFileLoad.open(file);
		PutAddr.open("addr.txt");
		PutData.open("data.txt");
		cout << "starting location: " << StartingLocation << endl;
		if (Loading)
		{

			int count = 0;

			while (!(initFileLoad.eof()))
			{
				i = 0;
				if (i < memRange)
				{
					if (count == StartingLocation)
					{
						initFileLoad >> data;
						mem[i] = data;
						//	cout << "data is  " << mem[i] << endl;
						i++;
					}
					else
					{
						initFileLoad >> data;
						count++;
					}
				}
			}
			initFileLoad.close();
			if (DebugON)
			{
				cout << "Loading to MEM *************************************************************\n";
			}
		}

		if (PuttingData)
		{

			int addr;
			std::string addr_s, data_s;
			int data_i;
			while (getline(PutAddr, addr_s))
			{
				i = 0;
				if (i < memRange)
				{

					PutData >> data;
					addr = std::stoi(addr_s);
					mem[addr] = data;
					//	cout << "addr and data is: "<< addr<<"  "<<mem[addr]<<endl;

					i++;
				}
			}

			PutAddr.close();
			PutData.close();
			if (DebugON)
			{
				cout << "Putting Data to MEM *************************************************************\n";
			}
		}

		else if (!(Loading) && !(PuttingData))
		{
			cout << "wow" << endl;
			int count = 0;
			while (!(initFile.eof()))
			{
				if (i < memRange)
				{

					initFile >> data;
					mem[i] = data;
					// cout << "data is  " << mem[i] << endl;
					i++;
				}
			}
			initFile.close();
		}
	}

	void readMem()
	{
		sc_lv<N> tempAdr;
		tempAdr = s_addr;
		s_out = 0;
		if (s_cs == '1')
		{
			if (s_rd == '1')
			{
				if (tempAdr.to_uint() < memRange)
				{
					s_out = mem[tempAdr.to_uint()];
				}
			}
		}
	}
	void writeMem()
	{
		sc_lv<N> tempAd;

		if (s_cs == '1')
		{
			tempAd = s_addr;
			if (tempAd.to_uint() < memRange)
			{
				if (s_wr == '1')
				{
					mem[tempAd.to_uint()] = s_in;
				}
			}
		}
	}
	void dump()
	{
		ofstream out;
		wait(30, SC_NS);
		out.open("dump.txt");
		for (int i = 0; i < memRange; i++)
		{
			out << i << "\t" << mem[i] << endl;
		}
		out.close();
	}
	void setMemReady()
	{
		sc_lv<N> tempAd;
		s_ready = SC_LOGIC_0;
		// cout << "ready Ready is " << ready << "\n";
		if (s_cs == '1')
		{
			tempAd = s_addr;
			if (tempAd.to_uint() < memRange)
			{
				if (s_wr == '1' || s_rd == '1')
				{
					s_ready = SC_LOGIC_1;
				}
			}
		}
	}
};
