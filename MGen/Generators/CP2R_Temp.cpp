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

int CP2R::EvaluateCP() {
	CLEAR_READY();
	ClearFlags(0, c_len);
	GetDiatonic(0, c_len);
	GetPitchClass(0, c_len);
	CreateLinks();
	GetNoteTypes();
	GetVca();
	GetLClimax();
	GetLeapSmooth();
	for (v = 0; v < av_cnt; ++v) {
		sp = vsp[v];
		vaccept = &accept[sp][av_cnt][0];
		GetMelodyInterval(0, c_len);
		FailStartPause();
		if (FailNoteLen()) return 1;
		if (FailBeat()) return 1;
		if (av_cnt == 1) {
			if (FailNoteRepeat()) return 1;
			if (FailFirstNotes()) return 1;
			if (FailLastNotes()) return 1;
		}
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
		if (FailLastNoteRes()) return 1;
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
		if (mminor) {
			if (FailMinor()) return 1;
			if (FailGisTrail()) return 1;
			if (FailFisTrail()) return 1;
		}
		GetBasicMsh();
		ApplyFixedPat();
		if (mminor) {
			if (FailMinorStepwise()) return 1;
		}
		GetDtp();
		if (FailLeap()) return 1;
		MakeMacc();
		if (FailLocalMacc(notes_arange[sp][av_cnt][0], min_arange[sp][av_cnt][0] / 10.0, 15)) return 1;
		if (FailLocalMacc(notes_arange2[sp][av_cnt][0], min_arange2[sp][av_cnt][0] / 10.0, 16)) return 1;
	}
	return 0;
}

void CP2R::CreateLinks() {
	SET_READY(DR_fli);
	// Set first steps in case there is pause
	for (int v = 0; v < av_cnt; ++v) {
		// Search for first note
		fin[v] = 0;
		for (s = 0; s < ep2; ++s) {
			if (cc[v][s]) {
				fin[v] = s;
				break;
			}
		}
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
		if (!i) {
			if (!accept[sp][vc][vp][fl]) st = "- ";
			else if (accept[sp][vc][vp][fl] == -1) st = "$ ";
			else st = "+ ";
			com = st + GetRuleName(fl, sp, vc, vp) + " (" + GetSubRuleName(fl, sp, vc, vp) + ")";
			if (show_severity) {
				st.Format(" [%d/%d]", fl, severity[sp][vc][vp][fl]);
				com += st;
			}
			if (GetRuleComment(fl, sp, vc, vp) != "") com += ". " + GetRuleComment(fl, sp, vc, vp);
			if (GetSubRuleComment(fl, sp, vc, vp) != "") com += " (" + GetSubRuleComment(fl, sp, vc, vp) + ")";
			//com += ". ";
			comment[pos][v].push_back(com);
			ccolor[pos][v].push_back(sev_color[severity[sp][vc][vp][fl]]);
		}
		// Set note color if this is maximum flag severity
		if (severity[sp][vc][vp][fl] > current_severity && severity[sp][vc][vp][fl] >= show_min_severity
			&& ruleinfo[fl].viz != vHarm) {
			current_severity = severity[sp][vc][vp][fl];
			color[pos + i][v] = sev_color[severity[sp][vc][vp][fl]];
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
				if (v % 2) {
					lining[step0 + s][v] = HatchStyleDiagonalCross;
				}
				len[step0 + s][vi] = llen[v][ls];
				coff[step0 + s][vi] = s - fli[v][ls];
				tempo[step0 + s] = cp_tempo;
				SendComment(step0 + fli[v][ls], s, s - fli[v][ls]);
			}
		}
		MergeNotes(step0, step0 + full_len - 1);
		st.Format("#%d (from %s)",
			cp_id + 1, bname_from_path(musicxml_file));
		AddMelody(step0, step0 + full_len - 1, vi, st);
	}
	for (int s = step0 + real_len; s < step0 + full_len; ++s) tempo[s] = tempo[s - 1];
	CountOff(step0, step0 + full_len - 1);
	CountTime(step0, step0 + full_len - 1);
	UpdateNoteMinMax(step0, step0 + full_len - 1);
	UpdateTempoMinMax(step0, step0 + full_len - 1);
	t_generated = step0 + full_len - 1;
	Adapt(step0, t_generated);
	t_sent = t_generated;
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
		FLAGVL(flag3, fli[v][leap_sum_i + 1], fli[v][max(0, leap_sum_i - mleapsteps)]);
	else if (win_leaps > mleaps)
		FLAGVL(flag1, fli[v][leap_sum_i + 1], fli[v][max(0, leap_sum_i - mleapsteps)]);
	if (win_leapnotes > mleaped2)
		FLAGVL(flag4, fli[v][leap_sum_i + 1], fli[v][max(0, leap_sum_i - mleapsteps)]);
	else if (win_leapnotes > mleaped)
		FLAGVL(flag2, fli[v][leap_sum_i + 1], fli[v][max(0, leap_sum_i - mleapsteps)]);
	return 0;
}

