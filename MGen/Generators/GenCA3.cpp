// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "GenCA3.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

CGenCA3::CGenCA3() {
}

CGenCA3::~CGenCA3() {
}

void CGenCA3::InitAnalysis() {
	cchn2.resize(2);
	cchn2[0].resize(12);
	cchn2[1].resize(12);
	cpos.resize(2);
	cpos[0].resize(7);
	cpos[1].resize(7);
	chn.resize(7);
	cchn.resize(12);
	cchnv.resize(2);
	cchnv[0].resize(12);
	cchnv[1].resize(12);
	shp.resize(16);
	nstat.resize(MAX_NOTE);
	nstat2.resize(MAX_NOTE);
	nstat3.resize(MAX_NOTE);
	minl.resize(av_cnt);
	maxl.resize(av_cnt);
	nmin.resize(av_cnt);
	nmax.resize(av_cnt);
	nmind.resize(av_cnt);
	nmaxd.resize(av_cnt);
	fli_size.resize(av_cnt);
	fli.resize(av_cnt);
	fli2.resize(av_cnt);
	bli.resize(av_cnt);
	nlen.resize(av_cnt);
	c.resize(av_cnt);
	cc.resize(av_cnt);
	pc.resize(av_cnt);
	pcc.resize(av_cnt);
	retr.resize(av_cnt);
	llen.resize(av_cnt);
	rlen.resize(av_cnt);
	vid.resize(av_cnt);
	lclimax.resize(av_cnt);
	lclimax2.resize(av_cnt);
	leap.resize(av_cnt);
	smooth.resize(av_cnt);
	tweight.resize(av_cnt);
	g_leaps.resize(av_cnt);
	fin.resize(av_cnt);
	fil.resize(av_cnt);
	g_leaped.resize(av_cnt);
	slur.resize(av_cnt);
	flag.resize(av_cnt);
	fsl.resize(av_cnt);
	fvl.resize(av_cnt);
	sus.resize(av_cnt);
	ssus.resize(av_cnt);
	isus.resize(av_cnt);
	msh.resize(av_cnt);
	mshb.resize(av_cnt);
	nih.resize(av_cnt);
	nihb.resize(av_cnt);
	pat.resize(av_cnt);
	dtp.resize(av_cnt);
	dtp_s.resize(av_cnt);
	pat_state.resize(av_cnt);
	beat.resize(av_cnt);
	rh_id.resize(av_cnt);
	rh_pid.resize(av_cnt);
	ha64.resize(c_len);
	hbcc.resize(c_len);
	hbc.resize(c_len);
	bhli.resize(c_len);
	vca.resize(c_len);
	hva.resize(c_len);
	lva.resize(c_len);
	macc.resize(c_len);
	macc2.resize(c_len);
	decc.resize(c_len);
	decc2.resize(c_len);
	maw.resize(c_len);
	for (v = 0; v < av_cnt; ++v) {
		fli[v].resize(c_len);
		fli2[v].resize(c_len);
		bli[v].resize(c_len);
		cc[v].resize(c_len);
		c[v].resize(c_len);
		pc[v].resize(c_len);
		pcc[v].resize(c_len);
		retr[v].resize(c_len);
		llen[v].resize(c_len);
		rlen[v].resize(c_len);
		lclimax[v].resize(c_len);
		lclimax2[v].resize(c_len);
		beat[v].resize(c_len);
		leap[v].resize(c_len);
		smooth[v].resize(c_len);
		tweight[v].resize(c_len);
		g_leaps[v].resize(c_len);
		g_leaped[v].resize(c_len);
		slur[v].resize(c_len);
		flag[v].resize(c_len);
		fsl[v].resize(c_len);
		fvl[v].resize(c_len);
		sus[v].resize(c_len);
		ssus[v].resize(c_len);
		isus[v].resize(c_len);
		msh[v].resize(c_len);
		mshb[v].resize(c_len);
		nih[v].resize(c_len);
		nihb[v].resize(c_len);
		pat[v].resize(c_len);
		dtp[v].resize(c_len);
		dtp_s[v].resize(c_len);
		pat_state[v].resize(c_len);
	}
}

