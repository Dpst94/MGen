// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "CP2Ly.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

CP2Ly::CP2Ly() {
}

CP2Ly::~CP2Ly() {
}

CString CP2Ly::GetLyNoteCP() {
	int no2, oct, alter;
	if (src_alter[v][s]) {
		no2 = (cc[v][s] - src_alter[v][s]) % 12;
		alter = src_alter[v][s];
		oct = (cc[v][s] - src_alter[v][s]) / 12;
	}
	else {
		GetRealNote(cc[v][s], maj_bn, 0, no2, oct, alter);
	}
	return LyNoteSharp[no2] + GetLyAlter(alter) + LyOctave[oct];
}

CString CP2Ly::GetLyNoteVisualCP(CString sz) {
	int no2, oct, alter;
	if (src_alter[v][s]) {
		no2 = (cc[v][s] - src_alter[v][s]) % 12;
		alter = src_alter[v][s];
		oct = (cc[v][s] - src_alter[v][s]) / 12;
	}
	else {
		GetRealNote(cc[v][s], maj_bn, 0, no2, oct, alter);
	}
	//GetRealNote(cc[v][s], bn, mode == 9, no2, oct, alter);
	return NoteName[no2] + GetLyAlterVisual(alter, sz);
}

// Get start of real note
int CP2Ly::GetNoteStart(int voice, int step) {
	int lstep = bli[voice][step];
	if (sus[voice][lstep]) {
		if (step < sus[voice][lstep]) return fli[voice][lstep];
		else return sus[voice][lstep];
	}
	else return fli[voice][lstep];
}

void CP2Ly::AddNLink(int f) {
	GetFlag(f);
	// Send comments and color only if rule is not ignored
	if (accept[sp][vc][vp][fl] == -1 && !show_ignored_flags) return;
	// Send comments and color only if rule is not ignored
	if (accept[sp][vc][vp][fl] == 1 && !show_allowed_flags) return;
	// Do not send if ignored
	if (severity[sp][vc][vp][fl] < show_min_severity) return;
	// Correct positions
	int s3 = s;
	int s4 = fsl[v][s][f];
	if (ruleinfo[fl].viz == vGlis) {
		s3 = GetNoteStart(v, s);
		s4 = GetNoteStart(v, fsl[v][s][f]);
	}
	int f2 = lyv[v].f[s3].size();
	lyv[v].f[s3].resize(f2 + 1);
	// Check if this shape is not allowed at hidden position
	if (!shinfo[ruleinfo[fl].viz].can_anyposition) {
		// Note start is ok
		// Downbeat is ok
		if ((s3 != fli[v][bli[v][s3]] && s3 % npm) ||
			(s4 != fli[v][bli[v][s4]] && s4 % npm)) {
			lyv[v].f[s3][f2].shide = 1;
			if (shinfo[ruleinfo[fl].viz].can_separate) {
				lyv[v].f[s3][f2].sep = 1;
			}
			else {
				CString est;
				est.Format("Detected shape %s at hidden position %d-%d (instead of %d-%d), voice %d counterpoint %d: [%d] %s (%s). Cannot send this type of shapes to separate staff. Shape will be removed",
					shinfo[ruleinfo[fl].viz].name,
					s3, s4, ssus[v][bli[v][s3]], ssus[v][bli[v][s4]], v, cp_id + 1, fl,
					ruleinfo[fl].RuleName, ruleinfo[fl].SubRuleName);
				WriteLog(5, est);
			}
		}
	}
	lyv[v].f[s3][f2].fid = fl;
	lyv[v].f[s3][f2].fsev = severity[sp][vc][vp][fl];
	lyv[v].f[s3][f2].sl = s4 - s3;
	lyv[v].f[s3][f2].s_src = s;
	lyv[v].f[s3][f2].sl_src = fsl[v][s][f];
	lyv[v].f[s3][f2].v = v;
	lyv[v].f[s3][f2].f_base = f2;
	lyv[v].f[s3][f2].s_base = s3;
	lyv[v].f[s3][f2].vl = fvl[v][s][f];
	// Check that this flag was already sent at this step
	pair<set<int>::iterator, bool> ufl_p = ly_ufl.insert(fl);
	// Check if insertion took place, which means that it is unique
	if (ufl_p.second) {
		lyv[v].f[s3][f2].dfgn = ly_flags + 1;
		++ly_flags;
		++lyv[v].flags;
	}
	// Add foreign flag with link to base flag
	v2 = fvl[v][s][f];
	if (v2 != v) {
		int s5 = s;
		int s6 = fsl[v][s][f];
		if (ruleinfo[fl].viz == vGlis) {
			s5 = GetNoteStart(v2, s);
			s6 = GetNoteStart(v2, fsl[v][s][f]);
		}
		int f3 = lyv[v2].f[s5].size();
		lyv[v2].f[s5].resize(f3 + 1);
		// Do not check for hidden positions, because gliss pos is corrected and notecolor pos will be corrected
		// Send comments and color only if rule is not ignored
		if (accept[sp][vc][vp][fl] == -1 && !show_ignored_flags) return;
		// Send comments and color only if rule is not ignored
		if (accept[sp][vc][vp][fl] == 1 && !show_allowed_flags) return;
		// Do not send if ignored
		if (severity[sp][vc][vp][fl] < show_min_severity) return;
		lyv[v2].f[s5][f3].fid = fl;
		lyv[v2].f[s5][f3].fsev = severity[sp][vc][vp][fl];
		lyv[v2].f[s5][f3].sl = s6 - s5;
		lyv[v2].f[s5][f3].s_src = s;
		lyv[v2].f[s5][f3].sl_src = fsl[v][s][f];
		lyv[v2].f[s5][f3].v = v;
		lyv[v2].f[s5][f3].f_base = f2;
		lyv[v2].f[s5][f3].s_base = s3;
		lyv[v2].f[s5][f3].vl = fvl[v][s][f];
	}
}

void CP2Ly::ParseNLinks() {
	ly_ufl.clear();
	for (int f = 0; f < flag[v][s].size(); ++f) {
		AddNLink(f);
	}
}

void CP2Ly::SetLyShape(int st1, int st2, int f, int fl, int sev, int vtype, int voice) {
	if (lyv[v].s[st1][vtype].sev <= sev) {
		if (lyv[v].s[st1][vtype].sev == -1) ++lyv[v].shapes;
		// Start
		lyv[v].s[st1][vtype].start = 1;
		// Finish
		lyv[v].s[st2][vtype].fin = 1;
		// Link to start
		lyv[v].s[st2][vtype].start_s = s1 - s2;
		lyv[v].s[st1][vtype].sev = sev;
		if (vtype == vInterval || vtype == vNoteName || vtype == vHarm) {
			// Detect trying to send to wrong voice
			if (vtype == vHarm && v) {
				CString est;
				est.Format("Trying to send vHarm shape for flag [%d] %s (%s) to voice %d instead of voice 0",
					fl, ruleinfo[fl].RuleName, ruleinfo[fl].SubRuleName, v);
				WriteLog(5, est);
			}
			if (lyv[v].s[st2][vtype].sev <= sev) {
				lyv[v].s[st2][vtype].sev = sev;
			}
			if (vtype == vNoteName) ++lyv[v].note_names;
			if (vtype == vInterval) ++lyv[v].intervals;
		}
		lyv[v].s[st1][vtype].txt = ruleinfo[fl].viz_text;
		// Save flag shape (step depends if link is forward or backward)
		if (fl) lyv[voice].f[s][f].sh = vtype;
		lyv[v].s[st1][vtype].fl = f;
		lyv[v].s[st1][vtype].fs = s;
		lyv[v].s[st1][vtype].fv = voice;
	}
}

