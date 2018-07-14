// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "CF1Ly.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

CF1Ly::CF1Ly() {
}

CF1Ly::~CF1Ly() {
}

void CF1Ly::GetLyRange(int step1, int step2, vector<int> &vm_min, vector<int> &vm_max) {
	vm_min.clear();
	vm_max.clear();
	vm_min.resize(v_cnt, 128);
	vm_max.resize(v_cnt, 0);
	ly_flags = 0;
	ly_has_lining = 0;
	for (int s = step1; s < step2; ++s) {
		for (int v = v_cnt - 1; v >= 0; --v) {
			if (!pause[s][v]) {
				if (vm_min[v] > note[s][v]) vm_min[v] = note[s][v];
				if (vm_max[v] < note[s][v]) vm_max[v] = note[s][v];
			}
			ly_flags += nlink[s][v].size();
			if (lining[s][v]) {
				//CString est;
				//est.Format("Detected lining at step %d voice %d", s, v);
				//WriteLog(1, est);
				ly_has_lining = 1;
			}
		}
	}
}

void CF1Ly::GetLyVcnt(int step1, int step2, vector<int> &vm_max) {
	ly_vm_cnt = 0;
	for (int v = v_cnt - 1; v >= 0; --v) {
		if (vm_max[v]) ++ly_vm_cnt;
	}
}

void CF1Ly::SendLyViz(ofstream &fs, int pos, CString &ev, int le, int i, int v, int phase) {
	int shape, sev;
	if (!lyi.size()) return;
	for (int task = ssFinish; task >= ssStart; --task) {
		for (auto it : shsc[phase][task]) {
			shape = it.first;
			if (task == ssFinish) {
				if (!lyi[ly_s2].shf[shape]) continue;
				sev = lyi[ly_s2 + lyi[ly_s2].shsl[shape]].shse[shape];
			}
			if (task == ssStart) {
				if (!lyi[ly_s2].shs[shape]) continue;
				sev = lyi[ly_s2].shse[shape];
			}
			CString script = it.second;
			CString text2;
			if (lyi[ly_s2].sht[shape].IsEmpty()) {
				text2 = "#f\n ";
			}
			else {
				text2 = "\\markup{ \\raise #0.6 \\teeny \"" + lyi[ly_s2].sht[shape] + "\" }\n ";
			}
			script.Replace("$n$", "\n");
			script.Replace("$COLOR$", GetLyColor(sev));
			script.Replace("$TEXT$", lyi[ly_s2].sht[shape]);
			script.Replace("$TEXT2$", text2);
			fs << script << "\n";
		}
	}
}

// Send note or pause
void CF1Ly::SendLyEvent(ofstream &fs, int pos, CString ev, int le, int i, int v) {
	// Length array
	vector<int> la;
	SplitLyNote(pos, le, la);
	SplitLyNoteMeasure(pos, le, la);
	for (int lc = 0; lc < la.size(); ++lc) {
		ly_s = i;
		ly_s2 = ly_s - ly_step1;
		SaveLyComments(i, v, pos);
		SendLyViz(fs, pos, ev, le, i, v, 1);
		if (show_lining && ly_has_lining && ev != "r") {
			if (la[lc] == 8) {
				if (lining[i][v] == HatchStyleNarrowHorizontal) fs << " \\speakOff \\override NoteHead.style = #'xcircle ";
				else if (lining[i][v] == HatchStyleLargeConfetti) fs << " \\speakOff \\override NoteHead.style = #'petrucci ";
				else fs << " \\speakOff \\revert NoteHead.style ";
			}
			else {
				if (lining[i][v] == HatchStyleNarrowHorizontal) fs << " \\revert NoteHead.style \\speakOn ";
				else if (lining[i][v] == HatchStyleLargeConfetti) fs << " \\speakOff \\override NoteHead.style = #'petrucci ";
				else fs << " \\speakOff \\revert NoteHead.style ";
			}
			if (lining[i][v] == HatchStyleLightUpwardDiagonal) {
				fs << " \\circle ";
			}
		}
		fs << ev + GetLyLen(la[lc]);
		if (lc < la.size() - 1 && ev != "r") fs << "~";
		fs << "\n";
		if (midifile_export_marks && !mark[i][v].IsEmpty()) {
			CString st = mark[i][v];
			st.Replace("\n", "");
			if (st == "PD" || st == "CA" || st == "DN") {
				if (ly_msh) {
					if (GetGreen(mark_color[i][v]) == GetRed(mark_color[i][v])) {
						fs << " \\staccato ";
					}
					else {
						fs << "  \\staccatissimo ";
					}
				}
			}
		}
		if (i > -1) {
			i += la[lc] / midifile_out_mul[i];
			pos += la[lc];
		}
		SendLyViz(fs, pos, ev, le, i, v, 9);
		SendLyViz(fs, pos, ev, le, i, v, 10);
		SendLyViz(fs, pos, ev, le, i, v, 11);
		SendLyViz(fs, pos, ev, le, i, v, 12);
	}
}