// Init vectors needed for analysis
void CGenCA3::GetAnalysisVectors() {
	bn = 0;
	mode = 0;
	mminor = 0;
	CreateLinks();
}

void CGenCA3::LoadConfigLine(CString* sN, CString* sV, int idata, float fdata) {
	LoadVar(sN, sV, "musicxml_file", &musicxml_file);

	CGenCP2::LoadConfigLine(sN, sV, idata, fdata);
}

int CGenCA3::XML_to_CP() {
	vector<int> im; // Intermediate measures vector
	vector<vector<CString>> it; // Intermediate text vector
	vector<vector<CString>> ilyr; // Intermediate lyrics vector
	vector<int> ifi; // Intermediate fifths vector
	vector<int> ibl; // Intermediate barlines vector
	vector<int> ibt; // Intermediate beat type
	vector<vector<int>> ial; // Intermediate alterations vector

	av_cnt = xfi.voice.size();
	ial.resize(av_cnt);
	InitAnalysis();
	cp_tempo = 0;
	vname.resize(av_cnt);
	it.resize(av_cnt);
	ilyr.resize(av_cnt);
	for (vi = 0; vi < xfi.voice.size(); ++vi) {
		v = av_cnt - vi - 1;
		// Position in resulting vector
		int pos = 0;
		// Position of measure inside xml file
		float xpos = 0;
		// Position of note inside xml measure
		float xpos2 = 0;
		vname[v] = xfi.voice[vi].name;
		for (int m = 1; m < xfi.mea.size(); ++m) {
			xpos2 = 0;
			int posm = pos;
			im.resize(pos + 1);
			ibl.resize(pos + 8.0 * xfi.mea[m].len);
			im[pos] = 1;
			if (xfi.mea[m].barline == "heavy" || 
				xfi.mea[m].barline == "light-light" || 
				xfi.mea[m].barline == "light-heavy" || 
				xfi.mea[m].barline == "heavy-light" || 
				xfi.mea[m].barline == "heavy-heavy") 
				ibl[pos + 8.0 * xfi.mea[m].len - 1] = 1;
			for (int ni = 0; ni < xfi.note[vi][m].size(); ++ni) {
				xpos2 += xfi.note[vi][m][ni].dur * 2.0 / xfi.note[vi][m][ni].dur_div;
				int ln = floor(xpos + xpos2) - pos; 
				// xfi.note[vi][m][ni].dur * 2.0 / xfi.note[vi][m][ni].dur_div;
				//if (pos + ln < floor(xpos + xpos2)) 
				c_len = max(c_len, pos + ln);
				it[v].resize(c_len);
				ilyr[v].resize(c_len);
				ifi.resize(c_len, 100);
				ibt.resize(c_len);
				cc[v].resize(c_len);
				ial[v].resize(c_len);
				retr[v].resize(c_len);
				if (xfi.note[vi][m][ni].tempo && !cp_tempo)
					cp_tempo = xfi.note[vi][m][ni].tempo;
				if (pos && !xfi.note[vi][m][ni].rest && !xfi.note[vi][m][ni].tie_stop &&
					xfi.note[vi][m][ni].pitch == cc[v][pos - 1]) retr[v][pos] = 1;
				// Check lowest note
				if (!xfi.note[vi][m][ni].rest && !xfi.note[vi][m][ni].pitch) {
					CString est;
					est.Format("Detected too low note. This note will be replaced with a rest. Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d, note %d of %d.",
						m, vi, xfi.voice[vi].id, xfi.voice[vi].name, xfi.voice[vi].staff, xfi.voice[vi].v, xfi.voice[vi].chord,
						xfi.mea[m].beats, xfi.mea[m].beat_type, ni + 1, xfi.note[vi][m].size());
					WriteLogLy(5, est, 0);
				}
				for (s = 0; s < ln; ++s) {
					cc[v][pos + s] = xfi.note[vi][m][ni].pitch;
					ial[v][pos + s] = xfi.note[vi][m][ni].alter;
				}
				// Get fifths
				ifi[pos] = xfi.note[vi][m][ni].fifths;
				ibt[pos] = xfi.mea[m].beat_type;
				// Concatenate text
				it[v][pos] = xfi.note[vi][m][ni].words;
				ilyr[v][pos] = xfi.note[vi][m][ni].lyric;
				pos += ln;
			}
			xpos += xfi.mea[m].len * 8.0;
		}
	}
	if (!cp_tempo) cp_tempo = 100;
	im.resize(c_len);
	ep2 = c_len;
	ResizeVectors(t_allocated, av_cnt);
	// Explode music into separate exercises
	// State: 0 - find note, 1 - find pause
	int state = 0;
	s1 = 0;
	cp_id = 0;
	for (s = 0; s < c_len; ++s) {
		int is_pause = 1;
		for (v = 0; v < av_cnt; ++v) {
			if (cc[v][s]) {
				is_pause = 0;
				break;
			}
		}
		// Find note
		if (!state) {
			if (!is_pause) {
				s1 = s;
				state = 1;
			}
		}
		// Find pause
		else {
			if (is_pause || s == c_len - 1 || ibl[s]) {
				if (!is_pause) s2 = s;
				else s2 = s - 1;
				// Move left to measure
				while (!im[s1] && s1 > 0) --s1;
				// Create new cp_id
				cp.resize(cp_id + 1);
				cp_alter.resize(cp_id + 1);
				cp_retr.resize(cp_id + 1);
				cp_vid.resize(cp_id + 1);
				cp_mea.resize(cp_id + 1);
				cp_text.resize(cp_id + 1);
				cp_lyrics.resize(cp_id + 1);
				cp_fi.resize(cp_id + 1, 100);
				cp_btype.resize(cp_id + 1, 100);
				cp_error.resize(cp_id + 1);
				// Fill new cp_id
				cp[cp_id].resize(av_cnt);
				cp_alter[cp_id].resize(av_cnt);
				cp_retr[cp_id].resize(av_cnt);
				cp_vid[cp_id].resize(av_cnt);
				// Set measures
				cp_mea[cp_id].resize(s2 - s1 + 1);
				for (int s3 = s1; s3 <= s2; ++s3) {
					cp_mea[cp_id][s3 - s1] = im[s3];
				}
				// Copy notes
				for (v = 0; v < av_cnt; ++v) {
					cp[cp_id][v].resize(s2 - s1 + 1);
					cp_alter[cp_id][v].resize(s2 - s1 + 1);
					cp_retr[cp_id][v].resize(s2 - s1 + 1);
					cp_vid[cp_id][v] = v;
					for (int s3 = s1; s3 <= s2; ++s3) {
						if (ibt[s3]) cp_btype[cp_id] = ibt[s3];
						if (ifi[s3] != 100) {
							if (cp_fi[cp_id] == 100) cp_fi[cp_id] = ifi[s3];
							else {
								if (cp_fi[cp_id] != ifi[s3] && !cp_error[cp_id]) {
									CString est;
									est.Format("Key changed in the middle of counterpoint %d. Ignoring this counterpoint.",
										cp_id + 1);
									WriteLog(5, est);
									cp_error[cp_id] = 1;
								}
							}
						}
						cp[cp_id][v][s3 - s1] = cc[v][s3];
						cp_alter[cp_id][v][s3 - s1] = ial[v][s3];
						cp_retr[cp_id][v][s3 - s1] = retr[v][s3];
						if (!cp_text[cp_id].IsEmpty() && !it[v][s3].IsEmpty()) cp_text[cp_id] += " ";
						cp_text[cp_id] += it[v][s3];
						if (!cp_lyrics[cp_id].IsEmpty() && !ilyr[v][s3].IsEmpty()) cp_lyrics[cp_id] += " ";
						cp_lyrics[cp_id] += ilyr[v][s3];
					}
				}
				if (cp_fi[cp_id] == 100) cp_fi[cp_id] = 0;
				cp_id++;
				state = 0;
			}
		}
	}
	// Remove empty voices
	for (cp_id = 0; cp_id < cp.size(); ++cp_id) {
		vector<int> empty;
		empty.resize(av_cnt, 1);
		for (v = 0; v < av_cnt; ++v) {
			for (s = 0; s < cp[cp_id][v].size(); ++s) {
				if (cp[cp_id][v][s]) {
					empty[v] = 0;
					break;
				}
			}
		}
		for (v = av_cnt-1; v >= 0; --v) {
			if (empty[v]) {
				verase(cp[cp_id], v);
				verase(cp_alter[cp_id], v);
				verase(cp_retr[cp_id], v);
				verase(cp_vid[cp_id], v);
			}
		}
	}
	// Make unique voice names
	vname2 = vname;
	for (v = av_cnt - 1; v > 0; --v) {
		int dupl = 1;
		for (v2 = v - 1; v2 >= 0; --v2) {
			if (vname2[v2] == vname2[v]) {
				++dupl;
				CString st;
				st.Format("%s (%d)", vname2[v2], dupl);
				vname2[v2] = st;
			}
		}
	}
	return 0;
}

