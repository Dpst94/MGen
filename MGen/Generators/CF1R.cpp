// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "CF1R.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

CF1R::CF1R() {
}

CF1R::~CF1R() {
}

// Detect repeating notes. Step2 excluding
int CF1R::FailNoteRepeat(vector<int> &cc, int step1, int step2) {
	for (int i = step1; i < step2; ++i) {
		if (cc[i] == cc[i + 1]) FLAG2(30, i);
	}
	return 0;
}

// Detect prohibited note sequences
int CF1R::FailNoteSeq(vector<int> &pc) {
	CHECK_READY(DR_fli);
	for (int x = 0; x < fli_size - 2; ++x) {
		s = fli[x];
		s1 = fli[x + 1];
		// Prohibit GC before cadence
		//if (pc[s] == 4 && pc[s1] == 0) FLAG2(48, s);
	}
	return 0;
}

int CF1R::FailLocalRange(vector<int> &c, int notes, int mrange, int flag) {
	CHECK_READY(DR_fli, DR_c);
	// Do not test if flag disabled and not testing
	if (task != tEval && accept[flag] == -1) return 0;
	// Do not test if not enough notes. If melody is short, than global range check is enough
	if (fli_size < notes) return 0;
	int lmin, lmax, s;
	int ls_max = fli_size - notes;
	int ls_max2;
	int fired = 0;
	// Loop through windows
	for (ls = 0; ls <= ls_max; ++ls) {
		lmin = MAX_NOTE;
		lmax = 0;
		ls_max2 = ls + notes;
		// Loop inside each window
		for (int ls2 = ls; ls2 < ls_max2; ++ls2) {
			s = fli[ls2];
			if (c[s] < lmin) lmin = c[s];
			if (c[s] > lmax) lmax = c[s];
		}
		// Check range
		if (lmax - lmin < mrange) {
			if (fired) {
				fpenalty[flag] += severity[flag] + 1;
			}
			else {
				FLAG2L(flag, fli[ls], fli[ls_max2 - 1]);
				fired = 1;
			}
		}
	}
	return 0;
}

int CF1R::FailLocalPiCount(vector<int> &cc, int notes, int picount, int flag) {
	CHECK_READY(DR_fli, DR_nmin);
	// Do not test if flag disabled and not testing
	if (task != tEval && accept[flag] == -1) return 0;
	// Do not test if not enough notes
	if (fli_size < notes) return 0;
	int picount2, i;
	// Clear nstat
	for (i = nmin; i <= nmax; ++i) nstat[i] = 0;
	for (ls = 0; ls < fli_size; ++ls) {
		s = fli[ls];
		// Add new note to stagnation array
		++nstat[cc[s]];
		// Subtract old note
		if (ls >= notes) --nstat[cc[fli[ls - notes]]];
		// Check if little pitches
		if (ls >= notes - 1) {
			picount2 = 0;
			for (i = nmin; i <= nmax; ++i) if (nstat[i]) ++picount2;
			if (picount2 < picount) FLAG2L(flag, fli[ls - notes + 1], fli[ls]);
		}
	}
	return 0;
}

// Moving average
void CF1R::maVector(vector<float> &v, vector<float> &v2, int range) {
	int pos1, pos2;
	float ma, maw_sum;
	for (int s = 0; s < ep2; ++s) {
		pos1 = max(0, s - range);
		pos2 = min(ep2 - 1, s + range);
		ma = 0;
		maw_sum = 0;
		for (int x = pos1; x <= pos2; ++x) if (v[x]) {
			ma += v[x];
			++maw_sum;
		}
		if (maw_sum) v2[s] = ma / maw_sum;
		else v2[s] = 0;
	}
}

void CF1R::maVector(vector<int> &v, vector<float> &v2, int range) {
	int pos1, pos2;
	float ma, maw_sum;
	for (int s = 0; s < ep2; ++s) {
		pos1 = max(0, s - range);
		pos2 = min(ep2 - 1, s + range);
		ma = 0;
		maw_sum = 0;
		for (int x = pos1; x <= pos2; ++x) if (v[x]) {
			ma += v[x];
			++maw_sum;
		}
		if (maw_sum) v2[s] = ma / maw_sum;
		else v2[s] = 0;
	}
}

void CF1R::mawVector(vector<int> &v, vector<float> &v2, int range) {
	int pos1, pos2;
	float ma, maw_sum;
	for (int s = 0; s < ep2; ++s) {
		pos1 = max(0, s - range);
		pos2 = min(ep2 - 1, s + range);
		ma = 0;
		maw_sum = 0;
		for (int x = pos1; x <= pos2; ++x) if (v[x]) {
			ma += maw[abs(x - s)] * v[x];
			maw_sum += maw[abs(x - s)];
		}
		if (maw_sum) v2[s] = ma / maw_sum;
		else v2[s] = 0;
	}
}

void CF1R::mawVector(vector<float> &v, vector<float> &v2, int range) {
	int pos1, pos2;
	float ma, maw_sum;
	for (int s = 0; s < ep2; ++s) {
		pos1 = max(0, s - range);
		pos2 = min(ep2 - 1, s + range);
		ma = 0;
		maw_sum = 0;
		for (int x = pos1; x <= pos2; ++x) if (v[x]) {
			ma += maw[abs(x - s)] * v[x];
			maw_sum += maw[abs(x - s)];
		}
		if (maw_sum) v2[s] = ma / maw_sum;
		else v2[s] = 0;
	}
}

void CF1R::MakeMacc(vector<int> &cc) {
	SET_READY(DR_macc);
	int pos1, pos2;
	int ma_range = 2 * minl;
	macc_range = ma_range;
	macc2_range = ma_range * 2;
	// Deviation weight
	maw.clear();
	for (int i = 0; i <= ma_range; ++i) {
		maw.push_back(1 - i * 0.5 / ma_range);
	}
	float de, maw_sum;
	// Moving average
	mawVector(cc, macc, ma_range);
	// Smooth
	mawVector(macc, macc2, ma_range);
	// Deviation
	for (int s = 0; s < ep2; ++s) {
		pos1 = max(0, s - ma_range);
		pos2 = min(ep2 - 1, s + ma_range);
		de = 0;
		maw_sum = 0;
		for (int x = pos1; x <= pos2; ++x) if (cc[x]) {
			de += maw[abs(x - s)] * SQR(cc[x] - macc[s]);
			maw_sum += maw[abs(x - s)];
		}
		if (maw_sum) decc[s] = SQRT(de / maw_sum);
		else decc[s] = 0;
	}
	// Smooth
	mawVector(decc, decc2, ma_range);
}

int CF1R::FailLocalMacc(int notes, float mrange, int flag) {
	CHECK_READY(DR_fli, DR_macc);
	// Do not test if flag disabled and not testing
	if (task != tEval && accept[flag] == -1) return 0;
	// Do not test if not enough notes. If melody is short, than global range check is enough
	if (fli_size < notes) return 0;
	float lmin, lmax, maccr;
	int maccr_count = 0;
	int s;
	int ls_max = fli_size - notes;
	int ls_max2;
	int fired = 0;
	pm_maccr_min = INT_MAX;
	pm_maccr_max = 0;
	pm_maccr_av = 0;
	// Loop through windows
	for (ls = 0; ls <= ls_max; ++ls) {
		lmin = MAX_NOTE;
		lmax = 0;
		ls_max2 = ls + notes;
		// Do not check if later notes are not created
		if (ep2 < c_len && fli2[ls_max2 - 1] + macc2_range >= ep2) continue;
		// Loop inside each window
		for (s = fli[ls]; s <= fli2[ls_max2 - 1]; ++s) {
			if (macc2[s] < lmin) lmin = macc2[s];
			if (macc2[s] > lmax) lmax = macc2[s];
		}
		// Record range
		maccr = lmax - lmin;
		pm_maccr_av += maccr;
		++maccr_count;
		if (pm_maccr_min > maccr) pm_maccr_min = maccr;
		if (pm_maccr_max < maccr) pm_maccr_max = maccr;
		// Check range
		if (lmin < MAX_NOTE && lmax > 0 && maccr < mrange) {
			if (fired) {
				fpenalty[flag] += severity[flag] + 1;
			}
			else {
				FLAG2L(flag, fli[ls], fli[ls_max2 - 1]);
				fired = 1;
			}
		}
	}
	pm_maccr_av /= maccr_count;
	return 0;
}

// Count limits
void CF1R::GetMelodyInterval(vector<int> &cc, int step1, int step2, int &nmin, int &nmax) {
	SET_READY(DR_nmin);
	// Calculate range
	nmin = MAX_NOTE;
	nmax = 0;
	for (int i = step1; i < step2; ++i) {
		if (cc[i] < nmin) nmin = cc[i];
		if (cc[i] > nmax) nmax = cc[i];
	}
	// Calculate diatonic limits
	nmind = CC_C(nmin, tonic_cur, minor_cur);
	nmaxd = CC_C(nmax, tonic_cur, minor_cur);
}

// Calculate pitch class
void CF1R::GetPitchClass(vector<int> &c, vector<int> &cc, vector<int> &pc, vector<int> &pcc, int step1, int step2) {
	CHECK_READY(DR_c);
	SET_READY(DR_pc);
	for (int i = step1; i < step2; ++i) {
		pc[i] = c[i] % 7;
		pcc[i] = (cc[i] + 12 - tonic_cur) % 12;
	}
}

int CF1R::FailGisTrail(vector<int> &pcc) {
	CHECK_READY(DR_fli, DR_pc);
	int gis_trail = 0;
	for (ls = 0; ls < fli_size; ++ls) {
		s = fli[ls];
		if (pcc[s] == 11) {
			// Set to maximum on new G# note
			gis_trail = gis_trail_max;
		}
		else {
			if (pcc[s] == 10) {
				// Prohibit G note close to G#
				if (gis_trail) FLAG2L(200, s, fli[max(0, ls - gis_trail_max + gis_trail)]);
			}
		}
		// Decrease if not zero
		if (gis_trail) --gis_trail;
	}
	return 0;
}

int CF1R::FailFisTrail(vector<int> &pcc) {
	CHECK_READY(DR_fli, DR_pc);
	int pos1, pos2, found;
	for (ls = 0; ls < fli_size; ++ls) {
		s = fli[ls];
		if (pcc[s] == 9) {
			// Find VII#
			pos1 = max(0, ls - fis_gis_max);
			pos2 = min(fli_size - 1, ls + fis_gis_max);
			found = 0;
			for (int x = pos1; x <= pos2; ++x) {
				if (pcc[fli[x]] == 11) {
					found = 1;
					break;
				}
			}
			if (!found) {
				// Flag only if full melody analysis or window is not cut
				if (ls + fis_gis_max <= fli_size - 1 || ep2 == c_len)	FLAG2(199, s);
			}
			// Find VII before
			pos1 = max(0, ls - fis_g_max);
			for (int x = pos1; x < ls; ++x) {
				if (pcc[fli[x]] == 10) {
					FLAG2L(349, s, fli[x]);
					break;
				}
			}
			// Find VII after
			pos2 = min(fli_size - 1, ls + fis_g_max2);
			for (int x = ls + 1; x <= pos2; ++x) {
				if (pcc[fli[x]] == 10) {
					FLAG2(350, s);
					break;
				}
			}
		}
	}
	return 0;
}

int CF1R::FailMelodyHarmStep(int i, const int* hv, int &count, int &wcount, int &last_flag, int &max_p, int repeat_letters, int miss_letters, int flagr, int flagm) {
	if (hv[chm[i]]) {
		++count;
		wcount = 0;
	}
	else {
		++wcount;
		count = 0;
	}
	if (count > repeat_letters) FLAG3(flagr, i);
	if (wcount > miss_letters) FLAG3(flagm, i);
	return 0;
}

int CF1R::EvalMelodyHarm(int hp, int &last_flag, int &max_p) {
	int pen1;
	int p2c = 0; // Count of consecutive penalty 2
	int dcount = 0;
	int scount = 0;
	int tcount = 0;
	int wdcount = 0;
	int wscount = 0;
	int wtcount = 0;
	for (int i = 0; i <= hp; ++i) {
		if (i > 0) {
			// Check GC for low cantus and not last note (last note in any window is ignored)
			if (!cantus_high && i < fli_size - 1 && chm[i] == 0 && chm[i - 1] == 4) {
				if (m_pc[fli[i]] == 0 && m_pc[fli[i - 1]] == 4) FLAG3(48, i);
			}
			// Check harmonic penalty	
			pen1 = hsp[chm[i - 1]][chm[i]];
			if (pen1 == 3) FLAG3(99, i);
			else if (pen1 == 1) FLAG3(77, i);
			if (pen1 == 2) {
				FLAG3(57, i);
				if (culm_ls == i) FLAG3(195, i);
				if (abs(m_cc[fli[i]] - m_cc[fli[i - 1]]) > hsp_leap) FLAG3(194, i);
				++p2c;
				if (p2c == 2) FLAG3(92, i + 1);
				else if (p2c == 3) FLAG3(23, i + 1);
			}
			else {
				p2c = 0;
			}
		}
		// Check letter repeat and miss
		if (FailMelodyHarmStep(i, hvt, tcount, wtcount, last_flag, max_p, repeat_letters_t, miss_letters_t, 17, 20)) return 1;
		if (FailMelodyHarmStep(i, hvd, dcount, wdcount, last_flag, max_p, repeat_letters_d, miss_letters_d, 428, 430)) return 1;
		if (FailMelodyHarmStep(i, hvs, scount, wscount, last_flag, max_p, repeat_letters_s, miss_letters_s, 429, 431)) return 1;
	}
	return 0;
}

int CF1R::FailMelodyHarm(vector<int> &pc, vector<int> &pcc) {
	CHECK_READY(DR_fli, DR_pc);
	//CHECK_READY_PERSIST(DP_hv, DP_hsp);
	int h;
	int is_first_tonic = 0;
	// Build hm vector
	for (ls = 0; ls < fli_size; ++ls) {
		s = fli2[ls];
		hm[ls].clear();
		for (int x = 0; x < hv[pc[s]].size(); ++x) {
			h = hv[pc[s]][x];
			// Check tonic
			if (!h) {
				// Is this first or last tonic?
				if (!is_first_tonic || s == c_len - 1) {
					is_first_tonic = 1;
					// Set only tonic for this step
					hm[ls].clear();
					hm[ls].push_back(h);
					break;
				}
			}
			// If tonic allowed or not tonic
			hm[ls].push_back(h);
		}
		// Shuffle
		if (task == tEval) {
			random_shuffle(hm[ls].begin(), hm[ls].end());
		}
	}
	// Scan vector
	vector<int> chm_saved;
	chm.clear();
	chm_alter.clear();
	chm_alter.resize(fli_size, 0);
	chmp.clear();
	chm.resize(fli_size, 0);
	chmp.resize(fli_size, 0);
	for (int i = 0; i < fli_size; ++i) {
		chm[i] = hm[i][0];
		if (minor_cur && (pcc[fli[i]] == 9 || pcc[fli[i]] == 11)) chm_alter[i] = 1;
		if (minor_cur && (pcc[fli[i]] == 8 || pcc[fli[i]] == 10)) chm_alter[i] = -1;
	}
	int hp = 0;
	int finished = 0;
	int found = 0;
	int hcycle = 0;
	int last_flag = 0;
	int last_flag2 = 0;
	int max_p = 0;
	int max_p2 = 0;
	//LogCantus(pc);
	while (true) {
	check:
		//st.Format("%d: ", hp);
		//CGLib::AppendLineToFile("log/temp.log", st);
		if (need_exit) return 1;
		if (!hp) {
			++hp;
			if (hp > max_p) max_p = hp;
			goto check;
		}
		//if (task == tEval) {
		//	LogCantus("chm", hp, hp + 1, chm);
		//	LogCantus("cc", 0, ep2, m_cc);
		//}
		//LogCantus(chm);
		if (EvalMelodyHarm(hp, last_flag, max_p)) goto skip;
		// Success
		if (hp == fli_size - 1) {
			found = 1;
			break;
		}
		else {
			++hp;
			if (hp > max_p) {
				max_p = hp;
			}
			goto check;
		}
	skip:
		// Save maximum flag
		if (hp > max_p2) {
			max_p2 = hp;
			if (last_flag) {
				last_flag2 = last_flag;
				chm_saved = chm;
			}
		}
		// ScanLeft
		while (true) {
			if (chmp[hp] < hm[hp].size() - 1) break;
			// If current element is max, make it minimum
			chmp[hp] = 0;
			// Get current value
			chm[hp] = hm[hp][chmp[hp]];
			// Move left one element
			if (!hp) {
				finished = 1;
				break;
			}
			hp--;
		} // while (true)
		if (finished) {
			break;
		}
		// ScanRight
		// Increase rightmost element, which was not reset to minimum
		++chmp[hp];
		// Get current value
		chm[hp] = hm[hp][chmp[hp]];
		++hcycle;
	}
	// Detect possible variants
	if (!found) {
		// Load problematic chm if it exists
		if (chm_saved.size()) chm = chm_saved;
		if (max_p < fli_size - 1) {
			//fill(chm.begin(), chm.end(), -1);
		}
		// Increase penalty if flag was found at the beginning of melody
		fpenalty[last_flag2] = fli_size - max_p;
		// Report one of last flags at highest position
		FLAG2(last_flag2, max_p);
	}
	return 0;
}

// Calculate chromatic positions
void CF1R::GetChromatic(vector<int> &c, vector<int> &cc, int step1, int step2, int minor_cur) {
	CHECK_READY(DR_c);
	if (minor_cur) {
		for (int i = step1; i < step2; ++i) {
			cc[i] = m_C_CC(c[i], tonic_cur);
		}
	}
	else {
		for (int i = step1; i < step2; ++i) {
			cc[i] = maj_C_CC(c[i], tonic_cur);
		}
	}
}

// Calculate diatonic positions
int CF1R::FailDiatonic(vector<int> &c, vector<int> &cc, int step1, int step2, int minor_cur) {
	SET_READY(DR_c);
	if (minor_cur) {
		for (int i = step1; i < step2; ++i) {
			c[i] = m_CC_C(cc[i], tonic_cur);
		}
	}
	else {
		for (int i = step1; i < step2; ++i) {
			c[i] = maj_CC_C(cc[i], tonic_cur);
		}
	}
	return 0;
}

int CF1R::IsRepeat(int ls1, int ls2, vector<int> &c, vector<int> &cc, vector<int> &leap, int relen) {
	int found = 1;
	for (int i = 0; i < relen; ++i) {
		if (c[fli[ls1 + i]] != c[fli[ls2 + i]]) {
			found = 0;
			break;
		}
		if (i < relen - 1 && llen[ls1 + i] != llen[ls2 + i]) {
			found = 0;
			break;
		}
	}
	return found;
}