void CF1Ly::AddNLink(int i, int i2, int v, int fl, int ln, int foreign) {
	lyi[i2 - ly_step1].nflags.push_back(fl / 10);
	lyi[i2 - ly_step1].fsev.push_back(fsev[i][v][fl]);
	cspecies = fl % 10;
	if (foreign) {
		lyi[i2 - ly_step1].nfl.push_back(i + ln - coff[i + ln][ly_v] - i2);
	}
	else {
		lyi[i2 - ly_step1].nfl.push_back(ln);
	}
	lyi[i2 - ly_step1].nfn.push_back(ly_flags + 1);
	lyi[i2 - ly_step1].nff.push_back(foreign);
	lyi[i2 - ly_step1].nfs.push_back(0);
	//lyi[i2 - ly_step1].nfc.push_back(st);
	lyi[i2 - ly_step1].nfc.push_back("");
	if (!foreign) ++ly_flags;
}

void CF1Ly::ParseNLinks(int i, int i2, int v, int foreign) {
	CString com;
	int x = 0;
	for (auto const& it : nlink[i][v]) {
		if (foreign && !rule_viz_v2[it.first / 10]) continue;
		AddNLink(i, i2, v, it.first, it.second, foreign);
		++x;
	}
}

void CF1Ly::SaveLyComments(int i, int v, int pos) {
	CString st, com, note_st;
	int pos1, pos2, found;
	if (!lyi.size()) return;
	if (lyi[ly_s2].nflags.size()) {
		note_st = "\\markup \\wordwrap \\bold {\n  ";
		// Show voice number if more than 1 voice
		if (ly_vm_cnt > 1) {
			st.Format("PART %d ", ly_vcnt - v);
			note_st += st;
		}
		st.Format("[bar %d, beat %d] note %s", // ly_nnum
			pos / 8 + 1, (pos % 8) / 2 + 1,
			GetLyNoteVisual(i, v, "\\raise #0.3 \\magnify #0.7 "));
		if (coff[i][v])
			st += " (slur)";
		note_st += st + "\n}\n";
		found = 0;
		for (int c = 0; c < lyi[ly_s2].nflags.size(); ++c) {
			// Do not process foreign flags
			if (lyi[ly_s2].nff[c]) break;
			int fl = lyi[ly_s2].nflags[c];
			int sev = lyi[ly_s2].fsev[c];
			if (!accept[fl]) st = "- ";
			else if (accept[fl] == -1) st = "$ ";
			else st = "+ ";
			CString rule_name = RuleName[cspecies][fl];
			rule_name.SetAt(0, rule_name.Left(1).MakeLower().GetAt(0));
			if (ly_debugexpect) {
				CString st2;
				st2.Format("[%d/%d] ", fl, sstep[ly_s] + 1);
				rule_name = st2 + rule_name;
			}
			else {
				if (!ly_rule_verbose) {
					if (rule_name.Find(":") != -1) {
						rule_name = rule_name.Left(rule_name.Find(":"));
					}
				}
			}
			com = st + RuleClass[fl] + ": " + rule_name;
			CString subrule_name = SubRuleName[cspecies][fl];
			if (!ly_rule_verbose) {
				if (subrule_name.Left(1) != ":") subrule_name.Empty();
			}
			if (!subrule_name.IsEmpty()) {
				if (subrule_name.Left(1) == ":") {
					subrule_name = subrule_name.Mid(1);
				}
				com += " (" + subrule_name + ")";
			}
			if (ly_rule_verbose > 1 && !RuleComment[cspecies][fl].IsEmpty()) com += ". " + RuleComment[cspecies][fl];
			if (ly_rule_verbose > 1 && !SubRuleComment[cspecies][fl].IsEmpty()) com += " (" + SubRuleComment[cspecies][fl] + ")";
			com += " " + lyi[ly_s2].nfc[c];
			// Send note number with first comment
			if (!found) {
				found = 1;
				ly_com_st += note_st;
			}
			ly_com_st += "\\markup \\wordwrap \\with-color #(rgb-color " +
				GetLyColor(sev) + ") {\n  ";
			com.Replace("\"", "\\\"");
			com.Replace(" ", "\" \"");
			st.Format("\\teeny \\raise #0.2 \\circle %d \"", lyi[ly_s2].nfn[c]);
			ly_com_st += st;
			ly_com_st += com + "\"\n";
			ly_com_st += "\n}\n";
		}
	}
}

