// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "SinRand.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

CSinRand::CSinRand(int snum0, float ampl10, float ampl20, float freq10, float freq20) {
	snum = snum0;
	ampl1 = ampl10 / snum;
	ampl2 = ampl20 / snum;
	freq1 = freq10;
	freq2 = freq20;
	freq.resize(snum, (freq1 + freq2) / 2);
	ampl.resize(snum, (ampl1 + ampl2) / 2);
	sign.resize(snum);
	dgr.resize(snum);
	for (int s = 0; s < snum; ++s) {
		sign[s] = s % 2 ? -1 : 1;
	}
	InitRandom();
}

CSinRand::~CSinRand() {
}

float CSinRand::MakeNext() {
	val = 0;
	for (int s = 0; s < snum; s++) {
		val += sign[s] * sin(dgr[s] * PI / 180) * ampl[s];
		dgr[s] = fmod(dgr[s] + freq[s], 360);
		freq[s] += (rand01() * 2 - 1) * dfreq;
		if (freq[s] > freq2) freq[s] = freq2;
		if (freq[s] < freq1) freq[s] = freq1;
		float dgr2 = fmod(dgr[s], 180);
		if ((dgr2 < 10 || dgr2 > 135) && freq[s] > 18) {
			ampl[s] += (rand01() * 2 - 1) * dampl;
			if (ampl[s] > ampl2) ampl[s] = ampl2;
			if (ampl[s] < ampl1) ampl[s] = ampl1;
		}
	}
	return val;
}
