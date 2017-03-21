#pragma once
#include "GenTemplate.h"
class CGenCF1 :
	public CGenTemplate
{
public:
	CGenCF1();
	~CGenCF1();
	void Generate() override;
	void LoadConfigLine(CString * sN, CString * sV, int idata, double fdata);

protected:
	// Parameters
	int max_interval = 5; // Maximum diatonic interval in cantus (7 = octave)
	int c_len = 9; // Number of measures in each cantus. Usually 9 to 11
	int first_note = 72; // Starting note of each cantus
	int allow_tritone = 0; // If tritone can be used (correctly prepared and released)
	int allow_sept = 0; // If sept can be used (correctly prepared and released)
	int allow_arp = 0; // If arpedgio can be used (two neighbouring same-direction moves over a third)
	int allow_leap_second_release = 0; // If leap can be released on second melody move
	int allow_joined_leaps = 0; // If joined leaps in different directions are allowed
	int allow_unfilled_leaps = 0; // If leaps can be unfilled
	int max_leaps = 2; // Maximum allowed max_leaps during max_leap_steps
	int max_leap_steps = 7;
	int stag_notes = 2; // Maximum allowed stag_notes (same notes) during stag_note_steps
	int stag_note_steps = 7;
	int min_tempo = 100;
	int max_tempo = 130;
};
