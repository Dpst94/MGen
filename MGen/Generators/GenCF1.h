#pragma once
#include "CF1Ly.h"

class CGenCF1 :
	public CF1Ly
{
public:
	CGenCF1();
	~CGenCF1();
	void Generate() override;

protected:
	int InitGen();
	int InitCantus();
	void TestDiatonic();
	void CheckSASEmulatorFlags(vector<int> &cc);
	void EmulateSAS();
	void RandomSWA();
	void SWA(int i, int dp);
	void LogPerf();

};