int CGenCA3::CheckXML() {
	for (int m = 1; m < xfi.mea.size(); ++m) {
		for (vi = 0; vi < xfi.voice.size(); ++vi) {
			for (int ni = 0; ni < xfi.note[vi][m].size(); ++ni) {
				float len = xfi.note[vi][m][ni].dur * 0.25 / xfi.note[vi][m][ni].dur_div;
				if (len < 0.125) {
					CString est;
					est.Format("Note length %.3f is shorter than 1/8 (0.125). Currently not supported. Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d, note %d of %d",
						len, m, vi, xfi.voice[vi].id, xfi.voice[vi].name, 
						xfi.voice[vi].staff, xfi.voice[vi].v, xfi.voice[vi].chord,
						xfi.mea[m].beats, xfi.mea[m].beat_type, ni + 1, xfi.note[vi][m].size());
					WriteLog(5, est);
					error = 9;
					return 1;
				}
			}
		}
	}
	return 0;
}

int CGenCA3::GetCP() {
	CString est;
	av_cnt = cp[cp_id].size();
	cc.resize(av_cnt);
	src_alter.resize(av_cnt);
	retr.resize(av_cnt);
	c_len = cp[cp_id][0].size();
	InitAnalysis();
	mli.clear();
	bmli.resize(c_len);
	npm = 0;
	btype = cp_btype[cp_id];
	// Detect mli and npm
	for (int s = 0; s < c_len; ++s) {
		if (cp_mea[cp_id][s]) {
			mli.push_back(s);
			if (mli.size() > 1) {
				if (npm) {
					if (npm != mli.end()[-1] - mli.end()[-2]) {
						est.Format("Measure %zu size is changed from %d to %d inside counterpoint %d",
							mli.size(), npm, mli.end()[-1] - mli.end()[-2], cp_id + 1);
						WriteLogLy(5, est, 0);
						return 1;
					}
				}
				else npm = mli.end()[-1] - mli.end()[-2];
			}
		}
		bmli[s] = mli.size() - 1;
	}

	// If there are no measures, then it is a single measure
	if (!npm) npm = c_len;
	for (v = 0; v < av_cnt; ++v) {
		cc[v].resize(c_len);
		src_alter[v].resize(c_len);
		retr[v].resize(c_len);
		vid[v] = cp_vid[cp_id][v];
		for (int s = 0; s < c_len; ++s) {
			cc[v][s] = cp[cp_id][v][s];
			src_alter[v][s] = cp_alter[cp_id][v][s];
			retr[v][s] = cp_retr[cp_id][v][s];
		}
	}

	// Check if cp starts with pause
	int has_note = 0;
	for (v = 0; v < av_cnt; ++v) {
		if (cc[v][0]) {
			has_note = 1;
			break;
		}
	}
	if (!has_note) {
		est.Format("Counterpoint %d starts not on downbeat. Please check counterpoint.",
			cp_id + 1);
		WriteLogLy(5, est, 0);
		return 1;
	}

	ep2 = c_len;
	return 0;
}