void CP2Ly::ClearLyShape(int st1, int st2, int vtype) {
	lyv[v].s[st1][vtype].start = 0;
	lyv[v].s[st2][vtype].fin = 0;
	// Clear severity
	lyv[v].s[st1][vtype].sev = -1;
	// Remove link
	lyv[lyv[v].s[st1][vtype].fv].f[lyv[v].s[st1][vtype].fs][lyv[v].s[st1][vtype].fl].sh = 0;
	lyv[v].s[st1][vtype].fl = -1;
	lyv[v].s[st1][vtype].fs = -1;
	--lyv[v].shapes;
}

void CP2Ly::ParseLyI() {
	for (s = 0; s < c_len; ++s) {
		ls = bli[v][s];
		s2 = fli[v][ls];
		// Find next note position
		int next_note_step = fli[v][min(ls + 1, fli_size[v] - 1)];
		// Find previous note position
		int prev_note_step = fli[v][max(0, ls - 1)];
		// Mark msh
		if (cc[v][s] && msh[v][s] < 0) SetLyShape(s, s, -1, 0, 0, vCircle, v);
		// Mark nih
		//if (cc[v][s] && nih[v][s] > 0) SetLyShape(s, s, -1, 0, 0, vStac, v);
		// Mark islt
		if (cc[v][s] && islt[v][s] > 0) SetLyShape(s, s, -1, 0, 0, vStac, v);
		// Parse flags
		for (int f = 0; f < lyv[v].f[s].size(); ++f) {
			fl = lyv[v].f[s][f].fid;
			int link = lyv[v].f[s][f].sl;
			int vlink = lyv[v].f[s][f].vl;
			int vtype = ruleinfo[fl].viz;
			int sev = lyv[v].f[s][f].fsev;
			int skip_shape = 0;
			int link_note_step = fli[v][bli[v][s + link]];
			// Previous note before link
			int prev_link_note = fli[v][max(0, bli[v][s + link] - 1)];
			if (lyv[v].f[s][f].shide) vtype = 0;
			if (ly_debugexpect && sev == 100) vtype = 0;
			// Get flag start/stop
			s1 = min(s, s + link);
			s2 = max(s, s + link);
			// If shape cannot highlight single note, but flag does not contain link, then link to next note
			if (!shinfo[vtype].can_singlenote && s1 == s2) s2 = next_note_step;
			// Highlight notes if flag is multivoice and is not gliss (gliss does not need note color)
			if (vlink != lyv[v].f[s][f].v) {
				if (ruleinfo[fl].viz_notecolor == 1) {
					for (ls = bli[v][s1]; ls <= bli[v][s2]; ++ls) {
						// Highlight first part of note
						SetLyShape(fli[v][ls], fli[v][ls], f, fl, sev, vNoteColor, v);
						// Highlight second part of note after tie
						if (sus[v][ls]) {
							SetLyShape(sus[v][ls], sus[v][ls], f, fl, sev, vNoteColor, v);
						}
					}
					SetLyShape(link_note_step, link_note_step, f, fl, sev, vNoteColor, v);
				}
				else if (ruleinfo[fl].viz_notecolor == 2) {
					// Highlight left step
					ls = bli[v][s1];
					if (sus[v][ls] && s1 >= sus[v][ls]) {
						// Highlight second part of note after tie
						SetLyShape(sus[v][ls], sus[v][ls], f, fl, sev, vNoteColor, v);
					}
					else {
						// Highlight first part of note
						SetLyShape(fli[v][ls], fli[v][ls], f, fl, sev, vNoteColor, v);
					}
				}
				else if (ruleinfo[fl].viz_notecolor == 3) {
					// Highlight right step
					ls = bli[v][s2];
					if (s2 >= sus[v][ls]) {
						// Highlight second part of note after tie
						SetLyShape(sus[v][ls], sus[v][ls], f, fl, sev, vNoteColor, v);
					}
					else {
						// Highlight first part of note
						SetLyShape(fli[v][ls], fli[v][ls], f, fl, sev, vNoteColor, v);
					}
				}
			}
			// Set interval if not debugexpect. If debugexpect, do not set for red flags
			if (!ly_debugexpect || sev != 100) {
				// Source positions
				int s3 = min(lyv[v].f[s][f].s_src, lyv[v].f[s][f].sl_src);
				int s4 = max(lyv[v].f[s][f].s_src, lyv[v].f[s][f].sl_src);
				if (ruleinfo[fl].viz_int == 1) {
					SetLyShape(s3, s4, f, fl, sev, vInterval, v);
				}
				if (ruleinfo[fl].viz_int == 2) {
					SetLyShape(s3, s3, f, fl, sev, vInterval, v);
				}
				if (ruleinfo[fl].viz_int == 3) {
					SetLyShape(s4, s4, f, fl, sev, vInterval, v);
				}
				if (ruleinfo[fl].viz_harm == 1) {
					SetLyShape(s1, s2, f, fl, sev, vHarm, v);
				}
				if (ruleinfo[fl].viz_harm == 2) {
					SetLyShape(s1, s1, f, fl, sev, vHarm, v);
				}
				if (ruleinfo[fl].viz_harm == 3) {
					SetLyShape(s2, s2, f, fl, sev, vHarm, v);
				}
				if (ruleinfo[fl].viz_harm == 4) {
					for (hs = bhli[s1]; hs <= bhli[s2]; ++hs) {
						SetLyShape(hli[hs], hli[hs], f, fl, sev, vHarm, v);
					}
				}
			}
			// Skip all foreign shapes (show only note color, intervals and harmony)
			if (v != lyv[v].f[s][f].v && vtype != vGlis) continue;
			if (vtype == vPedal) {
				if (bli[v][s1] == fli_size[v] - 1 && s1 == s2) continue;
			}
			if (!shinfo[vtype].can_overlap) {
				// Check that flag overlaps
				int overlap1 = -1;
				int overlap2 = -1;
				int overlap_border = 0;
				// For groups check for collision between borders
				if (shinfo[vtype].type == vtGroup || shinfo[vtype].type == vtVolta)
					overlap_border = 1;
				// For vbrackets check for collision between notes
				int overlap_limit = s1 - overlap_border;
				if (shinfo[vtype].type == vtVBracket)
					overlap_limit = min(prev_note_step, prev_link_note) - 1;
				// Check if shape can be blocked
				for (int x = c_len - 1; x > overlap_limit; --x) {
					if (lyv[v].s[x][vtype].fin) {
						overlap2 = x;
						overlap1 = x + lyv[v].s[x][vtype].start_s;
						if (overlap1 < s2 + overlap_border) {
							// Choose highest severity
							if (sev < lyv[v].s[overlap1][vtype].sev) {
								// Skip shape
								skip_shape = 1;
								break;
							}
							// Choose same severity, but longer shape
							if (sev == lyv[v].s[overlap1][vtype].sev && abs(link) <= abs(lyv[v].s[x][vtype].start_s)) {
								// Skip shape
								skip_shape = 1;
								break;
							}
						}
					}
				}
				if (skip_shape) continue;
				// Check if shape can block other shapes
				for (int x = c_len - 1; x > overlap_limit; --x) {
					if (lyv[v].s[x][vtype].fin) {
						overlap2 = x;
						overlap1 = x + lyv[v].s[x][vtype].start_s;
						if (overlap1 < s2 + overlap_border) {
							// Choose highest severity
							if (sev > lyv[v].s[overlap1][vtype].sev) {
								ClearLyShape(overlap1, overlap2, vtype);
							}
							// Choose same severity, but longer shape
							if (sev == lyv[v].s[overlap1][vtype].sev && abs(link) > abs(lyv[v].s[x][vtype].start_s)) {
								ClearLyShape(overlap1, overlap2, vtype);
							}
						}
					}
				}
			}
			SetLyShape(s1, s2, f, fl, sev, vtype, v);
		}
	}
}

