// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "CP2R.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CP2R::CP2R() {
	// Data ready
	data_ready.resize(MAX_DATA_READY);
	data_ready_persist.resize(MAX_DATA_READY_PERSIST);
	warn_data_ready.resize(MAX_DATA_READY);
	warn_data_ready_persist.resize(MAX_DATA_READY_PERSIST);
}

CP2R::~CP2R() {
}

void CP2R::ScanCP() {
	EvaluateCP();
}

inline void CP2R::FlagV(int voice, int fid, int step) {
	AssertRule(fid);
	flag[voice][step].push_back(fid);
	fsl[voice][step].push_back(step);
	fvl[voice][step].push_back(voice);
}

inline void CP2R::FlagVL(int voice, int fid, int step, int step2) {
	AssertRule(fid);
	flag[voice][step].push_back(fid);
	fsl[voice][step].push_back(step2);
	fvl[voice][step].push_back(voice);
}

inline void CP2R::Flag(int voice, int fid, int step, int voice2) {
	AssertRule(fid);
	flag[voice][step].push_back(fid);
	fsl[voice][step].push_back(step);
	fvl[voice][step].push_back(voice2);
}

inline void CP2R::FlagL(int voice, int fid, int step, int step2, int voice2) {
	AssertRule(fid);
	flag[voice][step].push_back(fid);
	fsl[voice][step].push_back(step2);
	fvl[voice][step].push_back(voice2);
}

// Accumulate flag
inline void CP2R::FlagA(int voice, int fid, int step, int step2, int voice2, int ihpe) {
	AssertRule(fid);
	temp_flaginfo.voice = voice; 
	temp_flaginfo.s = step;
	temp_flaginfo.id = fid;
	temp_flaginfo.fsl = step2;
	temp_flaginfo.fvl = voice2;
	flaga.push_back(temp_flaginfo);
	if (!accept[sp][vc][0][fid]) hpenalty += ihpe;
}

inline void CP2R::AssertRule(int fid) {
	if (ruleinfo[fid].SubRuleName.IsEmpty() && warn_rule_undefined < 5) { 
		++warn_rule_undefined; 
		CString est; 
		est.Format("Detected undefined rule usage: %d", fid); 
		WriteLog(5, est); 
		ASSERT(0); 
	} 
}

int CP2R::EvaluateCP() {
	CLEAR_READY();
	ClearFlags(0, c_len);
	GetDiatonic(0, c_len);
	GetPitchClass(0, c_len);
	CreateLinks();
	GetNoteLen();
	GetNoteTypes();
	GetVca();
	GetLClimax();
	GetLeapSmooth();
	if (FailVocalRanges()) return 1;
	if (FailMeasureLen()) return 1;
	FailStartPause();
	FlagRhythmStagnation();

	for (v = 0; v < av_cnt; ++v) {
		sp = vsp[v];
		vaccept = &accept[sp][av_cnt][0];
		GetMelodyInterval(0, c_len);
		// Analysis
		if (FailRetrInside()) return 1;
		if (FailMode()) return 1;
		if (FailPauses()) return 1;
		if (FailNoteLen()) return 1;
		if (FailBeat()) return 1;

		if (FailVRLimit()) return 1;
		if (FailVocalRangesConflict()) return 1;
		if (av_cnt == 1) {
			if (FailNoteRepeat()) return 1;
		}
		if (FailFirstNotes()) return 1;
		if (FailCross()) return 1;
		if (FailOverlap()) return 1;
		if (FailLocalPiCount(notes_picount[sp][av_cnt][0], min_picount[sp][av_cnt][0], 344)) return 1;
		if (FailLocalPiCount(notes_picount2[sp][av_cnt][0], min_picount2[sp][av_cnt][0], 345)) return 1;
		if (FailLocalPiCount(notes_picount3[sp][av_cnt][0], min_picount3[sp][av_cnt][0], 346)) return 1;
		//if (FailMaxNoteLen()) return 1;
		if (FailMissSlurs()) return 1;
		if (FailSlurs()) return 1;
		if (FailRhythm()) return 1;
		if (FailMultiCulm()) return 1;
		if (FailTonic(0)) return 1;
		if (FailTonic(1)) return 1;
		if (sp > 1) {
			if (FailAdjacentTritones()) return 1;
			if (FailTritones2()) return 1;
		}
		else {
			if (FailTritones()) return 1;
		}
		if (FailManyLeaps(max_leaps[sp][av_cnt][0], max_leaped[sp][av_cnt][0], max_leaps_r[sp][av_cnt][0],
			max_leaped_r[sp][av_cnt][0], max_leap_steps[sp][av_cnt][0],
			493, 494, 495, 496)) return 1;
		if (FailManyLeaps(max_leaps2[sp][av_cnt][0], max_leaped2[sp][av_cnt][0], max_leaps2_r[sp][av_cnt][0],
			max_leaped2_r[sp][av_cnt][0], max_leap_steps2[sp][av_cnt][0],
			497, 498, 499, 500)) return 1;
		if (FailLeapSmooth(max_smooth[sp][av_cnt][0], max_smooth_direct[sp][av_cnt][0],
			cse_leaps[sp][av_cnt][0], cse_leaps_r[sp][av_cnt][0], 302, 303, 501, 502, 1)) return 1;
		if (FailAdSymRepeat(3)) return 1;
		if (FailAdSymRepeat(4)) return 1;
		if (FailGlobalFill()) return 1;
		for (int iv = 0; iv < 4; ++iv) {
			if (FailLocalRange(notes_lrange[iv][sp][av_cnt][0], iv + 2, 434 + iv)) return 1;
		}
		if (FailStagnation(stag_note_steps[sp][av_cnt][0], stag_notes[sp][av_cnt][0], 10)) return 1;
		if (FailStagnation(stag_note_steps2[sp][av_cnt][0], stag_notes2[sp][av_cnt][0], 39)) return 1;
		FailIntervals();
		if (sp == 5) {
			FailSusCount();
		}
		if (FailMsh()) return 1;
		GetDtp();
		if (FailLeap()) return 1;
		MakeMacc();
		if (FailLocalMacc(notes_arange[sp][av_cnt][0], min_arange[sp][av_cnt][0] / 10.0, 15)) return 1;
		if (FailLocalMacc(notes_arange2[sp][av_cnt][0], min_arange2[sp][av_cnt][0] / 10.0, 16)) return 1;
	}
	GetMsh();
	FlagFullParallel();
	FlagParallelIco();
	FlagMultiSlur();
	if (FailRhythmRepeat()) return 1;
	if (FailAnapaest()) return 1;
	if (FailHarm()) return 1;
	FlagPcoApart();
	FlagHarmTriRes();
	FlagTriDouble();
	FlagLTDouble();
	FlagLTUnresolved();
	FlagLtLt();
	FlagSus();
	FlagSus2();
	for (v = 0; v < av_cnt; ++v) {
		sp = vsp[v];
		vaccept = &accept[sp][av_cnt][0];
		if (FailLastNotes()) return 1;
		if (mminor) {
			if (FailMinor()) return 1;
			if (FailGisTrail()) return 1;
			if (FailMinorStepwise()) return 1;
			if (FailFisTrail()) return 1;
		}
	}
	return 0;
}

int CP2R::FailMode() {
	for (ls = 0; ls < fli_size[v]; ++ls) {
		s = fli[v][ls];
		// Skip pause
		if (!cc[v][s]) continue;
		int maj_note = (cc[v][s] - bn + 12 + mode) % 12;
		if (maj_note == 8 || maj_note == 6) {
			if (!mminor) {
				FlagV(v, 521, s);
			}
		}
		if (maj_note == 1 || maj_note == 3 || maj_note == 10) {
			FlagV(v, 521, s);
		}
	}
	return 0;
}

// Find situations when one voice goes over previous note of another voice without actual crossing
int CP2R::FailOverlap() {
	CHECK_READY(DR_fli);
	for (v2 = v + 1; v2 < av_cnt; ++v2) {
		for (ls = 0; ls < fli_size[v] - 1; ++ls) {
			s = fli[v][ls];
			ls2 = bli[v2][s];
			//s2 = fli[v2][ls2];
			// Skip last note
			if (ls2 == fli_size[v2] - 1) break;
			vc = vca[s];
			// Skip pauses
			if (!cc[v][s]) continue;
			if (!cc[v2][s]) continue;
			// Next notes
			s3 = fli[v][ls + 1];
			s4 = fli[v2][ls2 + 1];
			// Skip oblique motion
			if (s3 != s4) continue;
			int found = 0;
			if (cc[v][s] < cc[v2][s]) {
				// Skip crossing
				if (cc[v][s3] > cc[v2][s4]) continue;
				if (cc[v][s3] >= cc[v2][s] || cc[v2][s4] <= cc[v][s]) found = 1;
			}
			else if (cc[v][s] > cc[v2][s]) {
				// Skip crossing
				if (cc[v][s3] < cc[v2][s4]) continue;
				if (cc[v][s3] <= cc[v2][s] || cc[v2][s4] >= cc[v][s]) found = 1;
			}
			else if (cc[v][s] == cc[v2][s]) {
				if ((cc[v][s3] <= cc[v2][s] && cc[v2][s4] <= cc[v2][s]) || 
					(cc[v][s3] >= cc[v2][s] && cc[v2][s4] >= cc[v2][s])) found = 1;
			}
			if (found) {
				// Detect non-adjacent
				int nonadj = 0;
				if (v2 - v > 1) {
					for (int i = s; i <= fli2[v][ls + 1]; ++i) {
						for (int v3 = v + 1; v3 < v2; ++v3) {
							if (cc[v3][i]) {
								nonadj = 1;
								break;
							}
						}
						if (nonadj) break;
					}
				}
				// Direct movement to 2nd
				if (abs(cc[v][s3] - cc[v2][s4]) < 3 && abs(cc[v][s3] - cc[v2][s4]) > 0 && 
					(cc[v][s3] - cc[v][s3 - 1]) * (cc[v2][s4] - cc[v2][s4 - 1]) > 0) {
					FlagL(v, 136, s, s3, v2);
				}
				else if (nonadj) {
					FlagL(v, 548, s, s3, v2);
				}
				else {
					FlagL(v, 24, s, s3, v2);
				}
			}
		}
	}
	return 0;
}

int CP2R::FailCross() {
	for (v2 = v + 1; v2 < av_cnt; ++v2) {
		int cross_start = -1;
		for (s = 0; s < ep2; ++s) {
			vc = vca[s];
			// Check if there is voice crossing
			int is_cross;
			if (cc[v][s] && cc[v2][s] && cc[v2][s] < cc[v][s]) is_cross = 1;
			else is_cross = 0;
			// Search for start of crossing
			if (cross_start == -1) {
				if (!is_cross) continue;
				cross_start = s;
			}
			// Search for end of crossing
			else {
				if (!is_cross) {
					if (FailOneCross(cross_start, s - 1)) return 1;
					cross_start = -1;
				}
			}
		}
		if (cross_start > -1) {
			if (FailOneCross(cross_start, ep2 - 1)) return 1;
		}
	}
	return 0;
}

int CP2R::FailOneCross(int cross_start, int cross_end) {
	// Is any crossing prohibited?
	if (!accept[sp][vc][0][543]) {
		FlagL(v, 543, cross_start, cross_end, v2);
	}
	// Is there crossing in first or last measure?
	else if (!accept[sp][vc][0][541] && bmli[cross_start] == 0) {
		FlagL(v, 541, cross_start, cross_end, v2);
	}
	else if (!accept[sp][vc][0][542] && bmli[cross_end] == mli.size() - 1) {
		FlagL(v, 542, cross_start, cross_end, v2);
	}
	else {
		// Check crossing length
		int clen = (cross_end - cross_start + 1) * 1.0 / npm;
		if (clen > cross_max_len[sp][av_cnt][0]) {
			if (clen > cross_max_len2[sp][av_cnt][0]) {
				FlagL(v, 519, cross_start, cross_end, v2);
			}
			else {
				FlagL(v, 518, cross_start, cross_end, v2);
			}
		}
	}
	// Check which voices cross
	if (v2 - v > 1) {
		int found = 0;
		for (int i = cross_start; i <= cross_end; ++i) {
			for (int v3 = v + 1; v3 < v2; ++v3) {
				if (cc[v3][i]) {
					found = 1;
					break;
				}
			}
			if (found) break;
		}
		// Flag non-adjacent voices crossing
		if (found) {
			FlagL(v, 520, cross_start, cross_end, v2);
		}
	}
	// Only if not first note and not oblique motion
	if (cross_start && cc[v][cross_start - 1] && cc[v2][cross_start - 1] && 
		cc[v][cross_start - 1] != cc[v][cross_start] && cc[v2][cross_start - 1] != cc[v2][cross_start]) {
		int int1 = abs(c[v][cross_start] - c[v2][cross_start]);
		int int2 = abs(c[v][cross_start - 1] - c[v2][cross_start - 1]);
		// 2 x 2nd intervals (sequential)
		if (int1 == 1 && int2 == 1) {
			FlagL(v, 545, ssus[v][bli[v][cross_start - 1]], cross_start, v2);
		}
		// Prohibit 2nd interval
		else if (int1 == 1) {
			Flag(v, 544, cross_start, v2);
		}
		// Prohibit 2nd interval
		else if (int2 == 1) {
			Flag(v, 544, ssus[v][bli[v][cross_start - 1]], v2);
		}
		// Prohibit direct motion
		if ((cc[v][cross_start] - cc[v][cross_start - 1]) * (cc[v2][cross_start] - cc[v2][cross_start - 1]) > 0) {
			// Both leaps
			if (leap[v][cross_start - 1] && leap[v2][cross_start - 1]) {
				FlagL(v, 547, ssus[v][bli[v][cross_start - 1]], cross_start, v2);
			}
			// One leap
			else {
				FlagL(v, 546, ssus[v][bli[v][cross_start - 1]], cross_start, v2);
			}
			// Zero leaps is impossible, because this will not lead to voice crossing
		}
	}
	// Only if not last note and not oblique motion
	if (cross_end < ep2 - 1 && cc[v][cross_end + 1] && cc[v2][cross_end + 1] && cc[v][cross_end + 1] != cc[v][cross_end] && cc[v2][cross_end + 1] != cc[v2][cross_end]) {
		int int1 = abs(c[v][cross_end + 1] - c[v2][cross_end + 1]);
		int int2 = abs(c[v][cross_end] - c[v2][cross_end]);
		// 2 x 2nd intervals (sequential)
		if (int1 == 1 && int2 == 1) {
			FlagL(v, 545, ssus[v][bli[v][cross_end]], cross_end + 1, v2);
		}
		// Prohibit 2nd interval
		else if (int1 == 1) {
			Flag(v, 544, cross_end + 1, v2);
		}
		// Prohibit 2nd interval
		else if (int2 == 1) {
			Flag(v, 544, ssus[v][bli[v][cross_end]], v2);
		}
		// Prohibit direct motion
		if ((cc[v][cross_end + 1] - cc[v][cross_end]) * (cc[v2][cross_end + 1] - cc[v2][cross_end]) > 0) {
			// Both leaps
			if (leap[v][cross_end] && leap[v2][cross_end]) {
				FlagL(v, 547, ssus[v][bli[v][cross_end]], cross_end + 1, v2);
			}
			else {
				FlagL(v, 546, ssus[v][bli[v][cross_end]], cross_end + 1, v2);
			}
		}
	}
	return 0;
}

void CP2R::CreateLinks() {
	SET_READY(DR_fli);
	// Set first steps in case there is pause
	for (int v = 0; v < av_cnt; ++v) {
		int prev_note = -1;
		int lpos = 0;
		int l = 0;
		minl[v] = 10000;
		maxl[v] = 0;
		for (int i = 0; i < ep2; ++i) {
			if (prev_note != cc[v][i] || retr[v][i]) {
				// Save linked note length
				if (prev_note != -1) {
					llen[v][lpos - 1] = l;
					rlen[v][lpos - 1] = l;
					if (minl[v] > l) minl[v] = l;
					if (maxl[v] < l) maxl[v] = l;
					l = 0;
				}
				prev_note = cc[v][i];
				fli[v][lpos] = i;
				++lpos;
			}
			fli2[v][lpos - 1] = i;
			bli[v][i] = lpos - 1;
			l++;
		}
		fli_size[v] = lpos;
		llen[v][lpos - 1] = l;
		rlen[v][lpos - 1] = l;
		// Search for first note
		fin[v] = 0;
		fil[v] = 0;
		for (s = 0; s < ep2; ++s) {
			if (cc[v][s]) {
				fin[v] = s;
				fil[v] = bli[v][s];
				break;
			}
		}
	}
}

void CP2R::GetVca() {
	SET_READY(DR_vca);
	// Set first steps in case there is pause
	for (int s = 0; s < ep2; ++s) {
		vca[s] = 0;
		hva[s] = 0;
		lva[s] = av_cnt - 1;
		for (int v = 0; v < av_cnt; ++v) {
			if (cc[v][s]) {
				++vca[s];
				if (v > hva[s]) hva[s] = v;
				if (v < lva[s]) lva[s] = v;
			}
		}
	}
}

void CP2R::SendComment(int pos, int x, int i) {
	CString st, com;
	int current_severity = -1;
	// Clear
	comment[pos + i][vi].clear();
	ccolor[pos + i][vi].clear();
	color[pos + i][vi] = color_noflag;
	if (flag[v][x].size() > 0) for (int f = 0; f < flag[v][x].size(); ++f) {
		GetFlag(f);
		// Send comments and color only if rule is not ignored
		if (accept[sp][vc][vp][fl] == -1 && !show_ignored_flags) continue;
		// Send comments and color only if rule is not ignored
		if (accept[sp][vc][vp][fl] == 1 && !show_allowed_flags) continue;
		// Do not send if ignored
		if (severity[sp][vc][vp][fl] < show_min_severity) continue;
		if (!accept[sp][vc][vp][fl]) st = "- ";
		else if (accept[sp][vc][vp][fl] == -1) st = "$ ";
		else st = "+ ";
		com = st + GetRuleName(fl, sp, vc, vp) + " (" + GetSubRuleName(fl, sp, vc, vp) + ")";
		if (show_severity) {
			st.Format(" [%d/%d] (%d-%d|%d)", fl, 
				severity[sp][vc][vp][fl], x, fsl[v][x][f], fvl[v][x][f]);
			com += st;
		}
		if (GetRuleComment(fl, sp, vc, vp) != "") com += ". " + GetRuleComment(fl, sp, vc, vp);
		if (GetSubRuleComment(fl, sp, vc, vp) != "") com += " (" + GetSubRuleComment(fl, sp, vc, vp) + ")";
		//com += ". ";
		comment[pos][vi].push_back(com);
		ccolor[pos][vi].push_back(sev_color[severity[sp][vc][vp][fl]]);
		// Set note color if this is maximum flag severity
		if (severity[sp][vc][vp][fl] > current_severity && severity[sp][vc][vp][fl] >= show_min_severity
			&& ruleinfo[fl].viz != vHarm) {
			current_severity = severity[sp][vc][vp][fl];
			color[pos + i][vi] = sev_color[severity[sp][vc][vp][fl]];
		}
	}
}

void CP2R::SendLining(int pos, int x, int i) {
	if (v % 2) {
		lining[pos + i][vi] = HatchStyleDiagonalCross;
	}
	else {
		lining[pos + i][vi] = 0;
	}
	if (show_hatch == 1) {
	}
	if (show_hatch == 2 && av_cnt > 1) {
		ls = bli[v][x];
		if (hli[bhli[x]] == x || fli[v][ls] == x) {
			if (msh[v][x] < 0) lining[pos + i][vi] = HatchStyleLargeConfetti;
			else lining[pos + i][vi] = 0;
		}
		else {
			lining[pos + i][vi] = lining[pos + i - 1][vi];
		}
	}
}

void CP2R::SendCP() {
	CString st;
	//CreateLinks();
	int real_len = cc[0].size();
	int full_len = floor((real_len + 1) / 8 + 1) * 8;
	ResizeVectors(step0 + full_len);
	for (v = 0; v < av_cnt; ++v) {
		vi = vid[v];
		for (ls = 0; ls < fli_size[v]; ++ls) {
			for (s = fli[v][ls]; s <= fli2[v][ls]; ++s) {
				if (cc[v][s]) {
					note[step0 + s][vi] = cc[v][s];
					pause[step0 + s][vi] = 0;
				}
				else {
					note[step0 + s][vi] = 0;
					pause[step0 + s][vi] = 1;
				}
				len[step0 + s][vi] = llen[v][ls];
				coff[step0 + s][vi] = s - fli[v][ls];
				tempo[step0 + s] = cp_tempo;
				SendComment(step0 + fli[v][ls], s, s - fli[v][ls]);
				SendLining(step0 + fli[v][ls], s, s - fli[v][ls]);
			}
		}
		MergeNotes(step0, step0 + full_len - 1);
		MakeBellDyn(vi, step0, step0 + full_len - 1, 50, 110, 0);
		st.Format("#%d (from %s)",
			cp_id + 1, bname_from_path(musicxml_file));
		AddMelody(step0, step0 + full_len - 1, vi, st);
	}
	SendHarmMarks();
	for (int s = step0 + real_len; s < step0 + full_len; ++s) tempo[s] = tempo[s - 1];
	CountOff(step0, step0 + full_len - 1);
	CountTime(step0, step0 + full_len - 1);
	UpdateNoteMinMax(step0, step0 + full_len - 1);
	UpdateTempoMinMax(step0, step0 + full_len - 1);
	t_generated = step0 + full_len - 1;
	Adapt(step0, t_generated);
	t_sent = t_generated;
}

// Create bell dynamics curve
void CP2R::MakeBellDyn(int v, int step1, int step2, int dyn1, int dyn2, int dyn_rand) {
	// Do not process if steps are equal or wrong
	if (step2 <= step1) return;
	int mids = (step1 + step2) / 2;
	int counts = step2 - step1;
	for (int s = step1; s <= step2; ++s) {
		if (s < mids)	dyn[s][v] = dyn1 + min(dyn2 - dyn1, (dyn2 - dyn1) * (s - step1) / counts * 2) + dyn_rand * rand2() / RAND_MAX;
		else dyn[s][v] = dyn1 + min(dyn2 - dyn1, (dyn2 - dyn1) * (step2 - s) / counts * 2) + dyn_rand * rand2() / RAND_MAX;
	}
}

void CP2R::SendHarmMarks() {
	for (hs = 0; hs < hli.size(); ++hs) {
		s = hli[hs];
		// Find lowest voice with note
		for (v = 0; v < av_cnt; ++v) {
			vi = vid[v];
			if (cc[v][s]) break;
		}
		mark[step0 + s][vi] = GetHarmName(chm[hs], chm_alter[hs]);
		if (show_harmony_bass && chm[hs] > -1 && hbc[hs] % 7 != chm[hs]) {
			if ((hbc[hs] % 7 - chm[hs] + 7) % 7 == 2) {
				mark[step0 + s][vi] += "6";
			}
			else {
				mark[step0 + s][vi] += "6/4";
			}
		}
		SendHarmColor();
	}
}

void CP2R::SendHarmColor() {
	DWORD fc;
	mark_color[step0 + s][vi] = MakeColor(255, 170, 170, 170);
	// Scan flags
	int max_severity = -1;
	int fl;
	for (int f = 0; f < flag[v][s].size(); ++f) {
		fl = flag[v][s][f];
		if (ruleinfo[fl].viz == vHarm && !accept[0][av_cnt][0][fl] && severity[0][av_cnt][0][fl] >= show_min_severity) {
			if (severity[0][av_cnt][0][fl] > max_severity) max_severity = severity[0][av_cnt][0][fl];
		}
	}
	if (max_severity > -1) {
		fc = sev_color[max_severity];
		mark_color[step0 + s][vi] = MakeColor(GetAlpha(fc), GetRed(fc),
			GetGreen(fc) / 1.5, GetBlue(fc));
	}
}


inline void CP2R::ClearReady() {
	fill(data_ready.begin(), data_ready.end(), 0);
}

inline void CP2R::SetReady(int id) {
	data_ready[id] = 1;
}

inline void CP2R::SetReady(int id, int id2) {
	data_ready[id] = 1;
	data_ready[id2] = 1;
}

inline void CP2R::SetReady(int id, int id2, int id3) {
	data_ready[id] = 1;
	data_ready[id2] = 1;
	data_ready[id3] = 1;
}

inline void CP2R::ClearReadyPersist(int id) {
	data_ready_persist[id] = 0;
}

inline void CP2R::ClearReadyPersist(int id, int id2) {
	data_ready_persist[id] = 0;
	data_ready_persist[id2] = 0;
}

inline void CP2R::ClearReadyPersist(int id, int id2, int id3) {
	data_ready_persist[id] = 0;
	data_ready_persist[id2] = 0;
	data_ready_persist[id3] = 0;
}

inline void CP2R::SetReadyPersist(int id) {
	data_ready_persist[id] = 1;
}

inline void CP2R::SetReadyPersist(int id, int id2) {
	data_ready_persist[id] = 1;
	data_ready_persist[id2] = 1;
}

inline void CP2R::SetReadyPersist(int id, int id2, int id3) {
	data_ready_persist[id] = 1;
	data_ready_persist[id2] = 1;
	data_ready_persist[id3] = 1;
}

inline void CP2R::CheckReady(int id) {
	if (!data_ready[id] && !warn_data_ready[id]) {
		++warn_data_ready[id];
		CString est;
		est.Format("Attemp to use data element '%d' while it is not ready yet", id);
		WriteLog(5, est);
		ASSERT(0);
	}
}

inline void CP2R::CheckReady(int id, int id2) {
	CheckReady(id);
	CheckReady(id2);
}

inline void CP2R::CheckReady(int id, int id2, int id3) {
	CheckReady(id);
	CheckReady(id2);
	CheckReady(id3);
}

inline void CP2R::CheckReadyPersist(int id) {
	if (!data_ready_persist[id] && !warn_data_ready_persist[id]) {
		++warn_data_ready_persist[id];
		CString est;
		est.Format("Attemp to use persistent data element '%d' while it is not ready yet", id);
		WriteLog(5, est);
		ASSERT(0);
	}
}

inline void CP2R::CheckReadyPersist(int id, int id2) {
	CheckReadyPersist(id);
	CheckReadyPersist(id2);
}

inline void CP2R::CheckReadyPersist(int id, int id2, int id3) {
	CheckReadyPersist(id);
	CheckReadyPersist(id2);
	CheckReadyPersist(id3);
}

int CP2R::FailManyLeaps(int mleaps, int mleaped, int mleaps2, int mleaped2, int mleapsteps,
	int flag1, int flag2, int flag3, int flag4) {
	CHECK_READY(DR_fli, DR_c);
	int leap_sum = 0;
	int leaped_sum = 0;
	int leap_sum_i = 0;
	g_leaps[v][fli_size[v] - 1] = 0;
	g_leaped[v][fli_size[v] - 1] = 0;
	int win_leaps = 0;
	int win_leapnotes = 0;
	for (ls = 0; ls < fli_size[v] - 1; ++ls) {
		s = fli2[v][ls];
		s1 = fli2[v][ls + 1];
		// Add new leap
		if (leap[v][s] != 0) {
			++leap_sum;
			leaped_sum += abs(c[v][s] - c[v][s1]) + 1;
		}
		// Subtract old leap
		if ((ls >= mleapsteps) && (leap[v][fli2[v][ls - mleapsteps]] != 0)) {
			leap_sum--;
			leaped_sum -= abs(c[v][fli[v][ls - mleapsteps]] - c[v][fli[v][ls - mleapsteps + 1]]) + 1;
		}
		// Get maximum leap_sum
		if (leap_sum > win_leaps) {
			win_leaps = leap_sum;
			leap_sum_i = ls;
		}
		if (leaped_sum > win_leapnotes) {
			win_leapnotes = leaped_sum;
			leap_sum_i = ls;
		}
		// Record for graph
		g_leaps[v][ls] = leap_sum;
		g_leaped[v][ls] = leaped_sum;
		// Calculate penalty 
		if (leap_sum > mleaps) {
			if (!accept[sp][av_cnt][0][flag1]) ++fpenalty;
		}
		else if (leap_sum > mleaps2) {
			if (!accept[sp][av_cnt][0][flag3]) ++fpenalty;
		}
		if (leaped_sum > mleaped) {
			if (!accept[sp][av_cnt][0][flag2]) ++fpenalty;
		}
		else if (leaped_sum > mleaped2) {
			if (!accept[sp][av_cnt][0][flag4]) ++fpenalty;
		}
	}
	if (win_leaps > mleaps2)
		FlagVL(v, flag3, fli[v][leap_sum_i + 1], fli[v][max(0, leap_sum_i - mleapsteps)]);
	else if (win_leaps > mleaps)
		FlagVL(v, flag1, fli[v][leap_sum_i + 1], fli[v][max(0, leap_sum_i - mleapsteps)]);
	if (win_leapnotes > mleaped2)
		FlagVL(v, flag4, fli[v][leap_sum_i + 1], fli[v][max(0, leap_sum_i - mleapsteps)]);
	else if (win_leapnotes > mleaped)
		FlagVL(v, flag2, fli[v][leap_sum_i + 1], fli[v][max(0, leap_sum_i - mleapsteps)]);
	return 0;
}