int CGenCA3::GetCPSpecies() {
	CString st, est;
	LoadSpecies(conf_species);
	// Check if species can be loaded from MusicXML
	int found = 0;
	xml_text = cp_text[cp_id];
	xml_lyrics = cp_lyrics[cp_id];
	if (!cp_text[cp_id].IsEmpty()) {
		vector<CString> sa;
		st = cp_text[cp_id];
		st.Replace(",", " ");
		Tokenize(st, sa, " ");
		for (int i = 0; i < sa.size(); ++i) {
			sa[i].Trim();
			if (sa[i].Left(2) == "sp") {
				LoadSpecies(sa[i].Mid(2));
				found = 1;
				break;
			}
		}
	}
	if (!found && !cp_lyrics[cp_id].IsEmpty()) {
		vector<CString> sa;
		st = cp_lyrics[cp_id];
		st.Replace(",", " ");
		Tokenize(st, sa, " ");
		for (int i = 0; i < sa.size(); ++i) {
			sa[i].Trim();
			if (sa[i].Left(2) == "sp") {
				LoadSpecies(sa[i].Mid(2));
				found = 1;
				break;
			}
		}
	}
	if (vsp.size() == 1 && av_cnt > 1) {
		// Find voice with maximum notes or pauses
		int my_sp = vsp[0];
		int best_fli = 0;
		int best_v = 0;
		if (my_sp <= 1) {
			// For species 1 search voice with minimum notes (because counterpoint can contain slurs)
			best_fli = 1000000;
			for (v = 0; v < av_cnt; ++v) {
				if (fli_size[v] < best_fli) {
					best_fli = fli_size[v];
					best_v = v;
				}
			}
		}
		else {
			// For species 2-5 search for voice with maximum notes or pauses
			for (v = 0; v < av_cnt; ++v) {
				if (fli_size[v] > best_fli) {
					best_fli = fli_size[v];
					best_v = v;
				}
			}
		}
		vsp.clear();
		vsp.resize(av_cnt, -1);
		vsp[best_v] = my_sp;
		int lowest_v;
		if (best_v) lowest_v = 0;
		else lowest_v = 1;
		for (v = 0; v < av_cnt; ++v) {
			if (vsp[v] == -1) {
				// Set species 1 for all voices except lowest
				if (v == lowest_v) vsp[v] = 0;
				// Set CF for lowest voice
				else vsp[v] = 1;
			}
		}
	}
	else if (vsp.size() != av_cnt) {
		est.Format("Check species parameter in config or MusicXML file: found %zu voices, but there are %d voices in counterpoint %d. Parameter in MusicXML will have precedence",
			vsp.size(), av_cnt, cp_id + 1);
		WriteLogLy(5, est, 0);
		return 1;
	}
	return 0;
}