void CP2Ly::ParseLyISep() {
	for (s = 0; s < c_len; ++s) {
		// Find next note position
		int next_note_step = min(s + 1, c_len - 1);
		// Find previous note position
		int prev_note_step = max(s - 1, 0);
		for (v2 = 0; v2 < av_cnt; ++v2) {
			// Parse flags
			for (int f = 0; f < lyv[v2].f[s].size(); ++f) {
				if (!lyv[v2].f[s][f].sep) continue;
				fl = lyv[v2].f[s][f].fid;
				int link = lyv[v2].f[s][f].sl;
				int vlink = lyv[v2].f[s][f].vl;
				int vtype = ruleinfo[fl].viz;
				int sev = lyv[v2].f[s][f].fsev;
				int skip_shape = 0;
				int link_note_step = s + link;
				// Previous note before link
				int prev_link_note = max(0, s + link - 1);
				// Get flag start/stop
				s1 = min(s, s + link);
				s2 = max(s, s + link);
				// If shape cannot highlight single note, but flag does not contain link, then link to next note
				if (!shinfo[vtype].can_singlenote && s1 == s2) s2 = next_note_step;
				// Set interval if not debugexpect. If debugexpect, do not set for red flags
				if (!shinfo[vtype].can_overlap) {
					// Check that flag overlaps
					int overlap1 = -1;
					int overlap2 = -1;
					int overlap_border = 0;
					// For groups check for collision between borders
					if (shinfo[vtype].type == vtGroup || shinfo[vtype].type == vtVolta)
						overlap_border = 1;
					// For vbrackets check for collision between notes
					int overlap_limit = s1 - overlap_border;
					if (shinfo[vtype].type == vtVBracket)
						overlap_limit = min(prev_note_step, prev_link_note) - 1;
					// Check if shape can be blocked
					for (int x = c_len - 1; x > overlap_limit; --x) {
						if (lyv[v].s[x][vtype].fin) {
							overlap2 = x;
							overlap1 = x + lyv[v].s[x][vtype].start_s;
							if (overlap1 < s2 + overlap_border) {
								// Choose highest severity
								if (sev < lyv[v].s[overlap1][vtype].sev) {
									// Skip shape
									skip_shape = 1;
									break;
								}
								// Choose same severity, but longer shape
								if (sev == lyv[v].s[overlap1][vtype].sev && abs(link) <= abs(lyv[v].s[x][vtype].start_s)) {
									// Skip shape
									skip_shape = 1;
									break;
								}
							}
						}
					}
					if (skip_shape) continue;
					// Check if shape can block other shapes
					for (int x = c_len - 1; x > overlap_limit; --x) {
						if (lyv[v].s[x][vtype].fin) {
							overlap2 = x;
							overlap1 = x + lyv[v].s[x][vtype].start_s;
							if (overlap1 < s2 + overlap_border) {
								// Choose highest severity
								if (sev > lyv[v].s[overlap1][vtype].sev) {
									ClearLyShape(overlap1, overlap2, vtype);
								}
								// Choose same severity, but longer shape
								if (sev == lyv[v].s[overlap1][vtype].sev && abs(link) > abs(lyv[v].s[x][vtype].start_s)) {
									ClearLyShape(overlap1, overlap2, vtype);
								}
							}
						}
					}
				}
				SetLyShape(s1, s2, f, fl, sev, vtype, v2);
			}
		}
	}
}

void CP2Ly::ExportLy() {
	ofstream fs;
	fs.open(as_dir + "\\lyi-" + as_fname + ".csv");
	for (v = 0; v < av_cnt + 1; ++v) {
		fs << "Step[" << cp_id << "];";
		for (int x = 0; x < MAX_VIZ; ++x) {
			fs << "shs[" << x << "];";
			fs << "shf[" << x << "];";
			fs << "shsl[" << x << "];";
			fs << "shse[" << x << "];";
			fs << "sht[" << x << "];";
		}
		fs << "\n";
		for (s = 0; s < c_len; ++s) {
			fs << s << ";";
			for (int x = 0; x < MAX_VIZ; ++x) {
				fs << lyv[v].s[s][x].start << ";";
				fs << lyv[v].s[s][x].fin << ";";
				fs << lyv[v].s[s][x].start_s << ";";
				fs << lyv[v].s[s][x].sev << ";";
				fs << lyv[v].s[s][x].txt << ";";
			}
			fs << "\n";
		}
		fs << "\n";
	}
	fs.close();
}

void CP2Ly::InitLy() {
	ly_flags = 0;
	lyv.clear();
	lyv.resize(av_cnt + 1);
	for (v = 0; v < av_cnt + 1; ++v) {
		lyv[v].f.resize(c_len + 1);
		lyv[v].s.resize(c_len + 1);
		lyv[v].st.resize(c_len + 1);
		lyv[v].fss.resize(c_len + 1);
		lyv[v].fss2.resize(c_len + 1);
		lyv[v].fss3.resize(c_len + 1);
		for (s = 0; s < c_len + 1; ++s) {
			lyv[v].s[s].resize(MAX_VIZ);
		}
	}
}

int CP2Ly::GetFCount() {
	int fcount = 0;
	for (int f = 0; f < flag[v][s].size(); ++f) {
		GetFlag(f);
		// Send comments and color only if rule is not ignored
		if (accept[sp][vc][vp][fl] == -1 && !show_ignored_flags) continue;
		// Send comments and color only if rule is not ignored
		if (accept[sp][vc][vp][fl] == 1 && !show_allowed_flags) continue;
		// Do not send if ignored
		if (severity[sp][vc][vp][fl] < show_min_severity) continue;
		++fcount;
	}
	return fcount;
}

void CP2Ly::DistributeFlags() {
	for (s = 0; s < c_len; ++s) {
		int changed = 0;
		for (v = 0; v < av_cnt; ++v) {
			// Skip voices with little flags
			int fcount = GetFCount();
			if (fcount < 4) continue;
			v3 = v;
			for (int f = 0; f < flag[v3][s].size(); ++f) {
				v = fvl[v3][s][f];
				// Do not process single-voice flags
				if (v3 == v) continue;
				// Do not process flags that will exceed limit if moved
				int fcount2 = GetFCount();
				if (fcount2 > 2) continue;
				// Do not move flag if second voice does not start note at the same place
				if (fli[v][bli[v][s]] != s) continue;
				// Copy flag
				flag[v][s].push_back(flag[v3][s][f]);
				fsl[v][s].push_back(fsl[v3][s][f]);
				fvl[v][s].push_back(v3);
				// Remove old flag
				flag[v3][s].erase(flag[v3][s].begin() + f);
				fsl[v3][s].erase(fsl[v3][s].begin() + f);
				fvl[v3][s].erase(fvl[v3][s].begin() + f);
				// Restart scanning step
				changed = 1;
				break;
			}
			// Restart scanning step
			if (changed) break;
			v = v3;
		}
		// Rescan step if flag was moved
		if (changed) --s;
	}
}

void CP2Ly::ParseLy() {
	// Parse main voices
	for (v = av_cnt - 1; v >= 0; --v) {
		vi = vid[v];
		for (s = 0; s < c_len; ++s) {
			ls = bli[v][s];
			ParseNLinks();
		}
	}
	for (v = av_cnt - 1; v >= 0; --v) {
		vi = vid[v];
		ParseLyI();
	}
	// Parse separate staff
	v = av_cnt;
	ParseLyISep();
	// Order all displayed
	SortFlagsBySev();
	// Hide flags, which have flag number in shape
	HideFlags();
	// Order flags that are shown with numbers
	SortFlagsBySev2();
}