// Search for adjacent or symmetric repeats
int CF1R::FailAdSymRepeat(vector<int> &c, vector<int> &cc, vector<int> &leap, int relen) {
	CHECK_READY(DR_fli, DR_c);
	// Do not test if flag disabled and not testing
	//if (task != tEval && accept[flag] == -1) return 0;
	// Cycle through all notes that can be repeated
	for (ls = 0; ls <= fli_size - relen * 2; ++ls) {
		int rpos1 = 0;
		int rpos2 = 0;
		// Check adjacent repeat
		if (IsRepeat(ls, ls + relen, c, cc, leap, relen)) {
			rpos1 = ls + relen;
		}
		// If same beat is not adjacent, get same beat and check
		else if (cspecies > 1 && ((fli[ls + relen] - fli[ls]) % (npm / 2))) {
			for (int x = 1; x < 4; ++x) {
				if (ls + x <= fli_size - relen * 2 && !((fli[ls + relen + x] - fli[ls]) % (npm / 2))) {
					if (IsRepeat(ls, ls + relen + x, c, cc, leap, relen)) {
						rpos1 = ls + relen + x;
					}
				}
			}
		}
		// If any repeat is found, check one more repeat
		if (rpos1 && rpos1 <= fli_size - relen * 2) {
			// Check adjacent repeat
			if (IsRepeat(ls, rpos1 + relen, c, cc, leap, relen)) {
				rpos2 = rpos1 + relen;
			}
			// If same beat is not adjacent, get same beat and check
			else if (cspecies > 1 && ((fli[rpos1 + relen] - fli[rpos1]) % (npm / 2))) {
				for (int x = 1; x < 4; ++x) {
					if (rpos1 + x <= fli_size - relen * 2 && !((fli[rpos1 + relen + x] - fli[rpos1]) % (npm / 2))) {
						if (IsRepeat(rpos1, rpos1 + relen + x, c, cc, leap, relen)) {
							rpos2 = rpos1 + relen + x;
						}
					}
				}
			}
		}
		if (relen == 3) {
			// Flag two repeats
			if (rpos2) {
				if (rpos2 == rpos1 + relen)
					FLAG2L(313, fli[ls], fli[ls + relen - 1]);
				else
					FLAG2L(407, fli[ls], fli[ls + relen - 1]);
			}
			// Flag one repeat
			else if (rpos1) {
				if (rpos1 == ls + relen)
					FLAG2L(311, fli[ls], fli[ls + relen - 1]);
				else
					FLAG2L(405, fli[ls], fli[ls + relen - 1]);
			}
		}
		if (relen == 4) {
			// Flag two repeats
			if (rpos2) {
				if (rpos2 == rpos1 + relen)
					FLAG2L(314, fli[ls], fli[ls + relen - 1]);
				else
					FLAG2L(408, fli[ls], fli[ls + relen - 1]);
			}
			// Flag one repeat
			else if (rpos1) {
				if (rpos1 == ls + relen)
					FLAG2L(312, fli[ls], fli[ls + relen - 1]);
				else
					FLAG2L(406, fli[ls], fli[ls + relen - 1]);
			}
		}
	}
	return 0;
}

// Search for outstanding repeats
int CF1R::FailOutstandingRepeat(vector<int> &c, vector<int> &cc, vector<int> &leap, int scan_len, int rlen, int flag) {
	CHECK_READY(DR_fli, DR_c);
	// Do not test if flag disabled and not testing
	if (task != tEval && accept[flag] == -1) return 0;
	int ok, f, f1;
	if (fli_size > rlen * 2) for (ls = 0; ls < fli_size - rlen * 2; ++ls) {
		s = fli2[ls];
		s1 = fli2[ls + 1];
		if (MELODY_SEPARATION(s, s1)) {
			// Search for repeat of note at same beat until last three notes
			int finish = ls + scan_len;
			if (finish > fli_size - rlen) finish = fli_size - rlen;
			// Do not analyse last note if end of scan window
			if (finish >= fli_size - rlen && ep2 < c_len) --finish;
			for (int x = ls + 2; x <= finish; ++x) {
				f = fli2[x];
				f1 = fli2[x + 1];
				// Check rhythm
				if ((f - s) % 2) continue;
				if (MELODY_SEPARATION(f, f1)) {
					// Check if same note
					if (c[f] == c[s] && llen[x] == llen[ls]) {
						// Check that more notes repeat
						ok = 0;
						for (int z = 1; z < rlen; ++z) {
							if (c[fli[x + z]] != c[fli[ls + z]] || llen[x + z] != llen[ls + z]) {
								ok = 1;
								break;
							}
						}
						if (!ok) {
							FLAG2L(flag, fli[ls], fli[ls + rlen - 1]);
						}
					}
				}
			}
		}
	}
	return 0;
}

int CF1R::FailLongRepeat(vector<int> &c, vector<int> &cc, vector<int> &leap, int scan_len, int rlen, int flag) {
	CHECK_READY(DR_fli, DR_c);
	// Do not test if flag disabled and not testing
	if (task != tEval && accept[flag] == -1) return 0;
	int ok;
	int f, f1;
	if (fli_size > rlen + 1) for (ls = 0; ls < fli_size - rlen - 1; ++ls) {
		s = fli2[ls];
		s1 = fli2[ls + 1];
		int finish = ls + scan_len;
		if (finish > fli_size - rlen) finish = fli_size - rlen;
		// Do not analyse last note if end of scan window
		if (finish >= fli_size - rlen && ep2 < c_len) --finish;
		for (int x = ls + rlen; x <= finish; ++x) {
			f = fli2[x];
			f1 = fli2[x + 1];
			// Check if same note
			if (c[f] == c[s] && llen[x] == llen[ls]) {
				// Check that more notes repeat
				ok = 0;
				for (int z = 1; z < rlen; ++z) {
					if (c[fli2[x + z]] != c[fli2[ls + z]] || llen[x + z] != llen[ls + z]) {
						ok = 1;
						break;
					}
				}
				if (!ok) {
					FLAG2L(flag, fli[ls], fli[ls + rlen - 1]);
				}
			}
		}
	}
	return 0;
}

int CF1R::FailManyLeaps(vector<int> &c, vector<int> &cc, vector<int> &leap, vector<int> &smooth,
	vector<int> &slur, int mleaps, int mleaped, int mleaps2, int mleaped2, int mleapsteps,
	int flag1, int flag2, int flag3, int flag4) {
	CHECK_READY(DR_fli, DR_c);
	// Do not test if flag disabled and not testing (warning, this disables parameter map calculation)
	if (task != tEval && accept[flag3] == -1 && accept[flag4] == -1) return 0;
	int leap_sum = 0;
	int leaped_sum = 0;
	pm_win_leaps = 0;
	pm_win_leapnotes = 0;
	int leap_sum_i = 0;
	g_leaps[fli_size - 1] = 0;
	g_leaped[fli_size - 1] = 0;
	for (ls = 0; ls < fli_size - 1; ++ls) {
		s = fli2[ls];
		s1 = fli2[ls + 1];
		// Add new leap
		if (leap[s] != 0) {
			++leap_sum;
			leaped_sum += abs(c[s] - c[s1]) + 1;
		}
		// Subtract old leap
		if ((ls >= mleapsteps) && (leap[fli2[ls - mleapsteps]] != 0)) {
			leap_sum--;
			leaped_sum -= abs(c[fli[ls - mleapsteps]] - c[fli[ls - mleapsteps + 1]]) + 1;
		}
		// Get maximum leap_sum
		if (leap_sum > pm_win_leaps) {
			pm_win_leaps = leap_sum;
			leap_sum_i = ls;
		}
		if (leaped_sum > pm_win_leapnotes) {
			pm_win_leapnotes = leaped_sum;
			leap_sum_i = ls;
		}
		// Record for graph
		g_leaps[ls] = leap_sum;
		g_leaped[ls] = leaped_sum;
		// Calculate penalty 
		if (leap_sum > mleaps) {
			if (!accept[flag1]) ++fpenalty[flag1];
		}
		else if (leap_sum > mleaps2) {
			if (!accept[flag3]) ++fpenalty[flag3];
		}
		if (leaped_sum > mleaped) {
			if (!accept[flag2]) ++fpenalty[flag2];
		}
		else if (leaped_sum > mleaped2) {
			if (!accept[flag4]) ++fpenalty[flag4];
		}
	}
	if (pm_win_leaps > mleaps)
		FLAG2L(flag1, fli[leap_sum_i + 1], fli[max(0, leap_sum_i - mleapsteps)]);
	else if (pm_win_leaps > mleaps2)
		FLAG2L(flag3, fli[leap_sum_i + 1], fli[max(0, leap_sum_i - mleapsteps)]);
	if (pm_win_leapnotes > mleaped)
		FLAG2L(flag2, fli[leap_sum_i + 1], fli[max(0, leap_sum_i - mleapsteps)]);
	else if (pm_win_leapnotes > mleaped2)
		FLAG2L(flag4, fli[leap_sum_i + 1], fli[max(0, leap_sum_i - mleapsteps)]);
	return 0;
}

// Calculate global leap smooth slur variables
void CF1R::GetLeapSmooth(vector<int> &c, vector<int> &cc, vector<int> &leap, vector<int> &smooth, vector<int> &slur) {
	CHECK_READY(DR_c);
	SET_READY(DR_leap, DR_slur);
	for (int i = 0; i < ep2 - 1; ++i) {
		// Find all leaps
		leap[i] = 0;
		smooth[i] = 0;
		slur[i + 1] = 0;
		if (cc[i] == cc[i + 1]) slur[i + 1] = 1;
		if (c[i + 1] - c[i] > 1) leap[i] = 1;
		else if (c[i + 1] - c[i] < -1) leap[i] = -1;
		// Find all smooth
		else if (c[i + 1] - c[i] == 1) smooth[i] = 1;
		else if (c[i + 1] - c[i] == -1) smooth[i] = -1;
	}
	leap[ep2 - 1] = 0;
	smooth[ep2 - 1] = 0;
	slur[0] = 0;
}

// Check if too many leaps
int CF1R::FailLeapSmooth(vector<int> &c, vector<int> &cc, vector<int> &leap, vector<int> &smooth,
	vector<int> &slur, int l_max_smooth, int l_max_smooth_direct, int csel, int csel2,
	int flag1, int flag2, int flag3, int flag4, int first_run) {
	CHECK_READY(DR_leap, DR_c, DR_fli);
	// Clear variables
	int leap_sum2 = 0;
	int thirds_sum = 0;
	int leap_sum_corrected = 0;
	int max_leap_sum2 = 0;
	int smooth_sum = 0;
	int smooth_sum2 = 0;
	int leap_sum_s2 = 0;
	int fired4 = 0, fired5 = 0;
	pm_leaps2 = 0;
	pm_leaps3 = 0;
	for (ls = 0; ls < fli_size - 1; ++ls) {
		s = fli2[ls];
		s1 = fli2[ls + 1];
		// Add new leap
		if (leap[s] != 0) {
			++leap_sum2;
			if (abs(c[s1] - c[s]) == 2) ++thirds_sum;
		}
		else {
			leap_sum2 = 0;
			thirds_sum = 0;
		}
		// Record
		if (leap_sum2 == 2) ++pm_leaps2;
		else if (leap_sum2 == 3) ++pm_leaps3;
		// Get maximum leap_sum
		leap_sum_corrected = leap_sum2 - min(thirds_sum, thirds_ignored);
		if (leap_sum_corrected > max_leap_sum2) {
			max_leap_sum2 = leap_sum_corrected;
			leap_sum_s2 = s;
		}
		// Calculate penalty
		if (leap_sum_corrected == csel) {
			if (accept[flag3] > 0) ++fpenalty[flag3];
		}
		if (leap_sum_corrected > csel2 && accept[flag4] > 0) ++fpenalty[flag4];
		// Prohibit long smooth movement
		if (smooth[s] != 0) {
			++smooth_sum;
			if (smooth_sum >= l_max_smooth) {
				if (fired4) {
					fpenalty[flag1] += severity[flag1] + 1;
				}
				else {
					FLAG2L(flag1, fli[ls + 1], fli[ls - smooth_sum + 1]);
					fired4 = 1;
				}
			}
		}
		else if (leap[s]) smooth_sum = 0;
		if (ls < fli_size - 2) {
			// Prohibit long smooth movement in one direction
			if (smooth[s] != 0 && smooth[s] == smooth[fli2[ls + 1]]) {
				++smooth_sum2;
				if (smooth_sum2 >= l_max_smooth_direct) {
					if (fired5) {
						fpenalty[flag2] += severity[flag2] + 1;
					}
					else {
						FLAG2L(flag2, fli[ls + 1], fli[ls - smooth_sum2 + 1]);
						fired5 = 1;
					}
				}
			}
			else if (smooth[s] || leap[s]) smooth_sum2 = 0;
			// Check if two notes repeat
			if (first_run && ls > 0 && (cc[s] == cc[fli2[ls + 2]]) &&
				(cc[fli2[ls - 1]] == cc[fli2[ls + 1]]) &&
				(ep2 == c_len || ls + 2 < fli_size - 1)) {
				if (svoices == 1 || species == 1 || species == 4) {
					// Same rhythm in first notes of repeat?
					if (llen[ls - 1] == llen[ls + 1]) {
						if (llen[ls - 1] == llen[ls]) {
							FLAG2L(402, fli[ls - 1], fli[ls + 2]);
						}
						else FLAG2L(403, fli[ls - 1], fli[ls + 2]);
					}
					else FLAG2L(404, fli[ls - 1], fli[ls + 2]);
				}
				else if (species == 2 || species == 3) {
					if (bmli[fli[ls - 1]] == bmli[fli[ls + 2]]) {
						// Same rhythm in first notes of repeat?
						if (llen[ls - 1] == llen[ls + 1]) {
							if (llen[ls - 1] == llen[ls]) {
								FLAG2L(411, fli[ls - 1], fli[ls + 2]);
							}
							else FLAG2L(412, fli[ls - 1], fli[ls + 2]);
						}
						else FLAG2L(413, fli[ls - 1], fli[ls + 2]);
					}
					else {
						// Same rhythm in first notes of repeat?
						if (llen[ls - 1] == llen[ls + 1]) {
							if (llen[ls - 1] == llen[ls]) {
								FLAG2L(402, fli[ls - 1], fli[ls + 2]);
							}
							else FLAG2L(403, fli[ls - 1], fli[ls + 2]);
						}
						else FLAG2L(404, fli[ls - 1], fli[ls + 2]);
					}
				}
				else if (species == 5) {
					if (bmli[fli[ls - 1]] == bmli[fli[ls + 2]]) {
						// Same rhythm in first notes of repeat?
						if (llen[ls - 1] == llen[ls + 1]) {
							if (llen[ls - 1] == llen[ls]) {
								FLAG2L(411, fli[ls - 1], fli[ls + 2]);
							}
							else FLAG2L(412, fli[ls - 1], fli[ls + 2]);
						}
						else FLAG2L(413, fli[ls - 1], fli[ls + 2]);
					}
					else {
						// Same rhythm in first notes of repeat?
						if (llen[ls - 1] == llen[ls + 1]) {
							if (llen[ls - 1] == llen[ls]) {
								FLAG2L(402, fli[ls - 1], fli[ls + 2]);
							}
							else FLAG2L(403, fli[ls - 1], fli[ls + 2]);
						}
						else FLAG2L(404, fli[ls - 1], fli[ls + 2]);
					}
				}
			}
		}
	}
	if (first_run && max_leap_sum2 >= csel) {
		if (max_leap_sum2 > csel2)
			FLAG2L(flag4, fli[bli[leap_sum_s2] + 1], fli[max(0, bli[leap_sum_s2] - max_leap_sum2 + 1)]);
		else FLAG2L(flag3, fli[bli[leap_sum_s2] + 1], fli[max(0, bli[leap_sum_s2] - max_leap_sum2 + 1)]);
	}
	return 0;
}

int CF1R::FailStagnation(vector<int> &cc, vector<int> &nstat, int steps, int notes, int flag) {
	CHECK_READY(DR_nmin, DR_fli);
	// Do not test if flag disabled and not evaluating
	if (task != tEval && accept[flag] == -1) return 0;
	// Clear nstat
	for (int i = nmin; i <= nmax; ++i) nstat[i] = 0;
	for (ls = 0; ls < fli_size; ++ls) {
		s = fli[ls];
		// Add new note to stagnation array
		++nstat[cc[s]];
		// Subtract old note
		if (ls >= steps) --nstat[cc[fli[ls - steps]]];
		// Check if too many repeating notes
		if (nstat[cc[s]] > notes) FLAG2L(flag, s, fli[max(0, ls - steps)]);
	}
	return 0;
}

// Prohibit multiple culminations
int CF1R::FailMultiCulm(vector<int> &cc, vector<int> &slur) {
	CHECK_READY(DR_fli);
	SET_READY(DR_culm_ls);
	pm_culm_count = 0;
	culm_ls = -1;
	int multi_culm_fired = 0;
	// Do not find culminations if too early
	if (ep2 < c_len) return 0;
	for (ls = 0; ls < fli_size; ++ls) {
		if (cc[fli[ls]] == nmax) {
			++pm_culm_count;
			culm_ls = ls;
			if (pm_culm_count > 1) {
				if (voice_high) {
					if (multi_culm_fired) fpenalty[12] += severity[12] + 1;
					else {
						multi_culm_fired = 1;
						FLAG2(12, fli[culm_ls]);
					}
				}
				else {
					if (multi_culm_fired) fpenalty[305] += severity[305] + 1;
					else {
						multi_culm_fired = 1;
						FLAG2(305, fli[culm_ls]);
					}
				}
			}
		}
	}
	if (culm_ls == -1) {
		culm_ls = 0;
		CString est;
		est.Format("Warning: culm_ls cannot be detected at step %d", step);
		WriteLog(5, est);
	}
	if (voice_high) {
		// Prohibit culminations at first steps
		if (culm_ls < (early_culm3 * fli_size) / 100) FLAG2(193, fli[culm_ls]);
		if (culm_ls < early_culm - 1) FLAG2(78, fli[culm_ls]);
		else if (culm_ls < early_culm2 - 1) FLAG2(79, fli[culm_ls]);
		// Prohibit culminations at last steps
		if (culm_ls >= fli_size - late_culm) FLAG2(21, fli[culm_ls]);
	}
	// Prohibit synchronized culminations
	if (svoices > 1 && fli[culm_ls] == cf_culm_s) FLAG2(6, fli[culm_ls]);
	return 0;
}

int CF1R::FailFirstNotes(vector<int> &pc) {
	CHECK_READY(DR_fli, DR_pc);
	// Prohibit first note not tonic
	if (pc[0] != 0) {
		if (cantus_high) {
			if (pc[0] == 4) FLAG2(49, 0);
			else if (pc[0] == 2) FLAG2(90, 0);
			else FLAG2(33, 0);
		}
		else {
			FLAG2(33, 0);
		}
	}
	return 0;
}

int CF1R::FailLastNotes(vector<int> &pc, vector<int> &pcc) {
	CHECK_READY(DR_fli, DR_pc);
	// Do not check if melody is short yet
	if (fli_size < 3) return 0;
	// Prohibit last note not tonic
	if (ep2 > c_len - 1) {
		s = fli[fli_size - 1];
		s_1 = fli[fli_size - 2];
		s_2 = fli[fli_size - 3];
		if (pc[s] != 0) FLAG2(50, s);
		if (minor_cur) {
			// Prohibit major second up before I
			if (pcc[s] == 0 && pcc[s_1] == 10) FLAG2(74, s_1, s);
			if (pcc[s] == 0 && pcc[s_2] == 10) FLAG2(74, s_2, s);
		}
	}
	// Wrong second to last note (last note never can be slurred)
	if (ep2 > c_len - 2) {
		s_1 = c_len - 2;
		if ((pc[s_1] == 0) || (pc[s_1] == 2) || (pc[s_1] == 3) || (pc[s_1] == 5)) FLAG2(13, s_1);
		if (pc[s_1] == 4) FLAG2(51, s_1);
	}
	// Wrong third to last note
	if (ep2 > c_len - 3) {
		s_2 = c_len - 3;
		if ((pc[s_2] == 0) || (pc[s_2] == 2) || (pc[s_2] == 4)) FLAG2(14, s_2);
		// Leading third to last note
		if (pc[s_2] == 6) FLAG2(34, s_2);
	}
	return 0;
}