void CGenCA3::Generate() {
	CString st;
	if (error) return;
	LoadLyShapes("configs\\ly\\shapes.csv");
	if (HarmName[0].IsEmpty()) {
		WriteLog(5, "Harmonic notation not loaded: please check harm_notation parameter in configuration file");
		error = 11;
		return;
	}
	if (!hsp.size()) {
		WriteLog(5, "Harmonic sequence penalty not loaded: please check hsp_file parameter in configuration file");
		error = 12;
		return;
	}
	if (musicxml_file == "") {
		WriteLog(5, "MusicXML file not specified in configuration file");
		error = 7;
		return;
	}
	xfi.LoadXML(musicxml_file);
	if (!xfi.error.IsEmpty()) {
		WriteLog(5, xfi.error);
		error = 8;
		return;
	}
	xfi.ValidateXML();
	if (!xfi.error.IsEmpty()) {
		WriteLog(5, xfi.error);
		error = 9;
		return;
	}
	if (CheckXML()) return;
	if (XML_to_CP()) return;
	for (cp_id = 0; cp_id < cp.size(); ++cp_id) if (!cp_error[cp_id]) {
		st.Format("Analyzing: %d of %zu", cp_id + 1, cp.size());
		SetStatusText(3, st);
		if (GetCP()) continue;
		int real_len = cc[0].size();
		int full_len = floor((real_len + 1) / 8 + 1) * 8;
		GetAnalysisVectors();
		if (GetCPSpecies()) continue;
		if (AnalyseCP()) continue;
		for (int v = 0; v < v_cnt; ++v) {
			FillPause(step0, full_len, v);
		}
		SendCP();
		SaveLyCP();
		step0 += full_len;
		if (need_exit) break;
	}
	st.Format("Analyzed %d of %d", cp_id, cp.size());
	SetStatusText(3, st);
	//WriteLog(1, "Loaded MusicXML successfully");
}