void CP2Ly::HideFlags() {
	for (v = av_cnt; v >= 0; --v) {
		for (s = 0; s < c_len; ++s) {
			for (int sh = 0; sh < lyv[v].s[s].size(); ++sh) {
				// Get flag voice
				int fv = lyv[v].s[s][sh].fv;
				// Do not hide flag for shape without flag
				if (lyv[v].s[s][sh].fl == -1) continue;
				// Do not hide flag for shapes which cannot output text
				if (!shinfo[sh].can_text) continue;
				// Do not hide flag without TEXT output
				if (lyv[v].s[s][sh].txt.IsEmpty()) continue;
				// Get flag of shape
				LY_Flag F = lyv[fv].f[lyv[v].s[s][sh].fs][lyv[v].s[s][sh].fl];
				// Do not hide flag if shape is hidden
				if (F.shide) continue;
				// Do not hide flag without dfgn
				if (!F.dfgn) continue;
				// Keep flag number for parallel pco apart
				ls3 = bli[fv][F.s_src];
				ls4 = bli[fv][F.sl_src];
				if (sh == vGlis) {
					if (ls3 - ls4 > 1 || F.sl_src < sus[fv][ls4] || (sus[fv][ls3] && F.s_src >= sus[fv][ls3]))
						continue;
				}
				// Hide flag
				lyv[fv].f[lyv[v].s[s][sh].fs][lyv[v].s[s][sh].fl].fhide = 1;
			}
		}
	}
}

void CP2Ly::SortFlagsBySev() {
	// Order flags
	for (v = 0; v < av_cnt + 1; ++v) {
		for (s = 0; s < c_len; ++s) {
			for (int f = 0; f < lyv[v].f[s].size(); ++f) {
				if (!lyv[v].f[s][f].dfgn) continue;
				lyv[v].fss[s].push_back(make_pair(lyv[v].f[s][f].fsev, f));
			}
			sort(lyv[v].fss[s].rbegin(), lyv[v].fss[s].rend());
		}
	}
	int lfn = 0;
	for (v = av_cnt; v >= 0; --v) {
		for (s = 0; s < c_len; ++s) {
			lyv[v].st[s].dfgn_count = 0;
			for (int ff = 0; ff < lyv[v].fss[s].size(); ++ff) {
				int f = lyv[v].fss[s][ff].second;
				++lfn;
				++lyv[v].st[s].dfgn_count;
				lyv[v].f[s][f].dfgn = lfn;
			}
		}
	}
	if (lfn != ly_flags) {
		WriteLog(5, "LY flag count mismatch detected");
	}
}

void CP2Ly::SortFlagsBySev2() {
	// Order flags Flags line output
	for (v = 0; v < av_cnt + 1; ++v) {
		lyv[v].flags = 0;
		lyv[v].flags_harm = 0;
		lyv[v].flags_noharm = 0;
		for (s = 0; s < c_len; ++s) {
			for (int f = 0; f < lyv[v].f[s].size(); ++f) {
				if (!lyv[v].f[s][f].dfgn) continue;
				if (lyv[v].f[s][f].fhide) continue;
				// Separate harmonic and interval shapes to separate Flags line
				int shape = ruleinfo[lyv[v].f[s][f].fid].viz;
				if (shape == vHarm) {
					lyv[v].fss3[s].push_back(make_pair(lyv[v].f[s][f].fsev, f));
					if (v) {
						CString est;
						est.Format("Detected vHarm flag in non-bass voice: [%d] %s (%s) in counterpoint %d, voice %d, step %d",
							lyv[v].f[s][f].fid, ruleinfo[lyv[v].f[s][f].fid].RuleName, ruleinfo[lyv[v].f[s][f].fid].SubRuleName, cp_id + 1, v, s);
						WriteLog(5, est);
					}
				}
				else {
					lyv[v].fss2[s].push_back(make_pair(lyv[v].f[s][f].fsev, f));
				}
			}
			sort(lyv[v].fss2[s].rbegin(), lyv[v].fss2[s].rend());
			sort(lyv[v].fss3[s].rbegin(), lyv[v].fss3[s].rend());
			lyv[v].flags += lyv[v].fss2[s].size() + lyv[v].fss3[s].size();
			lyv[v].flags_harm += lyv[v].fss3[s].size();
			lyv[v].flags_noharm += lyv[v].fss2[s].size();
		}
	}
}

