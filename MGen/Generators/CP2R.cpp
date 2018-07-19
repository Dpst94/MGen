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

void CP2R::SendComment(int pos, int v, int x, int i) {
	int vi = vid[v];
	CString st, com;
	int current_severity = -1;
	// Clear
	comment[pos + i][vi].clear();
	ccolor[pos + i][vi].clear();
	color[pos + i][vi] = color_noflag;
	if (flag[v][x].size() > 0) for (int f = 0; f < flag[v][x].size(); ++f) {
		int fl = flag[v][x][f];
		sp = vsp[v];
		vc = vca[x];
		v2 = fvl[v][x][f];
		if (v2 == v) {
			if (v == hva[x]) vp = vpExt;
			else if (v == lva[x]) vp = vpBas;
			else vp = vpNbs;
		}
		else {
			if (v == lva[x] && v2 == hva[x]) vp = vpExt;
			else if (v == lva[x] && v2 != hva[x]) vp = vpBas;
			else vp = vpNbs;
		}
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
				st.Format(" [%d/%d]", fl, severity[fl]);
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
	for (int v = 0; v < av_cnt; ++v) {
		int vi = vid[v];
		for (int ls = 0; ls < fli_size[v]; ++ls) {
			for (int s = fli[v][ls]; s <= fli2[v][ls]; ++s) {
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
				SendComment(step0 + fli[v][ls], v, s, s - fli[v][ls]);
			}
		}
		MergeNotes(step0, step0 + full_len - 1, v);
		// If  window-scan
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

void CP2R::AnalyseCP() {
	skip_flags = 0;
	EvaluateCP();
}

int CP2R::EvaluateCP() {
	CLEAR_READY();
	ClearFlags(0, c_len);
	GetDiatonic(0, c_len);
	GetPitchClass(0, c_len);
	CreateLinks();
	GetVca();
	GetLClimax();
	GetLeapSmooth();
	for (v = 0; v < av_cnt; ++v) {
		sp = vsp[v];
		vaccept = &accept[sp][1][0];
		GetBasicMsh();
		ApplyFixedPat();
		if (mminor) {
			if (FailMinor()) return 1;
			if (FailMinorStepwise()) return 1;
			if (FailGisTrail()) return 1;
			if (FailFisTrail()) return 1;
		}
	}
	return 0;
}

void CP2R::ClearFlags(int step1, int step2) {
	for (int v = 0; v < av_cnt; ++v) {
		for (int s = step1; s < step2; ++s) {
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
			else if (sm == 6) beat[v][ls] = 5;
			else if (sm == 7) beat[v][ls] = 13;
			else if (sm == 8) beat[v][ls] = 2;
			else if (sm == 9) beat[v][ls] = 14;
			else if (sm == 10) beat[v][ls] = 6;
			else if (sm == 11) beat[v][ls] = 15;
			else if (sm == 12) beat[v][ls] = 4;
			else if (sm == 13) beat[v][ls] = 16;
			else if (sm == 14) beat[v][ls] = 7;
			else if (sm == 15) beat[v][ls] = 17;
			// Beats for species 1: 0 0 0 0
			// Beats for species 2: 0 1 0 1
			// Beats for species 3: 0 3 1 5
			// Beats for species 4: 0 1 0 1
			// Beats for species 5: 0 10 3 11 1 12 5 13 2 14 6 15 4 16 7 17
			// Get suspension if cantus note changes during counterpoint note
			sus[v][ls] = 0;
			if (bmli[s] != bmli[s2]) {
				sus[v][ls] = mli[bmli[s] + 1];
			}
			// Build isus
			isus[v][ls] = sus[v][ls] ? sus[v][ls] : fli[v][ls];
		}
	}
}

int CP2R::FailGisTrail() {
	CHECK_READY(DR_fli, DR_pc);
	int gis_trail = 0;
	int _gis_trail_max = gis_trail_max[sp][1][0];
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
					if (gis_trail) FLAGV(200, s, fli[v][max(0, ls - _gis_trail_max + gis_trail)]);
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
	int _fis_gis_max = fis_gis_max[sp][1][0];
	int _fis_g_max = fis_g_max[sp][1][0];
	int _fis_g_max2 = fis_g_max2[sp][1][0];
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
				if (ls + _fis_gis_max <= fli_size[v] - 1 || ep2 == c_len)	FLAGV(199, s, s);
			}
			// Find VII before
			pos1 = max(0, ls - _fis_g_max);
			for (int x = pos1; x < ls; ++x) {
				s2 = fli[v][x];
				if (cc[v][s2] && pcc[v][s2] == 10) {
					FLAGV(349, s, s2);
					break;
				}
			}
			// Find VII after
			pos2 = min(fli_size[v] - 1, ls + _fis_g_max2);
			for (int x = ls + 1; x <= pos2; ++x) {
				s2 = fli[v][x];
				if (cc[v][s2] && pcc[v][s2] == 10) {
					FLAGV(350, s, s);
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
			if (pcc[v][s_1] == 10) FLAGV(153, s_1, s);
			if (pcc[v][s_1] == 8) FLAGV(154, s_1, s);
			if (pcc[v][s_1] == 3) {
				if (ls < fli_size[v] - 1) {
					// III-VII#-I downward
					if (pcc[v][fli[v][ls + 1]] == 0 && cc[v][s] - cc[v][s_1] < 0) FLAGV(432, s_1, fli[v][ls + 1]);
					// III-VII#
					else FLAGV(157, s_1, s);
				}
				else {
					if (ep2 == c_len) FLAGV(157, s_1, s);
				}
			}
			if (ls > 1) {
				s_2 = fli[v][ls - 2];
				if (pcc[v][s_2] == 10) FLAGV(159, s_2, s);
				if (pcc[v][s_2] == 8) FLAGV(160, s_2, s);
				if (pcc[v][s_2] == 3) FLAGV(163, s_2, s);
			}
			if (ls < fli_size[v] - 1) {
				s1 = fli[v][ls + 1];
				if (pcc[v][s1] == 10) FLAGV(153, s1, s);
				if (pcc[v][s1] == 8) FLAGV(154, s1, s);
				if (pcc[v][s1] == 3) FLAGV(156, s1, s);
				if (ls < fli_size[v] - 2) {
					s2 = fli[v][ls + 2];
					if (pcc[v][s2] == 10) FLAGV(159, s2, s);
					if (pcc[v][s2] == 8) FLAGV(160, s2, s);
					if (pcc[v][s2] == 3) FLAGV(162, s2, s);
				}
			}
		}
		if (pcc[v][s] == 9) {
			if (pcc[v][s_1] == 8) FLAGV(152, s_1, s);
			if (pcc[v][s_1] == 3) FLAGV(155, s_1, s);
			if (ls > 1) {
				s_2 = fli[v][ls - 2];
				if (pcc[v][s_2] == 8) FLAGV(158, s_2, s);
				if (pcc[v][s_2] == 3) FLAGV(161, s_2, s);
			}
			if (ls < fli_size[v] - 1) {
				s1 = fli[v][ls + 1];
				if (pcc[v][s1] == 8) FLAGV(152, s1, s);
				if (pcc[v][s1] == 3) FLAGV(155, s1, s);
				if (ls < fli_size[v] - 2) {
					s2 = fli[v][ls + 2];
					if (pcc[v][s2] == 8) FLAGV(158, s2, s);
					if (pcc[v][s2] == 3) FLAGV(161, s2, s);
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
			FLAGV(201, s_1, s1);
		// Prohibit harmonic VII natural not stepwize descending
		if ((sp < 2 || msh[v][ls] > 0) && pcc[v][s] == 10 &&
			(c[v][s] - c[v][s_1] != -1 || c[v][s1] - c[v][s] != -1))
			FLAGV(202, s_1, s1);
	}
	return 0;
}

// Merge notes of same pitch, that do not have pauses between them. Step2 inclusive
void CP2R::MergeNotes(int step1, int step2, int v) {
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

void CP2R::ApplyFixedPat() {
	CHECK_READY(DR_mshb);
	CHECK_READY(DR_fli);
	SET_READY(DR_msh);
	for (int ls = 0; ls < fli_size[v]; ++ls) msh[v][ls] = mshb[v][ls];
}