void CF1R::CreateLinks(vector<int> &cc, int multivoice) {
	SET_READY(DR_fli);
	int prev_note = -1;
	int lpos = 0;
	int l = 0;
	int npm2 = (svoices == 1 ? 1 : npm);
	minl = 10000;
	maxl = 0;
	// Set first steps in case there is pause
	for (int i = 0; i < ep2; ++i) {
		if (prev_note != cc[i]) {
			// Save linked note length
			if (prev_note != -1) {
				llen[lpos - 1] = l;
				rlen[lpos - 1] = l * 8.0 / npm2;
				if (minl > l) minl = l;
				if (maxl < l) maxl = l;
				l = 0;
			}
			prev_note = cc[i];
			fli[lpos] = i;
			++lpos;
		}
		fli2[lpos - 1] = i;
		bli[i] = lpos - 1;
		l++;
	}
	fli_size = lpos;
	llen[lpos - 1] = l;
	rlen[lpos - 1] = l * 8.0 / npm2;
	// Last note does not affect minl/maxl
	if (multivoice) {
		// For species 5 minl should be at least 4
		// For species 3 minl should be at least 2
		if (npm == 8) minl = max(4, minl);
		if (npm == 4) minl = max(2, minl);
	}
}

void CF1R::CountFillInit(vector<int> &c, int tail_len, int pre, int &t1, int &t2, int &fill_end) {
	// Create leap tail
	tc.clear();
	if (pre) {
		int pos1 = fleap_start - 1;
		int pos2 = max(fleap_start - tail_len, 0);
		if (c[leap_end] > c[leap_start]) {
			for (int i = pos1; i >= pos2; --i) {
				tc.push_back(128 - c[fli[i]]);
			}
			t1 = 128 - c[leap_end];
			t2 = 128 - c[leap_start];
		}
		else {
			for (int i = pos1; i >= pos2; --i) {
				tc.push_back(c[fli[i]]);
			}
			t1 = c[leap_end];
			t2 = c[leap_start];
		}
	}
	else {
		int pos1 = fleap_end + 1;
		int pos2 = min(fleap_end + tail_len, fli_size - 1);
		if (c[leap_end] > c[leap_start]) {
			for (int i = pos1; i <= pos2; ++i) {
				tc.push_back(c[fli[i]]);
			}
			t1 = c[leap_start];
			t2 = c[leap_end];
		}
		else {
			for (int i = pos1; i <= pos2; ++i) {
				tc.push_back(128 - c[fli[i]]);
			}
			t1 = 128 - c[leap_start];
			t2 = 128 - c[leap_end];
		}
	}
	for (int x = t1; x <= t2; ++x) nstat3[x] = 0;
	fill_end = -1;
}

void CF1R::CountFill(vector<int> &c, int tail_len, vector<int> &nstat2, vector<int> &nstat3, int &skips, int &fill_to, int pre, int &fill_to_pre, int &fill_from_pre, int &fill_from, int &deviates, int &dev_count, int leap_prev, int &fill_end, int &fill_goal) {
	// Leap starting and finishing note
	int t1, t2;
	int cur_deviation = 0;
	int dev_state = 0;
	int max_deviation = 0;
	if (accept[42 + leap_id]) max_deviation = 1;
	if (accept[120 + leap_id] && !pre) max_deviation = 2;
	CountFillInit(c, tail_len, pre, t1, t2, fill_end);
	// Detect fill_end
	deviates = 0;
	int dl2 = dev_late2;
	int dl3 = dev_late3;
	if (leap_size > 4) {
		dl2 = 1;
		dl3 = 2;
	}
	fill_goal = 0;
	// Deviation state: 0 = before deviation, 1 = in deviation, 2 = after deviation, 3 = multiple deviations
	for (int x = 0; x < tc.size(); ++x) {
		// If deviating, start deviation state and calculate maximum deviation
		if (tc[x] > t2) {
			cur_deviation = tc[x] - t2;
			// Detect long deviation for >5th 2nd
			if (cur_deviation == 1 && x == 0 && leap_size > 4 && fleap_end < fli_size - 1 &&
				rlen[fleap_end + 1] > dev2_maxlen * 2 && !accept[386]) break;
			// Detect late deviation
			if (cur_deviation == 1 && x >= dl2 && !accept[191]) break;
			if (cur_deviation == 2 && x >= dl3 && !accept[192]) break;
			if (cur_deviation > deviates) {
				// If deviation is unacceptable, break leap compensation
				if (cur_deviation > max_deviation) break;
				// Save deviation for later use if it is acceptable
				deviates = cur_deviation;
			}
			if (dev_state == 0) dev_state = 1;
			else if (dev_state == 2) {
				break;
			}
		}
		// If not deviating, switch deviation state
		else {
			if (dev_state == 1) dev_state = 2;
		}
		// Calculate goal
		fill_goal = max(fill_goal, t2 - tc[x]);
		// Break leap compensation if we are below the border
		if (tc[x] < t1) break;
		// Increment leap compensation window
		fill_end = x;
	}
	// Add middle note as compensation note if leap is compound
	if (leap_mid) {
		// Convert note to tc
		if (pre) {
			if (c[leap_end] > c[leap_start]) ++nstat3[128 - c[leap_mid]];
			else ++nstat3[c[leap_mid]];
		}
		else {
			if (c[leap_end] > c[leap_start]) ++nstat3[c[leap_mid]];
			else ++nstat3[128 - c[leap_mid]];
		}
	}
	// Calculate fill vector
	for (int x = 0; x <= fill_end; ++x) {
		++nstat3[tc[x]];
	}
	// Get deviations count
	if (dev_state == 0) dev_count = 0;
	else dev_count = 1;

	CountFillSkips(leap_prev, skips, t1, t2);
	CountFillLimits(c, pre, t1, t2, fill_to, fill_to_pre, fill_from_pre, fill_from);
}

void CF1R::CountFillSkips(int leap_prev, int &skips, int t1, int t2) {
	skips = 0;
	for (int x = t1 + 1; x < t2; ++x) if (!nstat3[x]) {
		++skips;
	}
}

void CF1R::CountFillLimits(vector<int> &c, int pre, int t1, int t2, int &fill_to, int &fill_to_pre, int &fill_from_pre, int &fill_from) {
	fill_to = leap_size;
	fill_to_pre = fill_to;
	fill_from_pre = fill_to;
	fill_from = leap_size;
	// Search for first compensated note
	for (int i = t2 - 1; i >= t1; --i) {
		if (nstat3[i]) {
			fill_from = t2 - i;
			break;
		}
	}
	for (int i = t1; i <= t2; ++i) {
		if (nstat3[i]) {
			fill_to = i - t1;
			break;
		}
	}
	// Check prepared fill to
	if (!pre && fill_to > 1) {
		int pos;
		if (fill_to == 2) pos = max(0, fleap_start - fill_pre3_notes);
		else pos = max(0, fleap_start - fill_pre4_notes);
		vector<int> nstat4;
		nstat4.resize(2, 0);
		if (c[leap_start] < c[leap_end]) {
			for (int x = pos; x < fleap_start; ++x) {
				if (c[fli[x]] == c[leap_start] + 1) nstat4[0] = 1;
				else if (c[fli[x]] == c[leap_start] + 2) nstat4[1] = 1;
			}
		}
		else {
			for (int x = pos; x < fleap_start; ++x) {
				if (c[fli[x]] == c[leap_start] - 1) nstat4[0] = 1;
				else if (c[fli[x]] == c[leap_start] - 2) nstat4[1] = 1;
			}
		}
		fill_to_pre = fill_to;
		if (fill_to == 2) {
			if (nstat4[0]) --fill_to_pre;
		}
		else if (fill_to >= 3) {
			if (nstat4[0]) --fill_to_pre;
			if (nstat4[1]) --fill_to_pre;
		}
	}
	// Check prepared fill from
	if (!pre && fill_from > 1) {
		int pos;
		if (fill_from == 2) pos = max(0, fleap_start - fill_pre3_notes);
		else pos = max(0, fleap_start - fill_pre4_notes);
		vector<int> nstat4;
		nstat4.resize(2, 0);
		if (c[leap_start] < c[leap_end]) {
			for (int x = pos; x < fleap_start; ++x) {
				if (c[fli[x]] == c[leap_end] - 1) nstat4[0] = 1;
				else if (c[fli[x]] == c[leap_end] - 2) nstat4[1] = 1;
			}
		}
		else {
			for (int x = pos; x < fleap_start; ++x) {
				if (c[fli[x]] == c[leap_end] + 1) nstat4[0] = 1;
				else if (c[fli[x]] == c[leap_end] + 2) nstat4[1] = 1;
			}
		}
		if (fill_from == 2) {
			if (nstat4[0]) fill_from_pre = 1;
		}
		else if (fill_from == 3) {
			if (nstat4[0] && nstat4[1]) fill_from_pre = 1;
		}
	}
}

void CF1R::FailLeapInit(vector<int> &c, int &late_leap, int &presecond, int &leap_next, int &leap_prev, int &arpeg, int &overflow, vector<int> &leap) {
	presecond = 0; // If leap has a filled second
	leap_next = 0; // Multiply consecutive leaps
	leap_prev = 0; // Multiply consecutive leaps
	overflow = 0; // Leap back overflow
	arpeg = 0; // Arpeggio 3+3
						 // Check if this leap is 3rd
	leap_start = s; // First step of leap
	leap_end = fli2[ls + 1]; // Last step of leap
	leap_mid = 0;
	fleap_start = ls;
	fleap_end = ls + 1;
	// leap_size = 4 for 5th
	leap_size = abs(c[leap_end] - c[s]);
	// Next is leap?
	if (fleap_end < fli_size - 1) leap_next = leap[leap_start] * leap[leap_end];
	// Prev is leap?
	if (fleap_start > 0) leap_prev = leap[leap_start] * leap[fli2[fleap_start] - 1];
	// Late leap?
	late_leap = fli_size - fleap_start;
	// Find late leap border 
	c4p_last_notes2 = min(c4p_last_notes, fli_size - bli[max(0, ep2 - c4p_last_steps)]);
}

int CF1R::FailLeapMulti(int leap_next, int &arpeg, int &overflow, int &child_leap, vector<int> &c, vector<int> &leap) {
	child_leap = 0; // If we have a child_leap
									// Check if leap is third
	if (leap_size == 2) {
		// Check if leap is second third
		if (fleap_start > 0 && abs(c[leap_end] - c[fli2[fleap_start - 1]]) == 4 &&
			abs(c[leap_start] - c[fli2[fleap_start - 1]]) == 2) {
			// If there is one more third forward (3 x 3rds total)
			if (fleap_end < fli_size - 1 && abs(c[fli2[fleap_end + 1]] - c[fli2[fleap_start - 1]]) == 6) {
				FLAG2L(504, fli[fleap_start - 1], fli[fleap_start + 1]);
			}
			else FLAG2L(503, fli[fleap_start - 1], fli[fleap_start + 1]);
			// Set middle leap note
			leap_mid = leap_start;
			// Set leap start to first note of first third
			--fleap_start;
			leap_start = fli2[fleap_start];
			// Set leap size to be compound
			leap_size = 4;
		}
	}
	leap_id = min(leap_size - 2, 3);
	if (fleap_end < fli_size - 1) {
		// Next leap in same direction
		if (leap_next > 0) {
			// Flag if greater than two thirds
			if (abs(c[fli2[fleap_end + 1]] - c[leap_start]) > 4)
				FLAG2L(505, fli[fleap_start], fli[bli[leap_end] + 1]);
			// Allow if both thirds, without flags (will process next cycle)
			else arpeg = 1;
		}
		// Next leap back
		else if (leap_next < 0) {
			int leap_size2 = abs(c[fli2[fleap_end + 1]] - c[leap_end]);
			// Flag if back leap greater than 6th
			if (leap_size2 > 5) FLAG2(22, fli[bli[leap_end]]);
			// Flag if back leap equal or smaller than 6th
			else FLAG2(8, fli[bli[leap_end]]);
			// Flag leap back overflow
			if (leap_size2 > leap_size) {
				FLAG2(58, fli[bli[leap_end]]);
				overflow = 1;
			}
		}
	}
	// Check if we have a greater opposite neighbouring leap
	if ((fleap_end < fli_size - 1 && abs(c[fli2[fleap_end + 1]] - c[leap_end]) >= leap_size - 1 && leap[leap_start] * leap[leap_end]<0) ||
		(fleap_start > 0 && abs(c[leap_start] - c[fli2[fleap_start - 1]]) > leap_size && leap[leap_start] * leap[fli2[fleap_start - 1]]<0)) {
		// Set that we are preleaped (even if we are postleaped)
		child_leap = 1;
	}
	return 0;
}

int CF1R::FailLeap(vector<int> &c, vector<int>& cc, vector<int> &leap, vector<int> &smooth, vector<int> &nstat2, vector<int> &nstat3) {
	CHECK_READY(DR_leap, DR_c, DR_fli);
	if (cspecies > 1) {
		CHECK_READY(DR_beat);
	}
	// Get leap size, start, end
	// Check if leap is compensated (without violating compensation rules)
	// If leap is not compensated, check uncompensated rules
	// If uncompensated rules not allowed, flag compensation problems detected (3rd, etc.)
	int child_leap, leap_next, leap_prev, presecond;
	int overflow, arpeg, late_leap;
	for (s = 0; s < ep2 - 1; ++s) {
		if (leap[s] != 0) {
			ls = bli[s];
			FailLeapInit(c, late_leap, presecond, leap_next, leap_prev,
				arpeg, overflow, leap);
			if (FailLeapMulti(leap_next, arpeg, overflow, child_leap, c, leap)) return 1;
			// If leap back overflow or arpeggio, do not check leap compensation, because compensating next leap will be enough
			if (!overflow && !arpeg)
				if (FailLeapFill(c, late_leap, leap_prev, child_leap)) return 1;
			if (!arpeg) if (FailLeapMDC(leap, cc)) return 1;
		}
	}
	return 0;
}

int CF1R::FailLeapFill(vector<int> &c, int late_leap, int leap_prev, int child_leap) {
	// Prefill parameters
	int ptail_len, pfill_to, pfill_to_pre, pfill_from_pre, pfill_from, pdeviates, pfill_end, pdev_count, pfill_goal;
	// Fill parameters
	int tail_len, fill_to, fill_to_pre, fill_from_pre, fill_from, deviates, fill_end, dev_count, fill_goal;
	filled = 1;
	prefilled = 0;
	int pskips = 10;
	int skips = 10;
	// Calculate allowed skips 
	int allowed_skips = 1;
	if (leap_size > 4) ++allowed_skips;
	if (leap_size > 6) ++allowed_skips;
	if (late_leap <= c4p_last_notes2 + 1) allowed_skips += 2;
	int allowed_pskips = 1;
	if (leap_size > 4) ++allowed_pskips;
	if (leap_size > 6) ++allowed_pskips;
	if (late_leap <= c4p_last_notes2 + 1) allowed_pskips += 1;
	// Check if leap is filled
	tail_len = 2 + (leap_size - 1) * fill_steps_mul;
	// Do not check fill if search window is cut by end of current not-last scan window
	if ((fleap_end + tail_len < fli_size) || (c_len == ep2)) {
		// Check fill only if enough length (checked second time in case of slurs)
		CountFill(c, tail_len, nstat2, nstat3, skips, fill_to, 0, fill_to_pre, fill_from_pre,
			fill_from, deviates, dev_count, leap_prev, fill_end, fill_goal);
		if (skips > allowed_skips) filled = 0;
		else if (fill_to >= 3 && fill_to <= fill_pre4_int &&
			(fill_to_pre == fill_to || late_leap > c4p_last_notes2 + 1 ||
				!accept[144 + leap_id] || (fleap_end < fli_size - 1 && !fill_goal))) filled = 0;
		else if (fill_to > 3 && late_leap > c4p_last_notes2 + 1) filled = 0;
		else if (fill_to > fill_pre4_int && late_leap <= c4p_last_notes2 + 1) filled = 0;
		else if (fill_to == 2 && (fill_to_pre < 2 || !fleap_start) && !accept[100 + leap_id]) filled = 0;
		else if (fill_to == 2 && fill_to_pre > 1 && fleap_start && !accept[104 + leap_id]) filled = 0;
		else if (fill_from >= 3 && fill_from <= fill_pre4_int && (!fill_from_pre || late_leap > c4p_last_notes2 + 1 || !accept[144 + leap_id])) filled = 0;
		else if (fill_from > 3 && late_leap > c4p_last_notes2 + 1) filled = 0;
		else if (fill_from > fill_pre4_int && late_leap <= c4p_last_notes2 + 1) filled = 0;
		else if (fill_from == 2 && !accept[53 + leap_id]) filled = 0;
		else if (deviates > 2) filled = 0;
		else if (deviates == 1 && !accept[42 + leap_id]) filled = 0;
		else if (deviates == 2 && !accept[120 + leap_id]) filled = 0;
		if (!filled) {
			// If starting 3rd
			if (fleap_start == 0 && leap_size == 2 && accept[1]) {
				FLAG2(1, fli[fleap_start]);
				return 0;
			}
			if (child_leap && accept[116 + leap_id]) FLAG2(116 + leap_id, fli[fleap_start]);
			// Check if  leap is prefilled
			else {
				if (ls > 0) {
					ptail_len = 2 + (leap_size - 1) * fill_steps_mul;
					CountFill(c, ptail_len, nstat2, nstat3, pskips, pfill_to, 1, pfill_to_pre, pfill_from_pre, pfill_from,
						pdeviates, pdev_count, leap_prev, pfill_end, pfill_goal);
					prefilled = 1;
					if (pskips > allowed_pskips) prefilled = 0;
					else if (pfill_to > 2) prefilled = 0;
					else if (pfill_to == 2 && !accept[104 + leap_id]) prefilled = 0;
					else if (pfill_from > 2) prefilled = 0;
					else if (pfill_from == 2 && !accept[53 + leap_id]) prefilled = 0;
					else if (pdeviates > 2) prefilled = 0;
					else if (pdeviates == 1 && !accept[42 + leap_id]) prefilled = 0;
					else if (pdeviates == 2 && !accept[120 + leap_id]) prefilled = 0;
				}
				if (prefilled) {
					if (late_leap <= pre_last_leaps + 1) FLAG2(204 + leap_id, fli[fleap_start]);
					else FLAG2(112 + leap_id, fli[fleap_start]);
				}
				else
					FLAG2(124 + leap_id, fli[fleap_start]);
			}
		}
		// Show compensation flags only if successfully compensated
		// This means that compensation errors are not shown if uncompensated (successfully or not)
		else {
			// Flag late uncompensated precompensated leap
			if (fill_to >= 3 && fill_to <= fill_pre4_int && late_leap <= c4p_last_notes2 + 1)
				FLAG2(144 + leap_id, fli[fleap_start]);
			else if (fill_from >= 3 && fill_from <= fill_pre4_int && late_leap <= c4p_last_notes2 + 1)
				FLAG2(144 + leap_id, fli[fleap_start]);
			// Flag prepared unfinished fill if it is not blocking 
			else if (fill_to == 2 && (fill_to_pre < 2 || !fleap_start)) FLAG2(100 + leap_id, fli[fleap_start]);
			// Flag unfinished fill if it is not blocking
			else if (fill_to == 2 && fill_to_pre > 1) FLAG2(104 + leap_id, fli[fleap_start]);
			// Flag after 3rd if it is not blocking
			if (fill_from == 2) FLAG2(53 + leap_id, fli[fleap_start]);
			// Flag deviation if it is not blocking
			if (deviates == 1) FLAG2(42 + leap_id, fli[fleap_start]);
			// Flag deviation if it is not blocking
			if (deviates == 2) FLAG2(120 + leap_id, fli[fleap_start]);
		}
	}
	return 0;
}

