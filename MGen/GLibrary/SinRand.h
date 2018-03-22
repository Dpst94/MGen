#pragma once

#include "GLib.h"

class CSinRand :
	public CGLib
{
public:
	CSinRand(int snum0, float ampl10, float ampl20, float freq10, float freq20);
	~CSinRand();

	float MakeNext();

	// Variables
	vector<float> freq; // Sin frequency
	vector<float> ampl; // Sin amplitude
	vector<float> sign; // Sin sign
	vector<float> dgr; // Sin degree
	float val = 0; // Resulting value

  // Constants
	int snum = 2; // Number of sin
	float freq1 = 1;
	float freq2 = 100;
	float ampl1 = 3;
	float ampl2 = 100;
	float dfreq = 1;
	float dampl = 0.1;
};

