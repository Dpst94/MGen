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
		s3 = isus[v][bli[v][s]];
		s4 = isus[v][bli[v][fsl[v][s][f]]];
	}
	lyi[s3].fhide.push_back(0);
	// Check if this shape is not allowed at hidden position
	if (!viz_anyposition[ruleinfo[fl].viz]) {
		// Note start is ok
		// Downbeat is ok
		if ((s3 != fli[v][bli[v][s3]] && s3 % npm) ||
			(s4 != fli[v][bli[v][s4]] && s4 % npm)) {
			lyi[s3].fhide[lyi[s3].fhide.size() - 1] = 1;
			if (viz_can_separate[ruleinfo[fl].viz]) {
				fsep[v][s][f] = 1;
			}
			else {
				CString est;
				est.Format("Detected shape at hidden position %d-%d (instead of %d-%d), voice %d counterpoint %d: [%d] %s (%s). Cannot send this type of shapes to separate staff",
					s3, s4, isus[v][bli[v][s3]], isus[v][bli[v][s4]], v, cp_id + 1, fl,
					ruleinfo[fl].RuleName, ruleinfo[fl].SubRuleName);
				WriteLog(5, est);
			}
		}
	}
	lyi[s3].nflags.push_back(fl);
	lyi[s3].fsev.push_back(severity[sp][vc][vp][fl]);
	lyi[s3].fsl.push_back(s4 - s3);
	lyi[s3].fv.push_back(v);
	lyi[s3].fvl.push_back(fvl[v][s][f]);
	lyi[s3].nfn.push_back(ly_flags + 1);
	lyi[s3].nfs.push_back(0);
	lyi[s3].nfc.push_back("");
	++ly_flags;
	++ly_vflags;
}

// Parse foreign flags from other voices: gliss and notecolor
void CP2Ly::AddNLinkForeign(int f) {
	GetFlag(f);
	int s3 = s;
	int s4 = fsl[v][s][f];
	if (ruleinfo[fl].viz == vGlis) {
		s3 = isus[v2][bli[v2][s]];
		s4 = isus[v2][bli[v2][fsl[v][s][f]]];
	}
	lyi[s3].fhide.push_back(0);
	// Do not check for hidden positions, because gliss pos is corrected and notecolor pos will be corrected
	// Send comments and color only if rule is not ignored
	if (accept[sp][vc][vp][fl] == -1 && !show_ignored_flags) return;
	// Send comments and color only if rule is not ignored
	if (accept[sp][vc][vp][fl] == 1 && !show_allowed_flags) return;
	// Do not send if ignored
	if (severity[sp][vc][vp][fl] < show_min_severity) return;
	lyi[s3].nflags.push_back(fl);
	lyi[s3].fsev.push_back(severity[sp][vc][vp][fl]);
	lyi[s3].fsl.push_back(s4 - s3);
	lyi[s3].fv.push_back(v);
	lyi[s3].fvl.push_back(fvl[v][s][f]);
	lyi[s3].nfn.push_back(0);
	lyi[s3].nfs.push_back(0);
	lyi[s3].nfc.push_back("");
}

void CP2Ly::AddNLinkSep(int f) {
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
	lyi[s3].nflags.push_back(fl);
	lyi[s3].fsev.push_back(severity[sp][vc][vp][fl]);
	lyi[s3].fsl.push_back(s4 - s3);
	lyi[s3].fv.push_back(v);
	lyi[s3].fvl.push_back(fvl[v][s][f]);
	lyi[s3].nfn.push_back(0);
	lyi[s3].nfs.push_back(0);
	lyi[s3].nfc.push_back("");
	++ly_flags;
}

void CP2Ly::ParseNLinks() {
	CString com;
	for (int f = 0; f < flag[v][s].size(); ++f) {
		AddNLink(f);
	}
	v2 = v;
	for (v = 0; v < av_cnt; ++v) if (v != v2) {
		for (int f = 0; f < flag[v][s].size(); ++f) {
			if (fvl[v][s][f] != v2) continue;
			//if (ruleinfo[flag[v][s][f]].viz != vGlis) continue;
			AddNLinkForeign(f);
		}
	}
	v = v2;
}

void CP2Ly::ParseNLinksSep() {
	CString com;
	for (v = 0; v < av_cnt; ++v) {
		for (int f = 0; f < flag[v][s].size(); ++f) {
			if (!fsep[v][s][f]) continue;
			AddNLinkSep(f);
		}
	}
}