int CF1R::FailLeapMDC(vector<int> &leap, vector<int> &cc) {
	// Melody direction change (MDC)
	// 0 = close, 1 = far, 2 = no
	// Default mdc is close, because beginning equals to close mdc
	mdc1 = 0;
	int prev_note = cc[leap_start];
	for (int pos = leap_start - 1; pos >= 0; --pos) {
		if (cc[pos] != prev_note) {
			// Check if direction changes or long without changes
			if (leap[leap_start] * (cc[pos] - prev_note) > 0 || mdc1 > 1) break;
			prev_note = cc[pos];
			++mdc1;
		}
	}
	mdc2 = 0;
	prev_note = cc[leap_end];
	for (int pos = leap_end + 1; pos < ep2; ++pos) {
		if (cc[pos] != prev_note) {
			// Check if direction changes or long without changes
			if (leap[leap_start] * (cc[pos] - prev_note) < 0 || mdc2 > 2) break;
			prev_note = cc[pos];
			++mdc2;
		}
	}
	// Do not flag close+close
	if (mdc1 == 0 && mdc2 == 0) return 0;
	// Do not flag last 3rd in SAS, because it can be later converted to 5th
	if (fleap_end == fli_size - 1 && ep2 < c_len && !leap_id) return 0;
	// Close + next
	if (!mdc1 && mdc2 == 1) {
		// Close + aux
		if ((cspecies == 3 || cspecies == 5) && (
			(fleap_end >= fli_size - 3 && ep2 < c_len) || (fleap_end < fli_size - 3 && cc[fli[fleap_end + 2]] == cc[leap_end] &&
			(cc[fli[fleap_end + 3]] - cc[fli[fleap_end + 2]]) * leap[leap_start] < 0)))
			FLAG2(510 + leap_id, fli[fleap_start]);
		else FLAG2(128 + leap_id, fli[fleap_start]);
	}
	// Close + far
	else if (!mdc1 && mdc2 == 2) FLAG2(140 + leap_id, fli[fleap_start]);
	// Close + no
	else if (!mdc1 && mdc2 == 3) FLAG2(108 + leap_id, fli[fleap_start]);
	// next + close
	else if (mdc1 == 1 && !mdc2) {
		if (cspecies < 2 || bmli[fli[fleap_end]] == bmli[leap_start]) {
			// Aux + close
			if ((cspecies == 3 || cspecies == 5) && fleap_start > 2 && cc[fli[fleap_start - 2]] == cc[leap_start] &&
				(cc[fli[fleap_start - 2]] - cc[fli[fleap_start - 3]]) * leap[leap_start] < 0)
				FLAG2(506 + leap_id, fli[fleap_start]);
			else FLAG2(59 + leap_id, fli[fleap_start]);
		}
		else FLAG2(476 + leap_id, fli[fleap_start]);
	}
	// Far + close
	else if (mdc1 == 2 && !mdc2) {
		if (cspecies < 2 || bmli[fli[fleap_end]] == bmli[leap_start]) FLAG2(132 + leap_id, fli[fleap_start]);
		else FLAG2(25 + leap_id, fli[fleap_start]);
	}
	// Next + next
	else if (mdc1 == 1 && mdc2 == 1) {
		if (cspecies < 2 || bmli[fli[fleap_end]] == bmli[leap_start]) {
			// Aux + aux
			if ((cspecies == 3 || cspecies == 5) && (
				(fleap_end >= fli_size - 3 && ep2 < c_len) || (fleap_end < fli_size - 3 && cc[fli[fleap_end + 2]] == cc[leap_end] &&
				(cc[fli[fleap_end + 3]] - cc[fli[fleap_end + 2]]) * leap[leap_start] < 0)) &&
				fleap_start > 2 && cc[fli[fleap_start - 2]] == cc[leap_start] &&
				(cc[fli[fleap_start - 2]] - cc[fli[fleap_start - 3]]) * leap[leap_start] < 0)
				FLAG2(414 + leap_id, fli[fleap_start]);
			else FLAG2(63 + leap_id, fli[fleap_start]);
		}
		else FLAG2(460 + leap_id, fli[fleap_start]);
	}
	// Next + far
	else if (mdc1 == 1 && mdc2 >= 2) {
		if (cspecies < 2 || bmli[fli[fleap_end]] == bmli[leap_start]) FLAG2(391 + leap_id, fli[fleap_start]);
		else FLAG2(464 + leap_id, fli[fleap_start]);
	}
	// Far + next
	else if (mdc1 >= 2 && mdc2 == 1) {
		if (cspecies < 2 || bmli[fli[fleap_end]] == bmli[leap_start]) FLAG2(148 + leap_id, fli[fleap_start]);
		else FLAG2(468 + leap_id, fli[fleap_start]);
	}
	// Far + far
	else if (mdc1 >= 2 && mdc2 >= 2) {
		if (cspecies < 2 || bmli[fli[fleap_end]] == bmli[leap_start]) FLAG2(398 + leap_id, fli[fleap_start]);
		else FLAG2(472 + leap_id, fli[fleap_start]);
	}
	return 0;
}

float CF1R::GetTonicWeight(int l_ls, vector<int> &c, vector<int> &cc, vector<int> &pc, int tt) {
	int l_s = fli[l_ls];
	// Not species 3 or 5?
	if (cspecies != 3 && cspecies != 5) return 1.0;
	// Tonic downbeat?
	if (!beat[l_ls]) return 1.0;
	// Length is above quarter (for species 5)?
	if (rlen[l_ls] > 2) return 1.0;
	// Length of tonic note is greater than previous note?
	if (l_ls > 0 && llen[l_ls] > llen[l_ls - 1]) return 1.0;
	// Leap to tonic?
	if (l_ls > 0 && abs(cc[l_s] - cc[fli[l_ls] - 1]) > 4) return 1.0;
	// Local climax?
	if (c[l_s] >= lclimax[l_s]) return 1.0;
	return 0.9;
}

int CF1R::FailTonic(vector<int> &c, vector<int> &cc, vector<int> &pc, int tt) {
	CHECK_READY(DR_pc, DR_fli);
	CHECK_READY(DR_lclimax);
	if (cspecies) {
		CHECK_READY(DR_beat);
	}
	vector<float> tcount;
	int s9;
	pm_tw_max = 0;
	tcount.resize(13);
	int fire, fired = 0;
	// Find first and last tonic
	first_tonic = -1;
	last_tonic = -1;
	for (ls = 0; ls < fli_size; ++ls) {
		if (!pc[fli[ls]]) {
			first_tonic = ls;
			break;
		}
	}
	for (ls = fli_size - 1; ls >= 0; --ls) {
		if (!pc[fli[ls]]) {
			last_tonic = ls;
			break;
		}
	}
	// Do not check if melody is short
	if (fli_size < 3) return 0;
	for (ls = 0; ls < fli_size; ++ls) {
		s = fli[ls];
		// Decrement for previous tonic note
		if (ls >= tonic_window[tt]) {
			s9 = fli[ls - tonic_window[tt]];
			if (!pc[s9])
				tcount[cc[s9] / 12] -= GetTonicWeight(ls - tonic_window[tt], c, cc, pc, tt);
		}
		if (!pc[s]) {
			// Increment for current tonic note
			tcount[cc[s] / 12] += GetTonicWeight(ls, c, cc, pc, tt);
			if (tcount[cc[s] / 12] > pm_tw_max) pm_tw_max = tcount[cc[s] / 12];
			// Check count of tonic notes
			if (tcount[cc[s] / 12] >= tonic_max[tt]) {
				if (fired) {
					fpenalty[70 + tt] += severity[70 + tt] + 1;
				}
				else {
					FLAG2L(70 + tt, s, fli[max(0, ls - tonic_window[tt])]);
					fired = 1;
				}
			}
		}
		tweight[ls] = vmax(tcount);
	}
	return 0;
}

int CF1R::FailLastNoteRes(vector<int> &pc) {
	CHECK_READY(DR_pc, DR_fli);
	if (ep2 < c_len) return 0;
	if (fli_size < 2) return 0;
	if (pc[fli[fli_size - 2]] == 6 && pc[c_len - 1] != 0) FLAG2(52, fli[fli_size - 2]);
	if (pc[fli[fli_size - 2]] == 3 && pc[c_len - 1] != 2) FLAG2(87, fli[fli_size - 2]);
	return 0;
}

int CF1R::FailIntervals(vector<int> &c, vector<int> &cc, vector<int> &pc, vector<int> &pcc) {
	CHECK_READY(DR_pc, DR_c, DR_fli);
	for (ls = 0; ls < fli_size - 1; ++ls) {
		s0 = fli[ls];
		s = fli2[ls];
		s1 = fli2[ls + 1];
		// Leap size prohibit
		if (cc[s1] - cc[s] == 8) FLAG2(175, s0);
		else if (cc[s1] - cc[s] == -8) FLAG2(181, s0);
		else if (cc[s1] - cc[s] == 9) FLAG2(176, s0);
		else if (cc[s1] - cc[s] == -9) FLAG2(182, s0);
		else if (cc[s1] - cc[s] == 10) FLAG2(177, s0);
		else if (cc[s1] - cc[s] == -10) FLAG2(183, s0);
		else if (cc[s1] - cc[s] == 11) FLAG2(178, s0);
		else if (cc[s1] - cc[s] == -11) FLAG2(184, s0);
		else if (cc[s1] - cc[s] == 12) FLAG2(179, s0);
		else if (cc[s1] - cc[s] == -12) FLAG2(185, s0);
		else if (cc[s1] - cc[s] > 12) FLAG2(180, s0);
		else if (cc[s1] - cc[s] < -12) FLAG2(186, s0);
		// Prohibit BB
		if (pcc[fli[ls + 1]] == 11 && pcc[s] == 11) FLAG2(348, s0);
	}
	return 0;
}

// Check tritone t1-t2 which has to resolve from ta to tb. Use fleap_start/fleap_end
void CF1R::GetTritoneResolution(int ta, int t1, int t2, int tb, int &res1, int &res2, vector<int> &c, vector<int> &cc, vector<int> &pc, vector<int> &pcc) {
	res1 = 0;
	res2 = 0;
	// Real resolution notes
	int ta2, tb2;
	// Get real resolution notes
	if (pcc[fli[fleap_end]] == t2) {
		ta2 = ta;
		tb2 = tb;
		// Outer resolution is not a resolution
		if (cc[fli[fleap_end]] > cc[fli[fleap_start]]) return;
	}
	else {
		ta2 = tb;
		tb2 = ta;
		// Outer resolution is not a resolution
		if (cc[fli[fleap_end]] < cc[fli[fleap_start]]) return;
	}
	// Get resolution window
	int rwin = 1;
	if (svoices > 1 && bmli[fli[fleap_start]] >= mli.size() - 2) rwin = max(1, (npm * tritone_res_quart) / 4);
	// Scan preparation
	if (fleap_start > 0) {
		int pos1 = max(0, fli[fleap_start] - rwin);
		int pos2 = min(ep2, fli[fleap_end]);
		for (int i = pos1; i < pos2; ++i) {
			if (pcc[i] == ta2 && abs(cc[i] - cc[fli[fleap_start]]) < 5) {
				res1 = 1;
				break;
			}
		}
	}
	// Do not check if cut by scan window
	if (fli2[fleap_end] + 1 + rwin > ep2 && ep2 < c_len) {
		res2 = 1;
		return;
	}
	// Scan resolution
	if (fleap_end < fli_size - 1) {
		int pos1 = max(0, fli2[fleap_start] + 1);
		int pos2 = min(ep2, fli2[fleap_end] + 1 + rwin);
		for (int i = pos1; i < pos2; ++i) {
			if (pcc[i] == tb2 && abs(cc[i] - cc[fli[fleap_end]]) < 5) {
				res2 = 1;
				break;
			}
		}
	}
}

// Check tritone t1-t2 which has to resolve from ta to tb
int CF1R::FailTritone(int ta, int t1, int t2, int tb, vector<int> &c, vector<int> &cc, vector<int> &pc, vector<int> &pcc, vector<int> &leap) {
	int found;
	int res1 = 0; // First note resolution flag
	int res2 = 0; // Second note resolution flag
								// Tritone prohibit
	leap_start = s;
	found = 0;
	// Check consecutive tritone
	if ((pcc[s1] == t2 && pcc[s] == t1) || (pcc[s1] == t1 && pcc[s] == t2)) found = 1;
	// Check tritone with additional note inside
	else if (ls > 0) {
		// Check pitches
		if ((pcc[s1] == t2 && pcc[s_1] == t1) || (pcc[s1] == t1 && pcc[s_1] == t2))
			// Check intermediate note and mdc
			if (c[s] > c[s1] && c[s] < c[s_1]) {
				if ((ls < 2 || c[s_2] < c[s_1] || leap[s_2]) && (ls > fli_size - 3 || c[s2] > c[s1] || leap[s1])) {
					found = 2;
					leap_start = s_1;
				}
			}
			else if (c[s] < c[s1] && c[s] > c[s_1]) {
				if ((ls<2 || c[s_2] > c[s_1] || leap[s_2]) && (ls>fli_size - 3 || c[s2] < c[s1] || leap[s1])) {
					found = 2;
					leap_start = s_1;
				}
			}
	}
	fleap_start = bli[leap_start];
	fleap_end = bli[s1];
	// Do not check tritone if it is at the end of not-last window
	if (ls == fli_size - 2 && ep2 != c_len) return 0;
	if (found) {
		// Check if tritone is highest leap if this is last window
		if (ep2 == c_len && (svoices == 1 || !cantus_high)) {
			if ((cc[leap_start] >= lclimax[leap_start]) || (cc[s1] >= lclimax[leap_start])) {
				// Consecutive
				if (found == 1) FLAG2L(32, s0, fli[ls + 1]);
				// Compound framed
				else if (found == 2) FLAG2L(373, fli[ls - 1], fli[ls + 1]); //-V547
			}
		}
		// Check if resolution is correct
		GetTritoneResolution(ta, t1, t2, tb, res1, res2, c, cc, pc, pcc);
		// Flag resolution for consecutive tritone
		if (found == 1) {
			if (res1*res2 == 0)
				FLAG2L(31, s0, fli[ls + 1]);
			else FLAG2L(2, s0, fli[ls + 1]);
		}
		// Flag resolution for tritone with intermediate note, framed
		else if (found == 2) { //-V547
			if (res1*res2 == 0) FLAG2L(372, fli[ls - 1], fli[ls + 1]);
			else FLAG2L(371, fli[ls - 1], fli[ls + 1]);
		}
	}
	return 0;
}

int CF1R::FailTritones(vector<int> &c, vector<int> &cc, vector<int> &pc, vector<int> &pcc, vector<int> &leap) {
	CHECK_READY(DR_pc, DR_c, DR_fli);
	CHECK_READY(DR_leap, DR_lclimax);
	for (ls = 0; ls < fli_size - 1; ++ls) {
		s0 = fli[ls];
		s = fli2[ls];
		s1 = fli2[ls + 1];
		if (ls > 0) s_1 = fli2[ls - 1];
		if (ls > 1) s_2 = fli2[ls - 2];
		if (ls < fli_size - 2) s2 = fli2[ls + 2];
		// Warning: tritone F#C in minor is not detected (can add FailTritone to detect) because it is already prohibited by Unaltered near altered.
		// If you allow Unaltered near altered, you should implement FailTritone for F#C.
		if (minor_cur) {
			if (FailTritone(3, 5, 11, 0, c, cc, pc, pcc, leap)) return 1;
			if (FailTritone(7, 8, 2, 3, c, cc, pc, pcc, leap)) return 1;
		}
		else {
			if (FailTritone(4, 5, 11, 0, c, cc, pc, pcc, leap)) return 1;
		}
	}
	return 0;
}

// Calculate global fill
int CF1R::FailGlobalFill(vector<int> &c, vector<int> &nstt2) {
	CHECK_READY(DR_nmin, DR_c);
	// Clear nstat2
	for (int i = nmind; i <= nmaxd; ++i) nstt2[i] = 0;
	// Count nstat2
	for (int x = 0; x < ep2; ++x) ++nstt2[c[x]];
	// Check nstat2
	if (ep2 < c_len) return 0;
	int skips = 0;
	int skips2 = 0;
	for (int x = nmind + 1; x < nmaxd; ++x) if (!nstt2[x]) {
		if (!nstt2[x + 1]) {
			++skips2;
			++x;
		}
		else ++skips;
	}
	// Set flags
	if (skips2) FLAG2(69, 0);
	if (skips == 1) FLAG2(67, 0);
	else if (skips >= 2) FLAG2(68, 0);
	return 0;
}

int CF1R::FailMinorStepwise(vector<int> &pcc, vector<int> &cc, vector<int> &c) {
	CHECK_READY(DR_pc, DR_fli);
	if (cspecies) {
		CHECK_READY(DR_msh);
	}
	// For non-border notes only, because border notes have their own rules
	for (ls = 1; ls < fli_size - 1; ++ls) {
		s = fli[ls];
		s_1 = fli[ls - 1];
		s1 = fli[ls + 1];
		// Prohibit harmonic VI# not stepwize ascending
		if ((cspecies < 2 || msh[ls] > 0) && pcc[s] == 9 &&
			(c[s] - c[s_1] != 1 || c[s1] - c[s] != 1))
			FLAG2L(201, s_1, s1);
		// Prohibit harmonic VII natural not stepwize descending
		if ((cspecies < 2 || msh[ls] > 0) && pcc[s] == 10 &&
			(c[s] - c[s_1] != -1 || c[s1] - c[s] != -1))
			FLAG2L(202, s_1, s1);
	}
	return 0;
}

int CF1R::FailMinor(vector<int> &pcc, vector<int> &cc) {
	CHECK_READY(DR_pc, DR_fli);
	for (ls = 1; ls < fli_size; ++ls) {
		s = fli[ls];
		s_1 = fli[ls - 1];
		// Prohibit minor second up before VII - absorbed
		// Prohibit augmented second up before VII - absorbed
		// Prohibit unaltered VI or VII two steps from altered VI or VII
		if (pcc[s] == 11) {
			if (pcc[s_1] == 10) FLAG2L(153, s_1, s);
			if (pcc[s_1] == 8) FLAG2L(154, s_1, s);
			if (pcc[s_1] == 3) {
				if (ls < fli_size - 1) {
					// III-VII#-I downward
					if (pcc[fli[ls + 1]] == 0 && cc[s] - cc[s_1] < 0) FLAG2L(432, s_1, fli[ls + 1]);
					// III-VII#
					else FLAG2L(157, s_1, s);
				}
				else {
					if (ep2 == c_len) FLAG2L(157, s_1, s);
				}
			}
			if (ls > 1) {
				s_2 = fli[ls - 2];
				if (pcc[s_2] == 10) FLAG2L(159, s_2, s);
				if (pcc[s_2] == 8) FLAG2L(160, s_2, s);
				if (pcc[s_2] == 3) FLAG2L(163, s_2, s);
			}
			if (ls < fli_size - 1) {
				s1 = fli[ls + 1];
				if (pcc[s1] == 10) FLAG2L(153, s1, s);
				if (pcc[s1] == 8) FLAG2L(154, s1, s);
				if (pcc[s1] == 3) FLAG2L(156, s1, s);
				if (ls < fli_size - 2) {
					s2 = fli[ls + 2];
					if (pcc[s2] == 10) FLAG2L(159, s2, s);
					if (pcc[s2] == 8) FLAG2L(160, s2, s);
					if (pcc[s2] == 3) FLAG2L(162, s2, s);
				}
			}
		}
		if (pcc[s] == 9) {
			if (pcc[s_1] == 8) FLAG2L(152, s_1, s);
			if (pcc[s_1] == 3) FLAG2L(155, s_1, s);
			if (ls > 1) {
				s_2 = fli[ls - 2];
				if (pcc[s_2] == 8) FLAG2L(158, s_2, s);
				if (pcc[s_2] == 3) FLAG2L(161, s_2, s);
			}
			if (ls < fli_size - 1) {
				s1 = fli[ls + 1];
				if (pcc[s1] == 8) FLAG2L(152, s1, s);
				if (pcc[s1] == 3) FLAG2L(155, s1, s);
				if (ls < fli_size - 2) {
					s2 = fli[ls + 2];
					if (pcc[s2] == 8) FLAG2L(158, s2, s);
					if (pcc[s2] == 3) FLAG2L(161, s2, s);
				}
			}
		}
		// Prohibit unresolved minor tritone DG# (direct or with inserted note)
	}
	return 0;
}