void CP2Ly::SaveLyCP() {
	CString st;
	InitLy();
	ParseLy();
	vector<CString> sv;
	CString clef, key, key_visual;
	int pos, pos2, le, le2, pause_accum, pause_pos, pause_i;
	ly_com_st.Empty();
	ly_ly_st.Empty();
	for (v = 0; v < av_cnt; ++v) {
		GetMelodyInterval(0, c_len);
	}
	// Key
	key_visual = "\\concat { " + NoteName[(bn + 12 - bn_alter) % 12] + " " + GetLyAlterVisual(bn_alter, "\\raise #0.4 \\magnify #0.8 ") + " }\n";
	// Spacer
	ly_ly_st += "\\markup {\n  ";
	ly_ly_st += "    \\vspace #1\n";
	ly_ly_st += "\n}\n";
	// Show logs
	for (int i = 0; i < ly_log.size(); ++i) {
		if (ly_log[i].pos == 0) {
			CString st;
			ly_ly_st += "\\markup \\smaller \\bold \\wordwrap \\with-color #(rgb-color 1.000 0.000 0.000) { \\char ##x26D4 \n";
			st = ly_log[i].st;
			st.Replace("\"", "\\\"");
			st.Replace(" ", "\" \"");
			ly_ly_st += "\"" + st + "\"\n";
			ly_ly_st += "}\n";
		}
	}
	// Spacer
	ly_ly_st += "\\markup {\n  ";
	ly_ly_st += "    \\vspace #1\n";
	ly_ly_st += "\n}\n";
	// First info
	CString st3;
	ly_ly_st += "\\markup \\wordwrap {\n  \\bold {\n";
	CString bname = CW2A(CA2W(bname_from_path(musicxml_file), CP_ACP), CP_UTF8);
	st.Format("\"#\"%d (from %s)",
		cp_id + 1, bname);
	ly_ly_st += st + " Key: " + key_visual;
	if (mode == 9) {
		if (mminor) ly_ly_st += " minor";
		else ly_ly_st += " aeolian";
	}
	else {
		ly_ly_st += " " + mode_name[mode];
	}
	ly_ly_st += "\n}\n";
	// Show text
	if (ly_show_xml_text && !xml_text.IsEmpty()) {
		ly_ly_st += "\\tiny { \n";
		st = "Text: " + xml_text;
		if (st.Right(1) != "." && st.Right(1) != "!" && st.Right(1) != "?") st += ".";
		st.Replace("\"", "\\\"");
		st.Replace(" ", "\" \"");
		ly_ly_st += "\"" + st + "\"\n";
		ly_ly_st += "}\n";
	}
	// Show lyrics
	if (ly_show_xml_lyrics && !xml_lyrics.IsEmpty()) {
		ly_ly_st += "\\tiny { \n";
		st = "Lyrics: " + xml_lyrics;
		if (st.Right(1) != "." && st.Right(1) != "!" && st.Right(1) != "?") st += ".";
		st.Replace("\"", "\\\"");
		st.Replace(" ", "\" \"");
		ly_ly_st += "\"" + st + "\"\n";
		ly_ly_st += "}\n";
	}
	ly_ly_st += "\n}\n";
	// Show logs
	for (int i = 0; i < ly_log.size(); ++i) {
		if (av_cnt < 5 && ly_log.size() < 5) ly_ly_st += "\\noPageBreak\n";
		if (ly_log[i].pos == 1) {
			CString st;
			ly_ly_st += "\\markup \\smaller \\bold \\wordwrap \\with-color #(rgb-color 1.000 0.000 0.000) { \\char ##x26D4 \n";
			st = ly_log[i].st;
			st.Replace("\"", "\\\"");
			st.Replace(" ", "\" \"");
			ly_ly_st += "\"" + st + "\"\n";
			ly_ly_st += "}\n";
		}
	}
	if (av_cnt < 5) ly_ly_st += "\\noPageBreak\n";
	// Save notes
	ly_ly_st += "<<\n";
	for (v = av_cnt - 1; v >= 0; --v) {
		vi = vid[v];
		// Select best clef
		clef = DetectLyClef(nmin[v], nmax[v]);
		st.Format("\\new Staff = \"staff%d\" {\n", v);
		ly_ly_st += st;
		if (av_cnt > 1) {
			if (vsp[v]) st3.Format(" \"[species %d]\" ", vsp[v]);
			else st3 = " \"[c.f.]\" ";
		}
		else st3 = "";
		CString vocra_st;
		if (vocra_detected[v] == 2) {
			vocra_st = " \"[" + vocra_info[vocra[v]].name + "]\"";
		}
		if (vocra_detected[v]) {
			clef = vocra_info[vocra[v]].clef;
		}
		ly_ly_st += "     \n";
		st.Format("  \\set Staff.instrumentName = \\markup { \\teeny \\override #'(baseline-skip . 2.0) \\center-column{ \"%s\" %s%s } }\n", vname2[vi], st3, vocra_st);
		//ly_ly_st += "    \\override #'(line-width . 100)  \n";
		ly_ly_st += st;
		ly_ly_st += "  \\clef \"" + clef + "\"\n  \\key ";
		if (fifths > 0) {
			ly_ly_st += LyMajorKeyByFifth[fifths];
		}
		else {
			ly_ly_st += LyMajorKeyByFifthNegative[-fifths];
		}
		ly_ly_st += "\n";
		ly_ly_st += "  \\major\n";
		st.Format("  \\time %d/%d\n", npm * btype / 8, btype);
		ly_ly_st += st;
		read_file_sv("configs\\ly2\\staff.ly", sv);
		for (int i = 0; i < sv.size(); ++i) {
			ly_ly_st += sv[i] + "\n";
		}
		ly_ly_st += "  { ";
		for (ls = 0; ls < fli_size[v]; ++ls) {
			s = fli[v][ls];
			GetSpVcVp();
			if (cc[v][s]) {
				SendLyEvent(GetLyNoteCP(), llen[v][ls]);
			}
			else {
				SendLyEvent("r", llen[v][ls]);
			}
		}
		ly_ly_st += "\n  }\n";
		ly_ly_st += "}\n";
		SaveLyComments();
		SendLyMistakes();
		SendLyNoteNames();
		SendLyIntervals();
		SendLyHarmMistakes();
	}
	v = 0;
	SendLyHarm();
	SendLySeparate();
	ly_ly_st += ">>\n";
	//if (st3 != "") ly_ly_st += "\\markup { " + st3 + " }\n";
	ly_ly_st += ly_com_st;
	if (ly_com_st == "") {
		if (!ly_debugexpect) {
			ly_ly_st += "\\markup \\bold \\with-color #(rgb-color 0 0.8 0) { \\char ##x2705 Excellent }\n ";
		}
	}
	if (ly_st.size() <= cp_id) ly_st.resize(cp_id + 1);
	ly_st[cp_id] = ly_ly_st;
	ly_log.clear();
}

void CP2Ly::SendLySeparate() {
	vector<CString> sv;
	CString st;
	v = av_cnt;
	if (!lyv[v].shapes) return;
	ly_ly_st += "\\new Staff = \"staffs\" \\with {\n";
	st.Format("  \\time %d/4\n", npm / 2);
	ly_ly_st += st;
	read_file_sv("configs\\ly2\\separate-staff.ly", sv);
	for (int i = 0; i < sv.size(); ++i) {
		ly_ly_st += sv[i] + "\n";
	}
	ly_ly_st += "  { \n";
	ly_ly_st += "  \\set Staff.pedalSustainStyle = #'mixed\n";
	for (s = 0; s < c_len; ++s) {
		SendLyEvent("r", 1);
	}
	ly_ly_st += "\n  }\n";
}

CString CP2Ly::SendLySkips(int count) {
	CString lst;
	for (int x = 0; x < count; ++x) {
		lst += " \\skip 8 ";
	}
	return lst;
}

CString CP2Ly::GetRealNoteNameCP(int no) {
	int no2, oct, alter;
	GetRealNote(no, bn, mode == 9, no2, oct, alter);
	return NoteName[no2] + GetAlterName(alter);
}

void CP2Ly::SendLyIntervals() {
	CString st;
	if (!lyv[v].intervals) return;
	if (av_cnt != 2) return;
	if (v) return;
	st.Format("  \\new Lyrics \\with { alignBelowContext = \"staff%d\" } {\n", 0);
	ly_ly_st += st;
	ly_ly_st += "    \\lyricmode {\n";
	ly_ly_st += "      \\override StanzaNumber.font-size = #-2\n";
	ly_ly_st += "      \\set stanza = #\" Interval:\"\n";
	//ly_ly_st += "      \\override InstrumentName #'X-offset = #1\n";
	ly_ly_st += "      \\override InstrumentName #'font-series = #'bold\n";
	ly_ly_st += "      \\override InstrumentName.font-size = #-2\n";
	ly_ly_st += "      \\set shortVocalName = \"I:\"\n";
	for (s = 0; s < c_len; ++s) {
		if (!lyv[v].s[s][vInterval].start && !lyv[v].s[s][vInterval].fin) {
			ly_ly_st += SendLySkips(1);
			continue;
		}
		CString st = GetRealIntName(s, 0, 1);
		ly_ly_st += "\\markup{ ";
		ly_ly_st += "\\teeny ";
		if (lyv[v].s[s][vInterval].sev > -1) {
			ly_ly_st += " \\on-color #(rgb-color " + GetLyMarkColor(lyv[v].s[s][vInterval].sev) + ") ";
		}
		ly_ly_st += " \\pad-markup #0.4 \\concat { " + st + " ";
		ly_ly_st += "} }8\n";
	}
	ly_ly_st += "    }\n";
	ly_ly_st += "  }\n";
}