void CGenCA3::SaveLy(CString dir, CString fname) {
	if (error) return;
	vector<CString> sv;
	CString title;
	// Remove server config prefix
	CString my_config;
	my_config = m_config;
	if (my_config.Left(3) == "sv_") {
		my_config = my_config.Mid(3);
	}
	title = m_algo_name + ": " + my_config + " (" +
		CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M") + ")";
	ly_fs.open(dir + "\\" + fname + ".ly");
	read_file_sv("configs\\ly\\header.ly", sv);
	for (int i = 0; i < sv.size(); ++i) {
		sv[i].Replace("$SUBTITLE$", title);
		sv[i].Replace("$TITLE$", "");
		sv[i].Replace("$DEDICATION$", "");
		ly_fs << sv[i] << "\n";
	}

	for (int cp_id = 0; cp_id < ly_st.size(); ++cp_id) {
		ly_fs << ly_st[cp_id];
	}
	CString ly_ly_st;
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
	ly_fs << ly_ly_st;
	ly_fs << "\\header {tagline = \"This file was created by MGen CA3 ";
	ly_fs << APP_VERSION << " at " << CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S") << "\"}\n";
	read_file_sv("configs\\ly\\footer.ly", sv);
	write_file_sv(ly_fs, sv);
	ly_fs << "\\markup { \\vspace #1.5\n Harmonic notation: " + harm_notation_name[harm_notation] + " }\n";
	ly_fs.close();
	ly_saved = 1;
}