void CF1R::GetLClimax() {
	SET_READY(DR_lclimax);
	if (cspecies) {
		GetMovingMax(acc[cpv], max(lclimax_notes, lclimax_mea*npm), lclimax);
		GetMovingMax(acc[cpv], lclimax_mea5*npm, lclimax2);
	}
	else {
		GetMovingMax(m_cc, max(lclimax_notes, lclimax_mea*npm), lclimax);
	}
}

void CF1R::ScanCantus(int t, int v, vector<int>* pcantus) {
	int finished = 0;
	int scycle = 0;
	// Load master parameters
	task = t;
	svoice = v;
	if (pcantus) scantus = pcantus;
	else scantus = 0;

	ScanCantusInit();
	if (task == tGen) MultiCantusInit(m_c, m_cc);
	else SingleCantusInit();
	if (FailWindowsLimit()) return;
	// Analyze combination
check:
	while (true) {
		CLEAR_READY();
		//LogCantus("CF1 ep2", ep2, m_cc);
		//if (method == mScan && task == tCor && cantus_id+1 == 1 && ep2 > 1 && MatchVectors(m_cc, test_cc, 0, ep2-1)) {
		//	CString est;
		//	est.Format("Found method %d id %d ep2 %d cycle %lld sp1 %d sp2 %d p %d", 
		//		method, cantus_id+1, ep2, cycle, sp1, sp2, p);
		//	WriteLog(1, est); 
		//}
		// Check if dpenalty is already too high
		if (task == tCor && !rpenalty_min) {
			if (method == mScan) {
				CalcStepDpenalty(cantus[cantus_id], m_cc, ep2 - 1);
				if (dpenalty_step[ep2 - 1] > dpenalty_min) goto skip;
			}
			else {
				dpenalty_cur = dpenalty_outside_swa + CalcDpenalty(cantus[cantus_id], m_cc, smap[swa1], smap[sp2 - 1]);
				if (dpenalty_cur > dpenalty_min) goto skip;
			}
		}
		else dpenalty_cur = 0;
		ClearFlags(0, ep2);
		if (FailNoteRepeat(m_cc, 0, ep2 - 1)) goto skip;
		GetMelodyInterval(m_cc, 0, ep2, nmin, nmax);
		++accepted3;
		// Limit melody interval
		if (nmax - nmin > max_interval) FLAG(37, 0);
		if (c_len == ep2 && nmax - nmin < min_interval &&
			c_len >= min_iv_minnotes) FLAG(38, 0);
		// Show status
		long long time = CGLib::time();
		scycle = (time - gen_start_time) / STATUS_PERIOD;
		if (scycle > status_cycle) {
			ShowScanStatus();
			status_cycle = scycle;
		}
		// Save last second for analysis  
		if (m_testing && task == tCor && m_algo_id != 101 &&
			time - gen_start_time > (m_test_sec - ANALYZE_RESERVE) * 1000)
			break;
		// Limit SAS correction time
		if (task == tCor && max_correct_ms && time - correct_start_time > max_correct_ms) break;
		if (FailDiatonic(m_c, m_cc, 0, ep2, minor_cur)) goto skip;
		GetPitchClass(m_c, m_cc, m_pc, m_pcc, 0, ep2);
		CreateLinks(m_cc, 0);
		if (minor_cur) {
			if (FailMinor(m_pcc, m_cc)) goto skip;
			if (FailMinorStepwise(m_pcc, m_cc, m_c)) goto skip;
			if (FailGisTrail(m_pcc)) goto skip;
			if (FailFisTrail(m_pcc)) goto skip;
		}
		GetLClimax();
		if (FailTonic(m_c, m_cc, m_pc, 0)) goto skip;
		if (FailTonic(m_c, m_cc, m_pc, 1)) goto skip;
		if (FailLastNotes(m_pc, m_pcc)) goto skip;
		//if (FailNoteSeq(m_pc)) goto skip;
		if (FailIntervals(m_c, m_cc, m_pc, m_pcc)) goto skip;
		if (FailLastNoteRes(m_pc)) goto skip;
		GetLeapSmooth(m_c, m_cc, m_leap, m_smooth, m_slur);
		if (FailTritones(m_c, m_cc, m_pc, m_pcc, m_leap)) goto skip;
		if (FailManyLeaps(m_c, m_cc, m_leap, m_smooth, m_slur, max_leaps, max_leaped, max_leaps_r, max_leaped_r, max_leap_steps,
			493, 494, 495, 496)) goto skip;
		if (FailManyLeaps(m_c, m_cc, m_leap, m_smooth, m_slur, max_leaps2, max_leaped2, max_leaps2_r, max_leaped2_r, max_leap_steps2,
			497, 498, 499, 500)) goto skip;
		if (FailLeapSmooth(m_c, m_cc, m_leap, m_smooth, m_slur, max_smooth2, max_smooth_direct2, cse_leaps, cse_leaps_r,
			302, 303, 501, 502, 1)) goto skip;
		if (FailAdSymRepeat(m_c, m_cc, m_leap, 3)) goto skip;
		if (FailAdSymRepeat(m_c, m_cc, m_leap, 4)) goto skip;
		if (FailGlobalFill(m_c, nstat2)) goto skip;
		for (int iv = 0; iv < 4; ++iv) {
			if (FailLocalRange(m_c, notes_lrange[iv][0], iv + 2, 434 + iv)) goto skip;
		}
		if (FailLocalPiCount(m_cc, notes_picount, min_picount, 344)) goto skip;
		if (FailLocalPiCount(m_cc, notes_picount2, min_picount2, 345)) goto skip;
		if (FailLocalPiCount(m_cc, notes_picount3, min_picount3, 346)) goto skip;
		if (FailStagnation(m_cc, nstat, stag_note_steps, stag_notes, 10)) goto skip;
		if (FailStagnation(m_cc, nstat, stag_note_steps2, stag_notes2, 39)) goto skip;
		if (FailMultiCulm(m_cc, m_slur)) goto skip;
		if (FailFirstNotes(m_pc)) goto skip;
		if (FailLeap(m_c, m_cc, m_leap, m_smooth, nstat2, nstat3)) goto skip;
		if ((fli_size>1) && FailMelodyHarm(m_pc, m_pcc)) goto skip;
		MakeMacc(m_cc);
		if (FailLocalMacc(notes_arange, min_arange, 15)) goto skip;
		if (FailLocalMacc(notes_arange2, min_arange2, 16)) goto skip;

		SaveBestRejected(m_cc);
		if (task == tCor && method == mSWA) {
			if (skip_flags) {
				SET_READY(DR_rpenalty_cur);
				rpenalty_cur = 0;
				if (ep2 < smap[swa2 - 1] + 1) {
					NextWindow(m_cc);
					goto check;
				}
			}
			else {
				CalcRpenalty(m_cc);
				if (ep2 < smap[swa2 - 1] + 1) {
					if (rpenalty_cur > src_rpenalty_step[smap[swa1]]) goto skip;
					NextWindow(m_cc);
					goto check;
				}
			}
		}
		// If we are window-scanning
		else if (task == tGen || task == tCor) {
			++accepted2;
			CalcFlagStat();
			if (FailFlagBlock()) goto skip;
			if (FailAccept()) goto skip;
			++accepted4[wid];
			// If this is not last window, go to next window
			if (ep2 < c_len) {
				NextWindow(m_cc);
				goto check;
			}
			// Check random_choose
			if (random_choose < 100) if (rand2() >= (float)RAND_MAX*random_choose / 100.0) goto skip;
		}
		// Calculate rules penalty if we evaluate
		else {
			CalcRpenalty(m_cc);
		}
		// Accept cantus
		++accepted;
		TimeBestRejected();
		if (method == mScan && task == tCor) {
			SaveCantus();
		}
		else if (method == mSWA && task == tCor) {
			SaveCantusIfRp();
		}
		else {
			if (task == tGen && accept_reseed) {
				if (clib_vs.Insert(m_cc)) {
					if (SendCantus()) break;
					ReseedCantus();
					// Start evaluating without scan
					goto check;
				}
				else {
					++cantus_ignored;
					//CString est;
					//est.Format("Ignored cantus %d while sent=%d", cantus_ignored, cantus_sent);
					//WriteLog(1, est);
					//if (SendCantus()) break;
					ReseedCantus();
					// Start evaluating without scan
					goto check;
				}
			}
			else {
				// Calculate dpenalty if this is evaluation
				if (task == tEval && cantus.size()) dpenalty_cur = CalcDpenalty(cantus[cantus_id], m_cc, 0, ep2 - 1);
				if (SendCantus()) break;
			}
			// Exit if this is evaluation
			if (task == tEval) return;
		}
	skip:
		if (need_exit) break;
		ScanLeft(m_cc, finished);
		if (finished) {
			// Clear flag to prevent coming here again
			finished = 0;
			// Finish if this is last variant in first window and not SWA
			if ((p == 0) || (wid == 0)) {
				// Sliding Windows Approximation
				if (method == mSWA) {
					if (NextSWA(m_cc, m_cc_old)) {
						scan_full = 1;
						break;
					}
					goto check;
				}
				if (random_seed && random_range && accept_reseed) {
					// Infinitely cycle through ranges
					ReseedCantus();
					// Start evaluating without scan
					goto check;
				}
				WriteLog(0, "Last variant in first window reached");
				scan_full = 1;
				break;
			}
			ShowBestRejected(m_cc);
			BackWindow(m_cc);
			// Goto next variant calculation
			goto skip;
		} // if (finished)
		ScanRight(m_cc);
	}
	if (accepted3 > 100000) {
		WritePerfLog();
		ShowScanStatus();
	}
	WriteFlagCor();
	ShowFlagStat();
	ShowFlagBlock();
}

void CF1R::ScanRight(vector<int> &cc) {
	// Increase rightmost element, which was not reset to minimum
	++cc_id[p];
	cc[p] = cc_order[p][cc_id[p]];
	// Go to rightmost element
	if (task == tGen) {
		p = sp2 - 1;
	}
	else {
		pp = sp2 - 1;
		p = smap[pp];
	}
	++cycle;
	++tcycle;
}

// Save accepted time if we are showing best rejected
void CF1R::TimeBestRejected() {
	if (best_rejected) {
		accept_time = CGLib::time();
		rcycle = 0;
	}
}

void CF1R::SaveCantus() {
	if (method == mScan) dpenalty_cur = dpenalty_step[c_len - 1];
	if (!dpenalty_cur) dpenalty_cur = CalcDpenalty(cantus[cantus_id], m_cc, 0, c_len - 1);
	// If rpenalty is same as min, calculate dpenalty
	if (rpenalty_cur == rpenalty_min) {
		// Do not save cantus if it has higher dpenalty
		if (dpenalty_cur > dpenalty_min) return;
		// Do not save cantus if it is same as source
		if (!dpenalty_cur) return;
		dpenalty_min = dpenalty_cur;
	}
	// If rpenalty lowered, set new dpenalty_min
	else {
		dpenalty_min = dpenalty_cur;
	}
	dpenalty.push_back(dpenalty_cur);
	clib.push_back(m_cc);
	rpenalty.push_back(rpenalty_cur);
	rpenalty_min = rpenalty_cur;
}

void CF1R::SaveCantusIfRp() {
	CHECK_READY(DR_rpenalty_cur);
	// Is penalty not greater than minimum of all previous?
	if (rpenalty_cur <= rpenalty_min) {
		// If rpenalty 0, we can skip_flags (if allowed)
		if (!skip_flags && rpenalty_cur == 0)
			skip_flags = !calculate_blocking && !calculate_correlation && !calculate_stat;
		// Insert only if cc is unique
		if (clib_vs.Insert(m_cc)) {
			SaveCantus();
			// Save flags for SWA stuck flags
			if (rpenalty_cur) best_flags = flags;
			TestRpenalty();
		}
	}
}

void CF1R::ScanInit() {
	if (!is_animating) {
		progress_size = c_len;
		fill(progress.begin(), progress.end(), 0);
		if (task != tEval) scan_full = 0;
		scan_start_time = time();
		anflags.resize(av_cnt);
		anfl.resize(av_cnt);
		for (int i = 0; i < av_cnt; ++i) {
			anflags[i].resize(c_len, vector<int>(MAX_RULES)); // Flags for each note
			anfl[i].resize(c_len, vector<int>(MAX_RULES)); // Flags for each note
		}
		src_rpenalty_step.resize(c_len);
		uli.resize(c_len);
		fli.resize(c_len);
		fli2.resize(c_len);
		tweight.resize(c_len);
		g_leaps.resize(c_len);
		g_leaped.resize(c_len);
		macc.resize(c_len);
		macc2.resize(c_len);
		decc.resize(c_len);
		decc2.resize(c_len);
		cpos.resize(c_len);
		llen.resize(c_len);
		rlen.resize(c_len);
		lclimax.resize(c_len);
		lclimax2.resize(c_len);
		cc_order.resize(c_len);
		dpenalty_step.resize(c_len);
		cc_id.resize(c_len);
		bli.resize(c_len);
		fpenalty.resize(max_flags);
		wpos1.resize(c_len / s_len + 1);
		wpos2.resize(c_len / s_len + 1);
		swpos1.resize(c_len / s_len + 1);
		swpos2.resize(c_len / s_len + 1);
		min_c.resize(c_len);
		max_c.resize(c_len);
		min_cc.resize(c_len);
		max_cc.resize(c_len);
		hm.resize(c_len);
		//hm2.resize(c_len);
		accepted4.resize(MAX_WIND); // number of accepted canti per window
		accepted5.resize(MAX_WIND); // number of canti with neede flags per window
		flags.resize(MAX_RULES); // Flags for whole cantus
		fstat.resize(MAX_RULES); // number of canti with each flag
		fcor.resize(MAX_RULES, vector<long long>(MAX_RULES)); // Flags correlation matrix
		seed_cycle = 0; // Number of cycles in case of random_seed
		reseed_count = 0;
		nstat.resize(MAX_NOTE);
		nstat2.resize(MAX_NOTE);
		nstat3.resize(MAX_NOTE);
		cycle = 0;
		wscans.resize(MAX_WIND); // number of full scans per window
		wcount = 1; // Number of windows created
		swcount = 1; // Number of SWA windows created
		accepted = 0;
		accepted2 = 0;
		accepted3 = 0;
		// Initialize fblock if calculation is needed
		if (calculate_blocking) {
			fblock = vector<vector<vector<long>>>(MAX_WIND, vector<vector<long>>(max_flags, vector<long>(max_flags)));
		}
		// Init best rejected results
		if (best_rejected) {
			rcycle = 0;
			accept_time = CGLib::time();
			if (method == mScan) rpenalty_min = MAX_PENALTY;
		}
		for (int x = 0; x < c_len; ++x) {
			hm[x].resize(3);
			//hm2[x].resize(3);
		}
	}
	// Can we skip flags?
	skip_flags = !calculate_blocking && !calculate_correlation && !calculate_stat;
}

void CF1R::ScanCantusInit() {
	// Get cantus size
	if (task != tGen) c_len = scantus->size();
	ScanInit();
	// Resize global vectors
	m_c.resize(c_len); // cantus (diatonic)
	m_cc.resize(c_len); // cantus (chromatic)
	m_cc_old.resize(c_len); // Cantus diatonic saved for SWA
	m_pc.resize(c_len);
	m_pcc.resize(c_len);
	m_leap.resize(c_len);
	m_smooth.resize(c_len);
	m_slur.resize(c_len);
	ep2 = c_len;
	voice_high = cantus_high;
	max_interval = max_interval_cf;
	// Calculate last steps that are allowed to have C4P
	c4p_last_steps = c4p_last_meas;
	// Set scan voices count
	svoices = 1;
	// Set species to CF
	cspecies = 0;
	SelectSpeciesRules();
	SetRuleParams();
}

// Clear flags
void CF1R::ClearFlags(int step1, int step2) {
	if (!skip_flags || flags_need2) {
		fill(flags.begin(), flags.end(), 0);
		fill(fpenalty.begin(), fpenalty.end(), 0.0);
	}
	flags[0] = 1;
	for (int i = step1; i < step2; ++i) {
		anflags[cpv][i].clear();
		anfl[cpv][i].clear();
	}
	rpenalty_cur = 0;
}

void CF1R::CalcCcIncrement() {
	SET_READY_PERSIST(DP_cc_incr);
	int pos;
	for (int i = 0; i < 127; ++i) {
		pos = (i + 13 - tonic_cur) % 12;
		if (minor_cur) {
			if (m_diatonic_full[pos]) cc_incr[i] = 1;
			else cc_incr[i] = 2;
		}
		else {
			if (diatonic[pos]) cc_incr[i] = 1;
			else cc_incr[i] = 2;
		}
	}
	// Build decrement
	for (int i = 0; i < 127; ++i) {
		pos = (i + 23 - tonic_cur) % 12;
		if (minor_cur) {
			if (m_diatonic_full[pos]) cc_decr[i] = 1;
			else cc_decr[i] = 2;
		}
		else {
			if (diatonic[pos]) cc_decr[i] = 1;
			else cc_decr[i] = 2;
		}
	}
}


// Get minimum element in SWA window
int CF1R::GetMinSmap() {
	int my_ep1 = ep2;
	for (int i = sp1; i < sp2; ++i) {
		if (my_ep1 > smap[i]) my_ep1 = smap[i];
	}
	return my_ep1;
}

// Get maximum element in SWA window
int CF1R::GetMaxSmap() {
	int my_ep2 = 0;
	for (int i = sp1; i < sp2; ++i) {
		if (my_ep2 < smap[i]) my_ep2 = smap[i];
	}
	return my_ep2;
}

// Calculate real possible range
void CF1R::GetRealRange(vector<int>& c, vector<int>& cc) {
	// Get diatonic interval
	max_intervald = CC_C(cc[0] + max_interval, tonic_cur, minor_cur) - c[0];
	min_intervald = CC_C(cc[0] + min_interval, tonic_cur, minor_cur) - c[0];
	// If max_interval is below octave, you cannot go to the lowest note to avoid multiple culminations flag
	if (max_interval < 12) {
		minc = cc[0] - max_interval + 2;
		maxc = cc[0] + max_interval;
	}
	else {
		minc = cc[0] - max_interval;
		maxc = cc[c_len - 1] + max_interval;
	}
	if (random_range) {
		if (maxc - minc > max_interval) {
			int rstart = randbw(0, maxc - minc - max_interval);
			minc += rstart;
			maxc = minc + max_interval;
		}
	}
}