void CP2Ly::SendLyHarm() {
	CString st, lst;
	//if (av_cnt < 2) return;
	//if (!ly_flags) return;
	st.Format("  \\new Lyrics \\with { alignBelowContext = \"staff%d\" } {\n", 0);
	lst += st;
	lst += "    \\lyricmode {\n";
	lst += "      \\override StanzaNumber.font-size = #-2\n";
	lst += "      \\set stanza = #\" Harmony:\"\n";
	//lst += "      \\override InstrumentName #'X-offset = #1\n";
	lst += "      \\override InstrumentName #'font-series = #'bold\n";
	lst += "      \\override InstrumentName.font-size = #-2\n";
	lst += "      \\set shortVocalName = \"H:\"\n";
	int pos = -1;
	for (hs = 0; hs < hli.size(); ++hs) {
		s = hli[hs];
		lst += SendLySkips(s - pos - 1);
		pos = s;
		lst += "  \\markup{ ";
		lst += "  \\teeny \n";
		if (lyv[v].s[s][vHarm].start || lyv[v].s[s][vHarm].fin) {
			lst += "  \\on-color #(rgb-color " + GetLyMarkColor(lyv[v].s[s][vHarm].sev) + ") ";
		}
		lst += "  \\pad-markup #0.4 \n";
		st = GetHarmName(chm[hs], chm_fis[hs], chm_gis[hs]);
		if (chm[hs] != -1 && show_harmony_bass) {
			if (show_harmony_bass == 1) {
				if (hbc[hs] % 7 != chm[hs]) {
					st += "/" +
						GetRealNoteNameCP(hbcc[hs] % 12);
				}
			}
			else {
				if (cctp[hs][3] == 2) {
					if ((hbc[hs] % 7 - chm[hs] + 7) % 7 == 2) {
						st += "6/5";
					}
					else if ((hbc[hs] % 7 - chm[hs] + 7) % 7 == 4) {
						st += "4/3";
					}
					else if ((hbc[hs] % 7 - chm[hs] + 7) % 7 == 6) {
						st += "4/2";
					}
					else {
						st += "7";
					}
				}
				else {
					if ((hbc[hs] % 7 - chm[hs] + 7) % 7 == 2) {
						st += "6";
					}
					else if ((hbc[hs] % 7 - chm[hs] + 7) % 7 == 4) {
						st += "6/4";
					}
				}
			}
		}
		st.Replace("#", " \"#\" ");
		st.Replace("b", " \\raise #0.3 \\magnify #0.5 \\flat ");
		if (st.Right(1) == "6") {
			st.Replace("6", " \\raise #0.7 6");
			lst += "\\concat { " + st + " } ";
		}
		else if (st.Right(1) == "7") {
			st.Replace("7", " \\raise #0.7 7");
			lst += "\\concat { " + st + " } ";
		}
		else if (st.Right(3) == "6/4") {
			lst += "  \\concat { \n";
			lst += "    \\general-align #Y #0.5 {" + st.Left(st.GetLength() - 3) + "}\n";
			lst += "    \\teeny\n";
			lst += "    \\override #'(baseline-skip . 1.5) \n";
			lst += "    \\override #'(line-width . 100)  \n";
			lst += "    \\center-column{ 6 4 } \n";
			lst += "  }\n";
		}
		else if (st.Right(3) == "6/5") {
			lst += "  \\concat { \n";
			lst += "    \\general-align #Y #0.5 {" + st.Left(st.GetLength() - 3) + "}\n";
			lst += "    \\teeny\n";
			lst += "    \\override #'(baseline-skip . 1.5) \n";
			lst += "    \\override #'(line-width . 100)  \n";
			lst += "    \\center-column{ 6 5 } \n";
			lst += "  }\n";
		}
		else if (st.Right(3) == "4/3") {
			lst += "  \\concat { \n";
			lst += "    \\general-align #Y #0.5 {" + st.Left(st.GetLength() - 3) + "}\n";
			lst += "    \\teeny\n";
			lst += "    \\override #'(baseline-skip . 1.5) \n";
			lst += "    \\override #'(line-width . 100)  \n";
			lst += "    \\center-column{ 4 3 } \n";
			lst += "  }\n";
		}
		else if (st.Right(3) == "4/2") {
			lst += "  \\concat { \n";
			lst += "    \\general-align #Y #0.5 {" + st.Left(st.GetLength() - 3) + "}\n";
			lst += "    \\teeny\n";
			lst += "    \\override #'(baseline-skip . 1.5) \n";
			lst += "    \\override #'(line-width . 100)  \n";
			lst += "    \\center-column{ 4 2 } \n";
			lst += "  }\n";
		}
		else {
			lst += "\\concat { " + st + " } ";
		}
		lst += "}8\n";
	}
	lst += "    }\n";
	lst += "  }\n";
	ly_ly_st += lst;
}

void CP2Ly::SendLyNoteNames() {
	CString st;
	if (!lyv[v].note_names) return;
	st.Format("  \\new Lyrics \\with { alignBelowContext = \"staff%d\" } {\n", v);
	ly_ly_st += st;
	ly_ly_st += "    \\lyricmode {\n";
	ly_ly_st += "      \\override StanzaNumber.font-size = #-2\n";
	ly_ly_st += "      \\set stanza = #\" Note:\"\n";
	//ly_ly_st += "      \\override InstrumentName #'X-offset = #1\n";
	ly_ly_st += "      \\override InstrumentName #'font-series = #'bold\n";
	ly_ly_st += "      \\override InstrumentName.font-size = #-2\n";
	ly_ly_st += "      \\set shortVocalName = \"N:\"\n";
	for (s = 0; s < c_len; ++s) {
		if (!lyv[v].s[s][vNoteName].start && !lyv[v].s[s][vNoteName].fin) {
			ly_ly_st += SendLySkips(1);
			continue;
		}
		CString st = GetLyNoteVisualCP("\\raise #0.3 \\magnify #0.5 ");
		ly_ly_st += "\\markup{ ";
		ly_ly_st += "\\teeny ";
		if (lyv[v].s[s][vNoteName].sev > -1) {
			ly_ly_st += " \\on-color #(rgb-color " + GetLyMarkColor(lyv[v].s[s][vNoteName].sev) + ") ";
		}
		ly_ly_st += " \\pad-markup #0.4 \\concat { " + st + " } ";
		ly_ly_st += "}\n";
	}
	ly_ly_st += "    }\n";
	ly_ly_st += "  }\n";
}

void CP2Ly::SendLyMistakes() {
	CString st;
	if (!lyv[v].flags_noharm) return;
	st.Format("  \\new Lyrics \\with { alignAboveContext = \"staff%d\" } {\n", v);
	ly_ly_st += st;
	ly_ly_st += "    \\lyricmode {\n";
	ly_ly_st += "      \\override StanzaNumber.font-size = #-2\n";
	ly_ly_st += "      \\set stanza = #\" Flags:\"\n";
	for (s = 0; s < c_len; ++s) {
		int max_fss = lyv[v].fss2[s].size();
		if (!max_fss) {
			ly_ly_st += "      \\skip 8\n";
			continue;
		}
		ls = bli[v][s];
		ly_ly_st += "      \\markup{ \\teeny \\override #`(direction . ,UP) \\override #'(baseline-skip . 1.6) { \\dir-column {\n";
		// Do not show too many mistakes
		if (lyv[v].fss2[s].size() > 3) {
			max_fss = 3;
			ly_ly_st += "...\n";
		}
		for (int ff = max_fss - 1; ff >= 0; --ff) {
			int f = lyv[v].fss2[s][ff].second;
			int fl = lyv[v].f[s][f].fid;
			int sev = lyv[v].f[s][f].fsev;
			st.Format("        \\with-color #(rgb-color " +
				GetLyColor(sev) + ") %s %d\n", // \\circle 
				lyv[v].f[s][f].sh || lyv[v].f[s][f].shide ? "\\underline" : "", lyv[v].f[s][f].dfgn);
			// \override #'(offset . 5) \override #'(thickness . 2) 
			ly_ly_st += st;
		}
		ly_ly_st += "      } } }8\n";
		//ly_ly_st += SendLySkips(ly_mul - 1);
	}	
	ly_ly_st += "    }\n";
	ly_ly_st += "  }\n";
}