void CP2Ly::SetLyShape(int st1, int st2, int f, int fl, int sev, int vtype) {
	if (lyi[st1].shse[vtype] <= sev) {
		// Start
		lyi[st1].shs[vtype] = 1;
		// Finish
		lyi[st2].shf[vtype] = 1;
		// Link to start
		lyi[st2].shsl[vtype] = s1 - s2;
		lyi[st1].shse[vtype] = sev;
		if (vtype == vInterval || vtype == vNoteName || vtype == vHarm) {
			if (lyi[st2].shse[vtype] <= sev) {
				lyi[st2].shse[vtype] = sev;
			}
			if (vtype == vNoteName) ++ly_notenames;
		}
		lyi[st1].sht[vtype] = ruleinfo[fl].viz_text;
		// Save flag shape (step depends if link is forward or backward)
		lyi[s].nfs[f] = vtype;
		lyi[st1].shflag[vtype] = f;
		lyi[st1].shfp[vtype] = s;
	}
}

void CP2Ly::ClearLyShape(int st1, int st2, int vtype) {
	lyi[st1].shs[vtype] = 0;
	lyi[st2].shf[vtype] = 0;
	// Calculate maximum severity
	lyi[st1].shse[vtype] = -1;
	// Remove link
	lyi[lyi[st1].shfp[vtype]].nfs[lyi[st1].shflag[vtype]] = 0;
	lyi[st1].shflag[vtype] = -1;
	lyi[st1].shfp[vtype] = -1;
}

void CP2Ly::InitLyI() {
	ly_vflags = 0;
	ly_notenames = 0;
	lyi.clear();
	lyi.resize(c_len + 1);
	for (int i = 0; i < lyi.size(); ++i) {
		// Init vectors
		lyi[i].shs.resize(MAX_VIZ);
		lyi[i].shsl.resize(MAX_VIZ);
		lyi[i].shf.resize(MAX_VIZ);
		lyi[i].shse.resize(MAX_VIZ, -1);
		lyi[i].shflag.resize(MAX_VIZ, -1);
		lyi[i].shfp.resize(MAX_VIZ, -1);
		lyi[i].sht.resize(MAX_VIZ);
	}
}