// Calculate source melody range
void CF1R::GetSourceRange(vector<int> &cc) {
	// Get source melody range
	GetMelodyInterval(cc, 0, c_len, nmin, nmax);
	// Widen range
	src_nmin = nmin - correct_inrange;
	src_nmax = nmax + correct_inrange;
}

// Apply source melody range
void CF1R::ApplySourceRange() {
	if (src_nmax > MAX_NOTE) return;
	// Decrease current range if it is bigger
	if (minc > src_nmin) minc = src_nmin;
	if (maxc < src_nmax) maxc = src_nmax;
}

void CF1R::SingleCantusInit() {
	SET_READY_PERSIST(DP_cc_old);
	// Copy cantus
	m_cc = *scantus;
	// Get diatonic steps from chromatic
	first_note = m_cc[0];
	last_note = m_cc[c_len - 1];
	for (int i = 0; i < c_len; ++i) {
		m_c[i] = CC_C(m_cc[i], tonic_cur, minor_cur);
		// Save value for future use;
		m_cc_old[i] = m_cc[i];
	}
	if (!swa_inrange) {
		minc = src_nmin;
		maxc = src_nmax;
		//GetRealRange(m_c, m_cc);
		//ApplySourceRange();
	}
	// Set pitch limits
	// If too wide range is not accepted, correct range to increase scan performance
	for (int i = 0; i < c_len; ++i) {
		min_cc[i] = minc;
		max_cc[i] = maxc;
	}
	// Recalibrate 
	for (int i = 0; i < c_len; ++i) {
		if (minor_cur) {
			if (!m_diatonic_full[(min_cc[i] + 12 - tonic_cur) % 12]) ++min_cc[i];
		}
		else {
			if (!diatonic[(min_cc[i] + 12 - tonic_cur) % 12]) ++min_cc[i];
		}
	}
	// Convert limits to diatonic
	for (int i = 0; i < c_len; ++i) {
		min_c[i] = CC_C(min_cc[i], tonic_cur, minor_cur);
		max_c[i] = CC_C(max_cc[i], tonic_cur, minor_cur);
	}
	sp1 = 0;
	sp2 = c_len;
	ep1 = max(0, sp1 - 1);
	ep2 = c_len;
	// Clear flags
	++accepted3;
	fill(flags.begin(), flags.end(), 0);
	flags[0] = 1;
	for (int i = 0; i < ep2; ++i) {
		anflags[cpv][i].clear();
		anfl[cpv][i].clear();
	}
	// Matrix scan
	if (task != tEval) {
		// Exit if no violations
		if (!smatrixc) return;
		// Create map
		smap.resize(smatrixc);
		int map_id = 0;
		for (int i = 0; i < c_len; ++i) if (smatrix[i]) {
			smap[map_id] = i;
			++map_id;
		}
		sp1 = 0;
		sp2 = sp1 + s_len; // End of search window
		if (sp2 > smatrixc) sp2 = smatrixc;
		// Record window
		wid = 0;
		wpos1[wid] = sp1;
		wpos2[wid] = sp2;
		CalculateCcOrder(m_cc_old, 0, c_len);
		// Clear scan steps
		FillCantusMap(cc_id, smap, 0, smatrixc, 0);
		FillCantusMap(m_cc, smap, 0, smatrixc, cc_order);
		ep2 = GetMaxSmap() + 1;
		if (sp2 == smatrixc) ep2 = c_len;
		dpenalty_step.clear();
		dpenalty_step.resize(c_len, 0);		// End of evaluation window
		if (method == mScan) {
			ResizeToWindow();
			// Can skip flags - full scan must remove all flags
		}
		// For sliding windows algorithm evaluate whole melody
		if (method == mSWA) {
			sp2 = sp1 + min(swa_len, s_len);
			swa1 = 0;
			swa2 = swa1 + swa_len;
			// Record window
			swcount = 1;
			swid = 0;
			swpos1[swid] = swa1;
			swpos2[swid] = swa2;
			// Cannot skip flags - need them for penalty if cannot remove all flags
			skip_flags = 0;
			dpenalty_outside_swa = 0;
			// Next line is always false, but it is an important part of consistent approach
			//if (swa1 > 0) dpenalty_outside_swa += CalcDpenalty(cantus[cantus_id], m_cc, 0, smap[swa1 - 1]);
			if (swa2 < smap.size()) dpenalty_outside_swa += CalcDpenalty(cantus[cantus_id], m_cc, smap[swa2], c_len - 1);
			fill(src_rpenalty_step.begin(), src_rpenalty_step.end(), 0);
			if (sp2 == swa2) ep2 = c_len;
		}
		// Minimum element
		ep1 = max(0, GetMinSmap() - 1);
		// Minimal position in array to cycle
		pp = sp2 - 1;
		p = smap[pp];
	}
	else {
		// For single cantus scan - cannot skip flags - must show all
		skip_flags = 0;
		// SAS emulator
		if (fixed_ep2) {
			ep2 = fixed_ep2;
		}
	}
	// Absolute maximum of scan range for culminations
	max_cc2 = vmax(max_cc);
}

// Resize main scan vectors to current evaluation window size (ep2)
void CF1R::ResizeToWindow() {
	if (av_cnt > 1) {
		acc[cpv].resize(ep2);
		ac[cpv].resize(ep2);
		apcc[cpv].resize(ep2);
		apc[cpv].resize(ep2);
		aleap[cpv].resize(ep2);
		asmooth[cpv].resize(ep2);
		aslur[cpv].resize(ep2);
	}
	else {
		m_cc.resize(ep2);
		m_c.resize(ep2);
		m_pcc.resize(ep2);
		m_pc.resize(ep2);
		m_leap.resize(ep2);
		m_smooth.resize(ep2);
		m_slur.resize(ep2);
	}
	bli.resize(ep2);
	macc.resize(ep2);
	macc2.resize(ep2);
	decc.resize(ep2);
	decc2.resize(ep2);
	anflags[cpv].resize(ep2);
	anfl[cpv].resize(ep2);
}

// Step2 must be exclusive
void CF1R::RandCantus(vector<int>& c, vector<int>& cc, int step1, int step2)
{
	for (int i = step1; i < step2; ++i) { //-V756
		for (int x = 0; x < 1000; ++x) {
			c[i] = randbw(min_c[i], max_c[i]);
			// Convert to chromatic
			cc[i] = C_CC(c[i], tonic_cur, minor_cur);
			// Prevent note repeats in the starting cantus
			if ((i == 0 || cc[i] != cc[i - 1]) && (i > c_len - 2 || cc[i] != cc[i + 1])) break;
		}
	}
}

void CF1R::CalculateCcOrder(vector <int> &cc_old, int step1, int step2) {
	SET_READY(DR_cc_order);
	int x, x2;
	// First algorithm is needed when you correct existing melody with SAS or ASWA
	if (task == tCor) {
		CHECK_READY_PERSIST(DP_cc_old);
		int finished;
		// Fill notes starting with source melody, gradually moving apart
		for (int i = step1; i < step2; ++i) {
			cc_order[i].clear();
			// Send first note
			x = cc_old[i];
			if (x < min_cc[i]) x = min_cc[i];
			if (x > max_cc[i]) x = max_cc[i];
			cc_order[i].push_back(x);
			x2 = x;
			for (int z = 1; z < 1000; ++z) {
				finished = 1;
				x += cc_incr[x];
				x2 -= cc_decr[x2];
				if (rand() > RAND_MAX / 2) {
					if (x <= max_cc[i]) {
						cc_order[i].push_back(x);
						finished = 0;
					}
					if (x2 >= min_cc[i]) {
						cc_order[i].push_back(x2);
						finished = 0;
					}
				}
				else {
					if (x2 >= min_cc[i]) {
						cc_order[i].push_back(x2);
						finished = 0;
					}
					if (x <= max_cc[i]) {
						cc_order[i].push_back(x);
						finished = 0;
					}
				}
				if (finished) break;
			}
		}
	}
	// Second algorithm is needed when you create new melody for RSWA or SAS
	else {
		// Fill consecutive notes
		for (int i = step1; i < step2; ++i) {
			cc_order[i].clear();
			x = min_cc[i];
			while (x <= max_cc[i]) {
				cc_order[i].push_back(x);
				x += cc_incr[x];
			}
			// Shuffle
			if (random_seed)
				random_shuffle(cc_order[i].begin(), cc_order[i].end());
		}
	}
}

void CF1R::MakeNewCantus(vector<int> &c, vector<int> &cc) {
	// Set first and last notes
	c[0] = CC_C(first_note, tonic_cur, minor_cur);
	c[c_len - 1] = CC_C(last_note, tonic_cur, minor_cur);
	cc[0] = first_note;
	cc[c_len - 1] = last_note;
	GetRealRange(c, cc);
	// Set pitch limits
	for (int i = 0; i < c_len; ++i) {
		min_cc[i] = minc;
		max_cc[i] = maxc;
	}
	// Recalibrate 
	for (int i = 0; i < c_len; ++i) {
		if (minor_cur) {
			if (!m_diatonic_full[(min_cc[i] + 12 - tonic_cur) % 12]) ++min_cc[i];
		}
		else {
			if (!diatonic[(min_cc[i] + 12 - tonic_cur) % 12]) ++min_cc[i];
		}
	}
	// Convert limits to diatonic
	for (int i = 0; i < c_len; ++i) {
		min_c[i] = CC_C(min_cc[i], tonic_cur, minor_cur);
		max_c[i] = CC_C(max_cc[i], tonic_cur, minor_cur);
	}
	CalculateCcOrder(m_cc_old, 0, c_len);
	FillCantus(cc_id, 0, c_len, 0);
	FillCantus(cc, 0, c_len, cc_order);
}

void CF1R::MultiCantusInit(vector<int> &c, vector<int> &cc) {
	MakeNewCantus(c, cc);
	sp1 = 0; // Start of search window
	sp2 = sp1 + s_len; // End of search window
	if (sp2 > c_len) sp2 = c_len;
	// Record window
	wid = 0;
	wpos1[wid] = sp1;
	wpos2[wid] = sp2;
	ep1 = max(0, sp1 - 1);
	ep2 = sp2; // End of evaluation window
	p = sp2 - 1; // Minimal position in array to cycle
							 // Absolute maximum of scan range for culminations
	max_cc2 = vmax(max_cc);
	ResizeToWindow();
}

// Calculate flag statistics
void CF1R::CalcFlagStat() {
	if (calculate_stat || calculate_correlation) {
		SET_READY_PERSIST(DP_fstat);
		for (int i = 0; i < max_flags; ++i) if (!accept[i]) {
			if (flags[i]) {
				++fstat[i];
				// Calculate correlation
				if (calculate_correlation) {
					SET_READY_PERSIST(DP_fcor);
					for (int z = 0; z < max_flags; ++z) {
						if (flags[z]) ++fcor[i][z];
					}
				}
			}
		}
	}
}

// Calculate flag blocking
int CF1R::FailFlagBlock() {
	if (calculate_blocking) {
		SET_READY_PERSIST(DP_fblock);
		int flags_found = 0;
		int flags_found2 = 0;
		int flags_conflict = 0;
		// Find if any of accepted flags set
		for (int i = 0; i < max_flags; ++i) {
			if ((flags[i]) && (accept[i] > 0)) ++flags_found;
			if ((flags[i]) && (!accept[i])) ++flags_conflict;
			if ((flags[i]) && (accept[i] == 2)) ++flags_found2;
		}
		// Skip only if flags required
		if ((!late_require) || (ep2 == c_len)) {
			// Check if no needed flags set
			if (flags_found == 0) return 1;
			// Check if not enough 2 flags set
			if (flags_found2 < flags_need2) return 1;
		}
		++accepted5[wid];
		// Find flags that are blocking
		for (int i = 0; i < max_flags; ++i) {
			if ((flags[i]) && (!accept[i]))
				++fblock[wid][flags_conflict][i];
		}
	}
	return 0;
}

// Check if flags are accepted
int CF1R::FailAccept() {
	// Check prohibited strict flag only if not late_require or last window
	if ((!late_require) || (ep2 == c_len))
		if (flags[0] && !accept[0]) return 1;
	for (int i = 1; i < max_flags; ++i) {
		if (flags[i] && !accept[i]) return 1;
		if (!late_require || ep2 == c_len)
			if (!flags[i] && accept[i] == 2)
				return 1;
	}
	return 0;
}

// Check if too many windows
int CF1R::FailWindowsLimit() {
	if (((c_len - 2) / (float)s_len > MAX_WIND && task == tGen) || (method == mScan && task == tCor && smatrixc / s_len > MAX_WIND)) {
		CString est;
		est.Format("Error: generating %d notes with search window %d requires more than %d windows. Change MAX_WIND to allow more.",
			c_len, s_len, MAX_WIND);
		WriteLog(5, est);
		error = 1;
		return 1;
	}
	return 0;
}

void CF1R::NextWindow(vector<int> &cc) {
	if (task == tCor) {
		sp1 = sp2;
		sp2 = sp1 + min(swa_len, s_len);
		if (sp2 > smatrixc) sp2 = smatrixc;
		// Record window
		++wid;
		wpos1[wid] = sp1;
		wpos2[wid] = sp2;
		++wscans[wid];
		// End of evaluation window
		ep2 = GetMaxSmap() + 1;
		ep1 = max(0, GetMinSmap() - 1);
		if (method == mSWA) {
			if (sp2 == swa2) ep2 = c_len;
		}
		else {
			// Reserve last window with maximum length
			if ((smatrixc - sp1 < s_len * 2) && (smatrixc - sp1 > s_len)) sp2 = (smatrixc + sp1) / 2;
			if (sp2 == smatrixc) ep2 = c_len;
		}
		// Minimal position in array to cycle
		pp = sp2 - 1;
		p = smap[pp];
	}
	else {
		sp1 = sp2;
		sp2 = sp1 + s_len; // End of search window
											 // Move window forward
		if (!accept[267] && svoices > 1 && sp1 == mli.back()) {
			sp1 = c_len - s_len;
			sp2 = c_len;
		}
		if (sp2 > c_len) sp2 = c_len;
		// Reserve last window with maximum length
		//if ((c_len - sp1 < s_len * 2) && (c_len - sp1 > s_len)) sp2 = (c_len + sp1) / 2;
		// Record window
		++wid;
		wpos1[wid] = sp1;
		wpos2[wid] = sp2;
		++wscans[wid];
		// End of evaluation window
		ep1 = max(0, sp1 - 1);
		ep2 = sp2;
		// Go to rightmost element
		p = sp2 - 1;
	}
	if (wcount < wid + 1) {
		wcount = wid + 1;
		if (ep2 == c_len && method == mScan) {
			// Show window statistics
			CString est;
			CString st, st2;
			for (int i = 0; i < wcount; ++i) {
				if (i > 0) st2 += ", ";
				st.Format("%d-%d", wpos1[i], wpos2[i]);
				st2 += st;
			}
			//est.Format("Algorithm created %d windows: %s", wcount, st2);
			est.Format("Algorithm created %d windows", wcount);
			WriteLog(0, est);
		}
	}
	if (method == mScan) {
		ResizeToWindow();
		cc[ep2 - 1] = cc_order[ep2 - 1][cc_id[p]];
	}
}

// Check if rpenalty is not below than flags sum
void CF1R::TestRpenalty() {
	if (!flags.size()) return;
	int rp = 0;
	int found;
	//CString st, st2;
	for (int z = 0; z < max_flags; ++z) {
		if (!accept[z] && flags[z]) {
			rp += (severity[z] + 1) * flags[z] + fpenalty[z];
			found = 0;
			for (int x = 0; x < ep2; ++x) {
				if (anflags[cpv][x].size()) for (int i = 0; i < anflags[cpv][x].size(); ++i) if (anflags[cpv][x][i] == z) {
					found = 1;
				}
			}
			if (!found) {
				CString est;
				est.Format("Error: not found flag %d (which is set to %d) in anflags vector",
					z, flags[z]);
				WriteLog(5, est);
			}
		}
	}
	if (rpenalty_cur < rp) {
		CString est;
		est.Format("Error: rpenalty_cur %.0f is below rpenalty evaluation based on flags (%d)",
			rpenalty_cur, rp);
		WriteLog(5, est);
	}
}

// Check if rpenalty is not below than flags sum
void CF1R::TestBestRpenalty() {
	// Do not test if rpenalty == 0, because in this case best_flags are not saved
	if (!rpenalty_min) return;
	if (!best_flags.size()) return;
	int rp = 0;
	//CString st, st2;
	for (int x = 0; x < max_flags; ++x) {
		if (!accept[x] && best_flags[x]) rp += severity[x] + 1;
	}
	if (rpenalty_min < rp) {
		CString est;
		est.Format("Error: rpenalty_min %.0f is below rpenalty evaluation based on best_flags (%d)",
			rpenalty_min, rp);
		WriteLog(5, est);
	}
}

void CF1R::CalcRpenalty(vector<int> &cc) {
	SET_READY(DR_rpenalty_cur);
	// Calculate out of range penalty
	int real_range = nmax - nmin;
	if (!accept[37] && real_range > max_interval) {
		int nminr = nmin + (real_range - max_interval) / 2;
		int nmaxr = nminr + max_interval;
		for (int i = 0; i < ep2; ++i) {
			if (cc[i] < nminr) fpenalty[37] += nminr - cc[i];
			if (cc[i] > nmaxr) fpenalty[37] += cc[i] - nmaxr;
		}
	}
	// Calculate flags penalty
	rpenalty_cur = 0;
	for (int x = 0; x < ep2; ++x) {
		if (anflags[cpv][x].size()) for (int i = 0; i < anflags[cpv][x].size(); ++i) if (!accept[anflags[cpv][x][i]]) {
			rpenalty_cur += severity[anflags[cpv][x][i]] + 1;
		}
	}
	// Add flags penalty
	for (int x = 0; x < max_flags; ++x) {
		if (!accept[x]) rpenalty_cur += fpenalty[x];
	}
	// Save rpenalty for first scan position in swa window
	if (method == mSWA && !src_rpenalty_step[ep2 - 1] && sp1 == swa1 && cc_id[ep2 - 1] == 0) {
		src_rpenalty_step[ep2 - 1] = rpenalty_cur;
	}
}

void CF1R::ScanLeft(vector<int> &cc, int &finished) {
	while (true) {
		if (cc_id[p] < cc_order[p].size() - 1) break;
		// If current element is max, make it minimum
		cc_id[p] = 0;
		cc[p] = cc_order[p][0];
		// Move left one element
		if (task == tCor) {
			if (pp == sp1) {
				finished = 1;
				break;
			}
			pp--;
			p = smap[pp];
		}
		else {
			if (p == sp1) {
				finished = 1;
				break;
			}
			p--;
		}
	} // while (true)
}

// Show best rejected variant
void CF1R::ShowBestRejected(vector<int> &cc) {
	if (best_rejected) {
		long long time = CGLib::time();
		int rc = (time - accept_time) / best_rejected;
		if (debug_level > 2) {
			CString st;
			st.Format("Back window with rc %d", rc);
			WriteLog(3, st);
		}
		if (rc > rcycle) {
			rcycle = rc;
			if (br_cc.size() > 0) {
				// Save old cantus
				vector<int> cc_saved = cc;
				// Load best rejected cantus
				cc = br_cc;
				flags = br_f;
				anflags[cpv] = br_nf;
				anfl[cpv] = br_nfl;
				chm.clear();
				chm.resize(c_len, -1);
				SendCantus();
				cc = cc_saved;
				// Log
				if (debug_level > 0) {
					CString st;
					st.Format("Showing best rejected results with rpenalty %.0f", rpenalty_min);
					WriteLog(3, st);
				}
				// Clear
				br_cc.clear();
				rpenalty_min = MAX_PENALTY;
			}
			else {
				if (debug_level > 1)
					WriteLog(3, "No best rejected results to show");
			}
		}
	}
}