void CGenCA3::GetCPKey() {
	CString est;
	fifths = cp_fi[cp_id];
	// Detect major base note
	maj_bn = (fifths * 7 + 12 * 12) % 12;
	// Temporarily set base note to major base note for last chord detection
	bn = maj_bn;
	mode = 0;
	BuildPitchConvert();
	// Get diatonic pitch class for chord detection
	GetDiatonic(0, c_len);
	GetPitchClass(0, c_len);
	// Get base note as last note in bass
	for (s = c_len - 1; s >= 0; --s) {
		if (cc[0][s]) {
			//bn = cc[0][s] % 12;
			// Collect notes from all voices
			fill(chn.begin(), chn.end(), 0);
			fill(cchn.begin(), cchn.end(), 0);
			for (v = 0; v < av_cnt; ++v) {
				++chn[pc[v][s]];
				++cchn[pcc[v][s]];
			}
			int lchm;
			int lchm_alter;
			int rat;
			GetHarm(0, 0, lchm, lchm_alter, rat);
			bn = c_cc[lchm + 14] % 12;
			break;
		}
	}
	// Set mode
	mode = (bn - maj_bn + 12) % 12;
	BuildPitchConvert();
	// Find used notes
	vector<int> unote;
	unote.resize(12);
	mminor = 0;
	int detected_mminor = 0;
	int wrong_alt = 0;
	for (int v = 0; v < av_cnt; ++v) {
		for (int s = 0; s < c_len; ++s) {
			// Skip pauses
			if (!cc[v][s]) continue;
			// Count note stats
			++unote[cc[v][s] % 12];
			// Check if note is allowed
			int maj_note = (cc[v][s] - bn + 12 + mode) % 12;
			if (maj_note == 8 || maj_note == 6) {
				detected_mminor = 1;
			}
			if (maj_note == 1 || maj_note == 3 || maj_note == 10) {
				wrong_alt = 1;
			}
		}
	}
	// Detect chromatic collisions
	if (wrong_alt) {
		est.Format("Specified key was %s, but detected alterations (major I#, II# or VI#) cannot be used in this key. Please check source file",
			GetPrintKey(bn, mode));
		WriteLogLy(5, est, 1);
	}
	// Detect mminor
	if (detected_mminor) {
		if (mode != 9) {
			est.Format("Detected melodic minor alterations (major IV# or V#), but specified key was %s. This is impossible. Please check source file",
				GetPrintKey(bn, mode));
			WriteLogLy(5, est, 1);
		}
		else {
			mminor = 1;
		}
	}
	// TODO: Select better key if alterations do not fit, while preserving bn
}

int CGenCA3::GetVocalRanges() {
	CString est;
	// Get vocal ranges from part names
	vocra_used.resize(vocra_info.size());
	vocra.clear();
	vocra.resize(av_cnt);
	vocra_detected.clear();
	vocra_detected.resize(av_cnt);
	for (v = 0; v < av_cnt; ++v) {
		vi = vid[v];
		CString st;
		st = vname[vi];
		st.MakeLower();
		if (st == "soprano") {
			vocra[v] = vrSop;
			++vocra_used[vrSop];
			vocra_detected[v] = 1;
		}
		if (st == "alto") {
			vocra[v] = vrAlt;
			++vocra_used[vrAlt];
			vocra_detected[v] = 1;
		}
		if (st == "tenor") {
			vocra[v] = vrTen;
			++vocra_used[vrTen];
			vocra_detected[v] = 1;
		}
		if (st == "bass") {
			vocra[v] = vrBas;
			++vocra_used[vrBas];
			vocra_detected[v] = 1;
		}
	}
	// Get note ranges
	for (v = 0; v < av_cnt; ++v) {
		GetMelodyInterval(0, c_len);
	}
	GetPossibleVocalRanges();
	for (v = 0; v < av_cnt; ++v) if (!vocra_p[v].size()) {
		est.Format("Cannot detect vocal range for counterpoint %d, part %d: %s",
			cp_id + 1, vid[v] + 1, vname[vid[v]]);
		WriteLogLy(5, est, 0);
		return 1;
	}
	ScanVocalRanges();
	for (v = 0; v < av_cnt; ++v) if (!vocra_detected[v]) {
		est.Format("Cannot detect vocal range for counterpoint %d, part %d: %s. Please specify vocal range in instrument name in source file %s",
			cp_id + 1, vid[v] + 1, vname[vid[v]], musicxml_file);
		WriteLogLy(5, est, 0);
	}
	return 0;
}