void CP2R::ClearFlags(int step1, int step2) {
	for (v = 0; v < av_cnt; ++v) {
		// Clear rhythm ids
		rh_id[v].clear();
		// Clear rhythm pause ids
		rh_pid[v].clear();
		for (s = step1; s < step2; ++s) {
			flag[v][s].clear();
			fsl[v][s].clear();
			fvl[v][s].clear();
		}
	}
	fpenalty = 0;
}

void CP2R::GetPitchClass(int step1, int step2) {
	CHECK_READY(DR_c);
	SET_READY(DR_pc);
	for (int v = 0; v < av_cnt; ++v) {
		for (int s = step1; s < step2; ++s) {
			if (cc[v][s]) {
				pc[v][s] = c[v][s] % 7;
				pcc[v][s] = (cc[v][s] + 12 - bn) % 12;
			}
			else {
				pc[v][s] = -1;
				pcc[v][s] = -1;
			}
		}
	}
}

void CP2R::GetDiatonic(int step1, int step2) {
	SET_READY(DR_c);
	for (int v = 0; v < av_cnt; ++v) {
		for (int s = step1; s < step2; ++s) {
			if (cc[v][s]) {
				c[v][s] = cc_c[cc[v][s]];
			}
			else {
				c[v][s] = 0;
			}
		}
	}
}

void CP2R::GetLeapSmooth() {
	CHECK_READY(DR_c);
	SET_READY(DR_leap, DR_slur);
	for (int v = 0; v < av_cnt; ++v) {
		for (int i = 0; i < ep2 - 1; ++i) {
			// Find all leaps
			leap[v][i] = 0;
			smooth[v][i] = 0;
			slur[v][i + 1] = 0;
			if (cc[v][i] == cc[v][i + 1]) slur[v][i + 1] = 1;
			if (!cc[v][i] || !cc[v][i + 1]) continue;
			if (c[v][i + 1] - c[v][i] > 1) leap[v][i] = c[v][i + 1] - c[v][i];
			else if (c[v][i + 1] - c[v][i] < -1) leap[v][i] = c[v][i + 1] - c[v][i];
			// Find all smooth
			else if (c[v][i + 1] - c[v][i] == 1) smooth[v][i] = 1;
			else if (c[v][i + 1] - c[v][i] == -1) smooth[v][i] = -1;
		}
		leap[v][ep2 - 1] = 0;
		smooth[v][ep2 - 1] = 0;
		slur[v][0] = 0;
	}
}

int CP2R::FailIntervals() {
	CHECK_READY(DR_pc, DR_c, DR_fli);
	for (ls = 0; ls < fli_size[v] - 1; ++ls) {
		s0 = fli[v][ls];
		s = fli2[v][ls];
		s1 = fli2[v][ls + 1];
		// Ignore pauses
		if (!cc[v][s1]) continue;
		if (!cc[v][s]) continue;
		// Leap size prohibit
		if (cc[v][s1] - cc[v][s] == 8) FlagV(v, 175, s0);
		else if (cc[v][s1] - cc[v][s] == -8) FlagV(v, 181, s0);
		else if (cc[v][s1] - cc[v][s] == 9) FlagV(v, 176, s0);
		else if (cc[v][s1] - cc[v][s] == -9) FlagV(v, 182, s0);
		else if (cc[v][s1] - cc[v][s] == 10) FlagV(v, 177, s0);
		else if (cc[v][s1] - cc[v][s] == -10) FlagV(v, 183, s0);
		else if (cc[v][s1] - cc[v][s] == 11) FlagV(v, 178, s0);
		else if (cc[v][s1] - cc[v][s] == -11) FlagV(v, 184, s0);
		else if (cc[v][s1] - cc[v][s] == 12) FlagV(v, 179, s0);
		else if (cc[v][s1] - cc[v][s] == -12) FlagV(v, 185, s0);
		else if (cc[v][s1] - cc[v][s] > 12) FlagV(v, 180, s0);
		else if (cc[v][s1] - cc[v][s] < -12) FlagV(v, 186, s0);
	}
	return 0;
}

void CP2R::FlagLtLt() {
	CHECK_READY(DR_pc, DR_c, DR_fli);
	CHECK_READY(DR_islt);
	for (v = 0; v < av_cnt; ++v) {
		for (ls = 0; ls < fli_size[v] - 1; ++ls) {
			s0 = fli[v][ls];
			s = fli2[v][ls];
			s1 = fli[v][ls + 1];
			// Ignore pauses
			if (!cc[v][s1]) continue;
			if (!cc[v][s]) continue;
			// Prohibit BB
			if (islt[v][fli[v][ls + 1]] || islt[v][s0]) {
				if (pcc[v][s] == pcc[v][s1])
					FlagVL(v, 348, s0, s1);
			}
			// Prohibit major second up to tonic
			if (pcc[v][s1] == 0 && pcc[v][s0] == 10 && nih[v][s1] && nih[v][s0]) 
				FlagVL(v, 74, s0, s1);
		}
	}
}

void CP2R::GetLClimax() {
	SET_READY(DR_lclimax);
	for (int v = 0; v < av_cnt; ++v) {
		GetMovingMax(cc[v], max(lclimax_notes, lclimax_mea * npm), lclimax[v]);
		GetMovingMax(cc[v], lclimax_mea5[vsp[v]][av_cnt][0] * npm, lclimax2[v]);
	}
}

void CP2R::GetNoteTypes() {
	CHECK_READY(DR_fli);
	SET_READY(DR_beat, DR_sus);
	for (v = 0; v < av_cnt; ++v) {
		int l;
		for (ls = 0; ls < fli_size[v]; ++ls) {
			s = fli[v][ls];
			s2 = fli2[v][ls];
			l = llen[v][ls];
			int sm = s % npm;
			// Get beat

			if (sm == 0) beat[v][ls] = 0;
			else if (sm == 1) beat[v][ls] = 10;
			else if (sm == 2) beat[v][ls] = 3;
			else if (sm == 3) beat[v][ls] = 11;
			else if (sm == 4) {
				if (nlen[v] == 6) beat[v][ls] = 4;
				else beat[v][ls] = 1;
			}
			else if (sm == 5) beat[v][ls] = 12;
			else if (sm == 6) {
				if (nlen[v] == 6) beat[v][ls] = 1;
				else beat[v][ls] = 4;
			}
			else if (sm == 7) beat[v][ls] = 13;
			else if (sm == 8) beat[v][ls] = 2;
			else if (sm == 9) beat[v][ls] = 14;
			else if (sm == 10) beat[v][ls] = 6;
			else if (sm == 11) beat[v][ls] = 15;
			else if (sm == 12) beat[v][ls] = 3;
			else if (sm == 13) beat[v][ls] = 16;
			else if (sm == 14) beat[v][ls] = 7;
			else if (sm == 15) beat[v][ls] = 17;
			// Beats for species 1: 0 0 0 0
			// Beats for species 2: 0 1 0 1
			// Beats for species 3: 0 3 1 5
			// Beats for species 4: 0 1 0 1
			// Beats for species 5: 0 10 4 11 1 12 5 13 2 14 6 15 3 16 7 17
			// Get suspension if cantus note changes during counterpoint note
			sus[v][ls] = 0;
			if (bmli[s] != bmli[s2]) {
				sus[v][ls] = mli[bmli[fli2[v][ls]]];
			}
			// Build ssus
			ssus[v][ls] = sus[v][ls] ? sus[v][ls] : fli[v][ls];
		}
	}
}

int CP2R::FailGisTrail() {
	CHECK_READY(DR_fli, DR_pc, DR_nih);
	int gis_trail = 0;
	int _gis_trail_max = gis_trail_max[sp][av_cnt][0];
	for (ls = 0; ls < fli_size[v]; ++ls) {
		s = fli[v][ls];
		if (cc[v][s] && nih[v][s]) {
			if (pcc[v][s] == 11) {
				// Set to maximum on new G# note
				gis_trail = _gis_trail_max;
			}
			else {
				if (pcc[v][s] == 10) {
					// Prohibit G note close to G#
					if (gis_trail && gis_trail < _gis_trail_max - 2) FlagVL(v, 200, s, fli[v][max(0, ls - _gis_trail_max + gis_trail)]);
				}
			}
		}
		// Decrease if not zero
		if (gis_trail) --gis_trail;
	}
	return 0;
}

int CP2R::FailFisTrail() {
	CHECK_READY(DR_fli, DR_pc, DR_nih);
	int pos1, pos2, found;
	int _fis_gis_max = fis_gis_max[sp][av_cnt][0];
	int _fis_g_max = fis_g_max[sp][av_cnt][0];
	int _fis_g_max2 = fis_g_max2[sp][av_cnt][0];
	for (ls = 0; ls < fli_size[v]; ++ls) {
		s = fli[v][ls];
		if (cc[v][s] && pcc[v][s] == 9 && nih[v][s]) {
			// Find VII#
			pos1 = max(0, ls - _fis_gis_max);
			pos2 = min(fli_size[v] - 1, ls + _fis_gis_max);
			found = 0;
			for (int x = pos1; x <= pos2; ++x) {
				s2 = fli[v][x];
				if (cc[v][s2] && pcc[v][s2] == 11) {
					found = 1;
					break;
				}
			}
			if (!found) {
				// Flag only if full melody analysis or window is not cut
				if (ls + _fis_gis_max <= fli_size[v] - 1 || ep2 == c_len)	FlagVL(v, 199, s, s);
			}
			// Find VII before
			pos1 = max(0, ls - _fis_g_max);
			for (int x = pos1; x < ls - 1; ++x) {
				s2 = fli[v][x];
				if (cc[v][s2] && pcc[v][s2] == 10 && nih[v][s2]) {
					FlagVL(v, 349, s, s2);
					break;
				}
			}
			// Find VII after
			pos2 = min(fli_size[v] - 1, ls + _fis_g_max2);
			for (int x = ls + 2; x <= pos2; ++x) {
				s2 = fli[v][x];
				if (cc[v][s2] && pcc[v][s2] == 10 && nih[v][s2]) {
					FlagVL(v, 350, s, s);
					break;
				}
			}
		}
	}
	return 0;
}

int CP2R::FailMinor() {
	CHECK_READY(DR_pc, DR_fli, DR_nih);
	for (ls = 1; ls < fli_size[v]; ++ls) {
		s = fli[v][ls];
		s_1 = fli[v][ls - 1];
		// Prohibit minor second up before VII - absorbed
		// Prohibit augmented second up before VII - absorbed
		// Prohibit immediate F# - G and G - F#
		if (pcc[v][s] == 10 && pcc[v][s_1] == 9 ) FlagVL(v, 203, s_1, s);
		if (pcc[v][s] == 9  && pcc[v][s_1] == 10) FlagVL(v, 203, s_1, s);
		// Prohibit unaltered VI or VII two steps from altered VI or VII
		if (pcc[v][s] == 11) {
			if (pcc[v][s_1] == 10) FlagVL(v, 153, s_1, s);
			if (pcc[v][s_1] == 8) FlagVL(v, 154, s_1, s);
			if (pcc[v][s_1] == 3) {
				if (ls < fli_size[v] - 1) {
					// III-VII#-I downward
					if (pcc[v][fli[v][ls + 1]] == 0 && cc[v][s] - cc[v][s_1] < 0) FlagVL(v, 432, s_1, fli[v][ls + 1]);
					// III-VII#
					else FlagVL(v, 157, s_1, s);
				}
				else {
					if (ep2 == c_len) FlagVL(v, 157, s_1, s);
				}
			}
			if (ls > 1) {
				s_2 = fli[v][ls - 2];
				if (pcc[v][s_2] == 10 && nih[v][s_2] && nih[v][s]) FlagVL(v, 159, s_2, s);
			}
			if (ls < fli_size[v] - 1) {
				s1 = fli[v][ls + 1];
				if (pcc[v][s1] == 10) FlagVL(v, 153, s1, s);
				if (pcc[v][s1] == 8) FlagVL(v, 154, s1, s);
				if (pcc[v][s1] == 3) FlagVL(v, 156, s1, s);
				if (ls < fli_size[v] - 2) {
					s2 = fli[v][ls + 2];
					if (pcc[v][s2] == 10 && nih[v][s2] && nih[v][s]) FlagVL(v, 159, s2, s);
				}
			}
		}
		if (pcc[v][s] == 9) {
			if (pcc[v][s_1] == 8) FlagVL(v, 152, s_1, s);
			if (pcc[v][s_1] == 3) FlagVL(v, 155, s_1, s);
			if (ls > 1) {
				s_2 = fli[v][ls - 2];
				if (pcc[v][s_2] == 8 && nih[v][s_2] && nih[v][s]) FlagVL(v, 158, s_2, s);
			}
			if (ls < fli_size[v] - 1) {
				s1 = fli[v][ls + 1];
				if (pcc[v][s1] == 8) FlagVL(v, 152, s1, s);
				if (pcc[v][s1] == 3) FlagVL(v, 155, s1, s);
				if (ls < fli_size[v] - 2) {
					s2 = fli[v][ls + 2];
					if (pcc[v][s2] == 8 && nih[v][s2] && nih[v][s]) FlagVL(v, 158, s2, s);
				}
			}
		}
		// Prohibit unresolved minor tritone DG# (direct or with inserted note)
	}
	return 0;
}

int CP2R::FailMinorStepwise() {
	CHECK_READY(DR_pc, DR_fli);
	CHECK_READY(DR_msh, DR_nih);
	// For non-border notes only, because border notes have their own rules
	for (ls = 1; ls < fli_size[v] - 1; ++ls) {
		s = fli[v][ls];
		s_1 = fli[v][ls - 1];
		s1 = fli[v][ls + 1];
		// Prohibit harmonic VI# not stepwize ascending
		if ((sp < 2 || nih[v][s]) && pcc[v][s] == 9 &&
			(c[v][s] - c[v][s_1] != 1 || c[v][s1] - c[v][s] != 1))
			FlagVL(v, 201, s_1, s1);
		// Prohibit harmonic VII natural not stepwize descending
		if ((sp < 2 || nih[v][s]) && pcc[v][s] == 10 &&
			(c[v][s] - c[v][s_1] != -1 || c[v][s1] - c[v][s] != -1))
			FlagVL(v, 202, s_1, s1);
	}
	return 0;
}

