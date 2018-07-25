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
	g_leaped.resize(av_cnt);
	slur.resize(av_cnt);
	flag.resize(av_cnt);
	fsl.resize(av_cnt);
	fvl.resize(av_cnt);
	sus.resize(av_cnt);
	susres.resize(av_cnt);
	isus.resize(av_cnt);
	msh.resize(av_cnt);
	mshb.resize(av_cnt);
	mshf.resize(av_cnt);
	pat.resize(av_cnt);
	dtp.resize(av_cnt);
	dtp_s.resize(av_cnt);
	pat_state.resize(av_cnt);
	beat.resize(av_cnt);
	hli.resize(c_len);
	hli2.resize(c_len);
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
	for (int v = 0; v < av_cnt; ++v) {
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
		susres[v].resize(c_len);
		isus[v].resize(c_len);
		msh[v].resize(c_len);
		mshb[v].resize(c_len);
		mshf[v].resize(c_len);
		pat[v].resize(c_len);
		dtp[v].resize(c_len);
		dtp_s[v].resize(c_len);
		pat_state[v].resize(c_len);
	}
}

void CGenCA3::LoadConfigLine(CString* sN, CString* sV, int idata, float fdata) {
	LoadVar(sN, sV, "musicxml_file", &musicxml_file);

	CGenCP2::LoadConfigLine(sN, sV, idata, fdata);
}