void CGenCA3::ScanVocalRanges() {
	int sv = 0;
	int finished = 0;
	cycle = 0;
	vector<int> vpos;
	vector<int> best_vocra;
	int min_penalty = 1000000;
	vocra_penalty = 1000000;
	vpos.resize(av_cnt);
	vocra[sv] = vocra_p[sv][vpos[sv]];
	while (true) {
	check:
		if (sv < av_cnt - 1) {
			++sv;
			vocra[sv] = vocra_p[sv][vpos[sv]];
			continue;
		}
		if (need_exit) return;
		//LogVector("vpos", cp_id * 100 + sv, 0, av_cnt, vpos, "log\\temp.log");
		EvalVocalRanges();
		if (min_penalty > vocra_penalty) {
			best_vocra = vocra;
			min_penalty = vocra_penalty;
		}
	skip:
		while (true) {
			if (vpos[sv] < vocra_p[sv].size() - 1) break;
			// If current element is max, make it minimum
			vpos[sv] = 0;
			vocra[sv] = vocra_p[sv][0];
			// Move left one element
			if (!sv) {
				finished = 1;
				break;
			}
			--sv;
		} 
		if (finished) break;
		// Increase rightmost element, which was not reset to minimum
		++vpos[sv];
		vocra[sv] = vocra_p[sv][vpos[sv]];
		// Go to rightmost element
		sv = av_cnt - 1;
		++cycle;
	}
	// Recalculate vocal ranges
	vocra = best_vocra;
	fill(vocra_used.begin(), vocra_used.end(), 0);
	for (v = 0; v < av_cnt; ++v) {
		if (vocra_detected[v] == 0) vocra_detected[v] = 2;
		++vocra_used[vocra[v]];
	}
}

void CGenCA3::EvalVocalRanges() {
	vocra_penalty = 0;
	fill(vocra_used.begin(), vocra_used.end(), 0);
	for (v = 0; v < av_cnt; ++v) {
		// Prohibit decreasing vocal ranges
		if (v && vocra[v] < vocra[v - 1]) vocra_penalty += 100;
		// Stats 
		++vocra_used[vocra[v]];
	}
	for (int vr = 1; vr < 5; ++vr) {
		if (vocra_used[vr] > 1) {
			vocra_penalty += vocra_used[vr] * vocra_used[vr]; //  + (5 - vr)
		}
	}
}

void CGenCA3::GetPossibleVocalRanges() {
	vocra_p.resize(av_cnt);
	for (v = 0; v < av_cnt; ++v) {
		vocra_p[v].clear();
		// Set single possible vocal range if it was specified in instrument name
		if (vocra_detected[v] == 1) {
			vocra_p[v].push_back(vocra[v]);
		}
		else {
			// Get all exact matches of vocal ranges if it was not specified in instrument name
			for (int vr = 1; vr < vocra_info.size(); ++vr) {
				if (nmin[v] >= vocra_info[vr].min_cc && nmax[v] <= vocra_info[vr].max_cc) {
					vocra_p[v].push_back(vr);
				}
			}
			// If voice does not match any of ranges exactly, get closest vocal range
			if (!vocra_p[v].size()) {
				int nmed = (nmin[v] + nmax[v]) / 2;
				int best_vr = -1;
				int best_dif = 1000000;
				for (int vr = 1; vr < vocra_info.size(); ++vr) {
					int vrmed = (vocra_info[vr].min_cc + vocra_info[vr].max_cc) / 2;
					int ndif = abs(vrmed - nmed);
					if (ndif < best_dif) {
						best_dif = ndif;
						best_vr = vr;
					}
				}
				if (best_vr > 0) {
					vocra_p[v].push_back(best_vr);
				}
			}
		}
	}
}

int CGenCA3::AnalyseCP() {
	skip_flags = 0;
	GetCPKey();
	if (GetVocalRanges()) return 1;
	if (FailSpeciesCombination()) return 1;
	EvaluateCP();
	return 0;
}

int CGenCA3::FailSpeciesCombination() {
	CString est;
	// [sp] Species stats
	vector <int> sps; 
	sps.resize(MAX_SPECIES + 1);
	for (v = 0; v < av_cnt; ++v) {
		++sps[vsp[v]];
	}
	// Multiple cantus firmus
	if (sps[0] > 1) {
		WriteLogLy(5, "Multiple cantus firmus detected", 1);
	}
	// Species 5 should not be combined with species 2, 3, 4
	if (sps[5] && (sps[2] || sps[3] || sps[4])) {
		WriteLogLy(5, "Species 5 should not be combined with species 2, 3, or 4", 1);
	}
	return 0;
}