void CP2Ly::SendLyHarmMistakes() {
	CString st;
	if (v) return;
	if (!lyv[v].flags_harm) return;
	st.Format("  \\new Lyrics \\with { alignBelowContext = \"staff%d\" } {\n", v);
	ly_ly_st += st;
	ly_ly_st += "    \\lyricmode {\n";
	ly_ly_st += "      \\override StanzaNumber.font-size = #-2\n";
	ly_ly_st += "      \\set stanza = #\" Flags:\"\n";
	for (s = 0; s < c_len; ++s) {
		int max_fss = lyv[v].fss3[s].size();
		if (!max_fss) {
			ly_ly_st += "      \\skip 8\n";
			continue;
		}
		ls = bli[v][s];
		ly_ly_st += "      \\markup{ \\teeny \\override #'(baseline-skip . 1.6) { \\dir-column {\n";
		// Do not show too many mistakes
		if (lyv[v].fss3[s].size() > 3) {
			max_fss = 3;
		}
		for (int ff = 0; ff < max_fss; ++ff) {
			int f = lyv[v].fss3[s][ff].second;
			int fl = lyv[v].f[s][f].fid;
			int sev = lyv[v].f[s][f].fsev;
			st.Format("        \\with-color #(rgb-color " +
				GetLyColor(sev) + ") %s %d\n", // \\circle 
				lyv[v].f[s][f].sh || lyv[v].f[s][f].shide ? "\\underline" : "", lyv[v].f[s][f].dfgn);
			// \override #'(offset . 5) \override #'(thickness . 2) 
			ly_ly_st += st;
		}
		// Do not show too many mistakes
		if (lyv[v].fss3[s].size() > 3) {
			ly_ly_st += "...\n";
		}
		ly_ly_st += "      } } }8\n";
		//ly_ly_st += SendLySkips(ly_mul - 1);
	}
	ly_ly_st += "    }\n";
	ly_ly_st += "  }\n";
}

void CP2Ly::SendLyViz(int phase) {
	int shape, sev;
	if (!lyv[v].s.size()) return;
	for (int task = ssFinish; task >= ssStart; --task) {
		for (int shape = 0; shape<MAX_VIZ; ++shape) {
			CString script = shinfo[shape].script[task][phase];
			if (script.IsEmpty()) continue;
			if (task == ssFinish) {
				if (!lyv[v].s[s][shape].fin) continue;
				sev = lyv[v].s[s + lyv[v].s[s][shape].start_s][shape].sev;
			}
			if (task == ssStart) {
				if (!lyv[v].s[s][shape].start) continue;
				sev = lyv[v].s[s][shape].sev;
			}
			CString shtext;
			int gn = 0;
			if (lyv[v].s[s][shape].fl != -1) {
				// Get shape voice
				int fv = lyv[v].s[s][shape].fv;
				// Get flag of shape
				LY_Flag F = lyv[fv].f[lyv[v].s[s][shape].fs][lyv[v].s[s][shape].fl];
				if (fv != F.v) {
					// Get base flag
					LY_Flag F2 = lyv[F.v].f[F.s_base][F.f_base];
					gn = F2.dfgn;
				}
				else {
					gn = F.dfgn;
				}
				// Remove flag number for parallel pco apart
				ls3 = bli[fv][F.s_src];
				ls4 = bli[fv][F.sl_src];
				if (shape == vGlis && 
					(ls3 - ls4 > 1 || F.sl_src < sus[fv][ls4] || 
					(sus[fv][ls3] && F.s_src >= sus[fv][ls3]))) 
					gn = 0;
			}
			if (!gn) {
				shtext = lyv[v].s[s][shape].txt;
				shtext.Replace("!fn!", "");
			}
			else {
				CString fl_st;
				fl_st.Format("%d", gn);
				if (lyv[v].s[s][shape].txt.IsEmpty() || lyv[v].s[s][shape].txt == "!fn!") {
					shtext = lyv[v].s[s][shape].txt;
				}
				else {
					shtext = fl_st + ". " + lyv[v].s[s][shape].txt;
				}
				shtext.Replace("!fn!", fl_st);
			}
			CString text2;
			if (lyv[v].s[s][shape].txt.IsEmpty()) {
				text2 = "#f\n ";
			}
			else {
				text2 = "\\markup{ \\raise #0.6 \\teeny \"" + shtext + "\" }\n ";
			}
			if (shtext.IsEmpty()) {
				if (shinfo[shape].empty_space == 1) shtext = " ";
				if (shinfo[shape].empty_space == 2) shtext = "_";
			}
			script.Replace("$n$", "\n");
			script.Replace("$COLOR$", GetLyColor(sev));
			script.Replace("$TEXT$", shtext); 
			script.Replace("$TEXT2$", text2);
			ly_ly_st += script + "\n";
		}
	}
}

void CP2Ly::SaveLyComments() {
	CString st, com, note_st;
	int pos1, pos2, found;
	if (!lyv[v].f.size()) return;
	for (s = 0; s < c_len; ++s) {
		ls = bli[v][s];
		note_st = "\\markup \\wordwrap \\tiny \\bold {\n  ";
		// Show voice number if more than 1 voice
		if (av_cnt > 1) {
			//st.Format("%d. %s", av_cnt - v, vname[vid[v]]);
			note_st += vname2[vid[v]];
		}
		st.Format(" [bar %d, beat %d] note \\concat { %s } ", // ly_nnum
			s / npm + 1, (s % npm) / 2 + 1,
			GetLyNoteVisualCP("\\raise #0.3 \\magnify #0.7 "));
		if (fli[v][ls] != s)
			st += " (middle)";
		note_st += st + "\n}\n";
		found = 0;
		for (int ff = 0; ff < lyv[v].fss[s].size(); ++ff) {
			int f = lyv[v].fss[s][ff].second;
			int fl = lyv[v].f[s][f].fid;
			int sev = lyv[v].f[s][f].fsev;
			if (!accept[sp][vc][vp][fl]) st = "- ";
			else if (accept[sp][vc][vp][fl] == -1) st = "$ ";
			else st = "+ ";
			CString rule_name = GetRuleName(fl, sp, vc, vp);
			//rule_name.SetAt(0, rule_name.Left(1).MakeLower().GetAt(0));
			if (ly_debugexpect) {
				CString st2;
				st2.Format("[%d/%d] ", fl, s + 1);
				rule_name = st2 + rule_name;
			}
			else {
				if (!ly_rule_verbose) {
					if (rule_name.Find(":") != -1) {
						rule_name = rule_name.Left(rule_name.Find(":"));
					}
				}
			}
			com = st + ruleinfo[fl].RuleClass + ": " + rule_name;
			CString subrule_name = GetSubRuleName(fl, sp, vc, vp);
			// Always hide hidden subrule names starting with /
			if (subrule_name.Left(1) == "/") subrule_name.Empty();
			// If minimum verbosity, hide all subrule names except starting with :
			if (!ly_rule_verbose) {
				if (subrule_name.Left(1) != ":") subrule_name.Empty();
			}
			if (!subrule_name.IsEmpty()) {
				// Always remove :
				if (subrule_name.Left(1) == ":") {
					subrule_name = subrule_name.Mid(1);
				}
				com += " (" + subrule_name + ")";
			}
			if (ly_rule_verbose > 1 && !GetRuleComment(fl, sp, vc, vp).IsEmpty())
				com += ". " + GetRuleComment(fl, sp, vc, vp);
			if (ly_rule_verbose > 1 && !GetSubRuleComment(fl, sp, vc, vp).IsEmpty())
				com += " (" + GetSubRuleComment(fl, sp, vc, vp) + ")";
			//st.Format("%d", lyv[v].f[s].vl[f]);
			//com += " " + st;
			// Print link to other part and step
			CString sl_st;
			sl_st.Format("bar %d, beat %d", lyv[v].f[s][f].sl_src / npm + 1, (lyv[v].f[s][f].sl_src % npm) / 2 + 1);
			if (lyv[v].f[s][f].vl != v && av_cnt > 2) {
				com += " - with " + vname2[vid[lyv[v].f[s][f].vl]];
				if (lyv[v].f[s][f].sl_src != lyv[v].f[s][f].s_src) {
					com += ", " + sl_st;
				}
			}
			else {
				if (lyv[v].f[s][f].sl_src < lyv[v].f[s][f].s_src) {
					com += " - from " + sl_st;
				}
				else if (lyv[v].f[s][f].sl_src > lyv[v].f[s][f].s_src) {
					com += " - to " + sl_st;
				}
			}
			// Send note number with first comment
			if (!found) {
				found = 1;
				ly_com_st += note_st;
			}
			ly_com_st += "\\markup \\smaller \\wordwrap \\with-color #(rgb-color " +
				GetLyColor(sev) + ") {\n  ";
			com.Replace("\"", "\\\"");
			com.Replace(" ", "\" \"");
			st.Format("%d \"", lyv[v].f[s][f].dfgn); // \\teeny \\raise #0.2 \\circle
			ly_com_st += st;
			ly_com_st += com + "\"\n";
			ly_com_st += "\n}\n";
		}
	}
}

