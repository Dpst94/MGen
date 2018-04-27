#pragma once

#include "GLib.h"

class CSmoothRandom :
	public CGLib
{
public:
	CSmoothRandom(int inter_steps0 = 1);
	~CSmoothRandom();

	float MakeNext();
	float MakeNextInter();

	void SetInter(int inter_steps0);

	// Variables
	float acc2 = 0; // Second derivative of velocity
	float acc = 0; // First derivative of velocity (acceleration)
	float vel = 0; // Velocity
	float sig = 0; // Signal

	// Interpolation
	int inter_steps = 1;
	int step = 0;
	float inter_sig = 0;
	float prev_inter_sig = 0;

	// Constants
	float a2_range;
	float a_range;
	float v_range;
	float s_range;

	// Cycles
	int cycles;

	// Gravitation
	float v_grav; // 1 = maximum gravity, 0 = no gravity
	float s_grav; // 1 = maximum gravity, 0 = no gravity
	float a_grav; // 1 = maximum gravity, 0 = no gravity
};