void CF1Ly::SetLyShape(int s1, int s2, int f, int fl, int sev, int vtype) {
	if (lyi[s1].shse[vtype] <= sev) {
		// Start
		lyi[s1].shs[vtype] = 1;
		// Finish
		lyi[s2].shf[vtype] = 1;
		// Link to start
		lyi[s2].shsl[vtype] = s1 - s2;
		lyi[s1].shse[vtype] = sev;
		if (vtype == vInterval || vtype == vNoteName || vtype == vHarm) {
			if (lyi[s2].shse[vtype] <= sev) {
				lyi[s2].shse[vtype] = sev;
			}
			if (vtype == vNoteName) ++ly_notenames;
		}
		lyi[s1].sht[vtype] = rule_viz_t[fl];
		// Save flag shape (step depends if link is forward or backward)
		lyi[ly_s2].nfs[f] = vtype;
		lyi[s1].shflag[vtype] = f;
		lyi[s1].shfp[vtype] = ly_s2;
	}
}

void CF1Ly::ClearLyShape(int s1, int s2, int vtype) {
	lyi[s1].shs[vtype] = 0;
	lyi[s2].shf[vtype] = 0;
	// Calculate maximum severity
	lyi[s1].shse[vtype] = -1;
	// Remove link
	lyi[lyi[s1].shfp[vtype]].nfs[lyi[s1].shflag[vtype]] = 0;
	lyi[s1].shflag[vtype] = -1;
	lyi[s1].shfp[vtype] = -1;
}

void CF1Ly::ExportLyI() {
	ofstream fs;
	fs.open(as_dir + "\\lyi-" + as_fname + ".csv", ios_base::app);
	fs << "Step[" << ly_mel << "];";
	for (int x = 0; x < MAX_VIZ; ++x) {
		fs << "shs[" << x << "];";
		fs << "shf[" << x << "];";
		fs << "shsl[" << x << "];";
		fs << "shse[" << x << "];";
		fs << "sht[" << x << "];";
	}
	fs << "\n";
	for (ly_s = ly_step1; ly_s < ly_step2; ++ly_s) {
		ly_s2 = ly_s - ly_step1;
		fs << ly_s2 << ";";
		for (int x = 0; x < MAX_VIZ; ++x) {
			fs << lyi[ly_s2].shs[x] << ";";
			fs << lyi[ly_s2].shf[x] << ";";
			fs << lyi[ly_s2].shsl[x] << ";";
			fs << lyi[ly_s2].shse[x] << ";";
			fs << lyi[ly_s2].sht[x] << ";";
		}
		fs << "\n";
	}
	fs << "\n";
	fs.close();
}

void CF1Ly::AddLyITest(int step1, int step2, int fl, int shape) {
	ly_s2 = step1;
	lyi[step1].nflags.push_back(fl);
	lyi[step1].nfl.push_back(step2 - step1);
	lyi[step1].nfn.push_back(ly_flags + 1);
	lyi[step1].nff.push_back(0);
	lyi[step1].nfs.push_back(0);
	lyi[step1].nfc.push_back("");
	lyi[step1].nfc[lyi[step1].nfc.size() - 1].Format("Type %d", shape);
	SetLyShape(step1, step2, lyi[step1].nfs.size() - 1, fl, randbw(0, 100), shape);
	++ly_flags;
}

