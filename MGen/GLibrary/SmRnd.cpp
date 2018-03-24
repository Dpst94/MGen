// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "SmRnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

CSmoothRandom::CSmoothRandom(int inter_steps0) {
	inter_steps = inter_steps0;
	a2_range = 0.03;
	a_range = 0.3;
	v_range = 3;
	s_range = 100;
	sig = 0;
	cycles = 6;
	v_grav = 1; // 1 = maximum gravity, 0 = no gravity
	s_grav = 1; // 1 = maximum gravity, 0 = no gravity
	a_grav = 1; // 1 = maximum gravity, 0 = no gravity
	InitRandom();
}

CSmoothRandom::~CSmoothRandom() {
}

float CSmoothRandom::MakeNext() {
	for (int i = 0; i < cycles; i++) {
		float a2_max = a2_range;
		float a2_min = -a2_range;
		float a_max = a_range;
		float a_min = -a_range;
		// Generate a2 range
		if (acc > 0) a2_max = a2_range * pow((a_range - acc) / a_range, 0.3) * a_grav;
		if (acc < 0) a2_min = -a2_range * pow((a_range + acc) / a_range, 0.3) * a_grav;
		// Create a2
		acc2 = rand01() * (a2_max - a2_min) + a2_min;
		// Calculate resistance
		float acc20 = 0;
		if (acc2 > 0 && vel > 0) acc20 = -a2_range * pow(abs(vel) / v_range, 4) * v_grav;
		if (acc2 < 0 && vel < 0) acc20 = a2_range * pow(abs(vel) / v_range, 4) * v_grav;
		if (acc2 + acc20 > a2_max) acc20 = a2_max - acc2;
		if (acc2 + acc20 < a2_min) acc20 = a2_min - acc2;
		// Calculate acceleration
		acc += acc2 + acc20;
		// Correct a2
		if (acc > a_max) acc = a_max;
		if (acc < a_min) acc = a_min;
		// Calculate gravity
		float acc0 = 0;
		if (acc > 0 && sig > 0) acc0 = -a_range * pow(abs(sig) / s_range, 4) * s_grav;
		if (acc < 0 && sig < 0) acc0 = a_range * pow(abs(sig) / s_range, 4) * s_grav;
		if (acc + acc0 > a_max) acc0 = a_max - acc;
		if (acc + acc0 < a_min) acc0 = a_min - acc;
		// Calculate velocity
		vel += acc + acc0;
		if (vel > v_range) {
			vel = v_range;
			if (acc > 0) acc = -acc / 4;
		}
		if (vel < -v_range) {
			vel = -v_range;
			if (acc < 0) acc = -acc / 4;
		}
		// Calculate signal
		sig += vel;
		if (sig > s_range) {
			sig = s_range;
			if (acc > 0) acc = -acc / 4;
			if (vel > 0) vel = -vel / 4;
		}
		if (sig < -s_range) {
			sig = -s_range;
			if (acc < 0) acc = -acc / 4;
			if (vel < 0) vel = -vel / 4;
		}
	}
	return sig;
}

float CSmoothRandom::MakeNextInter() {
	int sr_i = step % inter_steps;
	if (!sr_i) {
		prev_inter_sig = sig;
		MakeNext();
	}
	inter_sig = prev_inter_sig * (inter_steps - sr_i) / inter_steps +
		sig * (sr_i) / inter_steps;
	++step;
	return 0.0f;
}