void CP2R::ClearFlags(int step1, int step2) {
	for (v = 0; v < av_cnt; ++v) {
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
				c[v][s] = CC_C(cc[v][s], bn, mode);
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
			if (c[v][i + 1] - c[v][i] > 1) leap[v][i] = 1;
			else if (c[v][i + 1] - c[v][i] < -1) leap[v][i] = -1;
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
		if (cc[v][s1] - cc[v][s] == 8) FLAGV(175, s0);
		else if (cc[v][s1] - cc[v][s] == -8) FLAGV(181, s0);
		else if (cc[v][s1] - cc[v][s] == 9) FLAGV(176, s0);
		else if (cc[v][s1] - cc[v][s] == -9) FLAGV(182, s0);
		else if (cc[v][s1] - cc[v][s] == 10) FLAGV(177, s0);
		else if (cc[v][s1] - cc[v][s] == -10) FLAGV(183, s0);
		else if (cc[v][s1] - cc[v][s] == 11) FLAGV(178, s0);
		else if (cc[v][s1] - cc[v][s] == -11) FLAGV(184, s0);
		else if (cc[v][s1] - cc[v][s] == 12) FLAGV(179, s0);
		else if (cc[v][s1] - cc[v][s] == -12) FLAGV(185, s0);
		else if (cc[v][s1] - cc[v][s] > 12) FLAGV(180, s0);
		else if (cc[v][s1] - cc[v][s] < -12) FLAGV(186, s0);
		// Prohibit BB
		if (pcc[v][fli[v][ls + 1]] == 11 && pcc[v][s] == 11) FLAGV(348, s0);
	}
	return 0;
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
			else if (sm == 4) beat[v][ls] = 1;
			else if (sm == 5) beat[v][ls] = 12;
			else if (sm == 6) beat[v][ls] = 4;
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
			// Build isus
			isus[v][ls] = sus[v][ls] ? sus[v][ls] : fli[v][ls];
		}
	}
}

int CP2R::FailGisTrail() {
	CHECK_READY(DR_fli, DR_pc);
	int gis_trail = 0;
	int _gis_trail_max = gis_trail_max[sp][av_cnt][0];
	for (ls = 0; ls < fli_size[v]; ++ls) {
		s = fli[v][ls];
		if (cc[v][s]) {
			if (pcc[v][s] == 11) {
				// Set to maximum on new G# note
				gis_trail = _gis_trail_max;
			}
			else {
				if (pcc[v][s] == 10) {
					// Prohibit G note close to G#
					if (gis_trail) FLAGVL(200, s, fli[v][max(0, ls - _gis_trail_max + gis_trail)]);
				}
			}
		}
		// Decrease if not zero
		if (gis_trail) --gis_trail;
	}
	return 0;
}

int CP2R::FailFisTrail() {
	CHECK_READY(DR_fli, DR_pc);
	int pos1, pos2, found;
	int _fis_gis_max = fis_gis_max[sp][av_cnt][0];
	int _fis_g_max = fis_g_max[sp][av_cnt][0];
	int _fis_g_max2 = fis_g_max2[sp][av_cnt][0];
	for (ls = 0; ls < fli_size[v]; ++ls) {
		s = fli[v][ls];
		if (cc[v][s] && pcc[v][s] == 9) {
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
				if (ls + _fis_gis_max <= fli_size[v] - 1 || ep2 == c_len)	FLAGVL(199, s, s);
			}
			// Find VII before
			pos1 = max(0, ls - _fis_g_max);
			for (int x = pos1; x < ls; ++x) {
				s2 = fli[v][x];
				if (cc[v][s2] && pcc[v][s2] == 10) {
					FLAGVL(349, s, s2);
					break;
				}
			}
			// Find VII after
			pos2 = min(fli_size[v] - 1, ls + _fis_g_max2);
			for (int x = ls + 1; x <= pos2; ++x) {
				s2 = fli[v][x];
				if (cc[v][s2] && pcc[v][s2] == 10) {
					FLAGVL(350, s, s);
					break;
				}
			}
		}
	}
	return 0;
}

int CP2R::FailMinor() {
	CHECK_READY(DR_pc, DR_fli);
	for (ls = 1; ls < fli_size[v]; ++ls) {
		s = fli[v][ls];
		s_1 = fli[v][ls - 1];
		// Prohibit minor second up before VII - absorbed
		// Prohibit augmented second up before VII - absorbed
		// Prohibit unaltered VI or VII two steps from altered VI or VII
		if (pcc[v][s] == 11) {
			if (pcc[v][s_1] == 10) FLAGVL(153, s_1, s);
			if (pcc[v][s_1] == 8) FLAGVL(154, s_1, s);
			if (pcc[v][s_1] == 3) {
				if (ls < fli_size[v] - 1) {
					// III-VII#-I downward
					if (pcc[v][fli[v][ls + 1]] == 0 && cc[v][s] - cc[v][s_1] < 0) FLAGVL(432, s_1, fli[v][ls + 1]);
					// III-VII#
					else FLAGVL(157, s_1, s);
				}
				else {
					if (ep2 == c_len) FLAGVL(157, s_1, s);
				}
			}
			if (ls > 1) {
				s_2 = fli[v][ls - 2];
				if (pcc[v][s_2] == 10) FLAGVL(159, s_2, s);
				if (pcc[v][s_2] == 8) FLAGVL(160, s_2, s);
				if (pcc[v][s_2] == 3) FLAGVL(163, s_2, s);
			}
			if (ls < fli_size[v] - 1) {
				s1 = fli[v][ls + 1];
				if (pcc[v][s1] == 10) FLAGVL(153, s1, s);
				if (pcc[v][s1] == 8) FLAGVL(154, s1, s);
				if (pcc[v][s1] == 3) FLAGVL(156, s1, s);
				if (ls < fli_size[v] - 2) {
					s2 = fli[v][ls + 2];
					if (pcc[v][s2] == 10) FLAGVL(159, s2, s);
					if (pcc[v][s2] == 8) FLAGVL(160, s2, s);
					if (pcc[v][s2] == 3) FLAGVL(162, s2, s);
				}
			}
		}
		if (pcc[v][s] == 9) {
			if (pcc[v][s_1] == 8) FLAGVL(152, s_1, s);
			if (pcc[v][s_1] == 3) FLAGVL(155, s_1, s);
			if (ls > 1) {
				s_2 = fli[v][ls - 2];
				if (pcc[v][s_2] == 8) FLAGVL(158, s_2, s);
				if (pcc[v][s_2] == 3) FLAGVL(161, s_2, s);
			}
			if (ls < fli_size[v] - 1) {
				s1 = fli[v][ls + 1];
				if (pcc[v][s1] == 8) FLAGVL(152, s1, s);
				if (pcc[v][s1] == 3) FLAGVL(155, s1, s);
				if (ls < fli_size[v] - 2) {
					s2 = fli[v][ls + 2];
					if (pcc[v][s2] == 8) FLAGVL(158, s2, s);
					if (pcc[v][s2] == 3) FLAGVL(161, s2, s);
				}
			}
		}
		// Prohibit unresolved minor tritone DG# (direct or with inserted note)
	}
	return 0;
}