int CGenCA3::XML_to_CP() {
	vector<int> im; // Intermediate measures vector
	vector<vector<CString>> it; // Intermediate text vector
	vector<int> ifi; // Intermediate fifths vector
	vector<int> ibl; // Intermediate barlines vector

	av_cnt = xfi.voice.size();
	InitAnalysis();
	cp_tempo = 0;
	vname.resize(av_cnt);
	it.resize(av_cnt);
	for (vi = 0; vi < xfi.voice.size(); ++vi) {
		v = av_cnt - vi - 1;
		int pos = 0;
		vname[v] = xfi.voice[vi].name;
		for (int m = 1; m < xfi.mea.size(); ++m) {
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
				int ln = xfi.note[vi][m][ni].dur * 2.0 / xfi.note[vi][m][ni].dur_div;
				it[v].resize(pos + ln);
				ifi.resize(pos + ln, 100);
				cc[v].resize(pos + ln);
				retr[v].resize(pos + ln);
				if (xfi.note[vi][m][ni].tempo && !cp_tempo)
					cp_tempo = xfi.note[vi][m][ni].tempo;
				if (!xfi.note[vi][m][ni].rest && !xfi.note[vi][m][ni].tie_stop) retr[v][pos] = 1;
				for (int s = 0; s < ln; ++s) {
					cc[v][pos + s] = xfi.note[vi][m][ni].pitch;
				}
				// Get fifths
				ifi[pos] = xfi.note[vi][m][ni].fifths;
				// Concatenate text
				it[v][pos] = xfi.note[vi][m][ni].words;
				if (!xfi.note[vi][m][ni].words.IsEmpty() && !xfi.note[vi][m][ni].lyric.IsEmpty())
					it[v][pos] += ",";
				it[v][pos] += xfi.note[vi][m][ni].lyric;
				pos += ln;
				c_len = max(c_len, pos);
			}
		}
	}
	if (!cp_tempo) cp_tempo = 100;
	im.resize(c_len);
	ep2 = c_len;
	ResizeVectors(t_allocated, av_cnt);
	// Explode music into separate exercises
	// State: 0 - find note, 1 - find pause
	int state = 0;
	int s1 = 0;
	cp_id = 0;
	for (int s = 0; s < c_len; ++s) {
		int is_pause = 1;
		for (int v = 0; v < av_cnt; ++v) {
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
				int s2;
				if (!is_pause) s2 = s;
				else s2 = s - 1;
				// Move left to measure
				while (!im[s1] && s1 > 0) --s1;
				// Create new cp_id
				cp.resize(cp_id + 1);
				cp_retr.resize(cp_id + 1);
				cp_vid.resize(cp_id + 1);
				cp_mea.resize(cp_id + 1);
				cp_text.resize(cp_id + 1);
				cp_fi.resize(cp_id + 1, 100);
				cp_error.resize(cp_id + 1);
				// Fill new cp_id
				cp[cp_id].resize(av_cnt);
				cp_retr[cp_id].resize(av_cnt);
				cp_vid[cp_id].resize(av_cnt);
				// Set measures
				cp_mea[cp_id].resize(s2 - s1 + 1);
				for (int s3 = s1; s3 <= s2; ++s3) {
					cp_mea[cp_id][s3 - s1] = im[s3];
				}
				// Copy notes
				for (int v = 0; v < av_cnt; ++v) {
					cp[cp_id][v].resize(s2 - s1 + 1);
					cp_retr[cp_id][v].resize(s2 - s1 + 1);
					cp_vid[cp_id][v] = v;
					for (int s3 = s1; s3 <= s2; ++s3) {
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
						cp_retr[cp_id][v][s3 - s1] = retr[v][s3];
						if (!cp_text[cp_id].IsEmpty() && !it[v][s3].IsEmpty()) cp_text[cp_id] += ",";
						cp_text[cp_id] += it[v][s3];
						cp_text[cp_id].Replace(" ", ",");
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
		for (int v = 0; v < av_cnt; ++v) {
			for (int s = 0; s < cp[cp_id][v].size(); ++s) {
				if (cp[cp_id][v][s]) {
					empty[v] = 0;
					break;
				}
			}
		}
		for (int v = av_cnt-1; v >= 0; --v) {
			if (empty[v]) {
				verase(cp[cp_id], v);
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
				vname2[v2].Format("%s (%d)", vname2[v2], dupl);
			}
		}
	}
	return 0;
}

int CGenCA3::CheckXML() {
	for (int m = 1; m < xfi.mea.size(); ++m) {
		if (m > 1) {
			if (xfi.mea[m].beats != xfi.mea[m - 1].beats || xfi.mea[m].beat_type != xfi.mea[m - 1].beat_type) {
				CString est;
				// This is not a big problem because each counterpoint can have its measure size
				// More important to have same measure size inside counterpoint
				//est.Format("Measure %d size is changed", m);
				//WriteLog(1, est);
				//error = 10;
				//return 1;
			}
		}
		for (vi = 0; vi < xfi.voice.size(); ++vi) {
			for (int ni = 0; ni < xfi.note[vi][m].size(); ++ni) {
				float len = xfi.note[vi][m][ni].dur * 0.25 / xfi.note[vi][m][ni].dur_div;
				if (len < 0.125) {
					CString est;
					est.Format("Note length %.3f is shorter than 1/8 (0.125). Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d, note %d of %d",
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
	retr.resize(av_cnt);
	c_len = cp[cp_id][0].size();
	mli.clear();
	bmli.resize(c_len);
	npm = 0;
	// Detect mli and npm
	for (int s = 0; s < c_len; ++s) {
		if (cp_mea[cp_id][s]) {
			mli.push_back(s);
			if (mli.size() > 1) {
				if (npm) {
					if (npm != mli.end()[-1] - mli.end()[-2]) {
						est.Format("Measure %zu size is changed from %d to %d inside counterpoint %d",
							mli.size(), npm, mli.end()[-1] - mli.end()[-2], cp_id + 1);
						WriteLog(5, est);
						error = 10;
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
		retr[v].resize(c_len);
		vid[v] = cp_vid[cp_id][v];
		for (int s = 0; s < c_len; ++s) {
			cc[v][s] = cp[cp_id][v][s];
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
		WriteLog(5, est);
		error = 10;
		return 1;
	}

	ep2 = c_len;
	return 0;
}

int CGenCA3::GetCPSpecies() {
	CString est;
	LoadSpecies(conf_species);
	// Check if species can be loaded from MusicXML
	if (!cp_text[cp_id].IsEmpty()) {
		vector<CString> sa;
		Tokenize(cp_text[cp_id], sa, ",");
		for (int i = 0; i < sa.size(); ++i) {
			sa[i].Trim();
			if (sa[i].Left(2) == "sp") {
				LoadSpecies(sa[i].Mid(2));
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
		WriteLog(5, est);
		error = 10;
		return 1;
	}
	return 0;
}

void CGenCA3::Generate() {
	CString st;
	LoadLyShapes("configs\\ly\\shapes.csv");
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
		InitAnalysis();
		CreateLinks();
		if (GetCPSpecies()) continue;
		for (int v = 0; v < v_cnt; ++v) {
			FillPause(step0, full_len, v);
		}
		AnalyseCP();
		ValidateFlags();
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

	ly_fs << "\\header {tagline = \"This file was created by MGen CP2 ";
	ly_fs << APP_VERSION << " at " << CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S") << "\"}\n";
	read_file_sv("configs\\ly\\footer.ly", sv);
	write_file_sv(ly_fs, sv);
	ly_fs.close();
	ly_saved = 1;
}

void CGenCA3::GetCPKey() {
	fifths = cp_fi[cp_id];
	// Get base note as last note in bass
	for (s = c_len - 1; s >= 0; ++s) {
		if (cc[0][s]) {
			bn = cc[0][s] % 12;
			break;
		}
	}
	// Convert fifths with base note to mode
	maj_bn = (fifths * 7) % 12;
	mode = (bn - maj_bn + 12) % 12;
	// TODO: Find alterations
	// TODO: Select better key if alterations do not fit
	// TODO: If mode == 9, detect mminor using alterations
	// Always select melodic minor if minor mode
	if (mode == 9) mminor = 1;
	else mminor = 0;
}

void CGenCA3::AnalyseCP() {
	skip_flags = 0;
	GetCPKey();
	EvaluateCP();
}