// Merge notes of same pitch, that do not have pauses between them. Step2 inclusive
void CP2R::MergeNotes(int step1, int step2) {
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

void CP2R::GetDtp() {
	CHECK_READY(DR_fli);
	SET_READY(DR_dtp);
	int pause_dist = 0;
	int pause_dist_s = 0;
	for (ls = fli_size[v] - 1; ls >= 0; --ls) {
		s = fli[v][ls];
		dtp[v][ls] = pause_dist;
		dtp_s[v][ls] = pause_dist_s;
		if (cc[v][s]) {
			++pause_dist;
			pause_dist_s += llen[v][ls];
		}
		else {
			pause_dist = 0;
			pause_dist_s = 0;
		}
	}
}

void CP2R::CountFillInit(int tail_len, int pre, int &t1, int &t2, int &fill_end) {
	// Create leap tail
	tc.clear();
	if (pre) {
		int pos1 = fleap_start - 1;
		int pos2 = max(fleap_start - tail_len, 0);
		if (c[v][leap_end] > c[v][leap_start]) {
			for (int i = pos1; i >= pos2; --i) {
				if (cc[v][fli[v][i]]) tc.push_back(128 - c[v][fli[v][i]]);
			}
			t1 = 128 - c[v][leap_end];
			t2 = 128 - c[v][leap_start];
		}
		else {
			for (int i = pos1; i >= pos2; --i) {
				if (cc[v][fli[v][i]]) tc.push_back(c[v][fli[v][i]]);
			}
			t1 = c[v][leap_end];
			t2 = c[v][leap_start];
		}
	}
	else {
		int pos1 = fleap_end + 1;
		int pos2 = min(fleap_end + tail_len, fli_size[v] - 1);
		if (c[v][leap_end] > c[v][leap_start]) {
			for (int i = pos1; i <= pos2; ++i) {
				if (cc[v][fli[v][i]]) tc.push_back(c[v][fli[v][i]]);
			}
			t1 = c[v][leap_start];
			t2 = c[v][leap_end];
		}
		else {
			for (int i = pos1; i <= pos2; ++i) {
				if (cc[v][fli[v][i]]) tc.push_back(128 - c[v][fli[v][i]]);
			}
			t1 = 128 - c[v][leap_start];
			t2 = 128 - c[v][leap_end];
		}
	}
	for (int x = t1; x <= t2; ++x) nstat3[x] = 0;
	fill_end = -1;
}

void CP2R::CountFill(int tail_len, int &skips, int &fill_to, int pre, int &fill_to_pre, int &fill_from_pre, int &fill_from, int &deviates, int &dev_count, int leap_prev, int &fill_end, int &fill_goal) {
	// Leap starting and finishing note
	int t1, t2;
	int cur_deviation = 0;
	int dev_state = 0;
	int max_deviation = 0;
	if (accept[sp][av_cnt][0][42 + leap_id]) max_deviation = 1;
	if (accept[sp][av_cnt][0][120 + leap_id] && !pre) max_deviation = 2;
	CountFillInit(tail_len, pre, t1, t2, fill_end);
	// Detect fill_end
	deviates = 0;
	int dl2 = dev_late2[sp][av_cnt][0];
	int dl3 = dev_late3[sp][av_cnt][0];
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
			if (cur_deviation == 1 && x == 0 && leap_size > 4 && fleap_end < fli_size[v] - 1 &&
				rlen[v][fleap_end + 1] > dev2_maxlen[sp][av_cnt][0] * 2 && !accept[sp][av_cnt][0][386]) break;
			// Detect late deviation
			if (cur_deviation == 1 && x >= dl2 && !accept[sp][av_cnt][0][191]) break;
			if (cur_deviation == 2 && x >= dl3 && !accept[sp][av_cnt][0][192]) break;
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
			if (c[v][leap_end] > c[v][leap_start]) ++nstat3[128 - c[v][leap_mid]];
			else ++nstat3[c[v][leap_mid]];
		}
		else {
			if (c[v][leap_end] > c[v][leap_start]) ++nstat3[c[v][leap_mid]];
			else ++nstat3[128 - c[v][leap_mid]];
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
	CountFillLimits(pre, t1, t2, fill_to, fill_to_pre, fill_from_pre, fill_from);
}

void CP2R::CountFillSkips(int leap_prev, int &skips, int t1, int t2) {
	skips = 0;
	for (int x = t1 + 1; x < t2; ++x) if (!nstat3[x]) {
		++skips;
	}
}

void CP2R::CountFillLimits(int pre, int t1, int t2, int &fill_to, int &fill_to_pre, int &fill_from_pre, int &fill_from) {
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
		if (fill_to == 2) pos = max(0, fleap_start - fill_pre3_notes[sp][av_cnt][0]);
		else pos = max(0, fleap_start - fill_pre4_notes[sp][av_cnt][0]);
		vector<int> nstat4;
		nstat4.resize(2, 0);
		if (c[v][leap_start] < c[v][leap_end]) {
			for (int x = pos; x < fleap_start; ++x) {
				if (c[v][fli[v][x]] == c[v][leap_start] + 1) nstat4[0] = 1;
				else if (c[v][fli[v][x]] == c[v][leap_start] + 2) nstat4[1] = 1;
			}
		}
		else {
			for (int x = pos; x < fleap_start; ++x) {
				if (c[v][fli[v][x]] == c[v][leap_start] - 1) nstat4[0] = 1;
				else if (c[v][fli[v][x]] == c[v][leap_start] - 2) nstat4[1] = 1;
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
		if (fill_from == 2) pos = max(0, fleap_start - fill_pre3_notes[sp][av_cnt][0]);
		else pos = max(0, fleap_start - fill_pre4_notes[sp][av_cnt][0]);
		vector<int> nstat4;
		nstat4.resize(2, 0);
		if (c[v][leap_start] < c[v][leap_end]) {
			for (int x = pos; x < fleap_start; ++x) {
				if (c[v][fli[v][x]] == c[v][leap_end] - 1) nstat4[0] = 1;
				else if (c[v][fli[v][x]] == c[v][leap_end] - 2) nstat4[1] = 1;
			}
		}
		else {
			for (int x = pos; x < fleap_start; ++x) {
				if (c[v][fli[v][x]] == c[v][leap_end] + 1) nstat4[0] = 1;
				else if (c[v][fli[v][x]] == c[v][leap_end] + 2) nstat4[1] = 1;
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

void CP2R::FailLeapInit(int &late_leap, int &presecond, int &leap_next, int &leap_prev, int &arpeg, int &overflow) {
	presecond = 0; // If leap has a filled second
	leap_next = 0; // Multiply consecutive leaps
	leap_prev = 0; // Multiply consecutive leaps
	overflow = 0; // Leap back overflow
	arpeg = 0; // Arpeggio 3+3
						 // Check if this leap is 3rd
	leap_start = s; // First step of leap
	leap_end = fli2[v][ls + 1]; // Last step of leap
	leap_mid = 0;
	fleap_start = ls;
	fleap_end = ls + 1;
	// leap_size = 4 for 5th
	leap_size = abs(c[v][leap_end] - c[v][s]);
	// Next is leap?
	if (fleap_end < fli_size[v] - 1) leap_next = leap[v][leap_start] * leap[v][leap_end];
	// Prev is leap?
	if (fleap_start > 0) leap_prev = leap[v][leap_start] * leap[v][fli2[v][fleap_start] - 1];
	// Late leap?
	late_leap = dtp_s[v][fleap_start] <= c4p_last_steps || dtp[v][fleap_start] <= c4p_last_notes[sp][av_cnt][0];
}

int CP2R::FailLeapMulti(int leap_next, int &arpeg, int &overflow, int &child_leap) {
	child_leap = 0; // If we have a child_leap
									// Check if leap is third
	if (fleap_end < fli_size[v] - 1) {
		// Next leap in same direction
		if (leap_next > 0) {
			// Flag if greater than two thirds
			if (abs(c[v][fli2[v][fleap_end + 1]] - c[v][leap_start]) > 4)
				FlagVL(v, 505, ssus[v][fleap_start], fli[v][bli[v][leap_end] + 1]);
			// Allow if both thirds, without flags (will process next cycle)
			else arpeg = 1;
		}
	}
	if (leap_size == 2) {
		// Check if leap is second third
		if (fleap_start > 0 && abs(c[v][leap_end] - c[v][fli2[v][fleap_start - 1]]) == 4 &&
			abs(c[v][leap_start] - c[v][fli2[v][fleap_start - 1]]) == 2) {
			// If there is one more third forward (3 x 3rds total)
			if (fleap_end < fli_size[v] - 1 && abs(c[v][fli2[v][fleap_end + 1]] - c[v][fli2[v][fleap_start - 1]]) == 6) {
				FlagVL(v, 504, ssus[v][fleap_start - 1], fli[v][fleap_start + 1]);
			}
			// If there is one more third backward (3 x 3rds total) - do not flag because it was already flagged
			else if (fleap_start > 1 && abs(c[v][leap_end] - c[v][fli2[v][fleap_start - 2]]) == 6) {
			}
			else FlagVL(v, 503, ssus[v][fleap_start - 1], fli[v][fleap_start + 1]);
			// Set middle leap note
			leap_mid = leap_start;
			// Set leap start to first note of first third
			--fleap_start;
			leap_start = fli2[v][fleap_start];
			// Set leap size to be compound
			leap_size = 4;
		}
	}
	leap_id = min(leap_size - 2, 3);
	if (fleap_end < fli_size[v] - 1) {
		// Next leap back
		if (leap_next < 0) {
			int leap_size2 = abs(c[v][fli2[v][fleap_end + 1]] - c[v][leap_end]);
			// Flag if back leap greater than 6th
			if (leap_size2 > 5) FlagV(v, 22, fli[v][bli[v][leap_end]]);
			// Flag if back leap equal or smaller than 6th
			else FlagV(v, 8, fli[v][bli[v][leap_end]]);
			// Flag leap back overflow
			if (leap_size2 > leap_size) {
				FlagV(v, 58, fli[v][bli[v][leap_end]]);
				overflow = 1;
			}
		}
	}
	// Check if we have a greater opposite neighbouring leap
	if ((fleap_end < fli_size[v] - 1 && abs(c[v][fli2[v][fleap_end + 1]] - c[v][leap_end]) >= leap_size - 1 && leap[v][leap_start] * leap[v][leap_end]<0) ||
		(fleap_start > 0 && abs(c[v][leap_start] - c[v][fli2[v][fleap_start - 1]]) > leap_size && leap[v][leap_start] * leap[v][fli2[v][fleap_start - 1]]<0)) {
		// Set that we are preleaped (even if we are postleaped)
		child_leap = 1;
	}
	return 0;
}

int CP2R::FailLeap() {
	CHECK_READY(DR_leap, DR_c, DR_fli);
	CHECK_READY(DR_dtp);
	if (sp > 1) {
		CHECK_READY(DR_beat);
	}
	// Get leap size, start, end
	// Check if leap is compensated (without violating compensation rules)
	// If leap is not compensated, check uncompensated rules
	// If uncompensated rules not allowed, flag compensation problems detected (3rd, etc.)
	int child_leap, leap_next, leap_prev, presecond;
	int overflow, arpeg, late_leap;
	// Calculate last steps that are allowed to have C4P
	c4p_last_steps = c4p_last_meas[sp][av_cnt][0] * npm;
	// Find late leap border 
	c4p_last_notes2 = min(c4p_last_notes[sp][av_cnt][0], fli_size[v] - bli[v][max(0, ep2 - c4p_last_steps)]);
	for (s = 0; s < ep2 - 1; ++s) {
		if (leap[v][s] != 0) {
			ls = bli[v][s];
			FailLeapInit(late_leap, presecond, leap_next, leap_prev,
				arpeg, overflow);
			if (FailLeapMulti(leap_next, arpeg, overflow, child_leap)) return 1;
			// If leap back overflow or arpeggio, do not check leap compensation, because compensating next leap will be enough
			if (!overflow && !arpeg)
				if (FailLeapFill(late_leap, leap_prev, child_leap)) return 1;
			if (!arpeg) if (FailLeapMDC()) return 1;
		}
	}
	return 0;
}

int CP2R::FailLeapFill(int late_leap, int leap_prev, int child_leap) {
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
	if (late_leap) allowed_skips += 2;
	int allowed_pskips = 1;
	if (leap_size > 4) ++allowed_pskips;
	if (leap_size > 6) ++allowed_pskips;
	if (late_leap) allowed_pskips += 1;
	// Check if leap is filled
	tail_len = 2 + (leap_size - 1) * fill_steps_mul;
	// Do not check fill if search window is cut by end of current not-last scan window
	if ((fleap_end + tail_len < fli_size[v]) || (c_len == ep2)) {
		// Check fill only if enough length (checked second time in case of slurs)
		CountFill(tail_len, skips, fill_to, 0, fill_to_pre, fill_from_pre,
			fill_from, deviates, dev_count, leap_prev, fill_end, fill_goal);
		if (skips > allowed_skips) filled = 0;
		else if (fill_to >= 3 && fill_to < fill_pre4_int[sp][av_cnt][0] &&
			(fill_to_pre == fill_to || !late_leap ||
				!accept[sp][av_cnt][0][144 + leap_id] || (fleap_end < fli_size[v] - 1 && !fill_goal))) filled = 0;
		else if (fill_to > 3 && !late_leap) filled = 0;
		else if (fill_to >= fill_pre4_int[sp][av_cnt][0] && late_leap) filled = 0;
		else if (fill_to == 2 && (fill_to_pre < 2 || !fleap_start) && !accept[sp][av_cnt][0][100 + leap_id]) filled = 0;
		else if (fill_to == 2 && fill_to_pre > 1 && fleap_start && !accept[sp][av_cnt][0][104 + leap_id]) filled = 0;
		else if (fill_from >= 3 && fill_from < fill_pre4_int[sp][av_cnt][0] && (!fill_from_pre || !late_leap || !accept[sp][av_cnt][0][144 + leap_id])) filled = 0;
		else if (fill_from > 3 && !late_leap) filled = 0;
		else if (fill_from >= fill_pre4_int[sp][av_cnt][0] && late_leap) filled = 0;
		else if (fill_from == 2 && !accept[sp][av_cnt][0][53 + leap_id]) filled = 0;
		else if (deviates > 2) filled = 0;
		else if (deviates == 1 && !accept[sp][av_cnt][0][42 + leap_id]) filled = 0;
		else if (deviates == 2 && !accept[sp][av_cnt][0][120 + leap_id]) filled = 0;
		if (!filled) {
			// If starting 3rd
			if (fleap_start == fin[v] && leap_size == 2 && accept[sp][av_cnt][0][1]) {
				FlagVL(v, 1, ssus[v][fleap_start], fli[v][fleap_end]);
				return 0;
			}
			if (child_leap && accept[sp][av_cnt][0][116 + leap_id])
				FlagVL(v, 116 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
			// Check if  leap is prefilled
			else {
				if (ls > 0) {
					ptail_len = 2 + (leap_size - 1) * fill_steps_mul;
					CountFill(ptail_len, pskips, pfill_to, 1, pfill_to_pre, pfill_from_pre, pfill_from,
						pdeviates, pdev_count, leap_prev, pfill_end, pfill_goal);
					prefilled = 1;
					if (pskips > allowed_pskips) prefilled = 0;
					else if (pfill_to > 2) prefilled = 0;
					else if (pfill_to == 2 && !accept[sp][av_cnt][0][104 + leap_id]) prefilled = 0;
					else if (pfill_from > 2) prefilled = 0;
					else if (pfill_from == 2 && !accept[sp][av_cnt][0][53 + leap_id]) prefilled = 0;
					else if (pdeviates > 2) prefilled = 0;
					else if (pdeviates == 1 && !accept[sp][av_cnt][0][42 + leap_id]) prefilled = 0;
					else if (pdeviates == 2 && !accept[sp][av_cnt][0][120 + leap_id]) prefilled = 0;
				}
				if (prefilled) {
					if (fli_size[v] - fleap_start <= pre_last_leaps[sp][av_cnt][0] + 1)
						FlagVL(v, 204 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
					else FlagVL(v, 112 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
				}
				else
					FlagVL(v, 124 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
			}
		}
		// Show compensation flags only if successfully compensated
		// This means that compensation errors are not shown if uncompensated (successfully or not)
		else {
			// Flag late uncompensated precompensated leap
			if (fill_to >= 3 && fill_to < fill_pre4_int[sp][av_cnt][0] && late_leap)
				FlagVL(v, 144 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
			else if (fill_from >= 3 && fill_from < fill_pre4_int[sp][av_cnt][0] && late_leap)
				FlagVL(v, 144 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
			// Flag prepared unfinished fill if it is not blocking 
			else if (fill_to == 2 && (fill_to_pre < 2 || !fleap_start)) FlagVL(v, 100 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
			// Flag unfinished fill if it is not blocking
			else if (fill_to == 2 && fill_to_pre > 1) FlagVL(v, 104 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
			// Flag after 3rd if it is not blocking
			if (fill_from == 2) FlagVL(v, 53 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
			// Flag deviation if it is not blocking
			if (deviates == 1) FlagVL(v, 42 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
			// Flag deviation if it is not blocking
			if (deviates == 2) FlagVL(v, 120 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
		}
	}
	return 0;
}

int CP2R::FailLeapMDC() {
	// Melody direction change (MDC)
	// 0 = close, 1 = far, 2 = no
	// Default mdc is close, because beginning equals to close mdc
	mdc1 = 0;
	int prev_note = cc[v][leap_start];
	for (int pos = leap_start - 1; pos >= 0; --pos) {
		if (cc[v][pos] != prev_note) {
			// Check if direction changes or pause or long without changes
			if (!cc[v][pos] || leap[v][leap_start] * (cc[v][pos] - prev_note) > 0 || mdc1 > 1) break;
			prev_note = cc[v][pos];
			++mdc1;
		}
	}
	mdc2 = 0;
	prev_note = cc[v][leap_end];
	for (int pos = leap_end + 1; pos < ep2; ++pos) {
		if (cc[v][pos] != prev_note) {
			// Check if direction changes or pause or long without changes
			if (!cc[v][pos] || leap[v][leap_start] * (cc[v][pos] - prev_note) < 0 || mdc2 > 2) break;
			prev_note = cc[v][pos];
			++mdc2;
		}
	}
	// Do not flag close+close
	if (mdc1 == 0 && mdc2 == 0) return 0;
	// Do not flag last 3rd in SAS, because it can be later converted to 5th
	if (fleap_end == fli_size[v] - 1 && ep2 < c_len && !leap_id) return 0;
	// Close + next
	if (!mdc1 && mdc2 == 1) {
		// Close + aux
		if ((sp == 3 || sp == 5) && (
			(fleap_end >= fli_size[v] - 3 && ep2 < c_len) ||
			(fleap_end < fli_size[v] - 3 && cc[v][fli[v][fleap_end + 2]] == cc[v][leap_end] &&
			(cc[v][fli[v][fleap_end + 3]] - cc[v][fli[v][fleap_end + 2]]) * leap[v][leap_start] < 0)))
			FlagVL(v, 510 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
		else FlagVL(v, 128 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
	}
	// Close + far
	else if (!mdc1 && mdc2 == 2) FlagVL(v, 140 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
	// Close + no
	else if (!mdc1 && mdc2 == 3) FlagVL(v, 108 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
	// next + close
	else if (mdc1 == 1 && !mdc2) {
		if (sp < 2 || bmli[fli[v][fleap_end]] == bmli[leap_start]) {
			// Aux + close
			if ((sp == 3 || sp == 5) && fleap_start > 2 && cc[v][fli[v][fleap_start - 2]] == cc[v][leap_start] &&
				(cc[v][fli[v][fleap_start - 2]] - cc[v][fli[v][fleap_start - 3]]) * leap[v][leap_start] < 0)
				FlagVL(v, 506 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
			else FlagVL(v, 59 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
		}
		else FlagVL(v, 476 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
	}
	// Far + close
	else if (mdc1 == 2 && !mdc2) {
		if (sp < 2 || bmli[fli[v][fleap_end]] == bmli[leap_start])
			FlagVL(v, 132 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
		else FlagVL(v, 25 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
	}
	// Next + next
	else if (mdc1 == 1 && mdc2 == 1) {
		if (sp < 2 || bmli[fli[v][fleap_end]] == bmli[leap_start]) {
			// Aux + aux
			if ((sp == 3 || sp == 5) && (
				(fleap_end >= fli_size[v] - 3 && ep2 < c_len) || (fleap_end < fli_size[v] - 3 && cc[v][fli[v][fleap_end + 2]] == cc[v][leap_end] &&
				(cc[v][fli[v][fleap_end + 3]] - cc[v][fli[v][fleap_end + 2]]) * leap[v][leap_start] < 0)) &&
				fleap_start > 2 && cc[v][fli[v][fleap_start - 2]] == cc[v][leap_start] &&
				(cc[v][fli[v][fleap_start - 2]] - cc[v][fli[v][fleap_start - 3]]) * leap[v][leap_start] < 0)
				FlagVL(v, 414 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
			else FlagVL(v, 63 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
		}
		else FlagVL(v, 460 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
	}
	// Next + far
	else if (mdc1 == 1 && mdc2 >= 2) {
		if (sp < 2 || bmli[fli[v][fleap_end]] == bmli[leap_start]) FlagVL(v, 391 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
		else FlagVL(v, 464 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
	}
	// Far + next
	else if (mdc1 >= 2 && mdc2 == 1) {
		if (sp < 2 || bmli[fli[v][fleap_end]] == bmli[leap_start]) FlagVL(v, 148 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
		else FlagVL(v, 468 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
	}
	// Far + far
	else if (mdc1 >= 2 && mdc2 >= 2) {
		if (sp < 2 || bmli[fli[v][fleap_end]] == bmli[leap_start]) FlagVL(v, 398 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
		else FlagVL(v, 472 + leap_id, ssus[v][fleap_start], fli[v][fleap_end]);
	}
	return 0;
}

// Count limits
void CP2R::GetMelodyInterval(int step1, int step2) {
	SET_READY(DR_nmin);
	// Calculate range
	nmin[v] = MAX_NOTE;
	nmax[v] = 0;
	for (int i = step1; i < step2; ++i) if (cc[v][i]) {
		if (cc[v][i] < nmin[v]) nmin[v] = cc[v][i];
		if (cc[v][i] > nmax[v]) nmax[v] = cc[v][i];
	}
	// Calculate diatonic limits
	nmind[v] = cc_c[nmin[v]];
	nmaxd[v] = cc_c[nmax[v]];
}

// Check if too many leaps
int CP2R::FailLeapSmooth(int l_max_smooth, int l_max_smooth_direct, int csel, int csel2,
	int flag1, int flag2, int flag3, int flag4, int first_run) {
	CHECK_READY(DR_leap, DR_c, DR_fli);
	// Clear variables
	int leap_sum2 = 0;
	int thirds_sum = 0;
	int leap_sum_corrected = 0;
	int max_leap_sum2 = 0;
	int max_leap_sum3 = 0;
	int smooth_sum = 0;
	int smooth_sum2 = 0;
	int leap_sum_s2 = 0;
	int fired4 = 0, fired5 = 0;
	for (ls = 0; ls < fli_size[v] - 1; ++ls) {
		s = fli2[v][ls];
		s1 = fli2[v][ls + 1];
		// Add new leap
		if (leap[v][s] != 0) {
			++leap_sum2;
			if (abs(c[v][s1] - c[v][s]) == 2) ++thirds_sum;
		}
		else {
			leap_sum2 = 0;
			thirds_sum = 0;
		}
		// Get maximum leap_sum
		leap_sum_corrected = leap_sum2 - min(thirds_sum, thirds_ignored[sp][av_cnt][0]);
		if (leap_sum_corrected > max_leap_sum2) {
			max_leap_sum2 = leap_sum_corrected;
			max_leap_sum3 = leap_sum2;
			leap_sum_s2 = s;
		}
		// Calculate penalty
		if (leap_sum_corrected == csel) {
			if (accept[sp][av_cnt][0][flag3] > 0) ++fpenalty;
		}
		if (leap_sum_corrected > csel2 && accept[sp][av_cnt][0][flag4] > 0) ++fpenalty;
		// Prohibit long smooth movement
		if (smooth[v][s] != 0) {
			++smooth_sum;
			if (smooth_sum >= l_max_smooth) {
				if (fired4) {
					fpenalty += severity[sp][av_cnt][0][flag1] + 1;
				}
				else {
					FlagVL(v, flag1, fli[v][ls + 1], fli[v][ls - smooth_sum + 1]);
					fired4 = 1;
				}
			}
		}
		else if (leap[v][s]) smooth_sum = 0;
		if (ls < fli_size[v] - 2) {
			// Prohibit long smooth movement in one direction
			if (smooth[v][s] != 0 && smooth[v][s] == smooth[v][fli2[v][ls + 1]]) {
				++smooth_sum2;
				if (smooth_sum2 >= l_max_smooth_direct) {
					if (fired5) {
						fpenalty += severity[sp][av_cnt][0][flag2] + 1;
					}
					else {
						FlagVL(v, flag2, fli[v][ls + 1], fli[v][ls - smooth_sum2 + 1]);
						fired5 = 1;
					}
				}
			}
			else if (smooth[v][s] || leap[v][s]) smooth_sum2 = 0;
			// Check if two notes repeat (including pause)
			if (first_run && ls > 0 && (cc[v][s] == cc[v][fli2[v][ls + 2]]) &&
				(cc[v][fli2[v][ls - 1]] == cc[v][fli2[v][ls + 1]]) &&
				(ep2 == c_len || ls + 2 < fli_size[v] - 1)) {
				// For cantus or species 1 / 4 do not check measure
				if (sp == 0 || sp == 1 || sp == 4) {
					// Same rhythm in first notes of repeat?
					if (llen[v][ls - 1] == llen[v][ls + 1]) {
						if (llen[v][ls - 1] == llen[v][ls]) {
							FlagVL(v, 402, fli[v][ls - 1], fli[v][ls + 2]);
						}
						else FlagVL(v, 403, fli[v][ls - 1], fli[v][ls + 2]);
					}
					else FlagVL(v, 404, fli[v][ls - 1], fli[v][ls + 2]);
				}
				// For species 2 / 3 / 5 check measure
				else {
					// Inside measure?
					if (bmli[fli[v][ls - 1]] == bmli[fli[v][ls + 2]]) {
						// Same rhythm in first notes of repeat?
						if (llen[v][ls - 1] == llen[v][ls + 1]) {
							if (llen[v][ls - 1] == llen[v][ls]) {
								FlagVL(v, 411, fli[v][ls - 1], fli[v][ls + 2]);
							}
							else FlagVL(v, 412, fli[v][ls - 1], fli[v][ls + 2]);
						}
						else FlagVL(v, 413, fli[v][ls - 1], fli[v][ls + 2]);
					}
					else {
						// Same rhythm in first notes of repeat?
						if (llen[v][ls - 1] == llen[v][ls + 1]) {
							if (llen[v][ls - 1] == llen[v][ls]) {
								FlagVL(v, 402, fli[v][ls - 1], fli[v][ls + 2]);
							}
							else FlagVL(v, 403, fli[v][ls - 1], fli[v][ls + 2]);
						}
						else FlagVL(v, 404, fli[v][ls - 1], fli[v][ls + 2]);
					}
				}
			}
		}
	}
	if (first_run && max_leap_sum2 >= csel) {
		if (max_leap_sum2 > csel2)
			FlagVL(v, flag4, fli[v][bli[v][leap_sum_s2] + 1],
				fli[v][max(0, bli[v][leap_sum_s2] - max_leap_sum3 + 1)]);
		else
			FlagVL(v, flag3, fli[v][bli[v][leap_sum_s2] + 1],
				fli[v][max(0, bli[v][leap_sum_s2] - max_leap_sum3 + 1)]);
	}
	return 0;
}

int CP2R::FailStagnation(int steps, int notes, int fl) {
	CHECK_READY(DR_nmin, DR_fli);
	// Do not test if flag disabled and not evaluating
	if (task != tEval && accept[sp][av_cnt][0][fl] == -1) return 0;
	// Clear nstat
	for (int i = nmin[v]; i <= nmax[v]; ++i) nstat[i] = 0;
	nstat[0] = 0;
	for (ls = 0; ls < fli_size[v]; ++ls) {
		s = fli[v][ls];
		// Add new note to stagnation array
		++nstat[cc[v][s]];
		// Subtract old note
		if (ls >= steps) --nstat[cc[v][fli[v][ls - steps]]];
		// Check if too many repeating notes
		if (nstat[cc[v][s]] > notes) 
			FlagVL(v, fl, s, fli[v][max(0, ls - steps)]);
	}
	return 0;
}

// Prohibit multiple culminations
int CP2R::FailMultiCulm() {
	CHECK_READY(DR_fli, DR_nmin);
	int culm_count = 0;
	int culm_ls = -1;
	int multi_culm_fired = 0;
	// Do not find culminations if too early
	if (ep2 < c_len) return 0;
	for (ls = 0; ls < fli_size[v]; ++ls) {
		if (cc[v][fli[v][ls]] == nmax[v]) {
			++culm_count;
			culm_ls = ls;
			if (culm_count > 1) {
				if (v == av_cnt - 1 && av_cnt > 1) {
					if (multi_culm_fired) fpenalty += severity[sp][av_cnt][0][12] + 1;
					else {
						multi_culm_fired = 1;
						FlagV(v, 12, fli[v][culm_ls]);
					}
				}
				else {
					if (multi_culm_fired) fpenalty += severity[sp][av_cnt][0][305] + 1;
					else {
						multi_culm_fired = 1;
						FlagV(v, 305, fli[v][culm_ls]);
					}
				}
			}
		}
	}
	if (culm_ls == -1) {
		culm_ls = 0;
		CString est;
		est.Format("Warning: culm_ls cannot be detected");
		WriteLog(5, est);
	}
	if (v == av_cnt - 1) {
		// Prohibit culminations at first steps
		if (culm_ls < (early_culm3[sp][av_cnt][0] * fli_size[v]) / 100) FlagV(v, 193, fli[v][culm_ls]);
		if (culm_ls < early_culm[sp][av_cnt][0] - 1) FlagV(v, 78, fli[v][culm_ls]);
		else if (culm_ls < early_culm2[sp][av_cnt][0] - 1) FlagV(v, 79, fli[v][culm_ls]);
		// Prohibit culminations at last steps
		if (culm_ls >= fli_size[v] - late_culm[sp][av_cnt][0]) FlagV(v, 21, fli[v][culm_ls]);
	}
	return 0;
}

int CP2R::FailFirstNotes() {
	CHECK_READY(DR_fli, DR_pc);
	if (v == av_cnt - 1) vp = vpExt;
	else if (v == 0) vp = vpBas;
	else vp = vpNbs;
	// Prohibit first note not tonic
	s = fin[v];
	if (pc[v][s] != 0 && !bmli[s]) {
		if (pc[v][s] == 4) Flag(v, 532, s, v);
		else if (pc[v][s] == 2) {
			if (sus[v][bli[v][s]]) Flag(v, 533, s, v);
			else Flag(v, 534, s, v);
		}
		else FlagV(v, 535, s);
	}
	return 0;
}

int CP2R::FailLastNotes() {
	CHECK_READY(DR_fli, DR_pc, DR_nih);
	if (v == av_cnt - 1) vp = vpExt;
	else if (v == 0) vp = vpBas;
	else vp = vpNbs;
	// Do not check if melody is short yet
	if (fli_size[v] < 3) return 0;
	// Prohibit last note not tonic
	if (ep2 != c_len) return 0;
	s = fli[v][fli_size[v] - 1];
	s_1 = fli[v][fli_size[v] - 2];
	s_2 = fli[v][fli_size[v] - 3];
	if (pc[v][s] != 0) {
		if (pc[v][s] == 4) Flag(v, 536, s, v);
		else if (pc[v][s] == 2) Flag(v, 537, s, v);
		else Flag(v, 538, s, v);
	}
	if (mminor) {
		// Prohibit major second up before I
		if (pcc[v][s] == 0 && pcc[v][s_2] == 10 && nih[v][s] && nih[v][s_2]) FlagVL(v, 74, s_2, s);
	}
	return 0;
}

// Search for adjacent or symmetric repeats
int CP2R::FailAdSymRepeat(int relen) {
	CHECK_READY(DR_fli, DR_c);
	// Check only same beat
	int sym_period = npm;
	// Do not test if flag disabled and not testing
	//if (task != tEval && accept[sp][av_cnt][0][flag] == -1) return 0;
	// Cycle through all notes that can be repeated
	for (ls = 0; ls <= fli_size[v] - relen * 2; ++ls) {
		int rpos1 = 0;
		int rpos2 = 0;
		// Check adjacent repeat
		if (IsRepeat(ls, ls + relen, relen)) {
			rpos1 = ls + relen;
		}
		// If same beat is not adjacent, get same beat and check
		else if (sp > 1 && ((fli[v][ls + relen] - fli[v][ls]) % sym_period)) {
			for (int x = 1; x < 4; ++x) {
				if (ls + x <= fli_size[v] - relen * 2 && !((fli[v][ls + relen + x] - fli[v][ls]) % sym_period)) {
					if (IsRepeat(ls, ls + relen + x, relen)) {
						rpos1 = ls + relen + x;
					}
				}
			}
		}
		// If any repeat is found, check one more repeat
		if (rpos1 && rpos1 <= fli_size[v] - relen * 2) {
			// Check adjacent repeat
			if (IsRepeat(ls, rpos1 + relen, relen)) {
				rpos2 = rpos1 + relen;
			}
			// If same beat is not adjacent, get same beat and check
			else if (sp > 1 && ((fli[v][rpos1 + relen] - fli[v][rpos1]) % sym_period)) {
				for (int x = 1; x < 4; ++x) {
					if (rpos1 + x <= fli_size[v] - relen * 2 && !((fli[v][rpos1 + relen + x] - fli[v][rpos1]) % sym_period)) {
						if (IsRepeat(rpos1, rpos1 + relen + x, relen)) {
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
					FlagVL(v, 313, fli[v][ls], fli[v][ls + relen - 1]);
				else
					FlagVL(v, 407, fli[v][ls], fli[v][ls + relen - 1]);
			}
			// Flag one repeat
			else if (rpos1) {
				if (rpos1 == ls + relen)
					FlagVL(v, 311, fli[v][ls], fli[v][ls + relen - 1]);
				else
					FlagVL(v, 405, fli[v][ls], fli[v][ls + relen - 1]);
			}
		}
		if (relen == 4) {
			// Flag two repeats
			if (rpos2) {
				if (rpos2 == rpos1 + relen)
					FlagVL(v, 314, fli[v][ls], fli[v][ls + relen - 1]);
				else
					FlagVL(v, 408, fli[v][ls], fli[v][ls + relen - 1]);
			}
			// Flag one repeat
			else if (rpos1) {
				if (rpos1 == ls + relen)
					FlagVL(v, 312, fli[v][ls], fli[v][ls + relen - 1]);
				else
					FlagVL(v, 406, fli[v][ls], fli[v][ls + relen - 1]);
			}
		}
	}
	return 0;
}

int CP2R::IsRepeat(int ls1, int ls2, int relen) {
	int found = 1;
	for (int i = 0; i < relen; ++i) {
		if (c[v][fli[v][ls1 + i]] != c[v][fli[v][ls2 + i]]) {
			found = 0;
			break;
		}
		if (i < relen - 1 && llen[v][ls1 + i] != llen[v][ls2 + i]) {
			found = 0;
			break;
		}
	}
	return found;
}

// Moving average
void CP2R::maVector(vector<float> &v, vector<float> &v2, int range) {
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

void CP2R::maVector(vector<int> &v, vector<float> &v2, int range) {
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

void CP2R::mawVector(vector<int> &v, vector<float> &v2, int range) {
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

void CP2R::mawVector(vector<float> &v, vector<float> &v2, int range) {
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

void CP2R::MakeMacc() {
	SET_READY(DR_macc);
	int pos1, pos2;
	int ma_range = 2 * minl[v];
	macc_range = ma_range;
	macc2_range = ma_range * 2;
	// Deviation weight
	maw.clear();
	for (int i = 0; i <= ma_range; ++i) {
		maw.push_back(1 - i * 0.5 / ma_range);
	}
	float de, maw_sum;
	// Moving average
	mawVector(cc[v], macc, ma_range);
	// Smooth
	mawVector(macc, macc2, ma_range);
	// Deviation
	for (int s = 0; s < ep2; ++s) {
		pos1 = max(0, s - ma_range);
		pos2 = min(ep2 - 1, s + ma_range);
		de = 0;
		maw_sum = 0;
		for (int x = pos1; x <= pos2; ++x) if (cc[v][x]) {
			de += maw[abs(x - s)] * SQR(cc[v][x] - macc[s]);
			maw_sum += maw[abs(x - s)];
		}
		if (maw_sum) decc[s] = SQRT(de / maw_sum);
		else decc[s] = 0;
	}
	// Smooth
	mawVector(decc, decc2, ma_range);
}

int CP2R::FailLocalMacc(int notes, float mrange, int fl) {
	CHECK_READY(DR_fli, DR_macc);
	// Do not test if flag disabled and not testing
	if (task != tEval && accept[sp][av_cnt][0][fl] == -1) return 0;
	// Do not test if not enough notes. If melody is short, than global range check is enough
	if (fli_size[v] < notes) return 0;
	float lmin, lmax, maccr;
	int maccr_count = 0;
	int s;
	int ls_max = fli_size[v] - notes;
	int ls_max2;
	int fired = 0;
	// Loop through windows
	for (ls = 0; ls <= ls_max; ++ls) {
		lmin = MAX_NOTE;
		lmax = 0;
		ls_max2 = ls + notes;
		// Do not check if later notes are not created
		if (ep2 < c_len && fli2[v][ls_max2 - 1] + macc2_range >= ep2) continue;
		// Loop inside each window
		for (s = fli[v][ls]; s <= fli2[v][ls_max2 - 1]; ++s) {
			if (macc2[s] < lmin) lmin = macc2[s];
			if (macc2[s] > lmax) lmax = macc2[s];
		}
		// Record range
		maccr = lmax - lmin;
		++maccr_count;
		// Check range
		if (lmin < MAX_NOTE && lmax > 0 && maccr < mrange) {
			if (fired) {
				fpenalty += severity[sp][av_cnt][0][fl] + 1;
			}
			else {
				FlagVL(v, fl, fli[v][ls], fli[v][ls_max2 - 1]);
				fired = 1;
			}
		}
	}
	return 0;
}

int CP2R::FailLocalRange(int notes, int mrange, int fl) {
	CHECK_READY(DR_fli, DR_c);
	// Do not test if flag disabled and not testing
	//if (task != tEval && accept[sp][av_cnt][0][fl] == -1) return 0;
	// Do not test if not enough notes. If melody is short, than global range check is enough
	if (fli_size[v] < notes) return 0;
	int lmin, lmax, s;
	int ls_max = fli_size[v] - notes;
	int ls_max2;
	int fired = 0;
	int finish = 0;
	// Loop through windows
	for (ls = 0; ls <= ls_max; ++ls) {
		lmin = MAX_NOTE;
		lmax = 0;
		ls_max2 = ls + notes;
		// Loop inside each window
		for (int ls2 = ls; ls2 < ls_max2; ++ls2) {
			s = fli[v][ls2];
			// Process non-pauses
			if (cc[v][s]) {
				if (c[v][s] < lmin) lmin = c[v][s];
				if (c[v][s] > lmax) lmax = c[v][s];
			}
			else {
				// If we see pause, grow window if possible by two notes (one for pause and one additionally, because pause makes stagnation less standing out
				if (ls_max2 < fli_size[v] - 1) ls_max2 += 2;
				// If not possible, finish scanning
				else {
					finish = 1;
				}
			}
		}
		// Finish if we faced pause at the end
		if (finish) break;
		// Check range
		if (lmax - lmin < mrange) {
			if (fired) {
				fpenalty += severity[sp][av_cnt][0][fl] + 1;
			}
			else {
				FlagVL(v, fl, fli[v][ls], fli[v][ls_max2 - 1]);
				fired = 1;
			}
		}
	}
	return 0;
}

int CP2R::FailLocalPiCount(int notes, int picount, int fl) {
	CHECK_READY(DR_fli, DR_nmin);
	// Do not test if flag disabled and not testing
	if (task != tEval && accept[sp][av_cnt][0][fl] == -1) return 0;
	// Do not test if not enough notes
	if (fli_size[v] < notes) return 0;
	int picount2, i;
	// Clear nstat
	for (i = nmin[v]; i <= nmax[v]; ++i) nstat[i] = 0;
	int pauses_inside = 0;
	int last_flag_ls = -10;
	for (ls = 0; ls < fli_size[v]; ++ls) {
		s = fli[v][ls];
		// Add new note to stagnation array
		++nstat[cc[v][s]];
		// Subtract old note
		if (ls >= notes) {
			--nstat[cc[v][fli[v][ls - notes]]];
		}
		// Check if little pitches
		if (ls >= notes - 1) {
			picount2 = 0;
			for (i = nmin[v]; i <= nmax[v]; ++i) if (nstat[i]) ++picount2;
			// Count pause as pitch for short windows
			if (notes < 10) {
				if (nstat[0]) ++picount2;
			}
			// For long windows do nothing - this shortens window a little, but this is not very important
			if (picount2 < picount) {
				if (ls - last_flag_ls > 1)
					FlagVL(v, fl, fli[v][ls - notes + 1], s);
				last_flag_ls = ls;
			}
		}
	}
	return 0;
}

float CP2R::GetTonicWeight(int l_ls, int tt) {
	int l_s = fli[v][l_ls];
	// Not species 3 or 5?
	if (sp != 3 && sp != 5) return 1.0;
	// Tonic downbeat?
	if (!beat[v][l_ls]) return 1.0;
	// Length is above quarter (for species 5)?
	if (rlen[v][l_ls] > 2) return 1.0;
	// Length of tonic note is greater than previous note?
	if (l_ls > 0 && llen[v][l_ls] > llen[v][l_ls - 1]) return 1.0;
	// Leap to tonic?
	if (l_ls > 0 && abs(cc[v][l_s] - cc[v][fli[v][l_ls] - 1]) > 4) return 1.0;
	// Local climax?
	if (c[v][l_s] >= lclimax[v][l_s]) return 1.0;
	return 0.9;
}

int CP2R::FailTonic(int tt) {
	CHECK_READY(DR_pc, DR_fli);
	CHECK_READY(DR_lclimax);
	if (sp) {
		CHECK_READY(DR_beat);
	}
	vector<float> tcount;
	int s9;
	tcount.resize(13);
	int fire, fired = 0;
	// Find first and last tonic
	int first_tonic = -1;
	int last_tonic = -1;
	for (ls = 0; ls < fli_size[v]; ++ls) {
		if (!pc[v][fli[v][ls]]) {
			first_tonic = ls;
			break;
		}
	}
	for (ls = fli_size[v] - 1; ls >= 0; --ls) {
		if (!pc[v][fli[v][ls]]) {
			last_tonic = ls;
			break;
		}
	}
	// Do not check if melody is short
	if (fli_size[v] < 3) return 0;
	int tw = tonic_window[tt][sp][av_cnt][0];
	int tm = tonic_max[tt][sp][av_cnt][0];
	for (ls = 0; ls < fli_size[v]; ++ls) {
		s = fli[v][ls];
		// Decrement for previous tonic note
		if (ls >= tw) {
			s9 = fli[v][ls - tw];
			if (!pc[v][s9])
				tcount[cc[v][s9] / 12] -= GetTonicWeight(ls - tw, tt);
		}
		if (!pc[v][s]) {
			// Increment for current tonic note
			tcount[cc[v][s] / 12] += GetTonicWeight(ls, tt);
			// Check count of tonic notes
			if (tcount[cc[v][s] / 12] >= tm) {
				if (fired) {
					fpenalty += severity[sp][av_cnt][0][70 + tt] + 1;
				}
				else {
					FlagVL(v, 70 + tt, s, fli[v][max(0, ls - tw)]);
					fired = 1;
				}
			}
		}
		tweight[v][ls] = vmax(tcount);
	}
	return 0;
}

// Check tritone t1-t2 which has to resolve from ta to tb. Use fleap_start/fleap_end
void CP2R::GetTritoneResolution(int ta, int t1, int t2, int tb, int &res1, int &res2) {
	res1 = 0;
	res2 = 0;
	// Real resolution notes
	int ta2, tb2;
	// Get real resolution notes
	if (pcc[v][fli[v][fleap_end]] == t2) {
		ta2 = ta;
		tb2 = tb;
		// Outer resolution is not a resolution
		if (cc[v][fli[v][fleap_end]] > cc[v][fli[v][fleap_start]]) return;
	}
	else {
		ta2 = tb;
		tb2 = ta;
		// Outer resolution is not a resolution
		if (cc[v][fli[v][fleap_end]] < cc[v][fli[v][fleap_start]]) return;
	}
	// Get resolution window
	int rwin = 1;
	if (av_cnt > 1 && bmli[fli[v][fleap_start]] >= mli.size() - 2) rwin = max(1, (2 * tritone_res_quart[sp][av_cnt][0]));
	// Scan preparation
	if (fleap_start > 0) {
		int pos1 = max(0, fli[v][fleap_start] - rwin);
		int pos2 = min(ep2, fli[v][fleap_end]);
		for (int i = pos1; i < pos2; ++i) {
			if (pcc[v][i] == ta2 && abs(cc[v][i] - cc[v][fli[v][fleap_start]]) < 5) {
				res1 = 1;
				break;
			}
		}
	}
	// Do not check if cut by scan window
	if (fli2[v][fleap_end] + 1 + rwin > ep2 && ep2 < c_len) {
		res2 = 1;
		return;
	}
	// Scan resolution
	if (fleap_end < fli_size[v] - 1) {
		int pos1 = max(0, fli2[v][fleap_start] + 1);
		int pos2 = min(ep2, fli2[v][fleap_end] + 1 + rwin);
		for (int i = pos1; i < pos2; ++i) {
			if (pcc[v][i] == tb2 && abs(cc[v][i] - cc[v][fli[v][fleap_end]]) < 5) {
				res2 = 1;
				break;
			}
		}
	}
}

// Check tritone t1-t2 which has to resolve from ta to tb
int CP2R::FailTritone(int ta, int t1, int t2, int tb) {
	int found;
	int res1 = 0; // First note resolution flag
	int res2 = 0; // Second note resolution flag
								// Tritone prohibit
	leap_start = s;
	found = 0;
	// Check consecutive tritone
	if ((pcc[v][s1] == t2 && pcc[v][s] == t1) || (pcc[v][s1] == t1 && pcc[v][s] == t2)) found = 1;
	// Check tritone with additional note inside
	else if (ls > 0) {
		// Check pitches
		if ((pcc[v][s1] == t2 && pcc[v][s_1] == t1) || (pcc[v][s1] == t1 && pcc[v][s_1] == t2))
			// Check intermediate note and mdc
			if (c[v][s] > c[v][s1] && c[v][s] < c[v][s_1]) {
				if ((ls < 2 || c[v][s_2] < c[v][s_1] || leap[v][s_2]) && (ls > fli_size[v] - 3 || c[v][s2] > c[v][s1] || leap[v][s1])) {
					found = 2;
					leap_start = s_1;
				}
			}
			else if (c[v][s] < c[v][s1] && c[v][s] > c[v][s_1]) {
				if ((ls<2 || c[v][s_2] > c[v][s_1] || leap[v][s_2]) && (ls>fli_size[v] - 3 || c[v][s2] < c[v][s1] || leap[v][s1])) {
					found = 2;
					leap_start = s_1;
				}
			}
	}
	fleap_start = bli[v][leap_start];
	fleap_end = bli[v][s1];
	// Do not check tritone if it is at the end of not-last window
	if (ls == fli_size[v] - 2 && ep2 != c_len) return 0;
	if (found) {
		/*
		// Check if tritone is highest leap if this is last window
		if (ep2 == c_len && (av_cnt == 1 || !cantus_high)) {
		if ((cc[v][leap_start] >= lclimax[v][leap_start]) || (cc[v][s1] >= lclimax[v][leap_start])) {
		// Consecutive
		if (found == 1) FlagVL(v, 32, s0, fli[v][ls + 1]);
		// Compound framed
		else if (found == 2) FlagVL(v, 373, fli[v][ls - 1], fli[v][ls + 1]); //-V547
		}
		}
		*/
		// Check if resolution is correct
		GetTritoneResolution(ta, t1, t2, tb, res1, res2);
		// Flag resolution for consecutive tritone
		if (found == 1) {
			if (res1*res2 == 0)
				FlagVL(v, 31, s0, fli[v][ls + 1]);
			else FlagVL(v, 2, s0, fli[v][ls + 1]);
		}
		// Flag resolution for tritone with intermediate note, framed
		else if (found == 2) { //-V547
			if (res1*res2 == 0) FlagVL(v, 372, fli[v][ls - 1], fli[v][ls + 1]);
			else FlagVL(v, 371, fli[v][ls - 1], fli[v][ls + 1]);
		}
	}
	return 0;
}

int CP2R::FailTritones() {
	CHECK_READY(DR_pc, DR_c, DR_fli);
	CHECK_READY(DR_leap, DR_lclimax);
	for (ls = 0; ls < fli_size[v] - 1; ++ls) {
		s0 = fli[v][ls];
		s = fli2[v][ls];
		s1 = fli2[v][ls + 1];
		if (ls > 0) s_1 = fli2[v][ls - 1];
		if (ls > 1) s_2 = fli2[v][ls - 2];
		if (ls < fli_size[v] - 2) s2 = fli2[v][ls + 2];
		// Warning: tritone F#C in minor is not detected (can add FailTritone to detect) because it is already prohibited by Unaltered near altered.
		// If you allow Unaltered near altered, you should implement FailTritone for F#C.
		if (mode == 9) {
			if (FailTritone(3, 5, 11, 0)) return 1;
			if (FailTritone(7, 8, 2, 3)) return 1;
			if (FailTritone(10, 9, 3, 2)) return 1;
		}
		else {
			if (FailTritone(4, 5, 11, 0)) return 1;
		}
	}
	return 0;
}

// Calculate global fill
int CP2R::FailGlobalFill() {
	CHECK_READY(DR_nmin, DR_c);
	// Clear nstat2
	for (int i = nmind[v]; i <= nmaxd[v]; ++i) nstat2[i] = 0;
	// Count nstat2
	for (int x = 0; x < ep2; ++x) ++nstat2[c[v][x]];
	// Check nstat2
	if (ep2 < c_len) return 0;
	int skips = 0;
	int skips2 = 0;
	for (int x = nmind[v] + 1; x < nmaxd[v]; ++x) if (!nstat2[x]) {
		if (!nstat2[x + 1]) {
			++skips2;
			++x;
		}
		else ++skips;
	}
	// Set flags
	if (skips2) FlagV(v, 69, fin[v]);
	if (skips == 1) FlagV(v, 67, fin[v]);
	else if (skips >= 2) FlagV(v, 68, fin[v]);
	return 0;
}

int CP2R::FailAdjacentTritone2(int ta, int t1, int t2, int tb) {
	int found = 0;
	int res1 = 0;
	int res2 = 0;
	// Check consecutive tritone
	if ((pcc[v][s2] != t2 || pcc[v][s] != t1) &&
		(pcc[v][s2] != t1 || pcc[v][s] != t2)) return 0;
	// Check different measures
	//if (bmli[s] != bmli[s2]) return 0;
	fleap_start = ls;
	fleap_end = ls + 1;
	// Do not check tritone if it is at the end of not-last window
	if (ls >= fli_size[v] - 2 && ep2 != c_len) return 0;
	// Check framed by ending, pause, leap or opposite movement
	if ((ls >= fli_size[v] - 2 || !cc[v][fli2[v][ls + 2]] ||
		leap[v][s] * (cc[v][fli2[v][ls + 2]] - cc[v][fli2[v][ls + 1]]) < 0 ||
		leap[v][fli2[v][ls + 1]]) &&
		(ls == 0 || !cc[v][fli2[v][ls - 1]] ||
			leap[v][s] * (cc[v][s] - cc[v][fli2[v][ls - 1]]) < 0 ||
			leap[v][fli2[v][ls - 1]])) found = 1;
	if (!found) {
		if (sp == 5) {
			// Is last note at least 1/2 and not shorter than previous?
			if (rlen[v][ls + 1] >= 4 && llen[v][ls + 1] >= llen[v][ls]) found = 2;
		}
		// Is last note longer than previous ?
		if (llen[v][ls + 1] > llen[v][ls]) found = 2;
	}
	// Search measure for repeat tritone second note
	if (!found) {
		ms = bmli[s2];
		int mea_end, ls1, ls2;
		int note_count = 0;
		if (ms < mli.size() - 1) mea_end = mli[ms + 1] - 1;
		else mea_end = c_len - 1;
		// Prevent going out of window
		if (mea_end < ep2) {
			ls1 = bli[v][mli[ms]];
			ls2 = bli[v][mea_end];
			// Loop inside measure
			for (int ls3 = ls1; ls3 <= ls2; ++ls3) {
				if (cc[v][fli[v][ls3]] == cc[v][s2]) ++note_count;
			}
			if (note_count > 1) found = 3;
		}
	}
	//if (!found) return 0;
	// Check if tritone is highest leap if this is last window
	/*
	if (ep2 == c_len && !cantus_high) {
	if ((cc[v][s] >= lclimax[v][s]) || (cc[v][s2] >= lclimax[v][s2])) {
	if (found == 0) FlagVL(v, 370, fli[v][fleap_start], fli[v][fleap_end]);
	else if (found == 1) FlagVL(v, 367, fli[v][fleap_start], fli[v][fleap_end]);
	else FlagVL(v, 362, fli[v][fleap_start], fli[v][fleap_end]);
	}
	}
	*/
	GetTritoneResolution(ta, t1, t2, tb, res1, res2);
	// Flag resolution for normal tritone
	if (found == 0) {
		if (res1*res2 == 0) FlagVL(v, 369, fli[v][fleap_start], fli[v][fleap_end]);
		else FlagVL(v, 368, fli[v][fleap_start], fli[v][fleap_end]);
	}
	// Flag resolution for framed tritone
	else if (found == 1) {
		if (res1*res2 == 0) FlagVL(v, 366, fli[v][fleap_start], fli[v][fleap_end]);
		else FlagVL(v, 365, fli[v][fleap_start], fli[v][fleap_end]);
	}
	// Flag resolution for accented tritone
	else {
		if (res1*res2 == 0) FlagVL(v, 361, fli[v][fleap_start], fli[v][fleap_end]);
		else FlagVL(v, 360, fli[v][fleap_start], fli[v][fleap_end]);
	}
	return 0;
}

// This function is for species 2-5
int CP2R::FailAdjacentTritones() {
	// Find adjacent notes
	CHECK_READY(DR_pc, DR_c, DR_fli);
	CHECK_READY(DR_leap);
	for (ls = 0; ls < fli_size[v] - 1; ++ls) {
		s = fli2[v][ls];
		s2 = fli[v][ls + 1];
		if (mode == 9) {
			if (FailAdjacentTritone2(3,  5, 11, 0)) return 1;
			if (FailAdjacentTritone2(7,  8, 2,  3)) return 1;
			if (FailAdjacentTritone2(10, 9, 3,  2)) return 1;
		}
		else {
			if (FailAdjacentTritone2(4, 5, 11, 0)) return 1;
		}
	}
	return 0;
}

// This function is for species 2-5
int CP2R::FailTritones2() {
	CHECK_READY(DR_pc, DR_c, DR_fli);
	CHECK_READY(DR_leap, DR_lclimax);
	// Find both tritone notes in measure (non-adjacent)
	int mea_end, ls1, ls2, lpcc, cch, ccl, exceed, found, res1, res2, last_repeat;
	for (ms = 0; ms < mli.size(); ++ms) {
		// Stop processing when last measure is not fully generated
		if (ms == mli.size() - 1 && ep2 < c_len) break;
		// Get first and last measure notes
		if (ms + 1 < mli.size()) mea_end = mli[ms + 1] - 1;
		else mea_end = c_len - 1;
		// Prevent going out of window
		if (mea_end >= ep2) break;
		ls1 = bli[v][mli[ms]];
		ls2 = bli[v][mea_end];
		vector<vector<int>> tfound, tfound2;
		tfound.resize(3);
		tfound2.resize(3);
		// Loop inside measure
		for (ls = ls1; ls <= ls2; ++ls) {
			lpcc = pcc[v][fli[v][ls]];
			// Find first and last notes of major tritone
			if (lpcc == 5) tfound[0].push_back(ls);
			if (lpcc == 11) tfound2[0].push_back(ls);
			if (mode == 9) {
				// Find first and last notes of minor tritone
				if (lpcc == 8) tfound[1].push_back(ls);
				if (lpcc == 2) tfound2[1].push_back(ls);
				if (lpcc == 9) tfound[2].push_back(ls);
				if (lpcc == 3) tfound2[2].push_back(ls);
			}
		}
		// Loop through tritone types 
		for (int tt = 0; tt < 3; ++tt) {
			// Check each note combination
			for (int tn = 0; tn < tfound[tt].size(); ++tn) {
				for (int tn2 = 0; tn2 < tfound2[tt].size(); ++tn2) {
					found = 0;
					// Do not check adjacent
					if (abs(tfound[tt][tn] - tfound2[tt][tn2]) == 1) continue;
					// Do intermediate notes exceed pitch range?
					if (tfound[tt][tn] > tfound2[tt][tn2]) {
						last_repeat = tfound[tt].size();
						fleap_start = tfound2[tt][tn2];
						fleap_end = tfound[tt][tn];
					}
					else {
						last_repeat = tfound2[tt].size();
						fleap_start = tfound[tt][tn];
						fleap_end = tfound2[tt][tn2];
					}
					// Do not check if fleap_end is last note in scan window
					if (fleap_end >= fli_size[v] - 1 && ep2 != c_len) return 0;
					// Low / high notes
					ccl = min(cc[v][fli[v][tfound[tt][tn]]], cc[v][fli[v][tfound2[tt][tn2]]]);
					cch = max(cc[v][fli[v][tfound[tt][tn]]], cc[v][fli[v][tfound2[tt][tn2]]]);
					exceed = 0;
					for (ls = fleap_start + 1; ls < fleap_end; ++ls) if (cc[v][fli[v][ls]]) {
						if (cc[v][fli[v][ls]] > cch || cc[v][fli[v][ls]] < ccl) {
							exceed = 1;
							break;
						}
					}
					if (exceed) continue;
					// Check framed 
					if ((fleap_end == fli_size[v] - 1 || !cc[v][fli2[v][fleap_end + 1]] ||
						(cc[v][fli[v][fleap_end]] - cc[v][fli2[v][fleap_start]]) *
						(cc[v][fli2[v][fleap_end + 1]] - cc[v][fli[v][fleap_end]]) < 0 ||
						leap[v][fli2[v][fleap_end]]) &&
						(fleap_start == 0 || !cc[v][fli[v][fleap_start - 1]] ||
						(cc[v][fli[v][fleap_end]] - cc[v][fli2[v][fleap_start]]) *
							(cc[v][fli2[v][fleap_start]] - cc[v][fli[v][fleap_start - 1]]) < 0 ||
							leap[v][fli2[v][fleap_start - 1]])) found = 1;
					if (!found) {
						if (sp == 5) {
							// Is last note at least 1/2 and not shorter than previous?
							if (rlen[v][fleap_end] >= 4 && llen[v][fleap_end] >= llen[v][fleap_start]) found = 2;
						}
						// Is last note longer than previous ?
						if (llen[v][fleap_end] > llen[v][fleap_start]) found = 2;
					}
					// Check last note repeats
					if (!found) {
						if (last_repeat > 1) found = 3;
					}
					if (mode == 9) {
						if (tt == 0)
							GetTritoneResolution(3, 5, 11, 0, res1, res2);
						if (tt == 1)
							GetTritoneResolution(7, 8, 2, 3, res1, res2);
						if (tt == 2)
							GetTritoneResolution(2, 3, 9, 10, res1, res2);
					}
					else {
						if (tt == 0)
							GetTritoneResolution(4, 5, 11, 0, res1, res2);
					}
					// Probably next line can be moved higher (before GetTritoneResolution) for performance optimization
					if (!found) continue;
					/*
					// Check if tritone is highest leap if this is last window
					if (ep2 == c_len && !cantus_high) {
					if ((cc[v][fli[v][fleap_start]] >= lclimax[v][fli[v][fleap_start]]) ||
					(cc[v][fli[v][fleap_end]] >= lclimax[v][fli[v][fleap_end]])) {
					if (found == 1) FlagVL(v, 363, fli[v][fleap_start], fli[v][fleap_end]);
					else FlagVL(v, 364, fli[v][fleap_start], fli[v][fleap_end]);
					}
					}
					*/
					// Flag resolution for framed tritone
					if (found == 1) {
						if (res1*res2 == 0) FlagVL(v, 19, fli[v][fleap_start], fli[v][fleap_end]);
						else FlagVL(v, 18, fli[v][fleap_start], fli[v][fleap_end]);
					}
					// Flag resolution for accented tritone
					else {
						if (res1*res2 == 0) FlagVL(v, 343, fli[v][fleap_start], fli[v][fleap_end]);
						else FlagVL(v, 342, fli[v][fleap_start], fli[v][fleap_end]);
					}
				}
			}
		}
	}
	return 0;
}

int CP2R::FailRhythm() {
	CHECK_READY(DR_fli, DR_beat, DR_sus);
	CHECK_READY(DR_leap, DR_nlen);
	if (sp == 2) {
		if (FailRhythm2()) return 1;
	}
	else if (sp == 3) {
		if (FailRhythm3()) return 1;
	}
	else if (sp == 4) {
		if (FailRhythm4()) return 1;
	}
	else if (sp == 5) {
		if (FailRhythm5()) return 1;
	}
	return 0;
}

// Fail rhythm for species 2
int CP2R::FailRhythm2() {
	// Last measure not whole
	if (c_len - fli[v][fli_size[v] - 1] < npm || 
		(c_len == ep2 && llen[v][fli_size[v] - 1] != npm)) {
		FlagV(v, 267, fli[v][fli_size[v] - 1]);
	}
	for (ls = 0; ls < fli_size[v] - 1; ++ls) {
		// Slurred note inside measure
		if (llen[v][ls] > nlen[v] && !sus[v][ls] && cc[v][fli[v][ls]]) FlagV(v, 160, fli[v][ls]);
	}
	return 0;
}

// Fail rhythm for species 4
int CP2R::FailRhythm4() {
	// Last measure not whole
	if (c_len - fli[v][fli_size[v] - 1] < npm ||
		(c_len == ep2 && llen[v][fli_size[v] - 1] != npm)) {
		FlagV(v, 267, fli[v][fli_size[v] - 1]);
	}
	for (ls = 0; ls < fli_size[v] - 1; ++ls) {
		// Slurred note inside measure
		if (llen[v][ls] > nlen[v] && !sus[v][ls] && cc[v][fli[v][ls]]) FlagV(v, 160, fli[v][ls]);
	}
	return 0;
}

// Fail rhythm for species 3
int CP2R::FailRhythm3() {
	// Check uneven pause
	if (fli_size[v] > 2 && !cc[v][0] && llen[v][0] % npm != llen[v][1]) FlagV(v, 237, 0);
	// Last measure not whole
	if (c_len - fli[v][fli_size[v] - 1] < npm ||
		(c_len == ep2 && llen[v][fli_size[v] - 1] != npm)) {
		FlagV(v, 267, fli[v][fli_size[v] - 1]);
		if (c_len - fli[v][fli_size[v] - 1] == 2) FlagV(v, 252, fli[v][fli_size[v] - 1]);
	}
	return 0;
}

// Fail rhythm for species 5
int CP2R::FailRhythm5() {
	// Rhythm id
	rh_id[v].resize(mli.size());
	int rid_cur = 0;
	// Pause rhythm id
	rh_pid[v].resize(mli.size());
	int pid_cur = 0;
	int count8;
	// Note lengths inside measure
	vector<int> l_len;
	vector<int> l_ls;
	l_len.resize(npm);
	l_ls.resize(npm);
	int s3;
	// Measure size in notes
	int m_size = 0;
	// Slurs at start and finish of measure (show length of slurred notes)
	int slur1 = 0;
	int slur2 = 0;
	// Full measure collected
	int full_measure = 0;
	// Position inside measure
	int pos = 0;
	int uneven_start_fired = 0;
	// Starting step of measure
	int mstart = 0;
	// Length sum
	int suml = 0;
	int ls2 = 0;
	for (ms = 0; ms < mli.size(); ++ms) {
		s = mli[ms];
		if (s >= ep2) break;
		ls = bli[v][s];
		l_len.clear();
		l_ls.clear();
		slur1 = 0;
		slur2 = 0;
		pos = 0;
		mstart = 0;
		// Build note lengths
		full_measure = 0;
		int has_croche = 0;
		for (ls2 = ls; ls2 < fli_size[v]; ++ls2) {
			pos = max(0, fli[v][ls2] - s);
			// Do not process last note if not full melody generated
			if (ep2 != c_len && ls2 == fli_size[v] - 1) {
				// Last measure without whole note
				if (ms == mli.size() - 1 && l_len.size() && cc[v][fli[v][ls2]]) FlagV(v, 267, fli[v][fli_size[v] - 1]);
				// Whole inside if it starts not from first measure, from first step and is not a suspension
				//if (llen[v][ls2] >= 8 && ms && !pos && !sus[v][ls2] && cc[v][fli[v][ls2]]) FlagV(v, 160, s);
				// 1/8 syncope
				else if (llen[v][ls2] > 1 && pos % 2) FlagV(v, 232, fli[v][ls2]);
				// 1/4 syncope (not last, because it is flagged in suspension)
				else if (llen[v][ls2] > 2 && pos == 2 && cc[v][fli[v][ls2]]) FlagV(v, 235, fli[v][ls2]);
				full_measure = 0;
				break;
			}
			s2 = fli[v][ls2];
			l_len.push_back(llen[v][ls2]);
			l_ls.push_back(ls2);
			if (llen[v][ls2] == 1) has_croche = 1;
			// Stop if out of measure
			if (mstart + fli2[v][ls2] - s + 1 >= npm) {
				full_measure = 1;
				break;
			}
			pos += l_len.back();
		}
		// Check if there is nothing to analyze
		if (!l_len.size()) continue;
		// First note in measure with slur
		if (fli[v][ls] < s) {
			l_len[0] = min(8, (fli2[v][ls] - s + 1));
			slur1 = s - fli[v][ls];
		}
		// Last note in measure with slur
		if (full_measure && sus[v][ls2] && fli[v][ls2] >= s) {
			l_len[l_len.size() - 1] = min(8, sus[v][ls2] - s2);
			slur2 = fli2[v][ls2] - sus[v][ls2] + 1;
		}
		// Full evaluation?
		if (ep2 == c_len) {
			// Last measure
			if (ms == mli.size() - 1) {
				// Check last whole note
				if (l_len[0] != 8)
					FlagV(v, 267, fli[v][fli_size[v] - 1]);
			}
		}
		// Set first rhythm id bit
		rid_cur = slur1 ? 0 : 1;
		pid_cur = 0;
		// Iterative rhythm checks
		count8 = 0;
		pos = 0;
		for (int lp = 0; lp < l_len.size(); ++lp) {
			s2 = s + pos;
			ls2 = bli[v][s2];
			// Last note
			if (ep2 == c_len && ls2 == fli_size[v] - 1 && ms == mli.size() - 1) {
				// Check length
				if (l_len[lp] == 1) FlagV(v, 253, fli[v][fli_size[v] - 1]);
				else if (l_len[lp] == 2) FlagV(v, 252, fli[v][fli_size[v] - 1]);
			}
			// Calculate rhythm id
			if (lp < l_len.size() - 1 || !slur2)
				rid_cur += 1 << (pos + l_len[lp]);
			if (!cc[v][s2]) {
				pid_cur += 1 << (pos + l_len[lp]);
			}
			// Check 1/8
			if (l_len[lp] == 1) {
				// Last 1/8 syncope
				if (pos == 7 && slur2) FlagV(v, 232, s2);
				// Other types of 1/8
				else {
					// If second 1/8
					if (pos % 2) {
						// Isolated 1/8
						if (l_len[lp - 1] != 1) FlagV(v, 231, s2);
					}
					// Too many 1/8
					++count8;
					if (count8 == 3) FlagV(v, 255, s2);
					else if (count8 > 3) ++fpenalty;
					// 1/8 in first measure
					if (ms == 0) FlagV(v, 230, s2);
					// If first 8th
					else {
						// 1/8 beats
						if (pos == 0) FlagV(v, 226, s2);
						else if (pos == 2) FlagV(v, 227, s2);
						else if (pos == 4) FlagV(v, 228, s2);
						else if (pos == 6) FlagV(v, 229, s2);
					}
				}
				// 1/8 on leap
				if (ls2 < fli_size[v] - 1 && leap[v][s2])
					FlagV(v, 88, s2);
				else if (ls2 > 0 && leap[v][s2 - 1]) {
					if (llen[v][ls2 - 1] > 1) FlagV(v, 88, ssus[v][bli[v][s2 - 1]]);
				}
			}
			else {
				// 1/8 syncope
				if (pos % 2) FlagV(v, 232, s2);
				// 1/4 syncope
				else if (l_len[lp] > 2 && pos == 2 && cc[v][s2]) FlagV(v, 235, s2);
			}
			// Uneven starting rhythm
			if (!ms && lp > 0 && l_len[lp] != l_len[lp - 1] && !uneven_start_fired) {
				// Check for exception: (pause + 1/4 + 1/2 slurred)
				if (!cc[v][0] && llen[v][0] == 2 && lp == 2 && l_len[lp] >= 4 && l_len[lp - 1] == 2 && slur2) {}
				else {
					uneven_start_fired = 1;
					FlagVL(v, 254, s2, fin[v]);
				}
			}
			pos += l_len[lp];
		}
		// Check rhythm repeat
		if (full_measure) {
			// Check only if no croches or less than 4 notes
			if (ms > 0 && (!has_croche || l_len.size() <4)) {
				// Fire if rhythm and pauses rhythm matches and there is no full measure pause
				if (rh_id[v][ms - 1] == rid_cur && rh_pid[v][ms - 1] == pid_cur &&
					(l_len[0] < 8 || cc[v][s]))
					FlagVL(v, 247, s, fli[v][bli[v][s + npm - 1]]);
			}
			rh_id[v][ms] = rid_cur;
			rh_pid[v][ms] = pid_cur;
		}
		// Check rhythm rules
		// First measure
		if (!ms) {
			// Uneven pause
			if (l_len.size() > 1 && l_len[0] == fin[v] && l_len[0] != l_len[1]) FlagV(v, 237, s);
		}
		// Whole inside
		//if (l_len[0] >= 8 && ms < mli.size() - 1 && ms && cc[v][s]) FlagV(v, 160, s);
		// 1/2.
		else if (l_len[0] == 6 && !slur1 && cc[v][s]) FlagV(v, 233, s);
		else if (l_len.size() > 1 && l_len[1] == 6 && cc[v][fli[v][l_ls[1]]]) 
			FlagVL(v, 234, fli[v][l_ls[1]], fli[v][l_ls[0]]);
		else if (l_len.size() > 2 && l_len[2] == 6 && cc[v][fli[v][l_ls[1]]]) 
			FlagVL(v, 234, fli[v][l_ls[2]], fli[v][l_ls[0]]);
		// Many notes in measure
		if (l_len.size() == 5) {
			if (slur1) FlagV(v, 301, s);
			else FlagVL(v, 245, s, fli[v][bli[v][s + npm - 1]]);
		}
		else if (l_len.size() > 5) FlagVL(v, 246, s, fli[v][bli[v][s + npm - 1]]);
		// Suspensions
		if (cc[v][s]) {
			if (slur1 == 4 && l_len[0] == 2) FlagV(v, 241, s);
			else if (slur1 == 4 && l_len[0] == 4) FlagV(v, 242, s);
			//else if (slur1 == 2) FlagV(v, 251, s)
			if (slur1 && l_len[0] == 6) FlagV(v, 243, s);
			if (slur1 == 6) FlagV(v, 244, s);
		}
	}
	return 0;
}

int CP2R::FailRhythmRepeat() {
	CHECK_READY(DR_fli, DR_beat, DR_sus);
	CHECK_READY(DR_leap, DR_nlen);
	for (v = 0; v < av_cnt; ++v) {
		// Only for sp5
		if (vsp[v] != 5) continue;
		for (v2 = v + 1; v2 < av_cnt; ++v2) {
			// Only for sp5
			if (vsp[v2] != 5) continue;
			for (ms = 1; ms < mli.size() - 1; ++ms) {
				s = mli[ms];
				if (s >= ep2) break;
				ls = bli[v][s];
				//if (ms >= rh_id[v].size() || ms >= rh_id[v2].size()) continue;
				// Fire if rhythm and pauses rhythm matches and there is no whole-measure pause
				if (rh_id[v][ms] == rh_id[v2][ms] && rh_pid[v][ms] == rh_pid[v2][ms] &&
					(fli2[v][ls] < s + npm - 1 || cc[v][s]))
					FlagL(v, 549, s, fli[v][bli[v][s + npm - 1]], v2);
			}
		}
	}
	return 0;
}

void CP2R::FlagFullParallel() {
	CHECK_READY(DR_fli);
	// Skip if not all voices are in whole notes
	for (v = 0; v < av_cnt; ++v) {
		if (vsp[v] > 1) return;
	}
	int fps = 0;
	for (ls = 0; ls < fli_size[0]; ++ls) {
		s = fli[0][ls];
		int fp = 1;
		for (v = 0; v < av_cnt; ++v) {
			// Wrong note lengths
			if (ls >= fli_size[v] || llen[v][ls] != npm) {
				fp = 0;
				break;
			}
			// Non-parallel motion
			else if (v && ls && c[v][s] - c[v][s - 1] != c[0][s] - c[0][s - 1]) {
				fp = 0;
				break;
			}
		}
		if (ls && fp) ++fps;
		else fps = 0;
		if (fps == 2) {
			FlagVL(0, 550, s, fli[0][ls - 2]);
		}
	}
}

void CP2R::FlagMultiSlur() {
	CHECK_READY(DR_fli);
	for (ms = 1; ms < mli.size(); ++ms) {
		s = mli[ms];
		int scount = 0;
		for (v = 0; v < av_cnt; ++v) {
			// Skip species except CF and 1
			if (vsp[v] > 1) continue;
			// Detect slur or note repeat
			if (cc[v][s] == cc[v][s - 1]) {
				++scount;
				if (scount == 1) v2 = v;
				else break;
			}
		}
		if (scount > 1) {
			FlagL(v, 557, s, s, v2);
		}
	}
}

int CP2R::FailAnapaest() {
	CHECK_READY(DR_fli);
	// Check only for 4/4
	if (npm != 8) return 0;
	// Do not run check if there are no sp5 voices
	int sp5_count = 0;
	for (v = 0; v < av_cnt; ++v) {
		if (vsp[v] == 5) ++sp5_count;
	}
	if (sp5_count < 1) return 0;
	for (ms = 0; ms < mli.size(); ++ms) {
		// Skip penultimate measure
		if (ms == mli.size() - 2) continue;
		s0 = mli[ms];
		// Detect note start at beat 4
		int start4 = -1;
		for (v = 0; v < av_cnt; ++v) {
			if (fli[v][bli[v][s0 + 6]] == s0 + 6) {
				start4 = v;
				break;
			}
		}
		if (start4 != -1) continue;
		// Detect note start at beat 2 if there is no beat 4
		for (v = 0; v < av_cnt; ++v) {
			if (fli[v][bli[v][s0 + 2]] == s0 + 2) {
				// If this is the only sp5 voice, check if it ends with slur
				if (sp5_count == 1 && sus[v][bli[v][s0 + npm - 1]]) {
					FlagV(v, 239, s0 + 2);
				}
				else FlagV(v, 240, s0 + 2);
				break;
			}
		}
	}
	return 0;
}

void CP2R::FlagRhythmStagnation() {
	CHECK_READY(DR_fli);
	// Do not run check if there are no sp5 voices
	int sp5_count = 0;
	for (v = 0; v < av_cnt; ++v) {
		if (vsp[v] == 5) {
			++sp5_count;
			v2 = v;
		}
	}
	if (sp5_count < 1) return;
	// Skip first and last measure
	for (ms = 1; ms < mli.size() - 1; ++ms) {
		s0 = mli[ms];
		// Detect note start inside measure
		int plain_measure = 1;
		for (v = 0; v < av_cnt; ++v) {
			if (fli2[v][bli[v][s0]] < s0 + npm - 1) {
				plain_measure = 0;
				break;
			}
		}
		if (!plain_measure) continue;
		// Send flag to any voice in species 5
		FlagV(v2, 236, s0);
	}
}

// Detect missing slurs
int CP2R::FailMissSlurs() {
	// Check only for species 4
	if (sp != 4) return 0;
	// Missing slurs array
	vector<int> msa;
	msa.resize(mli.size());
	// Number of missing slurs in window
	int scount = 0;
	int miss, max_miss = 0;
	int max_ls = 0;
	for (ms = 1; ms < mli.size() - 1; ++ms) {
		s = mli[ms];
		ls = bli[v][s];
		s2 = fli[v][ls];
		// Subtract old missing slur
		if (ms >= miss_slurs_window[sp][av_cnt][0] && msa[ms - miss_slurs_window[sp][av_cnt][0]])
			--scount;
		// Detect missing slur
		if (s == s2) {
			++scount;
			msa[ms] = 1;
			if (scount > max_miss) {
				max_miss = scount;
				max_ls = ls;
			}
		}
	}
	if (max_miss == 1) FlagV(v, 188, fli[v][max_ls]);
	else if (max_miss == 2) FlagV(v, 189, fli[v][max_ls]);
	else if (max_miss > 2) {
		FlagV(v, 190, fli[v][max_ls]);
		if (!accept[sp][av_cnt][0][190]) fpenalty += (max_miss - 2) * 50;
	}
	return 0;
}

// Detect many slurs
int CP2R::FailSlurs() {
	CHECK_READY(DR_fli, DR_nlen);
	// For species 5 there are separate rules (FailRhythm5)
	// For species 4 we can have all notes slurred
	if (sp >= 4) return 0;
	// Current window size
	int wsize = 0;
	// Number of slurs in window
	int scount = 0;
	int cnt, max_count = 0;
	int max_ls = 0;
	int sw = slurs_window[sp][av_cnt][0];
	for (ls = 0; ls < fli_size[v] - 1; ++ls) {
		if (!ls && !cc[v][0]) continue;
		// Subtract old slur or repeat
		if (ls >= sw && 
			(llen[v][ls - sw] > nlen[v] || 
				cc[v][fli2[v][ls - sw]] == cc[v][fli2[v][ls - sw] + 1]))
			--scount;
		// Check slurs and repeats in window
		if (llen[v][ls] > nlen[v] || cc[v][fli2[v][ls]] == cc[v][fli2[v][ls] + 1]) {
			++scount;
			if (scount > max_count) {
				max_count = scount;
				max_ls = ls;
			}
		}
	}
	if (max_count == 1) FlagV(v, 93, fli[v][max_ls]);
	else if (max_count == 2) 
		FlagV(v, 94, fli[v][max_ls]);
	else if (max_count > 2) {
		FlagV(v, 95, fli[v][max_ls]);
		if (!accept[sp][av_cnt][0][95]) fpenalty += (max_count - 2) * 50;
	}
	return 0;
}

int CP2R::FailMaxNoteLen() {
	CHECK_READY(DR_fli);
	/*
	// Never check last note, either end of scan window or end of counterpoint
	for (ls = 0; ls < fli_size[v] - 1; ++ls) {
	if (rlen[v][ls] > max_note_len[sp][av_cnt][0] * 2)
	FlagV(v, 336, fli[v][ls]);
	// Check notes crossing multiple measures
	if (bmli[fli2[v][ls]] - bmli[fli[v][ls]] > 1) FlagV(v, 41, fli[v][ls]);
	}
	*/
	return 0;
}

// Detect sus mistakes: short preparation and sus sounding with res 
// TODO: need to implement
void CP2R::FlagSus() {
	for (v = 0; v < av_cnt; ++v) {
		sp = vsp[v];
		for (ls = 0; ls < fli_size[v]; ++ls) {
			s = fli[v][ls];
			s2 = fli2[v][ls];
			hs = bhli[s2];
			hstart = hli[hs];
			// Skip pauses
			if (!cc[v][s]) continue;
			// No intermeasure or intrameasure suspension
			if (hstart <= s) continue;
			hend = hli2[hs];
			// Preparation is shorter then suspension
			if ((hstart - s) * 2 < llen[v][ls])
				FlagV(v, 427, s);
			// Preparation is too short for measure size
			if (npm == 12 && btype == 4) {
				if (hstart - s < 6)
					FlagV(v, 274, s);
			}
			else if (npm == 4 || npm == 6) {
				if (hstart - s < 2)
					FlagV(v, 274, s);
			}
			// Skip resolution analysis if it does not exist
			if (!resol[v][hstart]) continue;
			// Get suspension resolution pitch class
			int lpcc = pcc[v][resol[v][hstart]];
			// Scan all voices from last harmony start to sus end and find simultaneous resolution note
			for (v2 = 0; v2 < av_cnt; ++v2) {
				// Skip same voice
				if (v2 == v) continue;
				for (s3 = hstart; s3 <= s2; ++s3) {
					// Skip wrong pcñ
					if (pcc[v2][s3] != lpcc) continue;
					// Skip pauses
					if (!cc[v2][s3]) continue;
					ls2 = bli[v2][s3];
					// Skip if not first step of harmony and not new note
					if (s3 > hstart && fli[v2][ls2] < s3) continue;
					civl = cc[v][s3] - cc[v2][s3];
					// Interval is 2nd
					if (abs(civl) == 1 || abs(civl) == 2) {
						Flag(v2, 218, s3, v);
						continue;
					}
					// Above sus
					if (civl < 0) {
						Flag(v2, 216, s3, v);
						continue;
					}
					// Sus resolution is in bass
					if (!v2) continue;
					// Less than 4 voices
					if (vca[s3] < 4) {
						Flag(v2, 217, s3, v);
						continue;
					}
					// Find chord tone between sus and susres
					int found = 0;
					for (v3 = v2 + 1; v3 < v; ++v3) {
						s4 = max(hstart, fli[v3][bli[v3][s3]]);
						if (nih[v3][s4]) {
							found = 1;
							break;
						}
					}
					if (!found) {
						Flag(v2, 217, s3, v);
					}
				}
			}
		}
	}
}

void CP2R::FlagSus2() {
	CHECK_READY(DR_islt);
	for (v = 0; v < av_cnt; ++v) {
		// Species 2
		if (vsp[v] != 2) continue;
		for (ls = 0; ls < fli_size[v]; ++ls) {
			s = fli[v][ls];
			// Skip not sus
			if (!sus[v][ls]) continue;
			// Skip pause
			if (!cc[v][s]) continue;
			// I -> LT penultimate
			if (bmli[sus[v][ls]] == mli.size() - 2 && ls < fli_size[v] - 1 &&
				pcc[v][sus[v][ls]] == 0 && 
				islt[v][fli[v][ls + 1]]) FlagV(v, 387, sus[v][ls]);
			// Other
			else FlagV(v, 388, sus[v][ls]);
		}
	}
}

void CP2R::GetLT() {
	SET_READY(DR_islt, DR_nih);
	for (v = 0; v < av_cnt; ++v) {
		for (ls = 0; ls < fli_size[v]; ++ls) {
			s = fli[v][ls];
			islt[v][s] = 0;
			// Set lt only for major and melodic minor
			if (mode && !mminor) continue;
			// Skip pause
			if (!cc[v][s]) continue;
			if (pcc[v][s] == 11 && nih[v][s]) {
				hs = bhli[s];
				// Last harmony
				if (hs >= hli.size() - 1) continue;
				// D or DVII harmony
				if (chm[hs] == 4 || chm[hs] == 6) islt[v][s] = 1;
				else {
					// DTIII followed by T chord
					if (chm[hs] == 2 && chm[hs + 1] == 0) islt[v][s] = 1;
				}
			}
		}
	}
}

int CP2R::FailSusCount() {
	CHECK_READY(DR_sus);
	int c_sus = 0;
	int c_anti = 0;
	for (ls = 0; ls < fli_size[v]; ++ls) {
		if (sus[v][ls]) {
			if (retr[v][sus[v][ls]]) ++c_anti;
			else ++c_sus;
		}
	}
	int mcount = bmli[ep2 - 1];
	// Do not check for first measure
	if (!mcount) return 0;
	// Check for not enough sus
	if ((c_sus + c_anti + 1) * 1.0 / mcount < 1.0 / mea_per_sus[sp][av_cnt][0])
		FlagV(v, 341, 0);
	return 0;
}

// Detect repeating notes. Step2 excluding
int CP2R::FailNoteRepeat() {
	for (ls = 0; ls < fli_size[v] - 1; ++ls) {
		if (cc[v][fli[v][ls]] == cc[v][fli[v][ls + 1]]) FlagV(v, 30, fli[v][ls]);
	}
	return 0;
}

// Detect pauses
int CP2R::FailPauses() {
	for (ls = 1; ls < fli_size[v]; ++ls) {
		if (!cc[v][fli[v][ls]]) FlagV(v, 517, fli[v][ls]);
	}
	return 0;
}

void CP2R::GetNoteLen() {
	SET_READY(DR_nlen);
	for (v = 0; v < av_cnt; ++v) {
		sp = vsp[v];
		nlen[v] = sp_nlen[sp];
		if (sp == 2 || sp == 4) {
			if (npm == 6 || (npm == 12 && btype == 2)) {
				nlen[v] = npm / 3;
			}
			else {
				nlen[v] = npm / 2;
			}
		}
	}
}

// Detect repeating notes. Step2 excluding
int CP2R::FailNoteLen() {
	CHECK_READY(DR_nlen);
	// Get min sus length
	min_sus = 6;
	//if (npm == 8) min_sus = 6;
	if (npm == 12) {
		if (btype == 2) min_sus = 6;
		if (btype == 4) min_sus = 8;
	}
	else if (npm <= 6) min_sus = 4;
	if (sp == 0) {
		if (av_cnt == 1) return 0;
		for (ls = 0; ls < fli_size[v]; ++ls) {
			s = fli[v][ls];
			if (!cc[v][s]) continue;
			if (llen[v][ls] == npm) continue;
			FlagV(v, 514, s);
		}
	}
	else if (sp == 1) {
		for (ls = 0; ls < fli_size[v]; ++ls) {
			s = fli[v][ls];
			if (!cc[v][s]) continue;
			if (llen[v][ls] == npm) continue;
			if (llen[v][ls] == npm * 2) continue;
			FlagV(v, 514, s);
		}
	}
	else if (sp == 2 || sp == 4) {
		for (ls = 0; ls < fli_size[v]; ++ls) {
			s = fli[v][ls];
			if (!cc[v][s]) continue;
			// Last note - do not check length, it is checked in FailRhythm
			if (ls == fli_size[v] - 1) continue;
			if (llen[v][ls] == nlen[v]) continue;
			if (llen[v][ls] == nlen[v] * 2) continue;
			FlagV(v, 514, s);
		}
	}
	else if (sp == 3) {
		for (ls = 0; ls < fli_size[v]; ++ls) {
			s = fli[v][ls];
			if (!cc[v][s]) continue;
			// Last note - do not check length, it is checked in FailRhythm
			if (ls == fli_size[v] - 1) continue;
			if (llen[v][ls] == 2) continue;
			FlagV(v, 514, s);
		}
	}
	else if (sp == 5) {
		for (ls = 0; ls < fli_size[v]; ++ls) {
			s = fli[v][ls];
			if (!cc[v][s]) continue;
			if (llen[v][ls] == 1) continue;
			if (llen[v][ls] == 2) continue;
			if (llen[v][ls] == 4) continue;
			if (llen[v][ls] == 6) continue;
			if (llen[v][ls] == 8) continue;
			if (llen[v][ls] == 12) continue;
			FlagV(v, 514, s);
		}
	}
	return 0;
}

// Detect repeating notes. Step2 excluding
int CP2R::FailBeat() {
	CHECK_READY(DR_nlen);
	if (sp == 0) {
		if (av_cnt == 1) return 0;
		for (ls = 0; ls < fli_size[v]; ++ls) {
			if (!beat[v][ls]) continue;
			s = fli[v][ls];
			FlagV(v, 515, s);
		}
	}
	else if (sp == 1) {
		for (ls = 0; ls < fli_size[v]; ++ls) {
			if (!beat[v][ls]) continue;
			s = fli[v][ls];
			FlagV(v, 515, s);
		}
	}
	else if (sp == 2 || sp == 4) {
		for (ls = 0; ls < fli_size[v]; ++ls) {
			s = fli[v][ls];
			if (beat[v][ls] < 4) continue;
			FlagV(v, 515, s);
		}
	}
	else if (sp == 3) {
		for (ls = 0; ls < fli_size[v]; ++ls) {
			if (beat[v][ls] < 10) continue;
			s = fli[v][ls];
			FlagV(v, 515, s);
		}
	}
	return 0;
}

// Detect retriggers inside measure
int CP2R::FailRetrInside() {
	for (ls = 0; ls < fli_size[v]; ++ls) {
		if (!beat[v][ls]) continue;
		s = fli[v][ls];
		if (retr[v][s]) {
			FlagV(v, 516, s);
		}
	}
	return 0;
}

// Take vector of diatonic notes and detect most possible chord
void CP2R::GetHarm(int found_gis, int found_fis, int &lchm, int &lchm_alter, int &rating) {
	rating = -10000;
	for (int x = 0; x < 7; ++x) {
		// No root note
		if (!chn[x]) continue;
		int rat = 0;
		// Each chord note adds to rating
		rat += chn[x] + chn[(x + 2) % 7] ? 1 : 0 + 
			chn[(x + 4) % 7] ? 1 : 0;
		// VII note means 7th chord
		if (chn[(x + 6) % 7]) rat -= 10;
		// VI note means other chord
		if (chn[(x + 5) % 7]) rat -= 100;
		// IV note means other chord
		if (chn[(x + 3) % 7]) rat -= 100;
		// II note means other chord
		if (chn[(x + 1) % 7]) rat -= 100;
		if (rat > rating) {
			lchm = x;
			rating = rat;
		}
	}
	if (found_gis == -100) return;
	lchm_alter = 0;
	if (mminor) {
		// Check chords with G or G#
		if (lchm && lchm % 2 == 0) {
			if (cchn[11]) lchm_alter = 1;
			else if (cchn[10]) lchm_alter = -1;
			else if (found_gis == 1) lchm_alter = 1;
			else if (found_gis == -1) lchm_alter = -1;
		}
		// Check chords with F or F#
		else if (lchm % 2) {
			if (cchn[9]) lchm_alter = 1;
			else if (cchn[8]) lchm_alter = -1;
			else if (found_fis == 1) lchm_alter = 1;
			else if (found_fis == -1) lchm_alter = -1;
		}
	}
}

int CP2R::FailHarm() {
	CHECK_READY(DR_fli, DR_c, DR_pc);
	SET_READY(DR_hli);
	//if (av_cnt < 2) return 0;
	int n, hcount, rat;
	int last_b; // First harmony in measure has b
	int found_gis, found_fis;
	int mea_end;
	hli.clear();
	hli2.clear();
	ha64.clear();
	chm.clear();
	hbcc.clear();
	hbc.clear();
	chm_alter.clear();
	// Build chm vector
	for (ms = 0; ms < mli.size(); ++ms) {
		// Stop processing when last measure is not fully generated
		if (ms == mli.size() - 1 && ep2 < c_len) break;
		// Get last measure step
		mea_end = mli[ms] + npm - 1;
		// Prevent going out of window
		if (mea_end >= ep2) break;
		// Clear harmonic notes vector
		fill(chn.begin(), chn.end(), 0);
		fill(cchn.begin(), cchn.end(), 0);
		hli.push_back(mli[ms]);
		hli2.push_back(0);
		hs = hli.size() - 1;
		if (hli2.size() > 1) hli2[hli2.size() - 2] = hli[hli.size() - 1] - 1;
		ha64.push_back(0);
		// Set harmony bass to random
		hbcc.push_back(127);
		hbc.push_back(0);
		chm.push_back(0);
		chm_alter.push_back(0);
		found_gis = 0;
		found_fis = 0;
		hcount = 0;
		int bad_harm = 0;
		// Loop inside measure
		for (s = mli[ms]; s <= mea_end; ++s) {
			for (v = 0; v < av_cnt; ++v) {
				ls = bli[v][s];
				// Skip if not note start or sus
				if (fli[v][ls] != s && hli.back() != s) continue;
				// Skip pauses
				if (!cc[v][s]) continue;
				sp = vsp[v];
				// At this point all notes are checked for alterations
				if (pcc[v][s] == 11) found_gis = 1;
				else if (pcc[v][s] == 10) found_gis = -1;
				else if (pcc[v][s] == 9) found_fis = 1;
				else if (pcc[v][s] == 8) found_fis = -1;
				// For first suspension in harmony, check harmony start msh
				if (fli[v][ls] <= hli.back() && s == hli.back()) {
					// If first suspension in harmony is non-harmonic, do not add it to harmony
					if (msh[v][hli.back()] < 0) continue;
				}
				else {
					// For all other notes, check msh at position
					if (msh[v][s] <= 0) continue;
				}
				// Pitch class
				n = pc[v][s];
				// Find harmonic conflict
				if (s > mli[ms] && (chn[(n + 1) % 7] || chn[(n + 6) % 7] ||
					(chn[n] && !cchn[pcc[v][s]]))) {
					// Remove notes of current step from chord, because this step belongs to next chord
					for (v2 = 0; v2 < v; ++v2) {
						ls = bli[v2][s];
						// Skip if not note start 
						if (fli[v2][ls] != s) continue;
						// Skip pauses
						if (!cc[v2][s]) continue;
						// Check msh
						if (msh[v2][s] <= 0) continue;
						// Remove note
						--chn[pc[v2][s]];
						--cchn[pcc[v][s]];
					}
					// More than two harmonies
					if (hcount) {
						FlagVL(0, 40, s, mli[ms]);
						chm[hs] = -1;
						chm_alter[hs] = -1;
						RemoveHarmDuplicate();
						bad_harm = 1;
						break;
					}
					else {
						// Two harmonies penultimate
						if (ms == mli.size() - 2) {
							FlagVL(0, 306, s, mli[ms]);
							// Prohibit wrong second harmony position
							int dist = s - mli[ms];
							if (npm == 8) {
								if (dist != 4 && dist != 6) FlagV(0, 556, s);
							}
							else if (npm == 4) {
								if (dist != 2) FlagV(0, 556, s);
							}
							else if (npm == 6) {
								if (dist != 4) FlagV(0, 556, s);
							}
							else if (npm == 10) {
								if (dist != 6) FlagV(0, 556, s);
							}
							else if (npm == 12) {
								if (btype == 4) {
									if (dist != 6) FlagV(0, 556, s);
								}
								else {
									if (dist != 8) FlagV(0, 556, s);
								}
							}
						}
						else {
							/*
							// Stepwize resolution of 5th to 6th or 6th to 5th with two harmonies in measure
							if ((sp == 4 ||
							(sp == 5 && sus[ls1] && fli2[ls1] - sus[ls1] == 3 && rlen[ls1 + 1] >= 4)) && (
							(ivlc[mli[ms]] == 4 && ivlc[s] == 5) ||
							(ivlc[mli[ms]] == 5 && ivlc[s] == 4)) &&
							abs(ac[cpv][mli[ms]] - ac[cpv][s]) < 2)
							FLAG2L(329, s, mli[ms]);
							else FLAG2L(307, s, mli[ms]);
							*/
							FlagVL(0, 307, s, mli[ms]);
							chm[hs] = -1;
							chm_alter[hs] = 0;
							RemoveHarmDuplicate();
							bad_harm = 1;
							break;
						}
					}
					GetHarm(found_gis, found_fis, chm[hs], chm_alter[hs], rat);
					if (rat < 0) {
						chm[hs] = -1;
						chm_alter[hs] = 0;
					}
					RemoveHarmDuplicate();
					fill(chn.begin(), chn.end(), 0);
					fill(cchn.begin(), cchn.end(), 0);
					hli.push_back(s);
					hli2.push_back(0);
					hs = hli.size() - 1;
					if (hli2.size() > 1) hli2[hli2.size() - 2] = hli[hli.size() - 1] - 1;
					chm.push_back(0);
					ha64.push_back(0);
					hbcc.push_back(127);
					hbc.push_back(0);
					found_gis = 0;
					found_fis = 0;
					chm_alter.push_back(0);
					// Reinitialize chord notes for new chord
					for (v2 = 0; v2 < av_cnt; ++v2) {
						ls = bli[v2][s];
						// Skip pauses
						if (!cc[v2][s]) continue;
						// For first suspension in harmony, check harmony start msh
						if (fli[v2][ls] < s) {
							// If first suspension in harmony is non-harmonic, do not add it to harmony
							if (msh[v2][s] < 0) continue;
						}
						else {
							// For all other notes, check msh at position
							if (msh[v2][s] <= 0) continue;
						}
						// Record note
						++chn[pc[v2][s]];
						++cchn[pcc[v2][s]];
					}
					// Next harmony counter
					++hcount;
				}
				// Record note
				++chn[n];
				++cchn[pcc[v][s]];
			}
			if (bad_harm) break;
		}
		if (!bad_harm) {
			GetHarm(found_gis, found_fis, chm[hs], chm_alter[hs], rat);
			if (rat < 0) {
				chm[hs] = -1;
				chm_alter[hs] = 0;
			}
			RemoveHarmDuplicate();
		}
		// If penultimate measure
		if (ms == mli.size() - 2 && hcount) {
			// Prohibit D or DVII harmony in penultimate measure before non-D / DVII harmony
			if (chm.size() > 1 && (chm[chm.size() - 2] == 4 || chm[chm.size() - 2] == 6) &&
				(chm[chm.size() - 1] > -1 && chm[chm.size() - 1] != 4 && chm[chm.size() - 1] != 6)) FlagV(0, 322, hli[chm.size() - 1]);
		}
		if (hli2.size()) hli2[hli2.size() - 1] = mea_end;
	}
	GetBhli();
	// Check penultimate harmony not D / DVII
	if (ep2 == c_len && hli.size() > 1) {
		hs = hli.size() - 2;
		if (chm[hs] > -1 && chm[hs] != 4 && chm[hs] != 6) FlagV(0, 335, hli[hs]);
	}
	GetHarmBass();
	// Check first harmony not T
	if (chm.size() && chm[0] > -1 && (chm[0] || hbc[0] % 7)) {
		FlagV(0, 137, hli[0]);
	}
	// Check last harmony not T
	if (chm.size() && (chm[chm.size() - 1] > 0 || hbc[chm.size() - 1] % 7)) {
		FlagV(0, 531, hli[chm.size() - 1]);
	}
	if (EvalHarm()) return 1;
	GetChordTones();
	if (FailTonicCP()) return 1;
	GetLT();
	return 0;
}

int CP2R::EvalHarm() {
	int pen1;
	int p2c = 0; // Count of consecutive penalty 2
	int p3c = 0; // Count of consecutive penalty 3
	int dcount = 0;
	int scount = 0;
	int tcount = 0;
	int wdcount = 0;
	int wscount = 0;
	int wtcount = 0;
	int harm_end, found;
	v = 0;
	for (int i = 0; i < chm.size(); ++i) {
		s = hli[i];
		ls = bli[v][s];
		// Skip wrong harmony
		if (chm[i] == -1) {
			FlagV(v, 555, s);
			continue;
		}
		// Prohibit 64 chord
		if ((hbc[i] % 7 - chm[i] + 7) % 7 == 4) {
			FlagV(v, 433, s);
		}
		// Prohibit audible 64 chord
		else if (ha64[i] == 1) FlagV(v, 196, s);
		// Prohibit audible 64 chord
		else if (ha64[i] == 2) FlagV(v, 383, s);
		if (i > 0 && chm[i - 1] > -1) {
			// Check GC for low voice and not last note (last note in any window is ignored)
			if (ls < fli_size[v] - 1 &&
				chm[i] == 0 && chm[i - 1] == 4 &&
				pc[0][s] == 0 && pc[1][s] == 0 &&
				s > 0 && pc[0][s - 1] == 4) FlagV(v, 48, s);
			if (mminor) {
				// Prohibit VI<->VI# containing progression
				if (chm[i] % 2 && chm[i - 1] % 2 && chm_alter[i] * chm_alter[i - 1] == -1) {
					FlagV(v, 377, s);
				}
				// Prohibit VII<->VII# containing progression
				if (chm[i] && chm[i] % 2 == 0 && chm[i - 1] && chm[i - 1] % 2 == 0 &&
					chm_alter[i] * chm_alter[i - 1] == -1) {
					FlagV(v, 378, s);
				}
				// Prohibit dVII (GBD) in root position after S (DF#A) in root position
				if (chm[i] == 6 && chm[i - 1] == 3 && chm_alter[i]<1 && chm_alter[i - 1] == 1) {
					if (ls > 0 && pc[0][s] == 6 && pc[0][fli[v][ls - 1]] == 3) FlagV(v, 308, s);
				}
				// Prohibit DTIII (CEG) in root position after dVII (GBD) in root position
				if (chm[i] == 2 && chm[i - 1] == 6 && chm_alter[i]<1 && chm_alter[i - 1]<1) {
					if (ls > 0 && pc[0][s] == 2 && pc[0][fli[v][ls - 1]] == 6) FlagV(v, 309, s);
				}
			}
			// Check harmonic penalty	
			pen1 = hsp[chm[i - 1]][chm[i]];
			if (pen1 == 1) FlagVL(v, 77, s, hli[i - 1]);
			if (pen1 == 2) {
				++p2c;
				if (p2c == 1) FlagVL(v, 57, s, hli[i - 1]);
				else if (p2c == 2) FlagVL(v, 92, s, hli[i - 2]);
				else if (p2c == 3) FlagVL(v, 23, s, hli[i - 3]);
			}
			else if (pen1 < 2) {
				p2c = 0;
			}
			// Harmonic repeats
			if (pen1 == 3) {
				++p3c;
				if (p3c == 1) {
					if (hbc[i] % 7 == hbc[i - 1] % 7) FlagVL(v, 99, s, hli[i - 1]);
					else FlagVL(v, 418, s, hli[i - 1]);
				}
				else if (p3c == 2) FlagVL(v, 321, s, hli[i - 2]);
			}
			else {
				p3c = 0;
			}
		}
		// Check letter repeat and miss
		hrepeat_fired = 0;
		hmiss_fired = 0;
		if (FailHarmStep(i, hvt, tcount, wtcount, repeat_letters_t[sp][av_cnt][0], miss_letters_t[sp][av_cnt][0], 17, 20)) return 1;
		if (FailHarmStep(i, hvd, dcount, wdcount, repeat_letters_d[sp][av_cnt][0], miss_letters_d[sp][av_cnt][0], 428, 430)) return 1;
		if (FailHarmStep(i, hvs, scount, wscount, repeat_letters_s[sp][av_cnt][0], miss_letters_s[sp][av_cnt][0], 429, 431)) return 1;
	}
	return 0;
}

int CP2R::FailTonicCP() {
	CHECK_READY(DR_hbc);
	float tcount = 0;
	int fire, fired = 0;
	// Do not check if melody is short
	if (hli.size() < 3) return 0;
	// Loop from second to second to last note
	for (int hs = 1; hs < hli.size() - 1; ++hs) {
		s = hli[hs];
		// Decrement for previous tonic note
		if (hs > tonic_window_cp[sp][av_cnt][0]) {
			if (!chm[hs - tonic_window_cp[sp][av_cnt][0]]) --tcount;
		}
		if (!chm[hs]) {
			// Increment for current tonic note (depending on inversion)
			if (hbc[hs] % 7 == chm[hs]) ++tcount;
			else tcount += tonic_wei_inv[sp][av_cnt][0] / 100.0;
			// Check count of tonic notes
			if (tcount > tonic_max_cp[sp][av_cnt][0]) {
				// Grant one more tonic in first window if first chord not tonic
				fire = 0;
				if (hs < tonic_window_cp[sp][av_cnt][0] && chm[0]) {
					if (tcount > tonic_max_cp[sp][av_cnt][0] + 1)	fire = 1;
				}
				else fire = 1;
				if (fire) {
					if (fired) {
						fpenalty += severity[sp][av_cnt][0][310] + 1;
					}
					else {
						FlagV(0, 310, s);
						fired = 1;
					}
				}
			}
		}
	}
	return 0;
}

void CP2R::RemoveHarmDuplicate() {
	int chm_id = hli.size() - 1;
	// Need to be at least two harmonies
	if (chm_id == 0) return;
	// Harmony should be not first in measure
	if (hli[chm_id] <= mli[ms]) return;
	// Harmonies should match
	if (chm[chm_id] != chm[chm_id - 1]) return;
	// Alterations should match
	if (chm_alter[chm_id] * chm_alter[chm_id - 1] == -1) return;
	// Remove duplicate
	hli.resize(chm_id);
	hli2.resize(chm_id);
	chm.resize(chm_id);
	chm_alter.resize(chm_id);
	hbc.resize(chm_id);
	hbcc.resize(chm_id);
	ha64.resize(chm_id);
}

int CP2R::FailHarmStep(int i, const int* hv, int &count, int &wcount, int repeat_letters, int miss_letters, int flagr, int flagm) {
	if (hv[chm[i]]) {
		++count;
		wcount = 0;
	}
	else {
		++wcount;
		count = 0;
	}
	if (count > repeat_letters && !hrepeat_fired) {
		if (count == repeat_letters + 1) {
			FlagVL(0, flagr, s, hli[i - count + 1]);
			hrepeat_fired = 1;
		}
		else {
			fpenalty += severity[sp][av_cnt][0][flagr] + 1;
		}
	}
	if (wcount > miss_letters && !hmiss_fired) {
		if (wcount == miss_letters + 1) {
			FlagVL(0, flagm, s, hli[i - wcount + 1]);
			hmiss_fired = 1;
		}
		else {
			fpenalty += severity[sp][av_cnt][0][flagm] + 1;
		}
	}
	return 0;
}

void CP2R::GetBhli() {
	fill(bhli.begin(), bhli.end(), 0);
	for (int hs = 0; hs < chm.size(); ++hs) {
		for (s = hli[hs]; s <= hli2[hs]; ++s) {
			bhli[s] = hs;
		}
	}
}

void CP2R::EvalMshHarm(int hvar) {
	// Get harmonic notes
	int de1 = hvar;
	int de2 = (de1 + 2) % 7;
	int de3 = (de1 + 4) % 7;
	// Init lhbcc - lowest harmonic note in bass
	int lhbc = 1000;
	for (ls = bli[0][hstart]; ls <= bli[0][hend]; ++ls) {
		s = fli[0][ls];
		if (!cc[0][s]) continue;
		int nt = c[0][s] % 7;
		// Do not process notes that are not harmonic
		if (nt != de1 && nt != de2 && nt != de3) continue;
		// Process only lower notes
		if (c[0][s] > lhbc) continue;
		// For left sus and isus check hstart
		if (s < hstart) {
			if (msh[0][hstart] > 0) {
				lhbc = c[0][s];
			}
		}
		// For other notes check note start
		else {
			if (msh[0][s] > 0) {
				lhbc = c[0][s];
			}
		}
	}
	// Detect 6/4 chord
	if (lhbc % 7 == (hvar + 4) % 7) hpenalty += 10;
	// Detect 6th chord
	if (lhbc % 7 == (hvar + 2) % 7) hpenalty += 1;
	// Find root in harmonic notes
	int found_de1 = 0;
	for (v = 0; v < av_cnt; ++v) {
		for (ls = bli[v][hstart]; ls <= bli[v][hend]; ++ls) {
			s = fli[v][ls];
			if (!cc[v][s]) continue;
			int nt = c[v][s] % 7;
			if (nt != de1) continue;
			// For left sus and isus check hstart
			if (s < hstart) {
				if (msh[v][hstart] > 0) {
					found_de1 = 1;
					break;
				}
			}
			// For other notes check note start
			else {
				if (msh[v][s] > 0) {
					found_de1 = 1;
					break;
				}
			}
		}
		if (found_de1) break;
	}
	// Increase penalty for chord without root (probably wrong chord detected)
	if (!found_de1) hpenalty += 1000;
	// Prohibit DTIII#5 augmented chord
	if (cchnv[shp[hstart % npm]][11] && cchnv[shp[hstart % npm]][3]) {
		FlagA(0, 375, hstart, hstart, 0, 3);
	}
}

// Detect ambiguous harmony
void CP2R::EvalHarmAmbig(int hvar) {
	// Downbeat harmonic notes
	int downbeat_harm = 0;
	for (v = 0; v < av_cnt; ++v) {
		ls = bli[v][hstart];
		if (!cc[v][hstart]) continue;
		if (fli[v][ls] == hstart && msh[v][hstart] > 0) ++downbeat_harm;
	}
	// Not a single harmonic note on first beat
	if (!downbeat_harm) FlagA(0, 558, hstart, hstart, 0, 30);
}

void CP2R::EvalHarmIncomplete(int hvar) {
	vc = vca[hstart];
	if (accept[0][vc][0][559]) return;
	// Get harmonic notes
	int de1 = hvar;
	int de2 = (de1 + 2) % 7;
	int de3 = (de1 + 4) % 7;
	// Count harmonic occurences
	int dc1 = 0;
	int dc2 = 0;
	int dc3 = 0;
	for (v = 0; v < av_cnt; ++v) {
		ls = bli[v][hstart];
		// Skip pauses
		if (!cc[v][hstart]) continue;
		// Incomplete
		if (msh[v][hstart] > 0 && nih[v][hstart]) {
			if (de1 == pc[v][hstart]) ++dc1;
			else if (de2 == pc[v][hstart]) ++dc2;
			else if (de3 == pc[v][hstart]) ++dc3;
		}
		else {
			if (resol[v][hstart]) {
				if (de1 == pc[v][resol[v][hstart]]) ++dc1;
				else if (de2 == pc[v][resol[v][hstart]]) ++dc2;
				else if (de3 == pc[v][resol[v][hstart]]) ++dc3;
			}
		}
	}
	// Penultimate incomplete D
	if (mli.size() > 1 && hvar == 4 && hend == mli[mli.size() - 1] - 1) {
		if (!dc1 || !dc2 || !dc3) FlagA(0, 172, hstart, hstart, 0, 0);
	}
	// Non penultimate incomplete
	else {
		if (!dc1 || !dc2) FlagA(0, 559, hstart, hstart, 0, 0);
	}
}

void CP2R::GetHarmBass() {
	SET_READY(DR_hbc);
	int ls1, ls2;
	int harm_end, nt;
	int de1, de2, de3;
	for (int hs = 0; hs < hli.size(); ++hs) {
		if (chm[hs] == -1) {
			hbc[hs] = -1;
			hbcc[hs] = -1;
			continue;
		}
		// Get harmonic notes
		de1 = chm[hs];
		de2 = (de1 + 2) % 7;
		de3 = (de1 + 4) % 7;
		// Init habcc - lowest harmonic note, including audible or suggested
		int habcc = 1000;
		// Loop inside harmony
		for (v = 0; v < min(av_cnt, 2); ++v) {
			for (ls = bli[v][hli[hs]]; ls <= bli[v][hli2[hs]]; ++ls) {
				s = fli[v][ls];
				// Skip pauses
				if (!cc[v][s]) continue;
				nt = c[v][s] % 7;
				// Do not process notes that are not harmonic
				if (nt != de1 && nt != de2 && nt != de3) continue;
				// Process only lower notes
				if (hbcc[hs] <= cc[v][s]) continue;
				if (nt == de3) {
					if (beat[v][ls] <= 1) {
						hbcc[hs] = cc[0][s];
						hbc[hs] = c[0][s];
						// Clear audible 64, because we have real 64 now
						ha64[hs] = 0;
					}
					else {
						int found = 0;
						// Search for note repeat
						for (int ls2 = bli[v][hli[hs]]; ls2 <= bli[v][hli2[hs]]; ++ls2) if (ls2 != ls) {
							if (cc[0][s] == cc[0][fli[v][ls2]]) found = 1;
						}
						// Set audible 6/4 for repeating 5th on upbeat
						if (found) {
							// Do not change harmony bass, because real harmony bass was already set. We set only audible 64
							ha64[hs] = 2;
							habcc = cc[0][s];
						}
						// Set suggestive 6/4 for non-repeating 5th on upbeat
						else {
							// Do not change harmony bass, because real harmony bass was already set. We set only audible 64
							ha64[hs] = 1;
							habcc = cc[0][s];
						}
					}
				}
				else {
					hbcc[hs] = cc[v][s];
					hbc[hs] = c[v][s];
					// Clear audible 64 if current note is lower than it
					if (ha64[hs] && cc[v][s] < habcc) ha64[hs] = 0;
				}
			}
		}
	}
}

int CP2R::FailVocalRanges() {
	for (v = 0; v < av_cnt; ++v) {
		// Prohibit decreasing vocal ranges
		if (v && vocra[v] < vocra[v - 1]) FlagV(v, 523, 0);
	}
	return 0;
}

int CP2R::FailVocalRangesConflict() {
	int conf_start = -1;
	for (v2 = v + 1; v2 < av_cnt; ++v2) {
		for (s = 0; s < c_len; ++s) {
			GetVp();
			vc = vca[s];
			if (!cc[v][s]) continue;
			if (!cc[v2][s]) continue;
			// Check if there is range conflict
			int is_conf = 0;
			if (cc[v][s] > vocra_info[vocra[v]].high_cc && cc[v2][s] < vocra_info[vocra[v2]].low_cc) is_conf = 1;
			else if (cc[v][s] < vocra_info[vocra[v]].low_cc && cc[v2][s] > vocra_info[vocra[v2]].high_cc) is_conf = 1;
			// Search for start of conflict
			if (conf_start == -1) {
				if (!is_conf) continue;
				conf_start = s;
			}
			// Search for end of conflict
			else {
				if (!is_conf) {
					if (s - conf_start > vocra_disbal_yel[sp][vc][vp] * 2) {
						if (s - conf_start > vocra_disbal_red[sp][vc][vp] * 2) {
							FlagL(v, 526, conf_start, s - 1, v2);
						}
						else {
							FlagL(v, 524, conf_start, s - 1, v2);
						}
					}
					conf_start = -1;
				}
			}
		}
		if (conf_start > -1) {
			s = c_len;
			if (s - conf_start > vocra_disbal_yel[sp][vc][vp] * 2) {
				if (s - conf_start > vocra_disbal_red[sp][vc][vp] * 2) {
					FlagL(v, 526, conf_start, s - 1, v2);
				}
				else {
					FlagL(v, 524, conf_start, s - 1, v2);
				}
			}
		}
	}
	return 0;
}

// Check vocal ranges limits
int CP2R::FailVRLimit() {
	CHECK_READY(DR_fli);
	int out_s = -1;
	for (ls = 0; ls < fli_size[v]; ++ls) {
		s = fli[v][ls];
		if (!cc[v][s]) continue;
		// Record
		if (cc[v][s] > vocra_info[vocra[v]].max_cc || cc[v][s] < vocra_info[vocra[v]].min_cc) {
			if (out_s == -1) out_s = s;
		}
		else {
			if (out_s > -1) {
				FlagVL(v, 522, out_s, s);
				fpenalty += s - out_s;
				out_s = -1;
			}
		}
	}
	if (out_s > -1) {
		FlagVL(v, 522, out_s, s);
		fpenalty += s - out_s;
	}
	return 0;
}

int CP2R::FailMeasureLen() {
	for (v = 0; v < av_cnt; ++v) {
		sp = vsp[v];
		if (sp == 0) continue;
		if (sp == 1) continue;
		if (sp == 2) {
			if (npm == 8) continue;
			if (npm == 4 || npm == 6 || npm == 12) continue;
		}
		if (sp == 3) {
			if (npm == 8) continue;
			if (npm == 10) continue;
			if (npm == 12) continue;
		}
		if (sp == 4) {
			if (npm == 8) continue;
			if (npm == 4 || npm == 6 || npm == 12) continue;
		}
		if (sp == 5) {
			if (npm == 8) continue;
		}
		FlagV(v, 525, 0);
	}
	return 0;
}

void CP2R::FlagParallelIco() {
	CHECK_READY(DR_fli);
	// Do not run check if there are no sp5 voices
	int sp_count = 0;
	for (v = 0; v < av_cnt; ++v) {
		if (vsp[v] > 1) ++sp_count;
	}
	if (sp_count > 1) return;
	for (v = 0; v < av_cnt; ++v) {
		for (v2 = v + 1; v2 < av_cnt; ++v2) {
			int ivl_prev = -1000;
			int pico_count = 0;
			int pico_flagged = 0;
			for (ls = 0; ls < fli_size[v]; ++ls) {
				s = fli[v][ls];
				ls2 = bli[v2][s];
				// Skip different notes, pauses
				if (!cc[v][s] || !cc[v2][s] || s != fli[v2][ls2] ||
					llen[v2][ls2] != llen[v][ls]) {
					pico_count = 0;
					pico_flagged = 0;
					continue;
				}
				ivl = c[v][s] - c[v2][s];
				ivlc = abs(ivl) % 7;
				civl = cc[v][s] - cc[v2][s];
				civlc = abs(civl) % 12;
				// Skip non-ico
				if (civlc != 3 && civlc != 4 && civlc != 8 && civlc != 9) {
					pico_count = 0;
					pico_flagged = 0;
					continue;
				}
				// New interval resets counter
				if (pico_count && ivl != ivl_prev) {
					pico_count = 0;
					pico_flagged = 0;
				}
				// Save interval
				ivl_prev = ivl;
				++pico_count;
				GetVp();
				vc = vca[s];
				if (pico_count > 3 && s - fli[v][ls - pico_count + 1] > npm && !pico_flagged) {
					pico_flagged = 1;
					if (ivlc == 2) 
						FlagL(v, 89, fli[v][ls - pico_count + 1], s, v2);
					else
						FlagL(v, 90, fli[v][ls - pico_count + 1], s, v2);
				}
			}
		}
	}
}

int CP2R::FailVIntervals() {
	CHECK_READY(DR_fli, DR_sus);
	for (v2 = v + 1; v2 < av_cnt; ++v2) {
		for (s = fin[v]; s < ep2; ++s) {
			ls = bli[v][s];
			ls2 = bli[v2][s];
			// Skip no note start
			if (s != fli[v][ls] && s != fli[v2][ls2]) continue;
			// Skip pauses
			if (!cc[v][s]) continue;
			if (!cc[v2][s]) continue;
			GetVp();
			vc = vca[s];
			// Prepare data
			civl = abs(cc[v][s] - cc[v2][s]);
			// Skip first note 
			if (ls <= fil[v]) continue;
			if (ls2 <= fil[v2]) continue;

			if (FailUnison()) return 1;
		}
	}
	return 0;
}

int CP2R::FailUnison() {
	// Unison
	if (!civl && fli[v][ls] != fli[v2][ls2]) {
		// Find previous interval
		if (fli[v][ls] > fli[v2][ls2]) {
			s3 = fli[v][ls] - 1;
			// 2nd -> unison
			civl2 = abs(cc[v2][s3] - cc[v][s3]);
			if (civl2 == 1) FlagL(v, 275, s, max(ssus[v][ls - 1], ssus[v2][ls2 - 1]), v2);
			else if (civl2 == 2) FlagL(v, 277, s, max(ssus[v][ls - 1], ssus[v2][ls2 - 1]), v2);
		}
		else {
			s3 = fli[v2][ls2] - 1;
			// 2nd -> unison
			civl2 = abs(cc[v2][s3] - cc[v][s3]);
			// Send flag to voice v2 instead of v
			swap(v, v2);
			if (civl2 == 1) FlagL(v, 275, s, max(ssus[v2][ls - 1], ssus[v][ls2 - 1]), v2);
			else if (civl2 == 2) FlagL(v, 277, s, max(ssus[v2][ls - 1], ssus[v][ls2 - 1]), v2);
			// Return to voice v
			swap(v, v2);
		}
	}
	return 0;
}

int CP2R::FailSyncVIntervals() {
	CHECK_READY(DR_fli, DR_sus);
	for (v2 = v + 1; v2 < av_cnt; ++v2) {
		for (ls = 1; ls < fli_size[v]; ++ls) {
			s = fli[v][ls];
			ls2 = bli[v2][s];
			GetVp();
			vc = vca[s];
			// Check only when both notes start simultaneously
			if (s != fli[v2][ls2]) continue;
			// Skip first note in second voice
			if (!ls2) continue;
			// Prepare data
			s2 = fli2[v][ls];
			civl = abs(cc[v][s] - cc[v2][s]);
			civlc = civl % 12;
			civl2 = abs(cc[v2][s - 1] - cc[v][s - 1]);
			civlc2 = civl2 % 12;
			//if (civl && civl % 12 == 0) civlc2 = 12;
			//else civlc2 = civl % 12;
			// Skip pauses
			if (!cc[v][s]) continue;
			if (!cc[v2][s]) continue;
			if (!cc[v][s - 1]) continue;
			if (!cc[v2][s - 1]) continue;
			if (FailPco()) return 1;
			if (!v && v2 == av_cnt - 1) {
				FlagDirectDis();
			}
		}
	}
	return 0;
}

void CP2R::FlagDirectDis() {
	if (civlc == 11 || civlc == 10 ||
		((civlc == 1 || civlc == 2) && civl > 12)) {
		if (ssus[v][ls - 1] > ssus[v2][ls2 - 1]) {
			s3 = ssus[v][ls - 1];
			v3 = v;
			v4 = v2;
		}
		else {
			s3 = ssus[v2][ls2 - 1];
			v3 = v2;
			v4 = v;
		}
		if ((cc[v][s] - cc[v][s - 1]) * (cc[v2][s] - cc[v2][s - 1]) > 0) {
			// Minor 7th
			if (civlc == 10) FlagL(v3, 169, s3, s, v4);
			// Major 7th
			else if (civlc == 11) FlagL(v3, 276, s3, s, v4);
			// Minor 9th
			else if (civlc == 1) FlagL(v3, 173, s3, s, v4);
			// Major 9th
			else if (civlc == 2) FlagL(v3, 174, s3, s, v4);
		}
	}
}

int CP2R::FailPco() {
	if (!civl) {
		// Unison (inside) downbeat without suspension
		if (!beat[v][ls] && ls < fli_size[v] - 1 && ls2 < fli_size[v2] - 1 && !sus[v][ls] && !sus[v2][ls2]) {
			// Ignore more than 4 voices and 2 lowest of 4 voices
			if (vca[s] < 4 || (vca[s] == 4 && v + v2 != 1) )
				Flag(v, 91, s, v2);
		}
	}
	if (civlc == 7 || civlc == 12 || civlc == 0) {
		// Choose best voices for flag visualization
		if (ssus[v][ls - 1] > ssus[v2][ls2 - 1]) {
			s3 = ssus[v][ls - 1];
			v3 = v;
			v4 = v2;
		}
		else {
			s3 = ssus[v2][ls2 - 1];
			v3 = v2;
			v4 = v;
		}
		// Do not prohibit consecutive first - first (this is for sus notes, which starts are parallel)
		// because they are detected as pco apart now
		// Prohibit consecutive last - first
		if (civl == civl2) {
			// Only if notes are different (ignore interval repeat)
			if (cc[v2][s - 1] != cc[v2][s] || cc[v][s - 1] != cc[v][s]) {
				if (civlc == 7) FlagL(v3, 84, s3, s, v4);
				else FlagL(v3, 481, s3, s, v4);
			}
		}
		else {
			// Prohibit contrary movement
			if (civlc == civlc2) {
				if (civlc == 7) FlagL(v3, 85, s3, s, v4);
				else FlagL(v3, 482, s3, s, v4);
			}
		}
	}
	return 0;
}

int CP2R::FailMsh() {
	if (FailSyncVIntervals()) return 1;
	if (FailVIntervals()) return 1;
	return 0;
}

void CP2R::EvaluateMsh() {
	// Get last measure step
	int mea_end = mli[ms] + npm - 1;
	// Prevent going out of window
	if (mea_end >= ep2) return;
	for (ls = bli[v][s0]; ls <= bli[v][mea_end]; ++ls) {
		s = fli[v][ls];
		// Skip pauses
		if (!cc[v][s]) continue;
		if (msh[v][s] <= 0) {
			// Detect auxiliary tone, not surrounded by chord tones
			if (ls < bli[v][mea_end] &&
				cc[v][fli[v][ls - 1]] == cc[v][fli[v][ls + 1]] && abs(c[v][fli[v][ls - 1]] - c[v][s]) == 1 && 
				(!nih[v][fli[v][ls - 1]] || !nih[v][fli[v][ls + 1]])) {
				msh[v][s] = pAuxWrong;
				if (!nih[v][s])
					FlagA(v, 170, s, s, v, 50);
			}
			// Detect non-harmonic tone, which is longer than previous non-harmonic tone
			else if (llen[v][ls] > llen[v][ls - 1] &&
				!nih[v][fli[v][ls - 1]]) {
				msh[v][s] = pLong;
				if (!nih[v][s])
					FlagA(v, 223, s, s, v, 50);
			}
			// Skip non-harmonic tones
			continue;
		}
		// Check if note started in previous measure
		if (s < s0) {
			// Check if note traverses multiple harmonies in current measure
			if (bhli[fli2[v][ls]] - bhli[s0] > 0) {
				// Set msh and check to measure start
				s = s0;
				msh[v][s] = pSusStart;
			}
			else continue;
		}
		if (!nih[v][s]) {
			if (msh[v][s] == pFirst) FlagA(v, 551, s, s, v, 100);
			else if (msh[v][s] == pDownbeat) FlagA(v, 83, s, s, v, 100);
			else if (msh[v][s] == pLeapTo) FlagA(v, 36, s, s, v, 100);
			else if (msh[v][s] == pLeapFrom) FlagA(v, 187, s, s, v, 100);
			else if (msh[v][s] == pSusStart) FlagA(v, 458, s, s, v, 100);
			// pSusRes does not have separate flag, because it is marked as not resolved
			// This is protection against wrong melodic shape value
			else if (msh[v][s] > 0) FlagA(v, 83, s, s, v, 100);
		}
	}
}

void CP2R::EvaluateMshSteps() {
	// Skip bass
	if (!v) return;
	// Get last measure step
	int mea_end = mli[ms] + npm - 1;
	// Prevent going out of window
	if (mea_end >= ep2) return;
	v2 = 0;
	GetVp();
	for (s = s0; s <= mea_end; ++s) {
		ls = bli[v][s];
		ls2 = bli[0][s];
		// Skip no note start
		if (s != fli[v][ls] && s != fli[0][ls2]) continue;
		// Skip non-chord tones
		if (msh[v][ssus[v][ls]] <= 0) continue;
		if (msh[0][ssus[0][ls2]] <= 0) continue;
		// Skip pauses
		if (!cc[v][s]) continue;
		if (!cc[0][s]) continue;
		vc = vca[s];
		// Prepare data
		civl = abs(cc[v][s] - cc[0][s]);
		// Flag 4th or tritone with bass
		if (civl % 12 == 5) FlagA(v, 171, s, s, 0, 100);
		if (civl % 12 == 6) FlagA(v, 331, s, s, 0, 100);
	}
}

void CP2R::GetMeasureMsh(int sec_hp) {
	// Get last measure step
	int mea_end = mli[ms] + npm - 1;
	// Prevent going out of window
	if (mea_end >= ep2) return;
	// Clear msh
	for (s = mli[ms]; s < mli[ms] + npm; ++s) msh[v][s] = 1;
	for (ls = bli[v][s0]; ls <= bli[v][mea_end]; ++ls) {
		s = fli[v][ls];
		msh[v][s] = pAux;
		if (!cc[v][s]) continue;
		s2 = fli2[v][ls];
		// First note is always downbeat
		if (s == fin[v]) msh[v][s] = pFirst;
		else if (ls == fli_size[v] - 1) msh[v][s] = pLast;
		// Sus start is always harmonic
		else if (sus[v][ls]) {
			msh[v][s] = pSusStart;
			msh[v][sus[v][ls]] = pSusHarm;
		}
		// Downbeat
		else if (s % npm == 0) msh[v][s] = pDownbeat;
		// First note in harmony
		else if (s == sec_hp) msh[v][s] = pDownbeat;
		// Anticipation
		else if (ls == bli[v][mea_end] && s2 < ep2 - 1 && s2 == s0 + npm - 1 &&
			cc[v][s2] == cc[v][s2 + 1] && llen[v][ls] <= 4 && ms == mli.size() - 2 &&
			llen[v][ls + 1] <= npm && llen[v][ls - 1] >= llen[v][ls]) msh[v][s] = pAux;
		else if (s > 0 && leap[v][s - 1]) msh[v][s] = pLeapTo;
		else if (s2 < ep2 - 1 && leap[v][s2]) msh[v][s] = pLeapFrom;
	}
	// Make last leading tone in penultimate measure harmonic
	if (ms == mli.size() - 2 && (!mode || mminor)) {
		int s9 = fli[v][fli_size[v] - 2];
		if (fli_size[v] >= 2 && pcc[v][s9] == 11 && msh[v][fli[v][fli_size[v] - 2]] < 0) {
			msh[v][fli[v][fli_size[v] - 2]] = pLastLT;
		}
	}
}

// Mark only notes in measure, which are definitely harmonic
void CP2R::GetMinimumMsh() {
	// Get last measure step
	int mea_end = mli[ms] + npm - 1;
	// Prevent going out of window
	if (mea_end >= ep2) return;
	// Clear msh
	for (s = mli[ms]; s < mli[ms] + npm; ++s) msh[v][s] = 1;
	for (ls = bli[v][s0]; ls <= bli[v][mea_end]; ++ls) {
		s = fli[v][ls];
		msh[v][s] = pAux;
		if (!cc[v][s]) continue;
		s2 = fli2[v][ls];
		// Skip first suspension, mark last suspension
		if (sus[v][ls]) {
			// Mark sus start
			msh[v][sus[v][ls]] = pAux;
			msh[v][s] = pSusStart;
		}
		// First note
		else if (s == fin[v]) msh[v][s] = pFirst;
		else if (ls == fli_size[v] - 1) msh[v][s] = pLast;
		// Downbeat
		else if (s % npm == 0) {
			// Long on downbeat
			if (llen[v][ls] > 4 || leap[v][s2])	msh[v][s] = pDownbeat;
			// Downbeat note not surrounded by descending stepwise movement
			// TODO: Optimize for generation
			else if (smooth[v][s - 1] != -1 || (s2 < ep2 - 1 && smooth[v][s2] != -1)) {
				msh[v][s] = pDownbeat;
			}
		}
		// Anticipation
		else if (ls == bli[v][mea_end] && s2 < ep2 - 1 && s2 == s0 + npm - 1 &&
			cc[v][s2] == cc[v][s2 + 1] && llen[v][ls] <= 4 && ms == mli.size() - 2 &&
			llen[v][ls + 1] <= npm && llen[v][ls - 1] >= llen[v][ls]) msh[v][s] = pAux;
		else if (abs(leap[v][s - 1]) > 2) msh[v][s] = pLeapTo;
		else if (s2 < ep2 - 1 && abs(leap[v][s2]) > 2) msh[v][s] = pLeapFrom;
		else if (leap[v][s - 1]) {
			// leap * leap/pause
			if (s2 < ep2 - 1) {
				if (leap[v][s2] || !cc[v][s2 + 1]) msh[v][s] = pLeapTo;
			}
			// leap between measures
			else if (ls == bli[v][s0]) msh[v][s] = pLeapTo;
			// leap/pause + leap *
			if (ls > 1) {
				if (leap[v][fli2[v][ls - 2]] || !cc[v][fli2[v][ls - 2]]) msh[v][s] = pLeapTo;
			}
		}
		else if (s2 < ep2 - 1 && leap[v][s2]) {
			// leap/pause * leap
			if (leap[v][s - 1] || !cc[v][s - 1]) msh[v][s] = pLeapFrom;
			// leap between measures
			else if (ls == bli[v][mea_end]) msh[v][s] = pLeapFrom;
			// * leap + leap/pause
			else if (ls < fli_size[v] - 2) {
				if (leap[v][fli2[v][ls + 1]] || !cc[v][fli[v][ls + 2]]) msh[v][s] = pLeapFrom;
			}
		}
	}
	// Make last leading tone in penultimate measure harmonic
	if (ms == mli.size() - 2 && (!mode || mminor)) {
		int s9 = fli[v][fli_size[v] - 2];
		if (fli_size[v] >= 2 && pcc[v][s9] == 11 && msh[v][fli[v][fli_size[v] - 2]] < 0) {
			msh[v][fli[v][fli_size[v] - 2]] = pLastLT;
		}
	}
}

// Detect all possible chords
void CP2R::GetHarmVar(vector<int> &cpos, int &poss_vars) {
	poss_vars = 0;
	for (hv = 0; hv < 7; ++hv) {
		// At least one note exists
		if (!chn[hv] && !chn[(hv + 2) % 7] && !chn[(hv + 4) % 7]) continue;
		// No other notes should exist
		if (chn[(hv + 1) % 7] || chn[(hv + 3) % 7] || chn[(hv + 5) % 7] || chn[(hv + 6) % 7]) continue;
		cpos[hv] = 1;
		++poss_vars;
#if defined(_DEBUG)
		CString st, est;
		est.Format("Possible chord %s in measure %d:%d",
			degree_name[hv], cp_id + 1, ms + 1);
		WriteLog(3, est);
#endif
	}
}

void CP2R::GetMsh() {
	SET_READY(DR_msh, DR_nih);
	flaga.clear();
	for (ms = 0; ms < mli.size(); ++ms) {
		hpenalty = 0;
		fill(chn.begin(), chn.end(), 0);
		fill(cchn.begin(), cchn.end(), 0);
		s0 = mli[ms];
		vc = vca[s0];
		int s9;
		// Get last measure step
		int mea_end = mli[ms] + npm - 1;
		// Prevent going out of window
		if (mea_end >= ep2) break;
		// Build harmony using already detected msh notes
		int hnotes = 0;
		for (v = 0; v < av_cnt; ++v) {
			GetMinimumMsh();
			// Clear note-in-harmony
			for (s = s0; s < s0 + npm; ++s) {
				nih[v][s] = 0;
			}
			for (ls = bli[v][s0]; ls <= bli[v][mea_end]; ++ls) {
				s = fli[v][ls];
				if (!cc[v][s]) continue;
				// Skip first suspension
				if (s < s0) continue;
				// Skip non-harmonic and ambiguous notes
				if (msh[v][s] <= 0) continue;
				// Record note
				++chn[pc[v][s]];
				++cchn[pcc[v][s]];
				++hnotes;
			}
		}
		// Main chord
		int lchm;
		int lchm_alter;
		int rat;
		// Possible chords
		vector <int> cpos;
		cpos.resize(7);
		int poss_vars;
		GetHarmVars(lchm, lchm_alter, rat, cpos, poss_vars);
		// If no harmonic notes found, scan all harmonies
		if (!poss_vars && !hnotes) {
			poss_vars = 7;
			for (int i = 0; i < 7; ++i) cpos[i] = 1;
		}
		if (!poss_vars && ms == mli.size() - 2) {
			GetMsh2();
			continue;
		}
		int min_hpenalty = 1000000;
		hstart = s0;
		hend = s0 + npm - 1;
		// Scan all possible chords
		for (int hv2 = lchm + 7; hv2 > lchm; --hv2) {
			hv = hv2 % 7;
			if (!cpos[hv]) continue;
			for (hv_alt = 0; hv_alt <= 1; ++hv_alt) {
				// Only for melodic minor
				if (hv_alt && !mminor) continue;
				fill(cchnv[0].begin(), cchnv[0].end(), 0);
				cchnv[0][(c_cc[hv + 14] + 24 - bn) % 12] = 1;
				cchnv[0][(c_cc[hv + 16] + 24 - bn) % 12] = 1;
				cchnv[0][(c_cc[hv + 18] + 24 - bn) % 12] = 1;
				if (mminor) {
					if (hv_alt) {
						if (cchnv[0][8]) {
							// Skip if this variant conflicts with detected notes
							if (cchn[8]) continue;
							// Convert to altered
							cchnv[0][8] = 0;
							cchnv[0][9] = 1;
						}
						else if (cchnv[0][10]) {
							// Skip if this variant conflicts with detected notes
							if (cchn[10]) continue;
							// Convert to altered
							cchnv[0][10] = 0;
							cchnv[0][11] = 1;
						}
						else continue;
					}
					else {
						if (cchnv[0][8]) {
							// Skip if this variant conflicts with detected notes
							if (cchn[9]) continue;
						}
						if (cchnv[0][10]) {
							// Skip if this variant conflicts with detected notes
							if (cchn[11]) continue;
						}
					}
				}
				flaga.clear();
				hpenalty = 0;
				for (v = 0; v < av_cnt; ++v) {
					sp = vsp[v];
					GetMeasureMsh(-1);
					// Clear resolutions
					for (s = s0; s < s0 + npm; ++s) {
						resol[v][s] = 0;
					}
					GetNotesInHarm();
					s = hstart;
					ls = bli[v][s];
					s2 = fli2[v][ls];
					DetectSus();
					DetectPDD();
					DetectDNT();
					DetectCambiata();
					EvaluateMsh();
					EvaluateMshSteps();
				}
				EvalMshHarm(hv);
				EvalHarmAmbig(hv);
				EvalHarmIncomplete(hv);
#if defined(_DEBUG)
				CString st, est;
				est.Format("Checked chord %s%s in measure %d:%d, hpenalty %d, flags %d:",
					degree_name[hv], hv_alt ? "*" : "", cp_id + 1, ms + 1,
					hpenalty, flaga.size());
				est += " msh:";
				for (v = 0; v < av_cnt; ++v) {
					for (int i = 0; i < npm; ++i) {
						st.Format(" %d", msh[v][s0 + i]);
						est += st;
					}
					if (v < av_cnt - 1) est += " /";
				}
				for (int fl = 0; fl < flaga.size(); ++fl) {
					st.Format(" [%d] %d %s (%s)", flaga[fl].id, flaga[fl].s,
						ruleinfo[flaga[fl].id].RuleName,
						ruleinfo[flaga[fl].id].SubRuleName);
					est += st;
				}
				est += " ch:";
				for (int i = 0; i < 12; ++i) {
					st.Format(" %d", cchn[i]);
					est += st;
					if (i == 5) est += " -";
				}
				WriteLog(3, est);
#endif
				// Save best variant
				if (hpenalty < min_hpenalty) {
#if defined(_DEBUG)
					WriteLog(3, "Selected best hpenalty");
#endif
					min_hpenalty = hpenalty;
					flagab = flaga;
					for (s = s0; s < s0 + npm; ++s) {
						for (v = 0; v < av_cnt; ++v) {
							mshb[v][s] = msh[v][s];
							nihb[v][s] = nih[v][s];
						}
					}
				}
				// Stop evaluating variants if all is ok
				if (!hpenalty) break;
			}
			// Stop evaluating variants if all is ok
			if (!hpenalty) break;
		}
		if (min_hpenalty < 1000000) {
			// Apply best msh
			for (s = s0; s < s0 + npm; ++s) {
				for (v = 0; v < av_cnt; ++v) {
					msh[v][s] = mshb[v][s];
					nih[v][s] = nihb[v][s];
				}
			}
			// Save flags
			for (int fl = 0; fl < flagab.size(); ++fl) {
				v = flagab[fl].voice;
				FlagL(v, flagab[fl].id, flagab[fl].s, flagab[fl].fsl, flagab[fl].fvl);
			}
		}
	}
}

void CP2R::GetNotesInHarm() {
	for (ls = bli[v][hstart]; ls <= bli[v][hend]; ++ls) {
		s = fli[v][ls];
		// Skip pauses
		if (!cc[v][s]) continue;
		if (s < hstart) s = hstart;
		if (cchnv[shp[s % npm]][pcc[v][s]]) {
			nih[v][s] = 1;
		}
		else {
			nih[v][s] = 0;
		}
	}
}

void CP2R::GetHarmVars(int &lchm, int &lchm_alter, int &rat, vector<int> & cpos, int & poss_vars) {
	GetHarm(-100, 0, lchm, lchm_alter, rat);
	GetHarmVar(cpos, poss_vars);
}

void CP2R::GetMsh2() {
	// Detect second chord position
	// Main chord
	vector<int> lchm;
	lchm.resize(2);
	int lchm_alter;
	int rat;
	int poss_vars;
	// Get last measure step
	int mea_end = mli[ms] + npm - 1;
	// Prevent going out of window
	if (mea_end >= ep2) return;
	// Clear harmonic notes vector
	fill(chn.begin(), chn.end(), 0);
	fill(cchn2[0].begin(), cchn2[0].end(), 0);
	fill(cchn2[1].begin(), cchn2[1].end(), 0);
	int hcount = 0;
	int sec_hp = 0;
	// Loop inside measure
	for (s = mli[ms]; s <= mea_end; ++s) {
		for (v = 0; v < av_cnt; ++v) {
			ls = bli[v][s];
			// Skip if not note start or sus
			if (fli[v][ls] != s && sus[v][ls] != s) continue;
			// Skip pauses
			if (!cc[v][s]) continue;
			sp = vsp[v];
			// For first suspension in measure, evaluate last step. In other cases - first step
			if (fli[v][ls] <= mli[ms] && sus[v][ls]) {
				// For first suspended dissonance resolved note do not check msh
				if (msh[v][sus[v][ls]] < 0) continue;
			}
			else {
				// For all other notes, check msh
				if (msh[v][s] <= 0) continue;
			}
			// Pitch class
			int n = pc[v][s];
			// Find harmonic conflict
			if (s > mli[ms] && (chn[(n + 1) % 7] || chn[(n + 6) % 7] ||
				(chn[n] && !cchn2[hcount][pcc[v][s]]))) {
				// More than two harmonies
				if (hcount) return;
				// Two harmonies penultimate
				else {
					// Get second harmony position
					sec_hp = s - mli[ms];
				}
				// Remove notes of current step from chord, because this step belongs to next chord
				for (v2 = 0; v2 < v; ++v2) {
					ls = bli[v2][s];
					// Skip if not note start 
					if (fli[v2][ls] != s) continue;
					// Skip pauses
					if (!cc[v2][s]) continue;
					// Check msh
					if (msh[v2][s] <= 0) continue;
					// Remove note
					--chn[pc[v2][s]];
					--cchn2[hcount][pcc[v][s]];
				}
				GetHarmVars(lchm[hcount], lchm_alter, rat, cpos[hcount], poss_vars);
				if (poss_vars == 0) return;
				// Next harmony counter
				++hcount;
				fill(chn.begin(), chn.end(), 0);
				// Reinitialize chord notes for new chord
				for (v2 = 0; v2 < av_cnt; ++v2) {
					ls = bli[v2][s];
					// Skip pauses
					if (!cc[v2][s]) continue;
					// For first suspension in measure, evaluate last step. In other cases - first step
					if (fli[v2][ls] < s) continue;
					if (msh[v2][fli[v2][ls]] <= 0) continue;
					// Record note
					++chn[pc[v2][s]];
					++cchn2[hcount][pcc[v2][s]];
				}
			}
			// Record note
			++chn[n];
			++cchn2[hcount][pcc[v][s]];
		}
	}
	// Process last chord
	GetHarmVars(lchm[hcount], lchm_alter, rat, cpos[hcount], poss_vars);
	if (poss_vars == 0) return;
	// Init step harmony position
	for (int i = 0; i < npm; ++i) {
		if (i < sec_hp) shp[i] = 0;
		else shp[i] = 1;
	}
	int min_hpenalty = 1000000;
	// Scan all possible chords
	for (int hv2 = lchm[0] + 7; hv2 > lchm[0]; --hv2) {
		hv = hv2 % 7;
		if (!cpos[0][hv]) continue;
		for (hv_alt = 0; hv_alt <= 1; ++hv_alt) {
			// Only for melodic minor
			if (hv_alt && !mminor) continue;
			fill(cchnv[0].begin(), cchnv[0].end(), 0);
			cchnv[0][(c_cc[hv + 14] + 24 - bn) % 12] = 1;
			cchnv[0][(c_cc[hv + 16] + 24 - bn) % 12] = 1;
			cchnv[0][(c_cc[hv + 18] + 24 - bn) % 12] = 1;
			if (mminor) {
				if (hv_alt) {
					if (cchnv[0][8]) {
						// Skip if this variant conflicts with detected notes
						if (cchn2[0][8]) continue;
						// Convert to altered
						cchnv[0][8] = 0;
						cchnv[0][9] = 1;
					}
					else if (cchnv[0][10]) {
						// Skip if this variant conflicts with detected notes
						if (cchn2[0][10]) continue;
						// Convert to altered
						cchnv[0][10] = 0;
						cchnv[0][11] = 1;
					}
					else continue;
				}
				else {
					if (cchnv[0][8]) {
						// Skip if this variant conflicts with detected notes
						if (cchn2[0][9]) continue;
					}
					if (cchnv[0][10]) {
						// Skip if this variant conflicts with detected notes
						if (cchn2[0][11]) continue;
					}
				}
			}
			for (int hv4 = lchm[1] + 7; hv4 > lchm[1]; --hv4) {
				int hv3 = hv4 % 7;
				if (!cpos[1][hv3]) continue;
				for (int hv_alt2 = 0; hv_alt2 <= 1; ++hv_alt2) {
					// Only for melodic minor
					if (hv_alt2 && !mminor) continue;
					fill(cchnv[1].begin(), cchnv[1].end(), 0);
					cchnv[1][(c_cc[hv3 + 14] + 24 - bn) % 12] = 1;
					cchnv[1][(c_cc[hv3 + 16] + 24 - bn) % 12] = 1;
					cchnv[1][(c_cc[hv3 + 18] + 24 - bn) % 12] = 1;
					if (mminor) {
						if (hv_alt2) {
							if (cchnv[1][8]) {
								// Skip if this variant conflicts with detected notes
								if (cchn2[1][8]) continue;
								// Convert to altered
								cchnv[1][8] = 0;
								cchnv[1][9] = 1;
							}
							else if (cchnv[1][10]) {
								// Skip if this variant conflicts with detected notes
								if (cchn2[1][10]) continue;
								// Convert to altered
								cchnv[1][10] = 0;
								cchnv[1][11] = 1;
							}
							else continue;
						}  
						else {
							if (cchnv[1][8]) {
								// Skip if this variant conflicts with detected notes
								if (cchn2[1][9]) continue;
							}
							if (cchnv[1][10]) {
								// Skip if this variant conflicts with detected notes
								if (cchn2[1][11]) continue;
							}
						}
					}
					flaga.clear();
					hpenalty = 0;
					for (v = 0; v < av_cnt; ++v) {
						sp = vsp[v];
						GetMeasureMsh(s0 + sec_hp);
						// Clear resolutions
						for (s = s0; s < s0 + npm; ++s) {
							resol[v][s] = 0;
						}
						// First harmony
						hstart = s0;
						hend = s0 + sec_hp - 1;
						GetNotesInHarm();
						s = hstart;
						ls = bli[v][s];
						s2 = fli2[v][ls];
						DetectSus();
						DetectPDD();
						// Second harmony
						hstart = s0 + sec_hp;
						hend = s0 + npm - 1;
						GetNotesInHarm();
						s = hstart;
						ls = bli[v][s];
						s2 = fli2[v][ls];
						DetectSus();
						DetectPDD();
						// Full measure
						DetectDNT();
						DetectCambiata();
						EvaluateMsh();
						EvaluateMshSteps();
					}
					// First harmony
					hstart = s0;
					hend = s0 + sec_hp - 1;
					EvalMshHarm(hv);
					EvalHarmAmbig(hv);
					EvalHarmIncomplete(hv);
					// Second harmony
					hstart = s0 + sec_hp;
					hend = s0 + npm - 1;
					EvalMshHarm(hv3);
					EvalHarmAmbig(hv3);
					EvalHarmIncomplete(hv3);
#if defined(_DEBUG)
					CString st, est;
					est.Format("Checked chords %s%s %s%s in measure %d:%d, hpenalty %d, flags %d:",
						degree_name[hv], hv_alt ? "*" : "",
						degree_name[hv3], hv_alt2 ? "*" : "",
						cp_id + 1, ms + 1, hpenalty, flaga.size());
					est += " msh:";
					for (v = 0; v < av_cnt; ++v) {
						for (int i = 0; i < npm; ++i) {
							st.Format(" %d", msh[v][s0 + i]);
							est += st;
						}
						if (v < av_cnt - 1) est += " /";
					}
					for (int fl = 0; fl < flaga.size(); ++fl) {
						st.Format(" [%d] %d %s (%s)", flaga[fl].id, flaga[fl].s,
							ruleinfo[flaga[fl].id].RuleName,
							ruleinfo[flaga[fl].id].SubRuleName);
						est += st;
					}
					est += " ch:";
					for (int i = 0; i < 12; ++i) {
						st.Format(" %d", cchn2[0][i]);
						est += st;
						if (i == 5) est += " -";
						if (i == 11) est += " /";
					}
					for (int i = 0; i < 12; ++i) {
						st.Format(" %d", cchn2[1][i]);
						est += st;
						if (i == 5) est += " -";
					}
					WriteLog(3, est);
#endif
					// Save best variant
					if (hpenalty < min_hpenalty) {
#if defined(_DEBUG)
						WriteLog(3, "Selected best hpenalty");
#endif
						min_hpenalty = hpenalty;
						flagab = flaga;
						for (s = s0; s < s0 + npm; ++s) {
							for (v = 0; v < av_cnt; ++v) {
								mshb[v][s] = msh[v][s];
								nihb[v][s] = nih[v][s];
							}
						}
					}
					// Stop evaluating variants if all is ok
					if (!hpenalty) break;
				}
				// Stop evaluating variants if all is ok
				if (!hpenalty) break;
			}
			// Stop evaluating variants if all is ok
			if (!hpenalty) break;
		}
		// Stop evaluating variants if all is ok
		if (!hpenalty) break;
	}
	if (min_hpenalty < 1000000) {
		// Apply best msh
		for (s = s0; s < s0 + npm; ++s) {
			for (v = 0; v < av_cnt; ++v) {
				msh[v][s] = mshb[v][s];
				nih[v][s] = nihb[v][s];
			}
		}
		// Save flags
		for (int fl = 0; fl < flagab.size(); ++fl) {
			v = flagab[fl].voice;
			FlagL(v, flagab[fl].id, flagab[fl].s, flagab[fl].fsl, flagab[fl].fvl);
		}
	}
	// Reset step harmony positions
	fill(shp.begin(), shp.end(), 0);
}

void CP2R::DetectDNT() {
	if (!accept[sp][vc][0][258]) return;
	// Suspension will conflict with DNT
	int ls0 = bli[v][mli[ms]];
	if (sus[v][ls0]) return;
	int ls_max = bli[v][mli[ms] + npm - 1] - 3;
	if (ls_max > fli_size[v] - 4) ls_max = fli_size[v] - 4;
	// Not enough notes for DNT
	if (ls_max < ls0) return;
	for (ls = ls0; ls <= ls_max; ++ls) {
		s = fli[v][ls];
		s2 = fli2[v][ls];
		// No pauses
		if (!cc[v][s] || !cc[v][s2 + 1] || !cc[v][fli[v][ls + 2]] || !cc[v][fli[v][ls + 3]]) continue;
		// First note must be chord tone
		if (!nih[v][s]) continue;
		// Movement is stepwize
		if (!smooth[v][s2]) continue;
		// Note 1 is short
		if (llen[v][ls] < 2) continue;
		// Note 2 is long
		if (llen[v][ls + 1] > 2) continue;
		if (ls < fli_size[v] - 2) {
			// Leap has same direction
			if (leap[v][fli2[v][ls + 1]] * smooth[v][s2] > 0) continue;
			// Wrong leap
			if (abs(leap[v][fli2[v][ls + 1]]) != 2) continue;
			// Note 2 is longer than 1
			if (llen[v][ls + 1] > llen[v][ls]) continue;
			// Note 3 is long
			if (llen[v][ls + 2] > 2) continue;
			if (ls < fli_size[v] - 3) {
				// Note 4 must be chord tone
				if (!nih[v][fli[v][ls + 3]]) continue;
				// Movements are stepwize
				if (!smooth[v][fli2[v][ls + 2]]) continue;
				// Both movements have same direction
				if (smooth[v][s2] != smooth[v][fli2[v][ls + 2]]) continue;
				// Note 4 is short
				if (llen[v][ls + 3] < 2) continue;
				// Note 1 and 4 are different
				if (cc[v][s] != cc[v][fli[v][ls + 3]]) continue;
				// Note 3 is longer than 4
				if (llen[v][ls + 2] > llen[v][ls + 3] && (ep2 == c_len || ls < fli_size[v] - 4)) continue;
				// Leap in (before DNT)
				if (ls > 0 && leap[v][fli2[v][ls - 1]]) {
					if (!accept[sp][vc][0][3]) FlagA(v, 3, s, s, v, 0);
				}
				if (ls < fli_size[v] - 4) {
					// Leap from note 4
					if (leap[v][fli2[v][ls + 3]]) {
						if (!accept[sp][vc][0][97]) FlagA(v, 97, fli[v][ls + 3], fli[v][ls + 3], v, 0);
					}
					// Apply pattern
					msh[v][fli[v][ls]] = pHarmonicDNT1;
					msh[v][fli[v][ls + 1]] = pAuxDNT1;
					msh[v][fli[v][ls + 2]] = pAuxDNT2;
					msh[v][fli[v][ls + 3]] = pHarmonicDNT2;
				}
			}
		}
	}
}

void CP2R::DetectCambiata() {
	if (!accept[sp][vc][0][256]) return;
	// Suspension will conflict with cambiata
	int ls0 = bli[v][mli[ms]];
	if (sus[v][ls0]) return;
	int ls_max = bli[v][mli[ms] + npm - 1] - 3;
	if (ls_max > fli_size[v] - 4) ls_max = fli_size[v] - 4;
	// Not enough notes for cambiata
	if (ls_max < ls0) return;
	for (ls = ls0; ls <= ls_max; ++ls) {
		s = fli[v][ls];
		s2 = fli2[v][ls];
		// No pauses
		if (!cc[v][s] || !cc[v][s2 + 1] || !cc[v][fli[v][ls + 2]] || !cc[v][fli[v][ls + 3]]) continue;
		// First note must be chord tone
		if (!nih[v][s]) continue;
		// Note 1 is short
		if (llen[v][ls] < 2) continue;
		// Note 2 is long
		if (llen[v][ls + 1] > 2) continue;
		// Movement is stepwize
		if (!smooth[v][s2]) continue;
		if (ls < fli_size[v] - 2) {
			// Note 2 is longer than 1
			if (llen[v][ls + 1] > llen[v][ls]) continue;
			// Note 3 is long
			if (llen[v][ls + 2] > 2) continue;
			// Wrong
			if (abs(leap[v][fli2[v][ls + 1]]) != 2) continue;
			// Leap has other direction
			if (leap[v][fli2[v][ls + 1]] * smooth[v][s2] < 0) continue;
			if (ls < fli_size[v] - 3) {
				// Fourth note must be chord tone
				if (!nih[v][fli[v][ls + 3]]) continue;
				// Note 4 is short
				if (llen[v][ls + 3] < 2) continue;
				// Note 3 is longer than 4
				if (llen[v][ls + 2] > llen[v][ls + 3] && (ep2 == c_len || ls < fli_size[v] - 4)) continue;
				// Both movements have different directions
				if (smooth[v][s2] != -smooth[v][fli2[v][ls + 2]]) continue;
				if (ls < fli_size[v] - 4) {
					// Leap from note 4
					if (abs(leap[v][fli2[v][ls + 3]]) > 3) {
						continue;
					}
					// Apply pattern
					msh[v][fli[v][ls]] = pHarmonicCam1;
					msh[v][fli[v][ls + 1]] = pAuxCam1;
					msh[v][fli[v][ls + 2]] = pAuxCam2;
					msh[v][fli[v][ls + 3]] = pHarmonicCam2;
				}
			}
		}
	}
}

void CP2R::DetectSus() {
	// Skip first measure
	if (!ms) return;
	// Skip pause
	if (!cc[v][s]) return;
	// Note must start before harmony start
	if (fli[v][ls] >= hstart) return;
	msh[v][hstart] = pSusHarm;
	msh[v][fli[v][ls]] = pSusStart;
	// Allow if not discord
	if (nih[v][s]) {
		// Are there enough notes for resolution ornament?
		if (ls < fli_size[v] - 2 && fli2[v][ls + 1] < hend) {
			s3 = fli2[v][ls + 1];
			s4 = fli2[v][ls + 2];
			// Is there a dissonance between two consonances, forming stepwise descending movement?
			if (!nih[v][fli[v][ls + 1]] && nih[v][fli[v][ls + 2]] && c[v][s2] - c[v][s4] == 1 &&
				llen[v][ls + 2] >= 2 && beat[v][ls + 2] < 10) {
				// Detect stepwise+leap or leap+stepwise
				if ((c[v][s3] - c[v][s2] == 1 && c[v][s3] - c[v][s4] == 2) ||
					(c[v][s2] - c[v][s3] == 2 && c[v][s4] - c[v][s3] == 1)) {
					msh[v][fli[v][ls + 1]] = pAux;
				}
			}
		}
		return;
	}
	// Last generated note is sus
	if (ls >= fli_size[v] - 1) {
		return;
	}
	// Full measure should be generated
	if (mli[ms] + npm >= ep2 && ep2 < c_len) {
		return;
	}
	// Available beats
	s3 = hstart + 2;
	s4 = hstart + 4;
	s5 = hstart + 6;
	// For species 2 and 4 check only 3rd beat
	if (sp == 2 || sp == 4) {
		s3 = 0;
		s5 = 0;
	}
	// Check that beats are before or at sus note
	if (s3 && s3 <= s2) s3 = 0;
	if (s4 && s4 <= s2) s4 = 0;
	if (s5 && s5 <= s2) s5 = 0;
	// Check that beats are in measure
	if (s3 && s3 >= mli[ms] + npm) s3 = 0;
	if (s4 && s4 >= mli[ms] + npm) s4 = 0;
	if (s5 && s5 >= mli[ms] + npm) s5 = 0;
	// Check which beats are allowed by rules
	if (!accept[sp][vc][0][419]) s3 = 0;
	if (!accept[sp][vc][0][420]) s5 = 0;
	if (s3) ls3 = bli[v][s3];
	if (s4) ls4 = bli[v][s4];
	if (s5) ls5 = bli[v][s5];
	// Notes not on beat? 
	if (!accept[sp][vc][0][286]) {
		if (s3 && s3 != fli[v][ls3]) s3 = 0;
		if (s4 && s4 != fli[v][ls4]) s4 = 0;
		if (s5 && s5 != fli[v][ls5]) s5 = 0;
		FLAGAR(v, 286, s, s, v, 100);
	}
	// Notes too short?
	if (!accept[sp][vc][0][291]) {
		if (s3 && llen[v][ls3] < 2 && ls3 < fli_size[v] - 1) s3 = 0;
		if (s4 && llen[v][ls4] < 2 && ls4 < fli_size[v] - 1) s4 = 0;
		if (s5 && llen[v][ls5] < 2 && ls5 < fli_size[v] - 1) s5 = 0;
		FLAGAR(v, 291, s, s, v, 100);
	}
	// Notes not harmonic?
	if (!accept[sp][vc][0][220]) {
		if (s3 && !nih[v][s3]) s3 = 0;
		if (s4 && !nih[v][s4]) s4 = 0;
		if (s5 && !nih[v][s5]) s5 = 0;
		FLAGAR(v, 220, s, s, v, 100);
	}
	// Resolution by leap
	if (!accept[sp][vc][0][221]) {
		if (s3 && abs(c[v][s3] - c[v][s2]) > 1) s3 = 0;
		if (s4 && abs(c[v][s4] - c[v][s2]) > 1) s4 = 0;
		if (s5 && abs(c[v][s5] - c[v][s2]) > 1) s5 = 0;
		FLAGAR(v, 221, s, s, v, 100);
	}
	// Resolution up not LT (support only major and melodic minor)
	if (!accept[sp][vc][0][219]) {
		if (s3 && cc[v][s3] > cc[v][s2] && (pcc[v][s2] != 11 || mode == 5)) s3 = 0;
		if (s4 && cc[v][s4] > cc[v][s2] && (pcc[v][s2] != 11 || mode == 5)) s4 = 0;
		if (s5 && cc[v][s5] > cc[v][s2] && (pcc[v][s2] != 11 || mode == 5)) s5 = 0;
		FLAGAR(v, 219, s, s, v, 100);
	}
	// Notes have too many insertions?
	if (!accept[sp][vc][0][292]) {
		if (s3 && ls3 - ls > 3) s3 = 0;
		if (s4 && ls4 - ls > 3) s4 = 0;
		if (s5 && ls5 - ls > 3) s5 = 0;
		FLAGAR(v, 292, s, s, v, 100);
	}
	// First leap is too long?
	if (abs(cc[v][fli[v][ls + 1]] - cc[v][s2]) > sus_insert_max_leap[sp][vc][0]) {
		FlagA(v, 295, fli[v][ls + 1], hstart, v, 100);
	}
	// Single insertion, second movement is leap
	if (!accept[sp][vc][0][296]) {
		if (s3 && ls3 == ls + 2 && leap[v][fli2[v][ls + 1]] > sus_insert_max_leap2[sp][vc][0]) s3 = 0;
		if (s4 && ls4 == ls + 2 && leap[v][fli2[v][ls + 1]] > sus_insert_max_leap2[sp][vc][0]) s4 = 0;
		if (s5 && ls5 == ls + 2 && leap[v][fli2[v][ls + 1]] > sus_insert_max_leap2[sp][vc][0]) s5 = 0;
		FLAGAR(v, 296, s, s, v, 100);
	}
	// Mark insertion as non-harmonic in basic msh if resolution is harmonic, sus ends with dissonance and not both movements are leaps
	if (s3 && ls3 == ls + 2 && nih[v][fli[v][ls + 2]] &&
		leap[v][fli2[v][ls]] * leap[v][fli2[v][ls + 1]] == 0) msh[v][fli[v][ls + 1]] = pAux;
	if (s4 && ls4 == ls + 2 && nih[v][fli[v][ls + 2]] &&
		leap[v][fli2[v][ls]] * leap[v][fli2[v][ls + 1]] == 0) msh[v][fli[v][ls + 1]] = pAux;
	if (s5 && ls5 == ls + 2 && nih[v][fli[v][ls + 2]] &&
		leap[v][fli2[v][ls]] * leap[v][fli2[v][ls + 1]] == 0) msh[v][fli[v][ls + 1]] = pAux;
	// Mark resolution as obligatory harmonic in basic msh and ending as non-harmonic
	// In this case all resolutions are marked, although one of them is enough, but this is a very rare case that several resolutions pass all checks
	if (s3) {
		msh[v][hstart] = pSusNonHarm;
		msh[v][fli[v][ls3]] = pSusRes;
		resol[v][hstart] = fli[v][ls3];
#if defined(_DEBUG)
		WriteLog(3, "Detected sus at s3");
#endif
	}
	if (s4) {
		msh[v][hstart] = pSusNonHarm;
		msh[v][fli[v][ls4]] = pSusRes;
		resol[v][hstart] = fli[v][ls4];
#if defined(_DEBUG)
		WriteLog(3, "Detected sus at s4");
#endif
	}
	if (s5) {
		msh[v][hstart] = pSusNonHarm;
		msh[v][fli[v][ls5]] = pSusRes;
		resol[v][hstart] = fli[v][ls5];
#if defined(_DEBUG)
		WriteLog(3, "Detected sus at s5");
#endif
	}
}

void CP2R::DetectPDD() {
	// First measure
	if (!ms) return;
	if (!cc[v][s]) return;
	// Last note
	if (s2 == ep2 - 1) return;
	// Both notes should be inside measure
	if (s2 >= hend) return;
	// Note must not start before harmony start
	if (fli[v][ls] != hstart) return;
	// Second note must be non-chord tone
	if (nih[v][s]) return;
	// No pauses
	if (!cc[v][s] || !cc[v][s - 1] || !cc[v][s2 + 1]) return;
	// Stepwize downward movement
	if (c[v][s] - c[v][s - 1] != -1) return;
	// Note 2 is not too long
	if (llen[v][ls] > 4) return;
	// Find at least one dissonance
	/*
	int has_dissonance = 0;
	for (v2 = 0; v2 < av_cnt; ++v2) {
		if (v2 == v) continue;
	}
	*/
	if (ls < fli_size[v] - 1) {
		// Stepwise descending movement
		if (c[v][s2 + 1] - c[v][s2] != -1) return;
		// Note 2 is not longer than 3
		if (llen[v][ls] > llen[v][ls + 1] && (ep2 == c_len || ls < fli_size[v] - 2)) return;
		// Third note must be chord note
		if (!nih[v][s2 + 1]) return;
		// Apply pattern
		msh[v][fli[v][ls]] = pAuxPDD;
		msh[v][fli[v][ls + 1]] = pHarmonicPDD;
		resol[v][hstart] = fli[v][ls + 1];
	}
}

int CP2R::FailStartPause() {
	CHECK_READY(DR_nlen);
	// Last measure with starting voice
	int last_start = 0;
	// How many voices start in this measure
	vector<int> mstarts;
	// How many voices start in this measure
	vector<int> vstarts;
	mstarts.resize(mli.size());
	vstarts.resize(mli.size());
	// Prohibit start at the same step
	for (v = 0; v < av_cnt; ++v) {
		sp = vsp[v];
		int count3 = 0;
		if (sp > 1) {
			ms = bmli[fin[v]];
			++mstarts[ms];
			vstarts[ms] = v;
			last_start = max(last_start, ms);
		}
		if (sp == 2 || sp == 4) {
			int count2 = 0;
			for (v2 = v + 1; v2 < av_cnt; ++v2) {
				// Skip other steps
				if (fin[v] != fin[v2]) continue;
				sp2 = vsp[v2];
				if (sp2 == 2 || sp2 == 4) {
					++count2;
					// More than 2 voices in sp2/4 start at same step
					if (count2 > 1) FlagV(v, 530, fin[v]);
				}
				else {
					// Voices start at same step
					FlagV(v, 530, fin[v]);
				}
			}
		}
		else if (sp == 3 || sp == 5) {
			for (v2 = v + 1; v2 < av_cnt; ++v2) {
				// Skip other steps
				if (fin[v] != fin[v2]) continue;
				// Voices start at same step
				FlagV(v, 530, fin[v]);
			}
		}
	}
	v = 0;
	int late_entrance = 0;
	for (ms = 0; ms < mli.size(); ++ms) {
		if (!mstarts[ms]) {
			// Detect empty measure
			if (ms < last_start) late_entrance = 1; 
		}
		else {
			if (late_entrance) {
				late_entrance = 0;
				v = vstarts[ms];
				FlagV(v, 527, mli[ms]);
			}
			if (mstarts[ms] > 2) {
				// Too many voices start
				v = vstarts[ms];
				FlagV(v, 529, mli[ms]);
			}
			else if (mstarts[ms] == 1 && last_start > ms) {
				// Only one voice starts, although there are more voices to start
				v = vstarts[ms];
				FlagV(v, 528, mli[ms]);
			}
		}
	}
	// Prohibit wrong pause length in measure
	for (v = 0; v < av_cnt; ++v) {
		sp = vsp[v];
		if (sp == 0 || sp == 1) {
			// Pauses are prohibited in this species
			if (fin[v] > 0) FlagV(v, 138, fin[v]);
		}
		else if (sp == 2 || sp == 4) {
			// Only halfnote pause is allowed
			if (fin[v] % npm != nlen[v]) FlagV(v, 138, fin[v]);
		}
		else if (sp == 3) {
			// Only particular pauses are allowed
			if (fin[v] % npm != 2) FlagV(v, 138, fin[v]);
		}
		else if (sp == 5) {
			// Only particular pauses are allowed
			if (fin[v] % npm != 2 && fin[v] % npm != 4) FlagV(v, 138, fin[v]);
		}
	}
	return 0;
}

void CP2R::FlagLTUnresolved() {
	CHECK_READY(DR_islt);
	for (v = 0; v < av_cnt; ++v) {
		// Up to penultimate note
		for (ls = 0; ls < fli_size[v] - 1; ++ls) {
			s = fli[v][ls];
			// Skip not lt
			if (!islt[v][s]) continue;
			// Skip pause
			if (!cc[v][s]) continue;
			hs = bhli[s];
			s5 = hli2[hs];
			// Check if note touches harmony end
			if (fli2[v][ls] < s5) continue;
			// Check if this is last harmony
			if (hli.size() <= hs + 1) continue;
			vc = vca[s];
			sp = vsp[v];
			s2 = fli[v][ls + 1];
			// Check if lt has to resolve up (do not check current chord, because we are already working with situations where islt=1)
			if (chm[hs + 1] == 0 || chm[hs + 1] == 5) {
				// Check if lt resolves up
				if (pc[v][s2] != 0) {
					FlagVL(v, 197, s, s2);
				}
			} 
			// Check if lt has to resolve up or down
			else if (chm[hs + 1] == 1 || chm[hs + 1] == 3) {
				// Check if lt resolves up or down
				if (pc[v][s2] != 0 && pc[v][s2] != 5) {
					FlagVL(v, 198, s, s2);
				}
			}
		}
	}
}

void CP2R::FlagHarmTriRes() {
	// Check all voices except bass
	for (v = 1; v < av_cnt; ++v) {
		for (v2 = v + 1; v2 < av_cnt; ++v2) {
			for (s = 0; s < ep2; ++s) {
				ls = bli[v][s];
				ls2 = bli[v2][s];
				// Skip no note start
				if (s != fli[v][ls] && s != fli[v2][ls2]) continue;
				// Skip non-chord tones
				if (!nih[v][ssus[v][ls]]) continue;
				if (!nih[v2][ssus[v2][ls2]]) continue;
				// Skip pauses
				if (!cc[v][s]) continue;
				if (!cc[v2][s]) continue;
				GetVp();
				vc = vca[s];
				// Prepare data
				civl = abs(cc[v][s] - cc[v2][s]);
				// Process only tritones
				if (civl % 12 != 6) continue;
				hs = bhli[s];
				s5 = hli2[hs];
				// Always prohibit harmonic tritone in archaic modes
				if (mode && !mminor) {
					Flag(v, 331, s, v2);
					continue;
				}
				// Check if first note touches harmony end
				if (fli2[v][ls] >= s5) {
					if (ls >= fli_size[v] - 1)
						FlagL(v, 379, fli[v][ls], fli[v][ls], v);
					else if (!GetTriRes(cc[v][s], cc[v][fli[v][ls + 1]]))
						FlagL(v, 379, fli[v][ls], fli[v][ls + 1], v);
				}
				// Check if second note touches harmony end
				if (fli2[v2][ls2] >= s5) {
					if (ls2 >= fli_size[v2] - 1)
						FlagL(v2, 379, fli[v2][ls2], fli[v2][ls2], v2);
					else if (!GetTriRes(cc[v2][s], cc[v2][fli[v2][ls2 + 1]]))
						FlagL(v2, 379, fli[v2][ls2], fli[v2][ls2 + 1], v2);
				}
			}
		}
	}
}

// Return 1 if tritone is resolved or if there are no resolution notes in next chord
int CP2R::GetTriRes(int cc1, int cc2) {
	// Unresolved if this is last harmony
	if (hli.size() <= hs + 1) return 0;
	int cm = chm[hs + 1];
	// Chromatic pitch classes
	int pcc1 = (cc1 - bn + 12) % 12;
	int pcc2 = (cc2 - bn + 12) % 12;
	if (mminor) {
		// CONFIRM
		if (pcc1 == 11) {
			if (pcc2 == 0 || (cm != 0 && cm != 3 && cm != 5)) return 1;
			else return 0;
		}
		if (pcc1 == 9) {
			if (pcc2 == 10 || (cm != 2 && cm != 4 && cm != 6)) return 1;
			else return 0;
		}
		if (pcc1 == 8) {
			if (pcc2 == 7 || (cm != 0 && cm != 2 && cm != 4)) return 1;
			else return 0;
		}
		if (pcc1 == 5) {
			if (pcc2 == 3 || (cm != 0 && cm != 2 && cm != 5)) return 1;
			else return 0;
		}
		if (pcc1 == 3) {
			if (pcc2 == 2 || (cm != 1 && cm != 6 && cm != 4)) return 1;
			else return 0;
		}
		if (pcc1 == 2) {
			if (pcc2 == 3 || (cm != 0 && cm != 2 && cm != 5)) return 1;
			else return 0;
		}
	}
	else {
		if (pcc1 == 11) {
			if (pcc2 == 0 || (cm != 0 && cm != 3 && cm != 5)) return 1;
			else return 0;
		}
		if (pcc1 == 5) {
			if (pcc2 == 4 || (cm != 0 && cm != 2 && cm != 5)) return 1;
			else return 0;
		}
	}
	return 0;
}

void CP2R::FlagLTDouble() {
	for (v = 0; v < av_cnt; ++v) {
		for (v2 = v + 1; v2 < av_cnt; ++v2) {
			for (s = 0; s < ep2; ++s) {
				// Skip not octave / unison
				if (pcc[v][s] != pcc[v2][s]) continue;
				ls = bli[v][s];
				ls2 = bli[v2][s];
				// Skip no note start
				if (s != fli[v][ls] && s != fli[v2][ls2]) continue;
				// Skip pauses
				if (!cc[v][s]) continue;
				if (!cc[v2][s]) continue;
				GetVp();
				vc = vca[s];
				// Skip if both are not leading tones
				if (!islt[v][fli[v][ls]] && !islt[v2][fli[v2][ls2]]) continue;
				if (fli[v][ls] < fli[v2][ls2]) {
					Flag(v2, 324, fli[v2][ls2], v);
				}
				else {
					Flag(v, 324, fli[v][ls], v2);
				}
			}
		}
	}
}

void CP2R::GetChordTones() {
	for (hs = 0; hs < hli.size(); ++hs) {
		GetHarmNotes(chm[hs], chm_alter[hs], cct[hs]);
	}
}

void CP2R::GetHarmNotes(int lchm, int lchm_alter, vector<int> &lcct) {
	lcct[0] = (c_cc[lchm + 7] - bn + 12) % 12;
	lcct[1] = (c_cc[lchm + 9] - bn + 12) % 12;
	lcct[2] = (c_cc[lchm + 11] - bn + 12) % 12;
	if (lchm_alter) {
		if (lcct[0] == 8) lcct[0] = 9;
		else if (lcct[0] == 10) lcct[0] = 11;
		if (lcct[1] == 8) lcct[1] = 9;
		else if (lcct[1] == 10) lcct[1] = 11;
		if (lcct[2] == 8) lcct[2] = 9;
		else if (lcct[2] == 10) lcct[2] = 11;
	}
}

void CP2R::FlagTriDouble() {
	for (hs = 0; hs < hli.size(); ++hs) {
		// Skip chords without tritone
		if ((cct[hs][2] - cct[hs][0] + 12) % 12 != 6) continue;
		// Check if both notes of this tritone exist in voices
		int found0 = 0;
		int found2 = 0;
		for (v = 0; v < av_cnt; ++v) {
			ls2 = bli[v][hli2[hs]];
			for (ls = bli[v][hli[hs]]; ls <= ls2; ++ls) {
				s = fli[v][ls];
				if (pcc[v][s] == cct[hs][0]) found0 = 1;
				else if (pcc[v][s] == cct[hs][2]) found2 = 1;
			}
		}
		if (!found0 || !found2) continue;
		// Find duplication of one of tritone notes
		for (v = 0; v < av_cnt; ++v) {
			sp = vsp[v];
			for (v2 = v + 1; v2 < av_cnt; ++v2) {
				for (s = 0; s < ep2; ++s) {
					// Skip not octave / unison
					if (pcc[v][s] != pcc[v2][s]) continue;
					ls = bli[v][s];
					ls2 = bli[v2][s];
					// Skip no note start
					if (s != fli[v][ls] && s != fli[v2][ls2]) continue;
					// Skip if note is not tritone note
					if (pcc[v][s] != cct[hs][0] && pcc[v][s] != cct[hs][2]) continue;
					// Skip pauses
					if (!cc[v][s]) continue;
					if (!cc[v2][s]) continue;
					GetVp();
					vc = vca[s];
					if (fli[v][ls] < fli[v2][ls2]) {
						Flag(v2, 222, fli[v2][ls2], v);
					}
					else {
						Flag(v, 222, fli[v][ls], v2);
					}
				}
			}
		}
	}
}

void CP2R::FlagPcoApart() {
	CHECK_READY(DR_nih);
	for (v = 0; v < av_cnt; ++v) {
		for (v2 = v + 1; v2 < av_cnt; ++v2) {
			for (s = 0; s < ep2; ++s) {
				ls = bli[v][s];
				ls2 = bli[v2][s];
				// Skip no note start
				if (s != fli[v][ls] && s != fli[v2][ls2]) continue;
				// Skip pauses
				if (!cc[v][s]) continue;
				if (!cc[v2][s]) continue;
				civl = abs(cc[v][s] - cc[v2][s]);
				// Skip not octave / unison / 5th
				if (civl % 12 != 0 && civl % 12 != 7) continue;
				GetVp();
				vc = vca[s];
				// Get interval end
				int iend = min(fli2[v][ls], fli2[v2][ls2]);
				// Scan for second interval
				int scan_end = iend + 8;
				if (scan_end > ep2) scan_end = ep2;
				for (s2 = iend + 2; s2 < scan_end; ++s2) {
					ls3 = bli[v][s2];
					ls4 = bli[v2][s2];
					// Skip no note start
					if (s2 != fli[v][ls3] && s2 != fli[v2][ls4]) continue;
					// Skip pauses
					if (!cc[v][s2]) continue;
					if (!cc[v2][s2]) continue;
					civl2 = abs(cc[v][s2] - cc[v2][s2]);
					// Skip different interval
					if (civl2 % 12 != civl % 12) continue;
					int is_contrary = (cc[v][s2] - cc[v][s]) * (cc[v2][s2] - cc[v2][s]) < 0;
					// Last contrary
					if (ls3 == fli_size[v] - 1 && ls4 == fli_size[v2] - 1 &&
						is_contrary) {
						if (civl % 12 == 0) FlagL(v, 485, s2, s, v2);
						else FlagL(v, 376, s2, s, v2);
					}
					// Downbeat
					else if (s2 % npm == 0) {
						// Suspension
						if (sus[v][ls3] == s2) {
							if (civl % 12 == 0) FlagL(v, 491, s2, s, v2);
							else FlagL(v, 385, s2, s, v2);
						}
						else if (sus[v2][ls4] == s2) {
							if (civl % 12 == 0) FlagL(v, 491, s2, s, v2);
							else FlagL(v, 385, s2, s, v2);
						}
						// Normal downbeat or anticipation
						else {
							if (civl % 12 == 0) FlagL(v, 490, s2, s, v2);
							else FlagL(v, 316, s2, s, v2);
						}
					}
					// Upbeat
					else {
						int is_oblique = (s != fli[v][ls] || s != fli[v2][ls2]);
						// Oblique contrary
						if (is_oblique && is_contrary) {
							if (civl % 12 == 0) FlagL(v, 484, s2, s, v2);
							else FlagL(v, 248, s2, s, v2);
						}
						// Oblique nct in sp3/5
						else if ((vsp[v] == 3 || vsp[v] == 5 || vsp[v2] == 3 || vsp[v2] == 5) && 
							is_oblique &&	(msh[v][fli[v][ls]] < 0 || msh[v2][fli[v2][ls2]] < 0)) {
							if (civl % 12 == 0) FlagL(v, 488, s2, s, v2);
							else FlagL(v, 249, s2, s, v2);
						}
						// Other upbeat
						else {
							if (civl % 12 == 0) FlagL(v, 492, s2, s, v2);
							else FlagL(v, 250, s2, s, v2);
						}
					}
				}
			}
		}
	}
}