int CP2R::FailMinorStepwise() {
	CHECK_READY(DR_pc, DR_fli);
	CHECK_READY(DR_msh);
	// For non-border notes only, because border notes have their own rules
	for (ls = 1; ls < fli_size[v] - 1; ++ls) {
		s = fli[v][ls];
		s_1 = fli[v][ls - 1];
		s1 = fli[v][ls + 1];
		// Prohibit harmonic VI# not stepwize ascending
		if ((sp < 2 || msh[v][ls] > 0) && pcc[v][s] == 9 &&
			(c[v][s] - c[v][s_1] != 1 || c[v][s1] - c[v][s] != 1))
			FLAGVL(201, s_1, s1);
		// Prohibit harmonic VII natural not stepwize descending
		if ((sp < 2 || msh[v][ls] > 0) && pcc[v][s] == 10 &&
			(c[v][s] - c[v][s_1] != -1 || c[v][s1] - c[v][s] != -1))
			FLAGVL(202, s_1, s1);
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

void CP2R::GetBasicMsh() {
	CHECK_READY(DR_c, DR_fli, DR_leap);
	SET_READY(DR_mshb);
	// First note is always downbeat
	mshb[v][0] = pDownbeat;
	// Main calculation
	for (ls = 1; ls < fli_size[v]; ++ls) {
		s = fli[v][ls];
		s2 = fli2[v][ls];
		if (s % npm == 0) mshb[v][ls] = pDownbeat;
		else if (s > 0 && leap[v][s - 1]) mshb[v][ls] = pLeapTo;
		else if (s2 < ep2 - 1 && leap[v][s2]) mshb[v][ls] = pLeapFrom;
		else {
			if (s > 0 && s2 < ep2 - 1 && c[v][s - 1] == c[v][s2 + 1]) mshb[v][ls] = pAux;
			else mshb[v][ls] = pPass;
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

void CP2R::ApplyFixedPat() {
	CHECK_READY(DR_mshb);
	CHECK_READY(DR_fli);
	SET_READY(DR_msh);
	for (int ls = 0; ls < fli_size[v]; ++ls) msh[v][ls] = mshb[v][ls];
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
				FLAGVL(505, fli[v][fleap_start], fli[v][bli[v][leap_end] + 1]);
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
				FLAGVL(504, fli[v][fleap_start - 1], fli[v][fleap_start + 1]);
			}
			// If there is one more third backward (3 x 3rds total) - do not flag because it was already flagged
			else if (fleap_start > 1 && abs(c[v][leap_end] - c[v][fli2[v][fleap_start - 2]]) == 6) {
			}
			else FLAGVL(503, fli[v][fleap_start - 1], fli[v][fleap_start + 1]);
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
			if (leap_size2 > 5) FLAGV(22, fli[v][bli[v][leap_end]]);
			// Flag if back leap equal or smaller than 6th
			else FLAGV(8, fli[v][bli[v][leap_end]]);
			// Flag leap back overflow
			if (leap_size2 > leap_size) {
				FLAGV(58, fli[v][bli[v][leap_end]]);
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
				FLAGVL(1, fli[v][fleap_start], fli[v][fleap_end]);
				return 0;
			}
			if (child_leap && accept[sp][av_cnt][0][116 + leap_id]) 
				FLAGVL(116 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
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
						FLAGVL(204 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
					else FLAGVL(112 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
				}
				else
					FLAGVL(124 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
			}
		}
		// Show compensation flags only if successfully compensated
		// This means that compensation errors are not shown if uncompensated (successfully or not)
		else {
			// Flag late uncompensated precompensated leap
			if (fill_to >= 3 && fill_to < fill_pre4_int[sp][av_cnt][0] && late_leap)
				FLAGVL(144 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
			else if (fill_from >= 3 && fill_from < fill_pre4_int[sp][av_cnt][0] && late_leap)
				FLAGVL(144 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
			// Flag prepared unfinished fill if it is not blocking 
			else if (fill_to == 2 && (fill_to_pre < 2 || !fleap_start)) FLAGVL(100 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
			// Flag unfinished fill if it is not blocking
			else if (fill_to == 2 && fill_to_pre > 1) FLAGVL(104 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
			// Flag after 3rd if it is not blocking
			if (fill_from == 2) FLAGVL(53 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
			// Flag deviation if it is not blocking
			if (deviates == 1) FLAGVL(42 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
			// Flag deviation if it is not blocking
			if (deviates == 2) FLAGVL(120 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
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
			FLAGVL(510 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
		else FLAGVL(128 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
	}
	// Close + far
	else if (!mdc1 && mdc2 == 2) FLAGVL(140 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
	// Close + no
	else if (!mdc1 && mdc2 == 3) FLAGVL(108 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
	// next + close
	else if (mdc1 == 1 && !mdc2) {
		if (sp < 2 || bmli[fli[v][fleap_end]] == bmli[leap_start]) {
			// Aux + close
			if ((sp == 3 || sp == 5) && fleap_start > 2 && cc[v][fli[v][fleap_start - 2]] == cc[v][leap_start] &&
				(cc[v][fli[v][fleap_start - 2]] - cc[v][fli[v][fleap_start - 3]]) * leap[v][leap_start] < 0)
				FLAGVL(506 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
			else FLAGVL(59 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
		}
		else FLAGVL(476 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
	}
	// Far + close
	else if (mdc1 == 2 && !mdc2) {
		if (sp < 2 || bmli[fli[v][fleap_end]] == bmli[leap_start]) 
			FLAGVL(132 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
		else FLAGVL(25 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
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
				FLAGVL(414 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
			else FLAGVL(63 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
		}
		else FLAGVL(460 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
	}
	// Next + far
	else if (mdc1 == 1 && mdc2 >= 2) {
		if (sp < 2 || bmli[fli[v][fleap_end]] == bmli[leap_start]) FLAGVL(391 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
		else FLAGVL(464 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
	}
	// Far + next
	else if (mdc1 >= 2 && mdc2 == 1) {
		if (sp < 2 || bmli[fli[v][fleap_end]] == bmli[leap_start]) FLAGVL(148 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
		else FLAGVL(468 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
	}
	// Far + far
	else if (mdc1 >= 2 && mdc2 >= 2) {
		if (sp < 2 || bmli[fli[v][fleap_end]] == bmli[leap_start]) FLAGVL(398 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
		else FLAGVL(472 + leap_id, fli[v][fleap_start], fli[v][fleap_end]);
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
	nmind[v] = CC_C(nmin[v], bn, mode);
	nmaxd[v] = CC_C(nmax[v], bn, mode);
}

void CP2R::ValidateFlags() {
	for (v = 0; v < av_cnt; ++v) {
		for (s = 0; s < c_len; ++s) {
			if (flag[v][s].size()) {
				// Note start is ok
				if (s == fli[v][bli[v][s]]) continue;
				// Downbeat is ok
				if (!(s % npm)) continue;
				for (int f = 0; f < flag[v][s].size(); ++f) {
					GetFlag(f);
					CString est;
					est.Format("Detected flag at hidden position %d/%d: [%d] %s %s (%s)",
						s, fsl[v][s][f], fl, accept[sp][vc][vp][fl] ? "+" : "-",
						GetRuleName(fl, sp, vc, vp), GetSubRuleName(fl, sp, vc, vp));
					WriteLog(5, est);
				}
			}
		}
	}
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
					FLAGVL(flag1, fli[v][ls + 1], fli[v][ls - smooth_sum + 1]);
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
						FLAGVL(flag2, fli[v][ls + 1], fli[v][ls - smooth_sum2 + 1]);
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
							FLAGVL(402, fli[v][ls - 1], fli[v][ls + 2]);
						}
						else FLAGVL(403, fli[v][ls - 1], fli[v][ls + 2]);
					}
					else FLAGVL(404, fli[v][ls - 1], fli[v][ls + 2]);
				}
				// For species 2 / 3 / 5 check measure
				else {
					// Inside measure?
					if (bmli[fli[v][ls - 1]] == bmli[fli[v][ls + 2]]) {
						// Same rhythm in first notes of repeat?
						if (llen[v][ls - 1] == llen[v][ls + 1]) {
							if (llen[v][ls - 1] == llen[v][ls]) {
								FLAGVL(411, fli[v][ls - 1], fli[v][ls + 2]);
							}
							else FLAGVL(412, fli[v][ls - 1], fli[v][ls + 2]);
						}
						else FLAGVL(413, fli[v][ls - 1], fli[v][ls + 2]);
					}
					else {
						// Same rhythm in first notes of repeat?
						if (llen[v][ls - 1] == llen[v][ls + 1]) {
							if (llen[v][ls - 1] == llen[v][ls]) {
								FLAGVL(402, fli[v][ls - 1], fli[v][ls + 2]);
							}
							else FLAGVL(403, fli[v][ls - 1], fli[v][ls + 2]);
						}
						else FLAGVL(404, fli[v][ls - 1], fli[v][ls + 2]);
					}
				}
			}
		}
	}
	if (first_run && max_leap_sum2 >= csel) {
		if (max_leap_sum2 > csel2)
			FLAGVL(flag4, fli[v][bli[v][leap_sum_s2] + 1], 
				fli[v][max(0, bli[v][leap_sum_s2] - max_leap_sum3 + 1)]);
		else 
			FLAGVL(flag3, fli[v][bli[v][leap_sum_s2] + 1], 
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
	for (ls = 0; ls < fli_size[v]; ++ls) {
		s = fli[v][ls];
		// Add new note to stagnation array
		++nstat[cc[v][s]];
		// Subtract old note
		if (ls >= steps) --nstat[cc[v][fli[v][ls - steps]]];
		// Check if too many repeating notes
		if (nstat[cc[v][s]] > notes) FLAGVL(fl, s, fli[v][max(0, ls - steps)]);
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
						FLAGV(12, fli[v][culm_ls]);
					}
				}
				else {
					if (multi_culm_fired) fpenalty += severity[sp][av_cnt][0][305] + 1;
					else {
						multi_culm_fired = 1;
						FLAGV(305, fli[v][culm_ls]);
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
		if (culm_ls < (early_culm3[sp][av_cnt][0] * fli_size[v]) / 100) FLAGV(193, fli[v][culm_ls]);
		if (culm_ls < early_culm[sp][av_cnt][0] - 1) FLAGV(78, fli[v][culm_ls]);
		else if (culm_ls < early_culm2[sp][av_cnt][0] - 1) FLAGV(79, fli[v][culm_ls]);
		// Prohibit culminations at last steps
		if (culm_ls >= fli_size[v] - late_culm[sp][av_cnt][0]) FLAGV(21, fli[v][culm_ls]);
	}
	return 0;
}

int CP2R::FailFirstNotes() {
	CHECK_READY(DR_fli, DR_pc);
	// Prohibit first note not tonic
	if (pc[v][fin[v]] != 0) {
		FLAGV(33, fin[v]);
	}
	return 0;
}

int CP2R::FailLastNotes() {
	CHECK_READY(DR_fli, DR_pc);
	// Do not check if melody is short yet
	if (fli_size[v] < 3) return 0;
	// Prohibit last note not tonic
	if (ep2 == c_len) {
		s = fli[v][fli_size[v] - 1];
		s_1 = fli[v][fli_size[v] - 2];
		s_2 = fli[v][fli_size[v] - 3];
		if (pc[v][s] != 0) FLAGV(50, s);
		if (mode == 9) {
			// Prohibit major second up before I
			if (pcc[v][s] == 0 && pcc[v][s_1] == 10) FLAGV(74, s_1, s);
			if (pcc[v][s] == 0 && pcc[v][s_2] == 10) FLAGV(74, s_2, s);
		}
	}
	return 0;
}

// Search for adjacent or symmetric repeats
int CP2R::FailAdSymRepeat(int relen) {
	CHECK_READY(DR_fli, DR_c);
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
		else if (sp > 1 && ((fli[v][ls + relen] - fli[v][ls]) % 4)) {
			for (int x = 1; x < 4; ++x) {
				if (ls + x <= fli_size[v] - relen * 2 && !((fli[v][ls + relen + x] - fli[v][ls]) % 4)) {
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
			else if (sp > 1 && ((fli[v][rpos1 + relen] - fli[v][rpos1]) % 4)) {
				for (int x = 1; x < 4; ++x) {
					if (rpos1 + x <= fli_size[v] - relen * 2 && !((fli[v][rpos1 + relen + x] - fli[v][rpos1]) % 4)) {
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
					FLAGVL(313, fli[v][ls], fli[v][ls + relen - 1]);
				else
					FLAGVL(407, fli[v][ls], fli[v][ls + relen - 1]);
			}
			// Flag one repeat
			else if (rpos1) {
				if (rpos1 == ls + relen)
					FLAGVL(311, fli[v][ls], fli[v][ls + relen - 1]);
				else
					FLAGVL(405, fli[v][ls], fli[v][ls + relen - 1]);
			}
		}
		if (relen == 4) {
			// Flag two repeats
			if (rpos2) {
				if (rpos2 == rpos1 + relen)
					FLAGVL(314, fli[v][ls], fli[v][ls + relen - 1]);
				else
					FLAGVL(408, fli[v][ls], fli[v][ls + relen - 1]);
			}
			// Flag one repeat
			else if (rpos1) {
				if (rpos1 == ls + relen)
					FLAGVL(312, fli[v][ls], fli[v][ls + relen - 1]);
				else
					FLAGVL(406, fli[v][ls], fli[v][ls + relen - 1]);
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
				FLAGVL(fl, fli[v][ls], fli[v][ls_max2 - 1]);
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
				FLAGVL(fl, fli[v][ls], fli[v][ls_max2 - 1]);
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
					FLAGVL(fl, fli[v][ls - notes + 1], s);
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
					FLAGVL(70 + tt, s, fli[v][max(0, ls - tw)]);
					fired = 1;
				}
			}
		}
		tweight[v][ls] = vmax(tcount);
	}
	return 0;
}

int CP2R::FailLastNoteRes() {
	CHECK_READY(DR_pc, DR_fli);
	if (ep2 < c_len) return 0;
	if (fli_size[v] < 2) return 0;
	if (pc[v][fli[v][fli_size[v] - 2]] == 6 && pc[v][c_len - 1] != 0) FLAGV(52, fli[v][fli_size[v] - 2]);
	if (pc[v][fli[v][fli_size[v] - 2]] == 3 && pc[v][c_len - 1] != 2) FLAGV(87, fli[v][fli_size[v] - 2]);
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
		if (found == 1) FLAGVL(32, s0, fli[v][ls + 1]);
		// Compound framed
		else if (found == 2) FLAGVL(373, fli[v][ls - 1], fli[v][ls + 1]); //-V547
		}
		}
		*/
		// Check if resolution is correct
		GetTritoneResolution(ta, t1, t2, tb, res1, res2);
		// Flag resolution for consecutive tritone
		if (found == 1) {
			if (res1*res2 == 0)
				FLAGVL(31, s0, fli[v][ls + 1]);
			else FLAGVL(2, s0, fli[v][ls + 1]);
		}
		// Flag resolution for tritone with intermediate note, framed
		else if (found == 2) { //-V547
			if (res1*res2 == 0) FLAGVL(372, fli[v][ls - 1], fli[v][ls + 1]);
			else FLAGVL(371, fli[v][ls - 1], fli[v][ls + 1]);
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
	if (skips2) FLAGV(69, fin[v]);
	if (skips == 1) FLAGV(67, fin[v]);
	else if (skips >= 2) FLAGV(68, fin[v]);
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
			if (found == 0) FLAGVL(370, fli[v][fleap_start], fli[v][fleap_end]);
			else if (found == 1) FLAGVL(367, fli[v][fleap_start], fli[v][fleap_end]);
			else FLAGVL(362, fli[v][fleap_start], fli[v][fleap_end]);
		}
	}
	*/
	GetTritoneResolution(ta, t1, t2, tb, res1, res2);
	// Flag resolution for normal tritone
	if (found == 0) {
		if (res1*res2 == 0) FLAGVL(369, fli[v][fleap_start], fli[v][fleap_end]);
		else FLAGVL(368, fli[v][fleap_start], fli[v][fleap_end]);
	}
	// Flag resolution for framed tritone
	else if (found == 1) {
		if (res1*res2 == 0) FLAGVL(366, fli[v][fleap_start], fli[v][fleap_end]);
		else FLAGVL(365, fli[v][fleap_start], fli[v][fleap_end]);
	}
	// Flag resolution for accented tritone
	else {
		if (res1*res2 == 0) FLAGVL(361, fli[v][fleap_start], fli[v][fleap_end]);
		else FLAGVL(360, fli[v][fleap_start], fli[v][fleap_end]);
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
			if (FailAdjacentTritone2(3, 5, 11, 0)) return 1;
			if (FailAdjacentTritone2(7, 8, 2, 3)) return 1;
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
		tfound.resize(2);
		tfound2.resize(2);
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
			}
		}
		// Loop through tritone types 
		for (int tt = 0; tt < 2; ++tt) {
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
							if (found == 1) FLAGVL(363, fli[v][fleap_start], fli[v][fleap_end]);
							else FLAGVL(364, fli[v][fleap_start], fli[v][fleap_end]);
						}
					}
					*/
					// Flag resolution for framed tritone
					if (found == 1) {
						if (res1*res2 == 0) FLAGVL(19, fli[v][fleap_start], fli[v][fleap_end]);
						else FLAGVL(18, fli[v][fleap_start], fli[v][fleap_end]);
					}
					// Flag resolution for accented tritone
					else {
						if (res1*res2 == 0) FLAGVL(343, fli[v][fleap_start], fli[v][fleap_end]);
						else FLAGVL(342, fli[v][fleap_start], fli[v][fleap_end]);
					}
				}
			}
		}
	}
	return 0;
}

int CP2R::FailRhythm() {
	CHECK_READY(DR_fli, DR_beat, DR_sus);
	CHECK_READY(DR_leap);
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
	if (c_len - fli[v][fli_size[v] - 1] < npm) {
		FLAGV(267, fli[v][fli_size[v] - 1]);
	}
	for (ls = 0; ls < fli_size[v] - 1; ++ls) {
		// Whole inside
		if (!beat[v][ls] && llen[v][ls] == npm && cc[v][fli[v][ls]]) FLAGV(236, fli[v][ls]);
	}
	return 0;
}

// Fail rhythm for species 4
int CP2R::FailRhythm4() {
	// Last measure not whole
	if (c_len - fli[v][fli_size[v] - 1] < npm) {
		FLAGV(267, fli[v][fli_size[v] - 1]);
	}
	for (ls = 0; ls < fli_size[v] - 1; ++ls) {
		// Whole inside
		if (!beat[v][ls] && llen[v][ls] == npm) FLAGV(236, fli[v][ls]);
	}
	return 0;
}

// Fail rhythm for species 3
int CP2R::FailRhythm3() {
	// Check uneven pause
	if (fli_size[v] > 2 && !cc[v][0] && llen[v][0] != llen[v][1]) FLAGV(237, 0);
	// Last measure not whole
	if (c_len - fli[v][fli_size[v] - 1] < npm) {
		FLAGV(267, fli[v][fli_size[v] - 1]);
		if (c_len - fli[v][fli_size[v] - 1] == 2) FLAGV(252, fli[v][fli_size[v] - 1]);
	}
	for (ls = 0; ls < fli_size[v]; ++ls) {
		s = fli[v][ls];
		// 1/4 syncope (not for last 1/4 because it is applied with anticipation or sus)
		if (beat[v][ls] == 4 && llen[v][ls] > 2) FLAGV(235, s);
		// 1/2 after 1/4
		if (ls > 0 && beat[v][ls] == 1 && llen[v][ls] > 2 && llen[v][ls - 1] == 2) {
			if (bmli[s] >= mli.size() - 2) FLAGVL(238, s, mli[bmli[s]]);
			// Flag slurred if sus or note is cut by scan window
			else if (sus[v][ls] || (ls == fli_size[v] - 1 && c_len > ep2)) FLAGVL(239, s, mli[bmli[s]]);
			else FLAGVL(240, s, mli[bmli[s]]);
		}
		// Non-uniform starting rhythm
		if (ls > 0 && bmli[s] == 0 && llen[v][ls] != llen[v][ls - 1] && ls < fli_size[v] - 1) {
			FLAGVL(254, s, fin[v]);
		}
	}
	return 0;
}

// Fail rhythm for species 5
int CP2R::FailRhythm5() {
	// Rhythm id
	vector<int> rid;
	int rid_cur = 0;
	// Pause rhythm id
	vector<int> pid;
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
	// Check pause length
	if (!cc[v][0] && llen[v][0] > 4) FLAGV(197, 0);
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
				if (ms == mli.size() - 1 && l_len.size()) FLAGV(267, fli[v][fli_size[v] - 1]);
				// Whole inside if it starts not from first measure, from first step and is not a suspension
				if (llen[v][ls2] >= 8 && ms && !pos && !sus[v][ls2]) FLAGV(236, s);
				// 1/8 syncope
				else if (llen[v][ls2] > 1 && pos % 2) FLAGV(232, fli[v][ls2]);
				// 1/4 syncope (not last, because it is flagged in suspension)
				else if (llen[v][ls2] > 2 && pos == 2) FLAGV(235, fli[v][ls2]);
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
					FLAGV(267, fli[v][fli_size[v] - 1]);
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
				if (l_len[lp] == 1) FLAGV(253, fli[v][fli_size[v] - 1]);
				else if (l_len[lp] == 2) FLAGV(252, fli[v][fli_size[v] - 1]);
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
				if (pos == 7 && slur2) FLAGV(232, s2);
				// Other types of 1/8
				else {
					// If second 1/8
					if (pos % 2) {
						// Isolated 1/8
						if (l_len[lp - 1] != 1) FLAGV(231, s2);
					}
					// Too many 1/8
					++count8;
					if (count8 == 3) FLAGV(255, s2);
					else if (count8 > 3) ++fpenalty;
					// 1/8 in first measure
					if (ms == 0) FLAGV(230, s2);
					// If first 8th
					else {
						// 1/8 beats
						if (pos == 0) FLAGV(226, s2);
						else if (pos == 2) FLAGV(227, s2);
						else if (pos == 4) FLAGV(228, s2);
						else if (pos == 6) FLAGV(229, s2);
					}
				}
				// 1/8 on leap
				if (ls2 < fli_size[v] - 1 && leap[v][s2])
					FLAGV(88, s2);
				else if (ls2 > 0 && leap[v][s2 - 1]) {
					if (llen[v][ls2 - 1] > 1) FLAGV(88, isus[v][bli[v][s2 - 1]]);
				}
			}
			else {
				// 1/8 syncope
				if (pos % 2) FLAGV(232, s2);
				// 1/4 syncope
				else if (l_len[lp] > 2 && pos == 2) FLAGV(235, s2);
				//else if (l_len[lp] == 2 && pos == 6 && slur2) FLAGV(235, s2);
			}
			// Uneven starting rhythm
			if (!ms && lp > 0 && l_len[lp] != l_len[lp - 1] && !uneven_start_fired) {
				// Check for exception: (pause + 1/4 + 1/2 slurred)
				if (!cc[v][0] && llen[v][0] == 2 && lp == 2 && l_len[lp] >= 4 && l_len[lp - 1] == 2 && slur2) {}
				else {
					uneven_start_fired = 1;
					FLAGVL(254, s2, fin[v]);
				}
			}
			pos += l_len[lp];
		}
		// Check rhythm repeat
		if (full_measure) {
			// Check only if no croches or less than 4 notes
			if (rid.size() && (!has_croche || l_len.size() <4)) {
				// Do not fire for first measure if measure starts with pause
				if (rid.back() == rid_cur && pid.back() == pid_cur)
					FLAGVL(247, s, fli[v][bli[v][s + npm - 1]]);
			}
			rid.push_back(rid_cur);
			pid.push_back(pid_cur);
		}
		// Check rhythm rules
		// First measure
		if (!ms) {
			// Uneven pause
			if (l_len.size() > 1 && l_len[0] == fin[v] && l_len[0] != l_len[1]) FLAGV(237, s);
		}
		// Whole inside
		if (l_len[0] >= 8 && ms < mli.size() - 1 && ms) FLAGV(236, s);
		// 1/2.
		else if (l_len[0] == 6 && !slur1) FLAGV(233, s);
		else if (l_len.size() > 1 && l_len[1] == 6) FLAGV(234, fli[v][l_ls[1]], fli[v][l_ls[0]]);
		else if (l_len.size() > 2 && l_len[2] == 6) FLAGV(234, fli[v][l_ls[2]], fli[v][l_ls[0]]);
		// 1/2 after 1/4 or 1/8 in measure
		else if (full_measure && l_len[l_len.size() - 1] == 4 && l_len[0] != 4) {
			s3 = fli[v][l_ls[l_ls.size() - 1]];
			if (ms >= mli.size() - 2) FLAGVL(238, s3, s);
			else if (slur2 != 0) FLAGVL(239, s3, s);
			else if (slur1 != 0) FLAGVL(278, s3, s);
			else FLAGVL(240, s3, s);
		}
		// Many notes in measure
		if (l_len.size() == 5) {
			if (slur1) FLAGV(301, s);
			else FLAGVL(245, s, fli[v][bli[v][s + npm - 1]]);
		}
		else if (l_len.size() > 5) FLAGVL(246, s, fli[v][bli[v][s + npm - 1]]);
		// Suspensions
		if (slur1 == 4 && l_len[0] == 2) FLAGV(241, s);
		else if (slur1 == 4 && l_len[0] == 4) FLAGV(242, s);
		//else if (slur1 == 2) FLAGV(251, s)
		if (slur1 && l_len[0] == 6) FLAGV(243, s);
		if (slur1 == 6) FLAGV(244, s);
	}
	return 0;
}

// Detect missing slurs
int CP2R::FailMissSlurs() {
	// Check only for species 4
	if (sp != 4) return 0;
	// Current window size
	int wsize = 0;
	// Number of slurs in window
	int scount = 0;
	int miss, max_miss = 0;
	int max_ls = 0;
	for (ls = 0; ls < fli_size[v] - 2; ++ls) {
		if (!ls && !cc[v][0]) continue;
		if (ls < miss_slurs_window[sp][av_cnt][0]) ++wsize;
		// Subtract old slur
		if (ls >= miss_slurs_window[sp][av_cnt][0] && sus[v][ls - miss_slurs_window[sp][av_cnt][0]])
			--scount;
		// Check slurs in window
		if (sus[v][ls]) {
			++scount;
		}
		else {
			miss = wsize - scount;
			if (miss > max_miss) {
				max_miss = miss;
				max_ls = ls;
			}
		}
	}
	if (max_miss == 1) FLAGV(188, fli[v][max_ls]);
	else if (max_miss == 2) FLAGV(189, fli[v][max_ls]);
	else if (max_miss > 2) {
		FLAGV(190, fli[v][max_ls]);
		if (!accept[sp][av_cnt][0][190]) fpenalty += (max_miss - 2) * 50;
	}
	return 0;
}

// Detect many slurs
int CP2R::FailSlurs() {
	CHECK_READY(DR_fli);
	// For species 5 there are separate rules (FailRhythm5)
	// For species 4 we can have all notes slurred
	if (sp >= 4) return 0;
	// Current window size
	int wsize = 0;
	// Number of slurs in window
	int scount = 0;
	int cnt, max_count = 0;
	int max_ls = 0;
	int stl = sp_nlen[sp];
	// Check pause length
	if (!cc[v][0] && llen[v][0] > 4) FLAGV(197, 0);
	for (ls = 0; ls < fli_size[v] - 1; ++ls) {
		if (!ls && !cc[v][0]) continue;
		// Subtract old slur
		if (ls >= slurs_window[sp][av_cnt][0] && llen[v][ls - slurs_window[sp][av_cnt][0]] > stl)
			--scount;
		// Check slurs in window
		if (llen[v][ls] > stl) {
			++scount;
			if (scount > max_count) {
				max_count = scount;
				max_ls = ls;
			}
		}
	}
	if (max_count == 1) FLAGV(93, fli[v][max_ls]);
	else if (max_count == 2) FLAGV(94, fli[v][max_ls]);
	else if (max_count > 2) {
		FLAGV(95, fli[v][max_ls]);
		if (!accept[sp][av_cnt][0][95]) fpenalty += (max_count - 2) * 50;
	}
	return 0;
}

int CP2R::FailStartPause() {
	if (sp <= 1 && fin[v]) {
		FLAGV(138, 0);
	}
	else if (sp > 1 && !fin[v]) {
		FLAGV(273, 0);
	}
	return 0;
}

int CP2R::FailMaxNoteLen() {
	CHECK_READY(DR_fli);
	/*
	// Never check last note, either end of scan window or end of counterpoint
	for (ls = 0; ls < fli_size[v] - 1; ++ls) {
		if (rlen[v][ls] > max_note_len[sp][av_cnt][0] * 2) 
			FLAGV(336, fli[v][ls]);
		// Check notes crossing multiple measures
		if (bmli[fli2[v][ls]] - bmli[fli[v][ls]] > 1) FLAGV(41, fli[v][ls]);
	}
	*/
	return 0;
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
		FLAGV(341, 0);
	return 0;
}

// Detect repeating notes. Step2 excluding
int CP2R::FailNoteRepeat() {
	for (ls = 0; ls < fli_size[v] - 1; ++ls) {
		if (cc[v][fli[v][ls]] == cc[v][fli[v][ls + 1]]) FLAGV(30, fli[v][ls]);
	}
	return 0;
}

// Detect repeating notes. Step2 excluding
int CP2R::FailNoteLen() {
	if (sp == 0) {
		for (ls = 0; ls < fli_size[v]; ++ls) {
			s = fli[v][ls];
			if (!cc[v][s]) continue;
			if (llen[v][ls] == 8) continue;
			FLAGV(514, s);
		}
	}
	else if (sp == 1) {
		for (ls = 0; ls < fli_size[v]; ++ls) {
			s = fli[v][ls];
			if (!cc[v][s]) continue;
			if (llen[v][ls] == 8) continue;
			if (llen[v][ls] == 16) continue;
			FLAGV(514, s);
		}
	}
	else if (sp == 2) {
		for (ls = 0; ls < fli_size[v]; ++ls) {
			s = fli[v][ls];
			if (!cc[v][s]) continue;
			if (llen[v][ls] == 4) continue;
			if (llen[v][ls] == 8) continue;
			FLAGV(514, s);
		}
	}
	else if (sp == 3) {
		for (ls = 0; ls < fli_size[v]; ++ls) {
			s = fli[v][ls];
			if (!cc[v][s]) continue;
			if (llen[v][ls] == 2) continue;
			FLAGV(514, s);
		}
	}
	else if (sp == 4) {
		for (ls = 0; ls < fli_size[v]; ++ls) {
			s = fli[v][ls];
			if (!cc[v][s]) continue;
			if (llen[v][ls] == 4) continue;
			if (llen[v][ls] == 8) continue;
			FLAGV(514, s);
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
			FLAGV(514, s);
		}
	}
	return 0;
}

// Detect repeating notes. Step2 excluding
int CP2R::FailBeat() {
	if (sp == 0) {
		for (ls = 0; ls < fli_size[v]; ++ls) {
			if (!beat[v][ls]) continue;
			s = fli[v][ls];
			FLAGV(515, s);
		}
	}
	else if (sp == 1) {
		for (ls = 0; ls < fli_size[v]; ++ls) {
			if (!beat[v][ls]) continue;
			s = fli[v][ls];
			FLAGV(515, s);
		}
	}
	else if (sp == 2) {
		for (ls = 0; ls < fli_size[v]; ++ls) {
			if (beat[v][ls] < 4) continue;
			s = fli[v][ls];
			FLAGV(515, s);
		}
	}
	else if (sp == 3) {
		for (ls = 0; ls < fli_size[v]; ++ls) {
			if (beat[v][ls] < 10) continue;
			s = fli[v][ls];
			FLAGV(515, s);
		}
	}
	else if (sp == 4) {
		for (ls = 0; ls < fli_size[v]; ++ls) {
			if (beat[v][ls] < 4) continue;
			s = fli[v][ls];
			FLAGV(515, s);
		}
	}
	return 0;
}