// Send note or pause
void CP2Ly::SendLyEvent(CString ev, int leng) {
	// Length array
	vector<int> la;
	la.push_back(leng);
	SplitLyNoteMeasure(s, la);
	SplitLyNote(s, la);
	for (int lc = 0; lc < la.size(); ++lc) {
		SendLyViz(1);
		SendLyViz(2);
		ly_ly_st += ev + GetLyLen(la[lc]);
		if (lc < la.size() - 1 && ev != "r") ly_ly_st += "~";
		ly_ly_st += "\n";
		SendLyViz(8);
		SendLyViz(9);
		SendLyViz(10);
		SendLyViz(11);
		SendLyViz(12);
		if (s > -1) {
			s += la[lc];
		}
	}
	s -= leng;
}

void CP2Ly::CheckLyCP() {
	ly_flags = 0;
	for (int s = 0; s < c_len; ++s) {
		for (int v = 0; v < av_cnt; ++v) {
			ly_flags += flag[v][s].size();
		}
	}
}

// Split note at first measure border
void CP2Ly::SplitLyNoteMeasure(int pos, vector<int> &la) {
	int inpos = pos % npm;
	while (inpos + la[0] > npm) {
		// Remove last short note part
		int left = ((inpos + la[0]) / npm) * npm - inpos;
		// If there is no short note part, remove whole measure
		if (left == la[0]) left -= npm;
		// Convert first part to second
		la[0] = la[0] - left;
		// Add first part
		vpush_front(la, left, 1);
	}
}

// Split note of length 5
void CP2Ly::SplitLyNote5(int pos, int i, vector<int> &la) {
	if (pos % 4 == 0) {
		la[i] = 1;
		la.insert(la.begin() + i, 4);
	}
	else if (pos % 4 == 1) {
		la[i] = 2;
		la.insert(la.begin() + i, 3);
	}
	else if (pos % 4 == 2) {
		la[i] = 3;
		la.insert(la.begin() + i, 2);
	}
	else if (pos % 4 == 3) {
		la[i] = 4;
		la.insert(la.begin() + i, 1);
	}
}

// Split note of length 9
void CP2Ly::SplitLyNote9(int pos, int i, vector<int> &la) {
	if (pos % 4 == 0) {
		la[i] = 1;
		la.insert(la.begin() + i, 8);
	}
	else if (pos % 4 == 1) {
		la[i] = 8;
		la.insert(la.begin() + i, 1);
	}
	else if (pos % 4 == 2) {
		la[i] = 1;
		la.insert(la.begin() + i, 8);
	}
	else if (pos % 4 == 3) {
		la[i] = 8;
		la.insert(la.begin() + i, 1);
	}
}

// Split note of length 10
void CP2Ly::SplitLyNote10(int pos, int i, vector<int> &la) {
	if (pos % 4 == 0) {
		la[i] = 2;
		la.insert(la.begin() + i, 8);
	}
	else if (pos % 4 == 1) {
		la[i] = 1;
		la.insert(la.begin() + i, 8);
		la.insert(la.begin() + i, 1);
	}
	else if (pos % 4 == 2) {
		la[i] = 8;
		la.insert(la.begin() + i, 2);
	}
}

// Split note of length 11
void CP2Ly::SplitLyNote11(int pos, int i, vector<int> &la) {
	if (pos % 4 == 0) {
		la[i] = 3;
		la.insert(la.begin() + i, 8);
	}
	else if (pos % 4 == 1) {
		la[i] = 2;
		la.insert(la.begin() + i, 8);
		la.insert(la.begin() + i, 1);
	}
}

// Create la array of common lengths if note is too long for single note
void CP2Ly::SplitLyNote(int pos, vector<int> &la) {
	int curpos = pos;
	for (int i = 0; i < la.size(); ++i) {
		if (la[i] == 5) SplitLyNote5(curpos % npm, i, la);
		if (la[i] == 9) SplitLyNote9(curpos % npm, i, la);
		if (la[i] == 10) SplitLyNote10(curpos % npm, i, la);
		if (la[i] == 11) SplitLyNote11(curpos % npm, i, la);
		curpos = pos + la[i];
	}
}

CString CP2Ly::GetRealIntName(int s, int v1, int v2) {
	// Exact interval
	int in = abs(cc[v2][s] - cc[v][s]);
	// Get lower and higher voices
	int lv, hv;
	if (cc[v1][s] > cc[v2][s]) {
		lv = v2;
		hv = v1;
	}
	else {
		lv = v1;
		hv = v2;
	}
	// Get major notes
	int low_maj_note = (cc[lv][s] - bn + 12 + mode) % 12;
	int high_maj_note = (cc[hv][s] - bn + 12 + mode) % 12;
	// Get interval name string
	CString iname = IntName[low_maj_note][high_maj_note];
	// Are there separate interval names for below and above octave?
	if (iname.Find("|") != -1) {
		// Return second name for long interval
		if (in > 11) {
			return iname.Mid(iname.Find("|") + 1);
		}
		// Return first name for short interval
		else {
			return iname.Left(iname.Find("|"));
		}
	}
	return iname;
}

void CP2Ly::ValidateShapeText() {
	for (int sh = 0; sh < MAX_VIZ; ++sh) {
		if (shinfo[sh].has_text == 1 && shinfo[sh].can_text == 0) {
			CString est;
			est.Format("I did not expect shape %d to have $TEXT macro in script",
				sh);
			WriteLog(5, est);
		}
		if (shinfo[sh].has_text == 1 && shinfo[sh].can_text == 0) {
			CString est;
			est.Format("I expected shape %d to have $TEXT macro in script",
				sh);
			WriteLog(5, est);
		}
	}
	for (int rid = 0; rid <= max_rule; ++rid) {
		if (ruleinfo[rid].viz_text.IsEmpty()) {
			if (shinfo[ruleinfo[rid].viz].has_text) {
				CString est;
				est.Format("Rule [%d] " + ruleinfo[rid].RuleName + " (" + ruleinfo[rid].SubRuleName + ") has no viz_text, but this shape %d has $TEXT in script",
					rid, ruleinfo[rid].viz);
				WriteLog(5, est);
				// If this shape cannot work with empty strings, replace with a space
				if (shinfo[ruleinfo[rid].viz].empty_space == 1) ruleinfo[rid].viz_text = " ";
				if (shinfo[ruleinfo[rid].viz].empty_space == 2) ruleinfo[rid].viz_text = "_";
			}
		}
		else {
			if (!shinfo[ruleinfo[rid].viz].has_text) {
				CString est;
				est.Format("Rule [%d] " + ruleinfo[rid].RuleName + " (" + ruleinfo[rid].SubRuleName + ") has viz_text '" + ruleinfo[rid].viz_text + "', but this shape %d does not have $TEXT in script",
					rid, ruleinfo[rid].viz);
				WriteLog(5, est);
			}
		}
	}
}