void CP2Ly::ParseLyI() {
	for (s = 0; s < c_len; ++s) {
		ls = bli[v][s];
		s2 = fli[v][ls];
		// Find next note position
		int next_note_step = fli[v][min(ls + 1, fli_size[v] - 1)];
		// Find previous note position
		int prev_note_step = fli[v][max(0, ls - 1)];
		// Parse flags
		for (int f = 0; f < lyi[s].nflags.size(); ++f) {
			int fl = lyi[s].nflags[f];
			int link = lyi[s].fsl[f];
			int vlink = lyi[s].fvl[f];
			int vtype = ruleinfo[fl].viz;
			int sev = lyi[s].fsev[f];
			int skip_shape = 0;
			int link_note_step = fli[v][bli[v][s + link]];
			// Previous note before link
			int prev_link_note = fli[v][max(0, bli[v][s + link] - 1)];
			if (lyi[s].fhide[f]) vtype = 0;
			if (ly_debugexpect && sev == 100) vtype = 0;
			// Get flag start/stop
			s1 = min(s, s + link);
			s2 = max(s, s + link);
			// If shape cannot highlight single note, but flag does not contain link, then link to next note
			if (!viz_singlenote[vtype] && s1 == s2) s2 = next_note_step;
			// Highlight notes if flag is multivoice and is not gliss (gliss does not need note color)
			if (vlink != lyi[s].fv[f] && vtype != vGlis) {
				for (ls = bli[v][s1]; ls <= bli[v][s2]; ++ls) {
					SetLyShape(fli[v][ls], fli[v][ls], f, fl, sev, vNoteColor);
				}
				SetLyShape(link_note_step, link_note_step, f, fl, sev, vNoteColor);
			}
			// Skip all foreign shapes (show only note color)
			if (v != lyi[s].fv[f] && vtype != vGlis) continue;
			// Set interval if not debugexpect. If debugexpect, do not set for red flags
			if (!ly_debugexpect || sev != 100) {
				if (ruleinfo[fl].viz_int == 1) {
					SetLyShape(s1, s2, f, fl, sev, vInterval);
				}
				if (ruleinfo[fl].viz_int == 2) {
					SetLyShape(s1, s1, f, fl, sev, vInterval);
				}
				if (ruleinfo[fl].viz_int == 3) {
					SetLyShape(s2, s2, f, fl, sev, vInterval);
				}
				if (ruleinfo[fl].viz_harm == 1) {
					SetLyShape(s1, s2, f, fl, sev, vHarm);
				}
				if (ruleinfo[fl].viz_harm == 2) {
					SetLyShape(s1, s1, f, fl, sev, vHarm);
				}
				if (ruleinfo[fl].viz_harm == 3) {
					SetLyShape(s2, s2, f, fl, sev, vHarm);
				}
			}
			if (vtype == vPedal) {
				if (bli[v][s1] == fli_size[v] - 1 && s1 == s2) continue;
			}
			if (!viz_can_overlap[vtype]) {
				// Check that flag overlaps
				int overlap1 = -1;
				int overlap2 = -1;
				int overlap_border = 0;
				// For groups check for collision between borders
				if (viz_type[vtype] == vtGroup || viz_type[vtype] == vtVolta)
					overlap_border = 1;
				// For vbrackets check for collision between notes
				int overlap_limit = s1 - overlap_border;
				if (viz_type[vtype] == vtVBracket)
					overlap_limit = min(prev_note_step, prev_link_note) - 1;
				// Check if shape can be blocked
				for (int x = c_len - 1; x > overlap_limit; --x) {
					if (lyi[x].shf[vtype]) {
						overlap2 = x;
						overlap1 = x + lyi[x].shsl[vtype];
						if (overlap1 < s2 + overlap_border) {
							// Choose highest severity
							if (sev <= lyi[overlap1].shse[vtype]) {
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
					if (lyi[x].shf[vtype]) {
						overlap2 = x;
						overlap1 = x + lyi[x].shsl[vtype];
						if (overlap1 < s2 + overlap_border) {
							// Choose highest severity
							if (sev > lyi[overlap1].shse[vtype]) {
								ClearLyShape(overlap1, overlap2, vtype);
							}
						}
					}
				}
			}
			SetLyShape(s1, s2, f, fl, sev, vtype);
		}
	}
#if defined(_DEBUG)
	ExportLyI();
#endif
}

void CP2Ly::ParseLyISep() {
	for (s = 0; s < c_len; ++s) {
		// Find next note position
		int next_note_step = min(s + 1, c_len - 1);
		// Find previous note position
		int prev_note_step = max(s - 1, 0);
		// Parse flags
		for (int f = 0; f < lyi[s].nflags.size(); ++f) {
			int fl = lyi[s].nflags[f];
			int link = lyi[s].fsl[f];
			int vlink = lyi[s].fvl[f];
			int vtype = ruleinfo[fl].viz;
			int sev = lyi[s].fsev[f];
			int skip_shape = 0;
			int link_note_step = s + link;
			// Previous note before link
			int prev_link_note = max(0, s + link - 1);
			// Get flag start/stop
			s1 = min(s, s + link);
			s2 = max(s, s + link);
			// If shape cannot highlight single note, but flag does not contain link, then link to next note
			if (!viz_singlenote[vtype] && s1 == s2) s2 = next_note_step;
			// Set interval if not debugexpect. If debugexpect, do not set for red flags
			if (!viz_can_overlap[vtype]) {
				// Check that flag overlaps
				int overlap1 = -1;
				int overlap2 = -1;
				int overlap_border = 0;
				// For groups check for collision between borders
				if (viz_type[vtype] == vtGroup || viz_type[vtype] == vtVolta)
					overlap_border = 1;
				// For vbrackets check for collision between notes
				int overlap_limit = s1 - overlap_border;
				if (viz_type[vtype] == vtVBracket)
					overlap_limit = min(prev_note_step, prev_link_note) - 1;
				// Check if shape can be blocked
				for (int x = c_len - 1; x > overlap_limit; --x) {
					if (lyi[x].shf[vtype]) {
						overlap2 = x;
						overlap1 = x + lyi[x].shsl[vtype];
						if (overlap1 < s2 + overlap_border) {
							// Choose highest severity
							if (sev <= lyi[overlap1].shse[vtype]) {
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
					if (lyi[x].shf[vtype]) {
						overlap2 = x;
						overlap1 = x + lyi[x].shsl[vtype];
						if (overlap1 < s2 + overlap_border) {
							// Choose highest severity
							if (sev > lyi[overlap1].shse[vtype]) {
								ClearLyShape(overlap1, overlap2, vtype);
							}
						}
					}
				}
			}
			SetLyShape(s1, s2, f, fl, sev, vtype);
		}
	}
#if defined(_DEBUG)
	ExportLyI();
#endif
}

void CP2Ly::ExportLyI() {
	ofstream fs;
	fs.open(as_dir + "\\lyi-" + as_fname + ".csv", ios_base::app);
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
			fs << lyi[s].shs[x] << ";";
			fs << lyi[s].shf[x] << ";";
			fs << lyi[s].shsl[x] << ";";
			fs << lyi[s].shse[x] << ";";
			fs << lyi[s].sht[x] << ";";
		}
		fs << "\n";
	}
	fs << "\n";
	fs.close();
}

// Init flags separate staff
void CP2Ly::InitFSep() {
	fsep.resize(av_cnt);
	for (v = 0; v < av_cnt; ++v) {
		fsep[v].resize(c_len);
		for (s = 0; s < c_len; ++s) {
			fsep[v][s].resize(flag[v][s].size());
		}
	}
}

void CP2Ly::SaveLyCP() {
	InitFSep();
	ly_flags = 0;
	vector<CString> sv;
	CString clef, key, key_visual;
	int pos, pos2, le, le2, pause_accum, pause_pos, pause_i;
	ly_com_st.Empty();
	ly_ly_st.Empty();
	// Voice melody min pitch
	vector<int> vm_min;
	// Voice melody max pitch
	vector<int> vm_max;
	for (v = 0; v < av_cnt; ++v) {
		GetMelodyInterval(0, c_len);
	}
	// Key
	if (mode == 9) {
		key = LyMinorKey[bn];
	}
	else {
		key = LyMajorKey[bn];
	}
	key_visual = key[0];
	key_visual.MakeUpper();
	if (key[1] == 'f') key_visual += "\\flat ";
	if (key[1] == 's') key_visual = "\"" + key_visual + "#\"";
	// First info
	CString st, st3;
	ly_ly_st += "\\markup \\wordwrap \\bold {\n  ";
	ly_ly_st += "    \\vspace #3\n";
	st.Format("\"#\"%d (from %s)",
		cp_id + 1, bname_from_path(musicxml_file));
	ly_ly_st += st + " Key: " + key_visual;
	ly_ly_st += " " + mode_name[mode];
	if (mode == 9) {
		if (mminor) ly_ly_st += " (melodic)";
		else ly_ly_st += " (natural)";
	}
	//else if (mode == 0) ly_ly_st += " major";
	/*
	ly_ly_st += ", Species: ";
	for (v = 0; v < av_cnt; ++v) {
		st.Format("%d", vsp[v]);
		if (v) ly_ly_st += "-";
		if (vsp[v]) ly_ly_st += st;
		else ly_ly_st += "CF";
	}
	*/
	// Init separate staff
	slyi.clear();
	slyi.resize(c_len + 1);
	ly_ly_st += "\n}\n";
	// Save notes
	ly_ly_st += "<<\n";
	for (v = av_cnt - 1; v >= 0; --v) {
		vi = vid[v];
		InitLyI();
		for (s = 0; s < c_len; ++s) {
			ls = bli[v][s];
			ParseNLinks();
		}
		ParseLyI();
		// Select best clef
		clef = DetectLyClef(nmin[v], nmax[v]);
		st.Format("\\new Staff = \"staff%d\" {\n", v);
		ly_ly_st += st;
		if (vsp[v]) st3.Format("species %d", vsp[v]);
		else st3 = "c.f.";
		CString vocra_st;
		if (vocra_detected[v] == 2) {
			vocra_st = " \"[" + vocra_info[vocra[v]].name + "]\"";
		}
		ly_ly_st += "     \n";
		st.Format("  \\set Staff.instrumentName = \\markup { \\tiny \\override #'(baseline-skip . 2.0) \\center-column{ \"%s\" \"[%s]\"%s } }\n", vname2[vi], st3, vocra_st);
		//ly_ly_st += "    \\override #'(line-width . 100)  \n";
		ly_ly_st += st;
		ly_ly_st += "  \\clef \"" + clef + "\"\n  \\key " + LyMajorKey[maj_bn] + "\n";
		ly_ly_st += "  \\major\n";
		st.Format("  \\time %d/4\n", npm / 2);
		ly_ly_st += st;
		read_file_sv("configs\\ly\\staff.ly", sv);
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
		SendLyMistakes();
		SendLyNoteNames();
		//SendLyIntervals();
	}
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
}

void CP2Ly::SendLySeparate() {
	ly_flags = 0;
	vector<CString> sv;
	CString st;
	InitLyI();
	for (s = 0; s < c_len; ++s) {
		ParseNLinksSep();
	}
	if (!ly_flags) return;
	ParseLyISep();
	ly_ly_st += "\\new Staff = \"staffs\" \\with {\n";
	st.Format("  \\time %d/4\n", npm / 2);
	ly_ly_st += st;
	read_file_sv("configs\\ly\\separate-staff.ly", sv);
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

void CP2Ly::SendLyHarm() {
	CString st, lst;
	if (av_cnt < 2) return;
	//if (!ly_flags) return;
	st.Format("  \\new Lyrics \\with { alignBelowContext = \"staff%d\" } {\n", 0);
	lst += st;
	lst += "    \\lyricmode {\n";
	lst += "      \\override StanzaNumber.font-size = #-2\n";
	lst += "      \\set stanza = #\" Harmony:\"\n";
	lst += "      \\override InstrumentName #'X-offset = #1\n";
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
		if (lyi[s].shs[vHarm] || lyi[s].shf[vHarm]) {
			lst += "  \\on-color #(rgb-color " + GetLyMarkColor(lyi[s].shse[vHarm]) + ") ";
		}
		lst += "  \\pad-markup #0.4 \n";
		st = GetHarmName(chm[hs], chm_alter[hs]);;
		if (show_harmony_bass && hbc[hs] % 7 != chm[hs]) {
			if (show_harmony_bass == 1) {
				st += "/" +
					GetRealNoteNameCP(hbcc[hs] % 12);
			}
			else {
				if ((hbc[hs] % 7 - chm[hs] + 7) % 7 == 2) {
					st += "6";
				}
				else {
					st += "6/4";
				}
			}
		}
		st.Replace("#", " \"#\" ");
		st.Replace("b", " \\raise #0.3 \\magnify #0.5 \\flat ");
		if (st.Right(1) == "6") {
			st.Replace("6", " \\raise #0.7 6");
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
	if (!ly_notenames) return;
	st.Format("  \\new Lyrics \\with { alignBelowContext = \"staff%d\" } {\n", v);
	ly_ly_st += st;
	ly_ly_st += "    \\lyricmode {\n";
	ly_ly_st += "      \\override StanzaNumber.font-size = #-2\n";
	ly_ly_st += "      \\set stanza = #\" Note:\"\n";
	ly_ly_st += "      \\override InstrumentName #'X-offset = #1\n";
	ly_ly_st += "      \\override InstrumentName #'font-series = #'bold\n";
	ly_ly_st += "      \\override InstrumentName.font-size = #-2\n";
	ly_ly_st += "      \\set shortVocalName = \"N:\"\n";
	for (s = 0; s < c_len; ++s) {
		if (!lyi[s].shs[vNoteName] && !lyi[s].shf[vNoteName]) {
			ly_ly_st += SendLySkips(1);
			continue;
		}
		CString st = GetLyNoteVisualCP("\\raise #0.3 \\magnify #0.5 ");
		ly_ly_st += "\\markup{ ";
		ly_ly_st += "\\teeny ";
		if (lyi[s].shse[vNoteName] > -1) {
			ly_ly_st += " \\on-color #(rgb-color " + GetLyMarkColor(lyi[s].shse[vNoteName]) + ") ";
		}
		ly_ly_st += " \\pad-markup #0.4 \\concat { " + st + " } ";
		ly_ly_st += "}\n";
	}
	ly_ly_st += "    }\n";
	ly_ly_st += "  }\n";
}

void CP2Ly::SendLyMistakes() {
	CString st;
	if (!ly_flags) return;
	st.Format("  \\new Lyrics \\with { alignAboveContext = \"staff%d\" } {\n", v);
	ly_ly_st += st;
	ly_ly_st += "    \\lyricmode {\n";
	ly_ly_st += "      \\override StanzaNumber.font-size = #-2\n";
	ly_ly_st += "      \\set stanza = #\" Flags:\"\n";
	for (s = 0; s < c_len; ++s) {
		ls = bli[v][s];
		if (!lyi[s].nflags.size()) {
			ly_ly_st += SendLySkips(1);
			continue;
		}
		SaveLyComments();
		ly_ly_st += "      \\markup{ \\teeny \\override #`(direction . ,UP) \\override #'(baseline-skip . 1.6) { \\dir-column {\n";
		int max_mist = lyi[s].nflags.size() - 1;
		// Do not show too many mistakes
		if (max_mist > 2) {
			max_mist = 1;
			ly_ly_st += "...\n";
		}
		for (int f = max_mist; f >= 0; --f) {
			if (!lyi[s].nfn[f]) continue;
			int fl = lyi[s].nflags[f];
			int sev = lyi[s].fsev[f];
			st.Format("        \\with-color #(rgb-color " +
				GetLyColor(sev) + ") %s %d\n", // \\circle 
				lyi[s].nfs[f] || lyi[s].fhide[f] ? "\\underline" : "", lyi[s].nfn[f]);
			// \override #'(offset . 5) \override #'(thickness . 2) 
			ly_ly_st += st;
		}
		ly_ly_st += "      } } }8\n";
		//ly_ly_st += SendLySkips(ly_mul - 1);
	}	
	ly_ly_st += "    }\n";
	ly_ly_st += "  }\n";
}

CString CP2Ly::GetLyNoteVisualCP(CString sz) {
	int no2, oct, alter;
	GetRealNote(cc[v][s], bn, mode == 9, no2, oct, alter);
	return NoteName[no2] + GetLyAlterVisual(alter, sz);
}

void CP2Ly::SendLyViz(int phase) {
	int shape, sev;
	if (!lyi.size()) return;
	for (int task = ssFinish; task >= ssStart; --task) {
		for (auto it : shsc[phase][task]) {
			shape = it.first;
			if (task == ssFinish) {
				if (!lyi[s].shf[shape]) continue;
				sev = lyi[s + lyi[s].shsl[shape]].shse[shape];
			}
			if (task == ssStart) {
				if (!lyi[s].shs[shape]) continue;
				sev = lyi[s].shse[shape];
			}
			CString script = it.second;
			CString text2;
			if (lyi[s].sht[shape].IsEmpty()) {
				text2 = "#f\n ";
			}
			else {
				text2 = "\\markup{ \\raise #0.6 \\teeny \"" + lyi[s].sht[shape] + "\" }\n ";
			}
			script.Replace("$n$", "\n");
			script.Replace("$COLOR$", GetLyColor(sev));
			script.Replace("$TEXT$", lyi[s].sht[shape]);
			script.Replace("$TEXT2$", text2);
			ly_ly_st += script + "\n";
		}
	}
}

void CP2Ly::SaveLyComments() {
	CString st, com, note_st;
	int pos1, pos2, found;
	if (!lyi.size()) return;
	if (lyi[s].nflags.size()) {
		note_st = "\\markup \\wordwrap \\tiny \\bold {\n  ";
		// Show voice number if more than 1 voice
		if (av_cnt > 1) {
			//st.Format("%d. %s", av_cnt - v, vname[vid[v]]);
			note_st += vname2[vid[v]];
		}
		st.Format(" [bar %d, beat %d] note %s", // ly_nnum
			s / 8 + 1, (s % 8) / 2 + 1,
			GetLyNoteVisualCP("\\raise #0.3 \\magnify #0.7 "));
		if (fli[v][ls] != s)
			st += " (middle)";
		note_st += st + "\n}\n";
		found = 0;
		for (int c = 0; c < lyi[s].nflags.size(); ++c) {
			if (!lyi[s].nfn[c]) continue;
			int fl = lyi[s].nflags[c];
			int sev = lyi[s].fsev[c];
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
			if (!ly_rule_verbose) {
				if (subrule_name.Left(1) != ":") subrule_name.Empty();
			}
			if (!subrule_name.IsEmpty()) {
				if (subrule_name.Left(1) == ":") {
					subrule_name = subrule_name.Mid(1);
				}
				com += " (" + subrule_name + ")";
			}
			if (ly_rule_verbose > 1 && !GetRuleComment(fl, sp, vc, vp).IsEmpty())
				com += ". " + GetRuleComment(fl, sp, vc, vp);
			if (ly_rule_verbose > 1 && !GetSubRuleComment(fl, sp, vc, vp).IsEmpty())
				com += " (" + GetSubRuleComment(fl, sp, vc, vp) + ")";
			com += " " + lyi[s].nfc[c];
			// Send note number with first comment
			if (!found) {
				found = 1;
				ly_com_st += note_st;
			}
			ly_com_st += "\\markup \\smaller \\wordwrap \\with-color #(rgb-color " +
				GetLyColor(sev) + ") {\n  ";
			com.Replace("\"", "\\\"");
			com.Replace(" ", "\" \"");
			st.Format("%d \"", lyi[s].nfn[c]); // \\teeny \\raise #0.2 \\circle
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
	SplitLyNote(s, leng, la);
	SplitLyNoteMeasure(s, leng, la);
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