void CF1R::BackWindow(vector<int> &cc) {
	if (task == tCor) {
		// Clear current window
		FillCantusMap(cc_id, smap, sp1, sp2, 0);
		FillCantusMap(cc, smap, sp1, sp2, cc_order);
		// If this is not first window, go to previous window
		if (wid > 0) wid--;
		sp1 = wpos1[wid];
		sp2 = wpos2[wid];
		// End of evaluation window
		ep2 = GetMaxSmap() + 1;
		ep1 = max(0, GetMinSmap() - 1);
		if (sp2 == smatrixc) ep2 = c_len;
		// Minimal position in array to cycle
		pp = sp2 - 1;
		p = smap[pp];
	}
	// Normal full scan
	else {
		// Clear current window
		// When random seeding, even back window movement should be randomized to avoid autorestart window cycle
		//if (random_seed) RandCantus(c, sp1, sp2);
		//else
		FillCantus(cc_id, sp1, sp2, 0);
		FillCantus(cc, sp1, sp2, cc_order);
		// If this is not first window, go to previous window
		if (wid > 0) wid--;
		sp1 = wpos1[wid];
		sp2 = wpos2[wid];
		// End of evaluation window
		ep1 = max(0, sp1 - 1);
		ep2 = sp2;
		// Go to rightmost element
		p = sp2 - 1;
	}
	if (method == mScan) ResizeToWindow();
}

int CF1R::CalcDpenalty(vector<int> &cc1, vector<int> &cc2, int s1, int s2) {
	int dpe = 0;
	for (int z = s1; z <= s2; ++z) {
		int dif = abs(cc1[z] - cc2[z]);
		if (dif) dpe += step_penalty + pitch_penalty * dif;
	}
	return dpe;
}

void CF1R::CalcStepDpenalty(vector<int> &cc1, vector<int> &cc2, int i) {
	int dif = abs(cc1[i] - cc2[i]);
	int dpe = 0;
	if (dif) dpe = step_penalty + pitch_penalty * dif;
	if (i > 0) dpenalty_step[i] = dpenalty_step[i - 1] + dpe;
	else dpenalty_step[i] = dpe;
}

int CF1R::NextSWA(vector<int> &cc, vector<int> &cc_old) {
	// If we slided to the end, break
	if (swa2 == smatrixc) return 1;
	// Slide window further
	++swa1;
	++swa2;
	ep1 = max(0, GetMinSmap() - 1);
	// Minimal position in array to cycle
	pp = swa2 - 1;
	p = smap[pp];
	// Restore previous step after sliding window
	cc[smap[swa1 - 1]] = cc_old[smap[swa1 - 1]];
	// Clear scan steps of current window
	FillCantusMap(cc_id, smap, swa1, swa2, 0);
	FillCantusMap(cc, smap, swa1, swa2, cc_order);
	// Init new scan window
	sp1 = swa1;
	sp2 = swa1 + min(swa_len, s_len);
	if (sp2 > smatrixc) sp2 = smatrixc;
	wcount = 1;
	wid = 0;
	wpos1[wid] = sp1;
	wpos2[wid] = sp2;
	ep2 = GetMaxSmap() + 1;
	if (sp2 == swa2) ep2 = c_len;
	// Clear scan steps
	FillCantusMap(cc_id, smap, swa1, swa2, 0);
	FillCantusMap(cc, smap, swa1, swa2, cc_order);
	dpenalty_step.clear();
	dpenalty_step.resize(c_len, 0);
	// Prepare dpenalty
	dpenalty_outside_swa = 0;
	if (av_cnt == 2) {
		if (swa1 > 0) dpenalty_outside_swa += CalcDpenalty(cpoint[cantus_id][cpv], acc[cpv], 0, smap[swa1 - 1]);
		if (swa2 < smap.size()) dpenalty_outside_swa += CalcDpenalty(cpoint[cantus_id][cpv], acc[cpv], smap[swa2], c_len - 1);
	}
	else {
		if (swa1 > 0) dpenalty_outside_swa += CalcDpenalty(cantus[cantus_id], m_cc, 0, smap[swa1 - 1]);
		if (swa2 < smap.size()) dpenalty_outside_swa += CalcDpenalty(cantus[cantus_id], m_cc, smap[swa2], c_len - 1);
	}
	return 0;
}

void CF1R::SaveBestRejected(vector<int> &cc) {
	// Save best rejected results if we can analyze full cantus
	if (best_rejected && ep2 == c_len) {
		CalcRpenalty(cc);
		// Add result only if there is penalty, it is low and there are not note repeats
		if (rpenalty_cur < rpenalty_min && rpenalty_cur) {
			br_cc = cc;
			br_f = flags;
			br_nf = anflags[cpv];
			br_nfl = anfl[cpv];
			rpenalty_min = rpenalty_cur;
			// Log
			if (debug_level > 1) {
				CString st;
				st.Format("Saving best rejected results with rpenalty %.0f", rpenalty_min);
				WriteLog(3, st);
			}
		}
	}
}

void CF1R::ShowScanSpeed() {
	CString st;
	long long scan_time;
	if (task == tCor) scan_time = time() - correct_start_time;
	else scan_time = time() - scan_start_time;
	// Push new values
	q_scan_cycle.push_back(tcycle);
	q_scan_ms.push_back(time());
	// Get delta
	long long dms = q_scan_ms.back() - q_scan_ms.front();
	long long dcycle = q_scan_cycle.back() - q_scan_cycle.front();
	if (dcycle < 0) {
		WriteLog(5, "Error calculating scan speed: dcycle < 0");
	}
	// If delta big, remove old
	if (dms > 2000 && q_scan_ms.size() > 2) {
		q_scan_cycle.pop_front();
		q_scan_ms.pop_front();
	}
	// Create string
	CString speed_st;
	if (dms) {
		double sspeed = (double)dcycle / (double)dms;
		speed_st = HumanFloat(sspeed) + "/ms";
	}
	// Create status
	if (clib.size() > 0) st.Format("CL %zu, ST %lld, CY %s", clib.size(), scan_time / 1000, speed_st);
	else st.Format("ST %lld, CY %s", scan_time / 1000, speed_st);
	// Send status
	SetStatusText(5, st);
}

char CF1R::GetScanVisualCode(int i) {
	if (!i) return '\'';
	else return scan_visual_code[(i - 1) % SCAN_VISUAL_CODE_BASE];
}

void CF1R::ShowScanStatus() {
	if (task == tEval) return;
	CString st;
	//CString progress_st;
	if (task == tGen && method == mScan) {
		for (int i = 0; i < wcount; ++i) {
			//st.Format("%d ", cc_id[wpos1[i]]);
			//progress_st += st;
			//progress_st += GetScanVisualCode(cc_id[wpos1[i]]);
			SetProgress(i, cc_id[wpos1[i]]);
		}
	}
	else if (task == tCor && method == mScan) {
		for (int i = 0; i < smatrixc; ++i) {
			//st.Format("%d ", cc_id[smap[i]]);
			//progress_st += st;
			//progress_st += GetScanVisualCode(cc_id[smap[i]]);
			SetProgress(i, cc_id[smap[i]]);
		}
	}
	else if (method == mSWA) {
		for (int i = 0; i < smatrixc; ++i) {
			//st.Format("%d ", cc_id[smap[i]]);
			//progress_st += st;
			//progress_st += GetScanVisualCode(cc_id[smap[i]]);
			SetProgress(i, cc_id[smap[i]]);
		}
	}
	//if (!progress_st.IsEmpty()) SetStatusText(2, progress_st + " (Scan progress)");
	if (task == tCor) st.Format("WI %d/%d, RP %.0f, DP %d", wid + 1, wcount, rpenalty_min, dpenalty_min);
	else st.Format("WI %d/%d", wid + 1, wcount);
	SetStatusText(1, st);
	st.Format("Sent: %ld (ignored %ld)", cantus_sent, cantus_ignored);
	SetStatusText(0, st);
	ShowScanSpeed();
}

void CF1R::CheckClibSize() {
	if (warn_clib_max) return;
	if (clib.size() > MAX_CLIB_WARN) {
		warn_clib_max = 1;
		CString st;
		st.Format("Clib size (%zu) is greater than allowed size (%d). Please check your algorithm or increase MAX_CLIB_WARN", clib.size(), (int)MAX_CLIB_WARN);
		WriteLog(5, st);
	}
}

void CF1R::ReseedCantus()
{
	CString st;
	MultiCantusInit(m_c, m_cc);
	// Allow two seed cycles for each accept
	seed_cycle = 0;
	++reseed_count;
	st.Format("Reseed: %d", reseed_count);
	SetStatusText(4, st);
	//CString est;
	//est.Format("Reseed: ignored cantus %d while sent=%d", cantus_ignored, cantus_sent);
	//WriteLog(1, est);
}

void CF1R::WriteFlagCor() {
	// Write flag correlation
	if (calculate_correlation) {
		CHECK_READY_PERSIST(DP_fcor);
		DeleteFile("cf1-cor.csv");
		CString st, st2, st3;
		st3 = "Flag; Total; ";
		for (int i = 0; i < max_flags; ++i) {
			int f1 = i;
			st2.Format("%s; %lld; ",
				RuleName[cspecies][f1] + " (" + SubRuleName[cspecies][f1] + ")",
				fcor[f1][f1]);
			st3 += RuleName[cspecies][f1] + " (" + SubRuleName[cspecies][f1] + "); ";
			for (int z = 0; z < max_flags; ++z) {
				int f2 = i;
				st.Format("%lld; ", fcor[f1][f2]);
				st2 += st;
			}
			CGLib::AppendLineToFile("cf1-cor.csv", st2 + "\n");
		}
		CGLib::AppendLineToFile("cf1-cor.csv", st3 + "\n");
	}
}

void CF1R::ShowFlagStat() {
	CString st, st2;
	int lines = 0;
	// Show flag statistics
	if (calculate_stat && data_ready_persist[DP_fstat]) {
		//CHECK_READY_PERSIST(DP_fstat);
		for (int d = 1; d < max_flags; ++d) {
			if (lines > 100) break;
			int flagc = 0;
			for (int x = 0; x < max_flags; ++x) {
				if (fstat[x] > 0) ++flagc;
			}
			if (!flagc) continue;
			int max_flag = 0;
			long max_value = -1;
			for (int x = 0; x < max_flags; ++x) {
				max_value = -1;
				// Find biggest value
				for (int i = 0; i < max_flags; ++i) {
					if (fstat[i] > max_value) {
						max_value = fstat[i];
						max_flag = i;
					}
				}
				if (max_value < 1) break;
				st.Format("\n%ld %s, ", max_value, RuleName[cspecies][max_flag] + " (" + SubRuleName[cspecies][max_flag] + ")");
				st2 += st;
				++lines;
				// Clear biggest value to search for next
				fstat[max_flag] = -1;
			}
		}
		CString est;
		est.Format("%d/%d: Accepted %lld/%lld/%lld/%lld variants of %lld: %s",
			c_len, max_interval, accepted4[wcount - 1], accepted, accepted2,
			accepted3, cycle, st2);
		WriteLog(3, est);
	}
}

void CF1R::ShowStuck() {
	if (!ssf.size()) return;
	CString st, st2;
	// Show flag statistics
	if (calculate_ssf) {
		st2 = "SWA stuck flags: ";
		int max_flag = 0;
		long max_value = -1;
		for (int x = 0; x < max_flags; ++x) {
			max_value = -1;
			// Find biggest value
			for (int i = 0; i < max_flags; ++i) {
				if (ssf[i] > max_value) {
					max_value = ssf[i];
					max_flag = i;
				}
			}
			if (max_value < 1) break;
			st.Format("\n%ld %s, ", max_value, RuleName[cspecies][max_flag] + " (" + SubRuleName[cspecies][max_flag] + ")");
			st2 += st;
			// Clear biggest value to search for next
			ssf[max_flag] = -1;
		}
		WriteLog(3, st2);
	}
}

CString CF1R::GetStuck() {
	if (!best_flags.size()) return "";
	CString st, st2;
	int max_flag = 0;
	long max_value = -1;
	for (int x = 0; x < max_flags; ++x) {
		max_value = -1;
		// Find biggest value
		for (int i = 0; i < max_flags; ++i) {
			if (best_flags[i] > max_value) {
				max_value = best_flags[i];
				max_flag = i;
			}
		}
		if (max_value < 1) break;
		if (!accept[max_flag]) {
			st.Format("\n%ld %s, ", max_value, RuleName[cspecies][max_flag] + " (" + SubRuleName[cspecies][max_flag] + ")");
			st2 += st;
		}
		// Clear biggest value to search for next
		best_flags[max_flag] = -1;
	}
	return st2;
}

void CF1R::ShowFlagBlock() {
	CString st, st2;
	// Show blocking statistics
	if (calculate_blocking && method == mScan && data_ready_persist[DP_fblock]) {
		//CHECK_READY_PERSIST(DP_fblock);
		for (int w = 0; w < wcount; ++w) {
			int lines = 0;
			CString est;
			st2.Empty();
			for (int d = 1; d < max_flags; ++d) {
				if (lines > 100) break;
				int flagc = 0;
				for (int x = 0; x < max_flags; ++x) {
					if (fblock[w][d][x] > 0) ++flagc;
				}
				if (!flagc) continue;
				int max_flag = 0;
				long max_value = -1;
				st.Format("\nTIER %d: ", d);
				st2 += st;
				for (int x = 0; x < max_flags; ++x) {
					max_value = -1;
					// Find biggest value
					for (int i = 0; i < max_flags; ++i) {
						if (fblock[w][d][i] > max_value) {
							max_value = fblock[w][d][i];
							max_flag = i;
						}
					}
					if (max_value < 1) break;
					st.Format("\n%ld %s, ", max_value, RuleName[cspecies][max_flag] + " (" + SubRuleName[cspecies][max_flag] + ")");
					st2 += st;
					++lines;
					// Clear biggest value to search for next
					fblock[w][d][max_flag] = -1;
				}
			}
			est.Format("Window %d: %lld scans, %lld of %lld variants blocked: %s", w, wscans[w],
				accepted5[w] - accepted4[w], accepted5[w], st2);
			WriteLog(3, est);
		}
	}
}

void CF1R::TransposeVector(vector<int> &v, int t) {
	for (int i = 0; i < v.size(); ++i) {
		v[i] += t;
	}
}

void CF1R::TransposeVector(vector<float> &v, int t) {
	for (int i = 0; i < v.size(); ++i) {
		v[i] += t;
	}
}

void CF1R::InterpolateNgraph(int v, int step0, int step) {
	// Interpolate ngraph
	int pos1, pos2;
	for (int n = 0; n < ngraph_size; ++n) {
		pos1 = 0;
		pos2 = 0;
		for (int i = step0; i < step; ++i) {
			if (!ngraph[i][v][n]) {
				// Detect start
				if (!pos1 && pos2) pos1 = i;
			}
			else {
				// Detect finish
				pos2 = i;
				if (pos1) {
					// Detected start and finish
					for (int x = pos1; x < pos2; ++x) {
						ngraph[x][v][n] =
							(ngraph[pos1 - 1][v][n] * (pos2 - x) +
								ngraph[pos2][v][n] * (x - pos1 + 1)) /
								(pos2 - pos1 + 1);
					}
				}
				pos1 = 0;
			}
		}
	}
}

void CF1R::SendNgraph(int pos, int i, int v, int x) {
	if (i == (cc_len[x] - 1) / 2) {
		float ma = macc2[x];
		float de = decc2[x];
		ngraph[pos + i][v][0] = ma - de;
		ngraph[pos + i][v][1] = ma;
		ngraph[pos + i][v][2] = ma + de;
	}
	else {
		ngraph[pos + i][v][0] = 0;
		ngraph[pos + i][v][1] = 0;
		ngraph[pos + i][v][2] = 0;
	}
}

void CF1R::SendGraph(int pos, int i, int v, int x) {
	graph[pos + i][v][0] = tweight[bli[x]];
	graph[pos + i][v][1] = g_leaps[bli[x]];
	graph[pos + i][v][2] = g_leaped[bli[x]];
}

void CF1R::SendLyrics(int pos, int v, int av, int x) {
	if (cantus_incom.size() > cantus_id && cantus_incom[cantus_id].size() > x) {
		lyrics[pos][v] = cantus_incom[cantus_id][x];
	}
}

void CF1R::SendComment(int pos, int v, int av, int x, int i) {
	CString st, com;
	int current_severity = -1;
	// Clear
	nlink[pos + i][v].clear();
	comment2[pos + i][v].Empty();
	comment[pos + i][v].clear();
	ccolor[pos + i][v].clear();
	if (anflags[av][x].size() > 0) for (int f = 0; f < anflags[av][x].size(); ++f) {
		// Do not show colors and comments for base voice
		if (av == cpv) {
			int fl = anflags[av][x][f];
			// Send comments and color only if rule is not ignored
			if (accept[fl] == -1 && !show_ignored_flags) continue;
			// Send comments and color only if rule is not ignored
			if (accept[fl] == 1 && !show_allowed_flags) continue;
			// Do not send if ignored
			if (severity[fl] < show_min_severity) continue;
			if (!i) {
				if (!accept[fl]) st = "- ";
				else if (accept[fl] == -1) st = "$ ";
				else st = "+ ";
				com = st + RuleName[cspecies][fl] + " (" + SubRuleName[cspecies][fl] + ")";
				if (!comment2[pos][v].IsEmpty()) comment2[pos][v] += ", ";
				comment2[pos][v] += RuleName[cspecies][fl] + " (" + SubRuleName[cspecies][fl] + ")";
				if (show_severity) {
					st.Format(" [%d/%d]", fl, severity[fl]);
					com += st;
				}
				if (!RuleComment[cspecies][fl].IsEmpty()) com += ". " + RuleComment[cspecies][fl];
				if (!SubRuleComment[cspecies][fl].IsEmpty()) com += " (" + SubRuleComment[cspecies][fl] + ")";
				//com += ". ";
				comment[pos][v].push_back(com);
				ccolor[pos][v].push_back(flag_color[severity[fl]]);
				if (!ly_debugexpect) {
					nlink[pos][v][fl * 10 + cspecies] = anfl[av][x][f] - x;
					fsev[pos][v][fl * 10 + cspecies] = severity[fl];
				}
			}
			// Set note color if this is maximum flag severity
			if (severity[fl] > current_severity && severity[fl] >= show_min_severity
				&& rule_viz[fl] != vHarm) {
				current_severity = severity[fl];
				color[pos + i][v] = flag_color[severity[fl]];
			}
		}
	}
}

void CF1R::TransposeCantusBack() {
	// Transpose cantus
	if (transpose_back && first_note) {
		int trans = 0;
		if (nmin > first_note0) {
			trans = -floor((nmin - first_note0) / 12 + 1) * 12;
			if (nmax + trans < first_note0) trans = 0;
		}
		if (nmax < first_note0) {
			trans = floor((first_note0 - nmax) / 12 + 1) * 12;
			if (nmin + trans > first_note0) trans = 0;
		}
		TransposeVector(m_cc, trans);
		TransposeVector(macc2, trans);
		TransposeVector(min_cc, trans);
		TransposeVector(max_cc, trans);
		TransposeVector(min_cc0, trans);
		TransposeVector(max_cc0, trans);
	}
}