void CF1Ly::InitLyITest() {
	if (m_config != "test-ly-overlap") return;
	if (ly_v != ly_vhigh) return;
	int step0, step1, step2, step3, step4, step5, step6;
	int fl = 31;
	step0 = 4;
	ly_flags = 0;
	for (int shape = 0; shape < MAX_VIZ; ++shape) {
		if (viz_type[shape] == vtPoint) {
			step1 = step0;
			step2 = step0;
			step3 = step0;
			step4 = step0;
		}
		else if (viz_type[shape] == vtVBracket) {
			step1 = step0 - 1;
			step2 = step0;
			step3 = step0 + 2;
			step4 = step0 + 3;
		}
		else if (viz_type[shape] == vtGroup || viz_type[shape] == vtVolta) {
			step1 = step0 - 1;
			step2 = step0;
			step3 = step0 + 1;
			step4 = step0 + 2;
		}
		else {
			step1 = step0 - 1;
			step2 = step0;
			step3 = step0;
			step4 = step0 + 1;
		}
		step5 = lyi.size() - 2;
		step6 = lyi.size() - 1;
		AddLyITest(step1, step2, fl, shape);
		AddLyITest(step3, step4, fl, shape);
		AddLyITest(step5, step6, fl, shape);
	}
}

void CF1Ly::InitLyI() {
	if (ly_mel == -1) return;
	ly_v2 = ly_v;
	if (ly_vm_cnt > 1) ly_v2 = (ly_v / 2) * 2 + !(ly_v % 2);
	ly_flags = 0;
	ly_notenames = 0;
	if (m_algo_id == 111) {
		ly_vhigh = ly_v;
		ly_vlow = ly_v;
	}
	else {
		ly_vhigh = max(ly_v, ly_v2);
		ly_vlow = min(ly_v, ly_v2);
	}
	lyi.clear();
	lyi.resize(ly_step2 - ly_step1 + 1);
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
	if (m_config == "test-ly-overlap") return;
	for (ly_s = ly_step1; ly_s < ly_step2; ++ly_s) {
		ly_s2 = ly_s - ly_step1;
		// Find current note position
		int cur_note_step = ly_s;
		if (ly_s2 > 0) {
			for (int x = ly_s; x >= ly_step1; --x) {
				if (note[x][ly_v] != note[ly_s][ly_v]) {
					cur_note_step = x + 1;
					break;
				}
				if (x == ly_step1) cur_note_step = x;
			}
		}
		// Parse flags
		ParseNLinks(ly_s, ly_s, ly_v, 0);
		if (!lyi[ly_s2].nflags.size() && v_cnt > 1) {
			if (ly_v2 < v_cnt) {
				ParseNLinks(ly_s, cur_note_step, ly_v2, 1);
			}
		}
		else {
			// Detect flags that are not at note start and not at cantus note start
			for (int f = 0; f < lyi[ly_s2].nflags.size(); ++f) {
				int link = lyi[ly_s2].nfl[f];
				if ((!coff[ly_s][ly_v] || !coff[ly_s][ly_v2]) &&
					(!coff[ly_s + link][ly_v] || !coff[ly_s + link][ly_v2])) continue;
				int fl = lyi[ly_s2].nflags[f];
				CString est;
				est.Format("Detected flag at hidden position %d/%d: [%d] %s %s (%s)",
					ly_s, ly_s + link, fl, accept[fl] ? "+" : "-",
					RuleName[cspecies][fl], SubRuleName[cspecies][fl]);
				WriteLog(5, est);
			}
		}
		SelectSpeciesRules();
	}
	for (ly_s = ly_step1; ly_s < ly_step2; ++ly_s) {
		ly_s2 = ly_s - ly_step1;
		// Find next note position
		int next_note_step = ly_s + noff[ly_s][ly_v];
		// Find previous note position
		int prev_note_step = max(ly_step1, ly_s - poff[ly_s][ly_v]);
		// Parse flags
		for (int f = 0; f < lyi[ly_s2].nflags.size(); ++f) {
			int fl = lyi[ly_s2].nflags[f];
			int link = lyi[ly_s2].nfl[f];
			int vtype = rule_viz[fl];
			int sev = lyi[ly_s2].fsev[f];
			int skip_shape = 0;
			// Previous note before link
			int prev_link_note = max(ly_step1, ly_s + link - poff[ly_s + link][ly_v]);
			if (ly_debugexpect && sev == 100) vtype = 0;
			// Find link note position
			int link_note_step = ly_s + link;
			if (ly_s2 > 0) {
				for (int x = ly_s + link; x >= ly_step1; --x) {
					if (note[x][ly_v] != note[ly_s + link][ly_v]) {
						link_note_step = x + 1;
						break;
					}
					if (x == ly_step1) link_note_step = x;
				}
			}
			// Get flag start/stop
			s1 = min(ly_s2, ly_s2 + link);
			s2 = max(ly_s2, ly_s2 + link);
			if (lyi[ly_s2].nff[f]) {
				s1 = min(ly_s2, link_note_step - ly_step1);
				s2 = max(ly_s2, link_note_step - ly_step1);
			}
			// If shape cannot highlight single note, but flag does not contain link, then link to next note
			if (!viz_singlenote[vtype] && s1 == s2) s2 = next_note_step - ly_step1;
			// Set interval
			if (!ly_debugexpect || sev != 100) {
				if (rule_viz_int[fl] == 1) {
					SetLyShape(s1, s2, f, fl, sev, vInterval);
				}
				if (rule_viz_int[fl] == 2) {
					SetLyShape(s1, s1, f, fl, sev, vInterval);
				}
				if (rule_viz_int[fl] == 3) {
					SetLyShape(s2, s2, f, fl, sev, vInterval);
				}
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
					overlap_limit = min(prev_note_step, prev_link_note) - ly_step1 - 1;
				// Check if shape can be blocked
				for (int x = ly_step2 - ly_step1 - 1; x > overlap_limit; --x) {
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
				for (int x = ly_step2 - ly_step1 - 1; x > overlap_limit; --x) {
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

void CF1Ly::SaveLySegment(ofstream &fs, int mel, int stp1, int stp2) {
	vector<CString> sv;
	CString clef, key, key_visual;
	int pos, pos2, le, le2, pause_accum, pause_pos, pause_i;
	ly_com_st.Empty();
	ly_ly_st.Empty();
	// Voice melody min pitch
	vector<int> vm_min;
	// Voice melody max pitch
	vector<int> vm_max;
	// Calculate stats
	ly_step1 = stp1;
	ly_step2 = stp2;
	GetLyRange(stp1, stp2, vm_min, vm_max);
	GetLyVcnt(stp1, stp2, vm_max);
	// When debugging expected confirmations, do not show segments without flags
	if (ly_debugexpect && !ly_flags) return;
	ly_mul = midifile_out_mul[stp1];
	//if (ly_vm_cnt == 1 && (m_algo_id == 121 || m_algo_id == 112)) mul = 8;
	// Key
	if (minor[stp1][0]) {
		key = LyMinorKey[tonic[stp1][0]];
	}
	else {
		key = LyMajorKey[tonic[stp1][0]];
	}
	key_visual = key[0];
	key_visual.MakeUpper();
	if (key[1] == 'f') key_visual += "\\flat ";
	if (key[1] == 's') key_visual = "\"" + key_visual + "#\"";
	// First info
	CString st, st3;
	if (mel == -1) st = "Whole piece";
	else {
		st = mel_info[mel];
		st3 = mel_info3[mel];
	}
	st.Replace("\n", ", ");
	st.Replace("#", "\"#\"");
	st.Replace("\\", "/");
	st.Replace("=>", " \\char ##x27F9 ");
	st.Replace("->", " \\char ##x27F6 ");
	fs << "\\markup \\wordwrap \\bold {\n  ";
	fs << "    \\vspace #3\n";
	fs << st << ", Key: " << key_visual << (minor[stp1][0] ? " minor" : " major") << "\n}\n";
	// Save notes
	fs << "<<\n";
	ly_vcnt = 0;
	for (int v = v_cnt - 1; v >= 0; --v) {
		// Do not show voice if no notes inside
		if (!vm_max[v]) continue;
		if (!ly_vcnt) ly_vcnt = v + 1;
		ly_v = v;
		InitLyI();
		InitLyITest();
		// Select best clef
		clef = DetectLyClef(vm_min[v], vm_max[v]);
		st.Format("\\new Staff = \"staff%d\" {\n", ly_v);
		fs << st;
		st.Format("  \\set Staff.instrumentName = \"Part %d\"\n", ly_vcnt - v);
		fs << st;
		fs << "  \\clef \"" << clef << "\" \\key " << key;
		fs << " \\" << (minor[stp1][0] ? "minor" : "major") << "\n";
		read_file_sv("configs\\ly\\staff.ly", sv);
		write_file_sv(fs, sv);
		fs << "  { ";
		ly_nnum = 0;
		pause_accum = 0;
		pause_pos = -1;
		for (int i = stp1; i < stp2; i++) {
			pos = ly_mul * (i - stp1);
			le = ly_mul * len[i][v];
			if (pause[i][v]) {
				pause_accum += le;
				if (pause_pos == -1) {
					pause_pos = pos;
					pause_i = i;
				}
			}
			else {
				++ly_nnum;
				SendLyEvent(fs, pos, GetLyNote(i, v), le, i, v);
			}
			if (pause_accum && (i == stp2 - 1 || !pause[i + 1][v])) {
				SendLyEvent(fs, pause_pos, "r", pause_accum, pause_i, v);
				pause_accum = 0;
				pause_pos = -1;
			}
			if (noff[i][v] == 0) break;
			i += noff[i][v] - 1;
		}
		// Finish with pause
		if ((pos + le) % 8) {
			SendLyEvent(fs, pos, "r", 8 - (pos + le) % 8, 0, 0);
		}
		fs << "\n  }\n";
		fs << "}\n";
		SendLyMistakes();
		SendLyNoteNames();
		SendLyHarm();
		SendLyIntervals();
	}
	fs << ly_ly_st;
	fs << ">>\n";
	if (st3 != "") fs << "\\markup { " << st3 << " }\n";
	fs << ly_com_st;
	if (ly_com_st == "") {
		if (!ly_debugexpect) {
			fs << "\\markup \\bold \\with-color #(rgb-color 0 0.8 0) { \\char ##x2705 Excellent }\n ";
		}
	}
	if (ly_pagebreak) fs << "\\pageBreak\n";
	// Second info
	//st2.Replace("\n", "\n}\n\\markup \\wordwrap \\italic {\n  ");
	//st2.Replace("#", "\"#\"");
	//fs << "\\markup \\wordwrap \\italic {\n  \\vspace #2\n  " << st2 << "\n}\n";
}

CString CF1Ly::SendLySkips(int count) {
	CString lst;
	for (int x = 0; x < count; ++x) {
		lst += " \\skip 8 ";
	}
	return lst;
}

void CF1Ly::SendLyMistakes() {
	CString st;
	if (!ly_flags) return;
	st.Format("  \\new Lyrics \\with { alignAboveContext = \"staff%d\" } {\n", ly_vhigh);
	ly_ly_st += st;
	ly_ly_st += "    \\lyricmode {\n";
	ly_ly_st += "      \\override StanzaNumber.font-size = #-2\n";
	ly_ly_st += "      \\set stanza = #\" Flags:\"\n";
	for (ly_s = ly_step1; ly_s < ly_step2; ++ly_s) {
		ly_s2 = ly_s - ly_step1;
		if (!lyi[ly_s2].nflags.size()) {
			ly_ly_st += SendLySkips(ly_mul);
			continue;
		}
		ly_ly_st += "      \\markup{ \\teeny \\override #`(direction . ,UP) { \\dir-column {\n";
		int max_mist = lyi[ly_s2].nflags.size() - 1;
		// Do not show too many mistakes
		if (max_mist > 5) {
			max_mist = 4;
			ly_ly_st += "...\n";
		}
		for (int f = max_mist; f >= 0; --f) {
			int fl = lyi[ly_s2].nflags[f];
			int sev = lyi[ly_s2].fsev[f];
			st.Format("        \\with-color #(rgb-color " +
				GetLyColor(sev) + ") %s \\circle %d\n",
				lyi[ly_s2].nfs[f] ? "\\underline" : "", lyi[ly_s2].nfn[f]);
			// \override #'(offset . 5) \override #'(thickness . 2) 
			ly_ly_st += st;
		}
		ly_ly_st += "      } } }8\n";
		ly_ly_st += SendLySkips(ly_mul - 1);
	}
	ly_ly_st += "    }\n";
	ly_ly_st += "  }\n";
}

void CF1Ly::SendLyHarm() {
	CString st, lst;
	int hcount = 0;
	//if (!ly_flags) return;
	st.Format("  \\new Lyrics \\with { alignBelowContext = \"staff%d\" } {\n", ly_vlow);
	lst += st;
	lst += "    \\lyricmode {\n";
	lst += "      \\override StanzaNumber.font-size = #-2\n";
	lst += "      \\set stanza = #\" Harmony:\"\n";
	lst += "      \\override InstrumentName #'X-offset = #1\n";
	lst += "      \\override InstrumentName #'font-series = #'bold\n";
	lst += "      \\override InstrumentName.font-size = #-2\n";
	lst += "      \\set shortVocalName = \"H:\"\n";
	for (ly_s = ly_step1; ly_s < ly_step2; ++ly_s) {
		ly_s2 = ly_s - ly_step1;
		CString st = mark[ly_s][ly_v2];
		st.Replace("\n", "");
		if (!st.IsEmpty() && st != "PD" && st != "CA" && st != "DN") {
			++hcount;
			lst += "  \\markup{ ";
			lst += "  \\teeny \n";
			if (lyi[ly_s2].shs[vHarm] || lyi[ly_s2].shf[vHarm]) {
				lst += "  \\on-color #(rgb-color " + GetLyMarkColor(lyi[ly_s2].shse[vHarm]) + ") ";
			}
			lst += "  \\pad-markup #0.4 \n";
			int found = 0;
			// Replace dominant symbol
			st.Replace("#", " \"#\" ");
			st.Replace("b", " \\raise #0.3 \\magnify #0.5 \\flat ");
			if (st.Right(1) == "6") {
				st.Replace("6", " \\raise #0.7 6");
				lst += "\\concat { " + st + " } ";
			}
			else if (st.Right(3) == "6/4") {
				lst += "  \\concat { \n";
				lst += "    \\general-align #Y #0.5 \"" + st.Left(st.GetLength() - 3) + "\"\n";
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
			found = 1;
			lst += SendLySkips(ly_mul - 1);
		}
		else {
			lst += SendLySkips(ly_mul);
		}
	}
	lst += "    }\n";
	lst += "  }\n";
	if (hcount) ly_ly_st += lst;
}

void CF1Ly::SendLyIntervals() {
	CString st;
	if (!ly_flags) return;
	if (ly_vlow == ly_vhigh) return;
	st.Format("  \\new Lyrics \\with { alignBelowContext = \"staff%d\" } {\n", ly_vlow);
	ly_ly_st += st;
	ly_ly_st += "    \\lyricmode {\n";
	ly_ly_st += "      \\override StanzaNumber.font-size = #-2\n";
	ly_ly_st += "      \\set stanza = #\" Interval:\"\n";
	ly_ly_st += "      \\override InstrumentName #'X-offset = #1\n";
	ly_ly_st += "      \\override InstrumentName #'font-series = #'bold\n";
	ly_ly_st += "      \\override InstrumentName.font-size = #-2\n";
	ly_ly_st += "      \\set shortVocalName = \"I:\"\n";
	for (ly_s = ly_step1; ly_s < ly_step2; ++ly_s) {
		ly_s2 = ly_s - ly_step1;
		if (!lyi[ly_s2].shs[vInterval] && !lyi[ly_s2].shf[vInterval]) {
			ly_ly_st += SendLySkips(ly_mul);
			continue;
		}
		//int in = note[ly_s][ly_vhigh] - note[ly_s][ly_vlow];
		//int in2 = in % 12;
		//in = in ? (in2 ? in2 : 12) : 0;
		//CString st = GetIntName(in);
		CString st = GetRealIntName(ly_s, ly_vhigh, ly_vlow);
		ly_ly_st += "\\markup{ ";
		ly_ly_st += "\\teeny ";
		if (lyi[ly_s2].shse[vInterval] > -1) {
			ly_ly_st += " \\on-color #(rgb-color " + GetLyMarkColor(lyi[ly_s2].shse[vInterval]) + ") ";
		}
		ly_ly_st += " \\pad-markup #0.4 \\concat { " + st + " ";
		ly_ly_st += "} }\n";
		ly_ly_st += SendLySkips(ly_mul - 1);
	}
	ly_ly_st += "    }\n";
	ly_ly_st += "  }\n";
}

void CF1Ly::SendLyNoteNames() {
	CString st;
	if (!ly_notenames) return;
	st.Format("  \\new Lyrics \\with { alignBelowContext = \"staff%d\" } {\n", ly_v);
	ly_ly_st += st;
	ly_ly_st += "    \\lyricmode {\n";
	ly_ly_st += "      \\override StanzaNumber.font-size = #-2\n";
	ly_ly_st += "      \\set stanza = #\" Note:\"\n";
	ly_ly_st += "      \\override InstrumentName #'X-offset = #1\n";
	ly_ly_st += "      \\override InstrumentName #'font-series = #'bold\n";
	ly_ly_st += "      \\override InstrumentName.font-size = #-2\n";
	ly_ly_st += "      \\set shortVocalName = \"N:\"\n";
	for (ly_s = ly_step1; ly_s < ly_step2; ++ly_s) {
		ly_s2 = ly_s - ly_step1;
		if (!lyi[ly_s2].shs[vNoteName] && !lyi[ly_s2].shf[vNoteName]) {
			ly_ly_st += SendLySkips(ly_mul);
			continue;
		}
		CString st = GetLyNoteVisual(ly_s, ly_v, "\\raise #0.3 \\magnify #0.5 ");
		ly_ly_st += "\\markup{ ";
		ly_ly_st += "\\teeny ";
		if (lyi[ly_s2].shse[vNoteName] > -1) {
			ly_ly_st += " \\on-color #(rgb-color " + GetLyMarkColor(lyi[ly_s2].shse[vNoteName]) + ") ";
		}
		ly_ly_st += " \\pad-markup #0.4 \\concat { " + st + " } ";
		ly_ly_st += "}\n";
		ly_ly_st += SendLySkips(ly_mul - 1);
	}
	ly_ly_st += "    }\n";
	ly_ly_st += "  }\n";
}

void CF1Ly::SaveLy(CString dir, CString fname) {
	if (emulate_sas) return;
	LoadLyShapes("configs\\ly\\shapes.csv");
	vector<CString> sv;
	CString title;
	// Remove server config prefix
	CString my_config;
	my_config = m_config;
	if (my_config.Left(3) == "sv_") {
		my_config = my_config.Mid(3);
	}
	DeleteFile(dir + "\\lyi-" + fname + ".csv");
	title = m_algo_name + ": " + my_config + " (" +
		CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M") + ")";
	ly_fs.open(dir + "\\" + fname + ".ly");
	read_file_sv("configs\\ly\\header.ly", sv);
	for (int i = 0; i < sv.size(); ++i) {
		sv[i].Replace("$SUBTITLE$", title);
		if (ly_debugexpect) {
			sv[i].Replace("$TITLE$", "Debug expected confirmations");
		}
		else sv[i].Replace("$TITLE$", "");
		sv[i].Replace("$DEDICATION$", "");
		ly_fs << sv[i] << "\n";
	}

	if (m_config == "test-ly-viz") {
		for (int i = 0; i <= 100; ++i) {
			CString est;
			est.Format("\\markup \\with-color #(rgb-color " + GetLyColor(i) + ") { Test color for severity %d }\n",
				i);
			ly_fs << est;
			i += 9;
		}
		for (int i = 0; i <= 100; ++i) {
			CString est;
			est.Format("\\markup \\on-color #(rgb-color " + GetLyMarkColor(i) + ") { Test color for severity %d }\n",
				i);
			ly_fs << est;
			i += 9;
		}
	}
	if (!mel_info.size()) {
		CString st;
		st = "Whole piece";
		ly_mel = -1;
		SaveLySegment(ly_fs, -1, 0, t_generated);
	}
	else {
		int first_step = 0;
		int last_step = 0;
		int found, s;
		for (int m = 0; m < mel_info.size(); ++m) {
			found = 0;
			for (s = last_step; s < t_generated; ++s) {
				if (!found && mel_id[s][0] == m && mel_id[s][0] > -1) {
					first_step = s;
					last_step = s;
					found = 1;
				}
				if (found && mel_id[s][0] != m) {
					last_step = s;
					break;
				}
			}
			if (s >= t_generated - 1 && mel_id[t_generated - 1][0] > -1 &&
				found && first_step == last_step)	last_step = t_generated - 1;
			ly_mel = m;
			if (found) SaveLySegment(ly_fs, m, first_step, last_step);
			//if (m < mel_info.size() - 1) fs << "\\pageBreak\n";
		}
	}
	ly_fs << "\\header {tagline = \"This file was created by MGen ";
	ly_fs << APP_VERSION << " at " << CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S") << "\"}\n";
	read_file_sv("configs\\ly\\footer.ly", sv);
	write_file_sv(ly_fs, sv);
	ly_fs.close();
	ly_saved = 1;
}