void CF1R::SendNotes(int pos, int i, int v, int av, int x, vector<int> &cc) {
	note[pos + i][v] = cc[x];
	tonic[pos + i][v] = tonic_cur;
	minor[pos + i][v] = minor_cur;
	midifile_out_mul[pos + i] = midifile_out_mul0 * midifile_out_mul2;
	len[pos + i][v] = len_export[x];
	pause[pos + i][v] = 0;
	coff[pos + i][v] = coff_export[x] + i;
	// Add scan range
	if (show_note_scan_range) {
		if (av == cpv) {
			// Send source scan range in all cases
			if (min_cc0.size()) nsr1[pos + i][v] = min_cc0[x];
			else nsr1[pos + i][v] = min_cc[x];
			if (max_cc0.size()) nsr2[pos + i][v] = max_cc0[x];
			else nsr2[pos + i][v] = max_cc[x];
		}
		else {
			// Send rswa scan range if rswa
			if (min_cc0.size()) nsr1[pos + i][v] = min_cc[x];
			if (max_cc0.size()) nsr2[pos + i][v] = max_cc[x];
		}
	}
	// Assign source tempo if exists
	if (cc_tempo[x]) {
		if (av_cnt != 1 && svoices == 1) tempo[pos + i] = cc_tempo[x] * 4;
		else tempo[pos + i] = cc_tempo[x];
	}
	// Generate tempo if no source
	else {
		if (pos + i == 0) {
			tempo[pos + i] = min_tempo + (float)(max_tempo - min_tempo) * (float)rand2() / (float)RAND_MAX;
		}
		else {
			tempo[pos + i] = tempo[pos + i - 1] + randbw(-1, 1);
			if (tempo[pos + i] > max_tempo) tempo[pos + i] = 2 * max_tempo - tempo[pos + i];
			if (tempo[pos + i] < min_tempo) tempo[pos + i] = 2 * min_tempo - tempo[pos + i];
		}
	}
}

// Create bell dynamics curve
void CF1R::MakeBellDyn(int v, int step1, int step2, int dyn1, int dyn2, int dyn_rand) {
	// Do not process if steps are equal or wrong
	if (step2 <= step1) return;
	int mids = (step1 + step2) / 2;
	int counts = step2 - step1;
	for (int s = step1; s <= step2; ++s) {
		if (s < mids)	dyn[s][v] = dyn1 + min(dyn2 - dyn1, (dyn2 - dyn1) * (s - step1) / counts * 2) + dyn_rand * rand2() / RAND_MAX;
		else dyn[s][v] = dyn1 + min(dyn2 - dyn1, (dyn2 - dyn1) * (step2 - s) / counts * 2) + dyn_rand * rand2() / RAND_MAX;
	}
}

// Create bell dynamics curve
void CF1R::MakeBellTempo(int step1, int step2, int tempo1, int tempo2) {
	// Do not process if steps are equal or wrong
	if (step2 <= step1) return;
	// Do not process if tempo is not uniform
	for (int s = step1; s <= step2; ++s) {
		if (tempo[s] != tempo[step1]) return;
	}
	int mids = (step1 + step2) / 2;
	int counts = step2 - step1;
	for (int s = step1; s <= step2; ++s) {
		if (s <= mids)	tempo[s] = tempo1 + (tempo2 - tempo1) * pow((s - step1) * 2.0 / counts, 0.5);
		else tempo[s] = tempo1 + (tempo2 - tempo1) * pow((step2 - s) * 2.0 / counts, 0.5);
	}
}

// Send inter-cantus pause
int CF1R::SendPause(int pos, int v) {
	int pause_len = floor((pos + 1) / 8 + 1) * 8 - pos;
	FillPause(pos, pause_len, v);
	for (int i = pos; i <= pos + pause_len; ++i) tempo[i] = tempo[i - 1];
	return pause_len;
}

void CF1R::MakeLenExport(vector<int> &cc, int av, int retr_on)
{
	int len_temp, last_pos;
	// Create note length
	last_pos = 0;
	len_temp = 0;
	for (s = 0; s < c_len; ++s) {
		if (cc[s] != cc[last_pos] || (retr_on && retrigger[s])) {
			for (int s2 = last_pos; s2 < s; ++s2) {
				len_export[s2] = len_temp;
			}
			last_pos = s;
			len_temp = 0;
		}
		coff_export[s] = len_temp;
		len_temp += cc_len[s];
	}
	for (int s2 = last_pos; s2 < c_len; ++s2) {
		len_export[s2] = len_temp;
	}
}

void CF1R::SendHarmColor(int pos, int v) {
	DWORD fc;
	mark_color[pos][v] = MakeColor(255, 150, 150, 150);
	// Scan flags
	int f_cnt = anflags[cpv][s].size();
	int max_severity = -1;
	int fl;
	for (int f = 0; f < f_cnt; ++f) {
		fl = anflags[cpv][s][f];
		if (rule_viz[fl] == vHarm && !accept[fl] && severity[fl] >= show_min_severity) {
			if (severity[fl] > max_severity) max_severity = severity[fl];
		}
	}
	if (max_severity > -1) {
		fc = flag_color[max_severity];
		mark_color[pos][v] = MakeColor(GetAlpha(fc), GetRed(fc),
			GetGreen(fc) / 1.5, GetBlue(fc));
	}
}

// Merge notes of same pitch, that do not have pauses between them. Step2 inclusive
void CF1R::MergeNotes(int step1, int step2, int v) {
	// Start of current note
	int first_pos = step1;
	DWORD col = color[step1][v];
	for (int x = step1 + 1; x <= step2; ++x) {
		// Detect steps that have same pitch and there is no retrigger 
		if (coff[x][v]) {
			// select best color: gray is ignored, then most red is selected
			if (color[x][v] != color_noflag &&
				(col == color_noflag || GetRed(color[x][v]) > GetRed(col))) {
				col = color[x][v];
				// update color of previous steps
				for (int z = first_pos; z < x; ++z) {
					color[z][v] = col;
				}
			}
			coff[x][v] = coff[x - 1][v] + 1;
			// Copy color forward
			color[x][v] = col;
		}
		// New note
		else {
			first_pos = x;
			col = color[x][v];
		}
	}
}

CString CF1R::GetHarmName(int pitch, int alter) {
	if (minor_cur) {
		if (alter == 1) return HarmName_ma[pitch];
		else return HarmName_m[pitch];
	}
	else return HarmName[pitch];
}

int CF1R::SendCantus() {
	int step000 = step;
	float l_rpenalty_cur;
	// Save culmination position
	cf_culm_cfs = culm_ls;
	if (svoice < 0) return 0;
	CString st, st2, rpst;
	int v = svoice;
	Sleep(sleep_ms);
	TransposeCantusBack();
	len_export.resize(c_len);
	coff_export.resize(c_len);
	CalcPmap(m_pcc, m_cc, m_c, m_smooth, m_leap);
	GetPmap();
	LogPmap();
	MakeLenExport(m_cc, 0, 0);
	// Copy cantus to output
	int pos = step;
	if (step + real_len >= t_allocated) ResizeVectors(t_allocated * 2);
	for (s = 0; s < ep2; ++s) {
		cpos[s] = pos;
		sstep[pos] = s;
		if (chm.size() > bli[s] && chm[bli[s]] > -1) {
			mark[pos][v] = GetHarmName(chm[bli[s]], m_cc[s] == 9 || m_cc[s] == 11);
			SendHarmColor(pos, v);
		}
		SendLyrics(pos, v, cpv, s);
		for (int i = 0; i < cc_len[s]; ++i) {
			if (av_cnt != 1) note_muted[pos + i][v] = 1;
			color[pos + i][v] = MakeColor(0, 100, 100, 100);
			SendNotes(pos, i, v, cpv, s, m_cc);
			SendNgraph(pos, i, v, s);
			SendGraph(pos, i, v, s);
			SendComment(pos, v, cpv, s, i);
		}
		pos += cc_len[s];
	}
	MakeBellDyn(v, step, pos - 1, 50, 110, 0);
	if (tempo_bell) MakeBellTempo(step, pos - 1, tempo[step], tempo_bell * tempo[step]);
	step = pos + SendPause(pos, v);
	InterpolateNgraph(v, step000, step);
	// Count additional variables
	CountOff(step000, step - 1);
	CountTime(step000, step - 1);
	UpdateNoteMinMax(step000, step - 1, !is_animating);
	UpdateTempoMinMax(step000, step - 1);
	// Increment cantus_sent only if is not animating
	if (!is_animating) ++cantus_sent;
	// Create rule penalty string
	l_rpenalty_cur = rpenalty_cur;
	if (!skip_flags) {
		for (int x = 0; x < max_flags; ++x) {
			if (!accept[x] && fpenalty[x]) {
				st.Format("%d=%.0f", x, fpenalty[x]);
				if (!rpst.IsEmpty()) rpst += ", ";
				rpst += st;
			}
		}
		if (rpenalty_cur == MAX_PENALTY) {
			l_rpenalty_cur = 0;
			rpst.Empty();
		}
	}
	// Save source fpenalty 
	if (!v) fpenalty_source = rpst;
	if (task == tGen) {
		if (!shuffle) {
			Adapt(step000, step - 1);
		}
		// If  window-scan
		st.Format("#%d\nCantus: %s",
			cantus_sent, cantus_high ? "upper part" : "lower part");
		st2.Format("Rule penalty: %.0f\nFlags penalty: %s\n%s", l_rpenalty_cur, rpst, pmap);
		AddMelody(step000, pos - 1, v, st, st2);
		if (v) AddMelody(step000, pos - 1, 0, st);
	}
	else if (task == tEval) {
		if (m_algo_id == 101) {
			// If RSWA
			st.Format("#%d\nCantus: %s",
				cantus_sent, cantus_high ? "upper part" : "lower part");
			st2.Format("Rule penalty: %.0f\nFlags penalty: %s\n%s", l_rpenalty_cur, rpst, pmap);
		}
		else {
			if (key_eval.IsEmpty()) {
				// If SWA
				st.Format("#%d (from %s)\nRule penalty: %.0f => %.0f\nDistance penalty: %d\nCantus: %s",
					cantus_id + 1, bname_from_path(midi_file),
					rpenalty_source, l_rpenalty_cur, dpenalty_cur, cantus_high ? "upper part" : "lower part");
				st2.Format("Flags penalty: %s => %s\n%s",
					fpenalty_source, rpst, pmap);
			}
			else {
				// If evaluating
				st.Format("#%d (from %s)\nCantus: %s",
					cantus_id + 1, bname_from_path(midi_file), cantus_high ? "upper part" : "lower part");
				st2.Format("Rule penalty: %.0f\nFlags penalty: %s\nKey selection: %s\n%s",
					l_rpenalty_cur, rpst, key_eval, pmap);
			}
		}
		AddMelody(step000, pos - 1, v, st, st2);
		if (v) AddMelody(step000, pos - 1, 0, st);
	}
	// Send
	t_generated = step;
	if (task == tGen) {
		if (!shuffle) {
			// Add line
			linecolor[t_sent] = MakeColor(255, 0, 0, 0);
			t_sent = t_generated;
		}
	}
	st.Format("Sent: %ld (ignored %ld)", cantus_sent, cantus_ignored);
	SetStatusText(0, st);
	// Check limit
	if (t_generated >= t_cnt) {
		WriteLog(3, "Reached t_cnt steps. Generation stopped");
		return 1;
	}
	return 0;
}

inline void CF1R::ClearReady() {
	fill(data_ready.begin(), data_ready.end(), 0);
}

inline void CF1R::SetReady(int id) {
	data_ready[id] = 1;
}

inline void CF1R::SetReady(int id, int id2) {
	data_ready[id] = 1;
	data_ready[id2] = 1;
}

inline void CF1R::SetReady(int id, int id2, int id3) {
	data_ready[id] = 1;
	data_ready[id2] = 1;
	data_ready[id3] = 1;
}

inline void CF1R::ClearReadyPersist(int id) {
	data_ready_persist[id] = 0;
}

inline void CF1R::ClearReadyPersist(int id, int id2) {
	data_ready_persist[id] = 0;
	data_ready_persist[id2] = 0;
}

inline void CF1R::ClearReadyPersist(int id, int id2, int id3) {
	data_ready_persist[id] = 0;
	data_ready_persist[id2] = 0;
	data_ready_persist[id3] = 0;
}

inline void CF1R::SetReadyPersist(int id) {
	data_ready_persist[id] = 1;
}

inline void CF1R::SetReadyPersist(int id, int id2) {
	data_ready_persist[id] = 1;
	data_ready_persist[id2] = 1;
}

inline void CF1R::SetReadyPersist(int id, int id2, int id3) {
	data_ready_persist[id] = 1;
	data_ready_persist[id2] = 1;
	data_ready_persist[id3] = 1;
}

inline void CF1R::CheckReady(int id) {
	if (!data_ready[id] && !warn_data_ready[id]) {
		++warn_data_ready[id];
		CString est;
		est.Format("Attemp to use data element '%d' while it is not ready yet", id);
		WriteLog(5, est);
		ASSERT(0);
	}
}

inline void CF1R::CheckReady(int id, int id2) {
	CheckReady(id);
	CheckReady(id2);
}

inline void CF1R::CheckReady(int id, int id2, int id3) {
	CheckReady(id);
	CheckReady(id2);
	CheckReady(id3);
}

inline void CF1R::CheckReadyPersist(int id) {
	if (!data_ready_persist[id] && !warn_data_ready_persist[id]) {
		++warn_data_ready_persist[id];
		CString est;
		est.Format("Attemp to use persistent data element '%d' while it is not ready yet", id);
		WriteLog(5, est);
		ASSERT(0);
	}
}

inline void CF1R::CheckReadyPersist(int id, int id2) {
	CheckReadyPersist(id);
	CheckReadyPersist(id2);
}

inline void CF1R::CheckReadyPersist(int id, int id2, int id3) {
	CheckReadyPersist(id);
	CheckReadyPersist(id2);
	CheckReadyPersist(id3);
}

// Calculate parameter map
void CF1R::CalcPmap(vector<int> &pcc, vector<int> &cc, vector<int> &c, vector<int> &smooth, vector<int> &leap) {
	pm_range = nmax - nmin;
	pm_tonic = 0;
	pm_sharp6 = 0;
	pm_sharp7 = 0;
	pm_flat6 = 0;
	pm_flat7 = 0;
	pm_decc_min = INT_MAX;
	pm_decc_max = 0;
	pm_decc_av = 0;
	pm_leapsum = 0;
	pm_leaps = 0;
	pm_smooth = 0;
	for (ls = 0; ls < fli_size; ++ls) {
		s = fli[ls];
		// Note frequency
		if (!pcc[s]) ++pm_tonic;
		else if (pcc[s] == 11) ++pm_sharp7;
		else if (pcc[s] == 10) ++pm_flat7;
		else if (pcc[s] == 9) ++pm_sharp6;
		else if (pcc[s] == 8) ++pm_flat6;
		// Average decc
		pm_decc_av += decc2[s];
		if (ls) {
			if (smooth[s - 1]) ++pm_smooth;
			else if (leap[s - 1]) {
				++pm_leaps;
				pm_leapsum += abs(c[s] - c[s - 1]) + 1;
			}
		}
	}
	pm_decc_av /= fli_size;
	for (s = 0; s < ep2; ++s) {
		if (pm_decc_min > decc2[s]) pm_decc_min = decc2[s];
		if (pm_decc_max < decc2[s]) pm_decc_max = decc2[s];
	}
}

// Get parameter map string
void CF1R::GetPmap() {
	CString st;
	pmap.Empty();
	st.Format("Range: %d semitones\n", pm_range);
	pmap += st;
	st.Format("Culminations: %d\n", pm_culm_count);
	pmap += st;
	st.Format("Notes: %d\n", fli_size);
	pmap += st;
	st.Format("Tonic notes / max tonic weight: %d / %s\n",
		pm_tonic, HumanFloat(pm_tw_max));
	pmap += st;
	st.Format("Min / av / max pitch deviation: %s / %s / %s\n",
		HumanFloatPrecision(pm_decc_min), HumanFloatPrecision(pm_decc_av),
		HumanFloatPrecision(pm_decc_max));
	pmap += st;
	st.Format("Min / av / max MA pitch range in window %d: %s / %s / %s\n",
		notes_arange2, HumanFloatPrecision(pm_maccr_min),
		HumanFloatPrecision(pm_maccr_av), HumanFloatPrecision(pm_maccr_max));
	pmap += st;
	st.Format("Smooth / leaps / leaped: %d / %d / %d\n", pm_smooth, pm_leaps, pm_leapsum);
	pmap += st;
	st.Format("2 / 3 consecutive leaps: %d / %d\n", pm_leaps2, pm_leaps3);
	pmap += st;
	st.Format("Leaps / leaped in window %d: %d / %d\n", max_leap_steps2, pm_win_leaps, pm_win_leapnotes);
	pmap += st;
	if (minor_cur) {
		st.Format("VI / VI# / VII / VII#: %d / %d / %d / %d\n",
			pm_flat6, pm_sharp6, pm_flat7, pm_sharp7);
		pmap += st;
	}
}

CString CF1R::GetPmapLogHeader() {
	CString st;
	st += "Time;Algorithm;Config;Midi;Voice;ID;High;Steps;Notes;Tonic;Minor;";
	st += "Range;Culminations;";
	st += "Decc_min;Decc_av;Decc_max;Maccr_min;Maccr_av;Maccr_max;";
	st += "Tonics;VI;VI#;VII;VII#;";
	st += "Smooth;Leaps;2leaps;3leaps;Leap size;Leaps_win;Leaped_win;";
	return st;
}

CString CF1R::GetPmapLogSt() {
	CString st, st2;
	st.Format("%s;%s;%s;%s;%d;%d;%d;%d;%d;%d;%d;",
		CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S"),
		m_algo_folder, m_config, bname_from_path(midi_file), svoice, cantus_id + 1, cantus_high, ep2,
		fli_size, tonic_cur, minor_cur);
	st2 += st;
	st.Format("%d;%d;",
		pm_range, pm_culm_count);
	st2 += st;
	st.Format("%s;%s;%s;%s;%s;%s;",
		HumanFloatPrecision(pm_decc_min), HumanFloatPrecision(pm_decc_av),
		HumanFloatPrecision(pm_decc_max), HumanFloatPrecision(pm_maccr_min),
		HumanFloatPrecision(pm_maccr_av), HumanFloatPrecision(pm_maccr_max));
	st2 += st;
	st.Format("%d;%d;%d;%d;%d;",
		pm_tonic, pm_flat6, pm_sharp6, pm_flat7, pm_sharp7);
	st2 += st;
	st.Format("%.0f%%;%.0f%%;%.0f%%;%.0f%%;%.2f;%d;%d;",
		100.0 * pm_smooth / (pm_leaps + pm_smooth),
		100.0 * pm_leaps / (pm_leaps + pm_smooth),
		100.0 * pm_leaps2 / pm_leaps,
		100.0 * pm_leaps3 / pm_leaps,
		1.0 * pm_leapsum / pm_leaps,
		pm_win_leaps, pm_win_leapnotes);
	st2 += st;
	st2.Replace(".", ",");
	return st2;
}

// Log parameter map
void CF1R::LogPmap() {
	CString fname = "log\\cf-pmap.csv";
	// Header
	if (!fileExists(fname)) {
		//AppendLineToFile(fname, GetPmapLogHeader() + "\n");
	}
	//AppendLineToFile(fname, GetPmapLogSt() + "\n");
}

