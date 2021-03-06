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
	resol.resize(av_cnt);
	msh.resize(av_cnt);
	mshb.resize(av_cnt);
	nih.resize(av_cnt);
	islt.resize(av_cnt);
	nihb.resize(av_cnt);
	resolb.resize(av_cnt);
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
	cct.resize(c_len, vector<int>(4));
	cctp.resize(c_len, vector<int>(4));
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
		resol[v].resize(c_len);
		msh[v].resize(c_len);
		mshb[v].resize(c_len);
		nih[v].resize(c_len);
		islt[v].resize(c_len);
		nihb[v].resize(c_len);
		resolb[v].resize(c_len);
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
	CheckVar(sN, sV, "voices_order_pitch", &voices_order_pitch);

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
	set<CString> unique_part_id; // Unique part ids

	av_cnt = xfi.voice.size();
	ial.resize(av_cnt);
	InitAnalysis();
	cp_tempo = 0;
	vname.resize(av_cnt);
	vname2.resize(av_cnt);
	vname_vocra_unique.resize(av_cnt);
	it.resize(av_cnt);
	ilyr.resize(av_cnt);
	for (v = 0; v < xfi.voice.size(); ++v) {
		// Position in resulting vector
		int pos = 0;
		// Position of measure inside xml file
		float xpos = 0;
		// Position of note inside xml measure
		float xpos2 = 0;
		// Convert from UTF8 to UTF16
		CStringW vname_w = CA2W(xfi.voice[v].name, CP_UTF8);
		// Build full name
		vname_w.Replace(L"MusicXML", L"");
		vname_w.Trim();
		if (vname_w.IsEmpty()) vname_w.Format(L"Part%d", v + 1);
		// Build short name
		CStringW vname_w2 = vname_w;
		if (vname_w2.GetLength() > ly_max_partname_len) vname_w2 = vname_w2.Left(ly_max_partname_len);
		// Conert back to UTF8
		vname[v] = CW2A(vname_w, CP_UTF8);
		vname2[v] = CW2A(vname_w2, CP_UTF8);
		unique_part_id.insert(xfi.voice[v].id);
		track_id[v] = unique_part_id.size();
		track_name[v].Format("%s (%s), staff %d, voice %d, chord %d", vname[v], xfi.voice[v].id, xfi.voice[v].staff, xfi.voice[v].v, xfi.voice[v].chord);
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
			for (int ni = 0; ni < xfi.note[v][m].size(); ++ni) {
				xpos2 += xfi.note[v][m][ni].dur * 2.0 / xfi.note[v][m][ni].dur_div;
				int ln = floor(xpos + xpos2) - pos; 
				// xfi.note[v][m][ni].dur * 2.0 / xfi.note[v][m][ni].dur_div;
				//if (pos + ln < floor(xpos + xpos2)) 
				c_len = max(c_len, pos + ln);
				it[v].resize(c_len);
				ilyr[v].resize(c_len);
				ifi.resize(c_len, 100);
				ibt.resize(c_len);
				cc[v].resize(c_len);
				ial[v].resize(c_len);
				retr[v].resize(c_len);
				if (xfi.note[v][m][ni].tempo && !cp_tempo)
					cp_tempo = xfi.note[v][m][ni].tempo;
				if (pos && !xfi.note[v][m][ni].rest && !xfi.note[v][m][ni].tie_stop &&
					xfi.note[v][m][ni].pitch == cc[v][pos - 1]) retr[v][pos] = 1;
				// Check lowest note
				if (!xfi.note[v][m][ni].rest && !xfi.note[v][m][ni].pitch) {
					CString est;
					est.Format("Detected too low note. This note will be replaced with a rest. Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d, note %d of %d.",
						m, v, xfi.voice[v].id, xfi.voice[v].name, xfi.voice[v].staff, xfi.voice[v].v, xfi.voice[v].chord,
						xfi.mea[m].beats, xfi.mea[m].beat_type, ni + 1, xfi.note[v][m].size());
					WriteLogLy(1, est, 0);
				}
				for (s = 0; s < ln; ++s) {
					cc[v][pos + s] = xfi.note[v][m][ni].pitch;
					ial[v][pos + s] = xfi.note[v][m][ni].alter;
				}
				// Get fifths
				ifi[pos] = xfi.note[v][m][ni].fifths;
				ibt[pos] = xfi.mea[m].beat_type;
				// Concatenate text
				it[v][pos] = xfi.note[v][m][ni].words;
				ilyr[v][pos] = xfi.note[v][m][ni].lyric;
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
				for (vi = 0; vi < av_cnt; ++vi) {
					v = av_cnt - vi - 1;
					cp[cp_id][v].resize(s2 - s1 + 1);
					cp_alter[cp_id][v].resize(s2 - s1 + 1);
					cp_retr[cp_id][v].resize(s2 - s1 + 1);
					cp_vid[cp_id][v] = vi;
					for (int s3 = s1; s3 <= s2; ++s3) {
						if (ibt[s3]) cp_btype[cp_id] = ibt[s3];
						if (ifi[s3] != 100) {
							if (cp_fi[cp_id] == 100) cp_fi[cp_id] = ifi[s3];
							else {
								if (cp_fi[cp_id] != ifi[s3] && !cp_error[cp_id]) {
									CString est;
									est.Format("Key changed in the middle of counterpoint %d. Ignoring this counterpoint.",
										cp_id + 1);
									WriteLog(1, est);
									cp_error[cp_id] = 1;
								}
							}
						}
						cp[cp_id][v][s3 - s1] = cc[vi][s3];
						cp_alter[cp_id][v][s3 - s1] = ial[vi][s3];
						cp_retr[cp_id][v][s3 - s1] = retr[vi][s3];
						if (!cp_text[cp_id].IsEmpty() && !it[vi][s3].IsEmpty()) cp_text[cp_id] += " ";
						cp_text[cp_id] += it[vi][s3];
						if (!cp_lyrics[cp_id].IsEmpty() && !ilyr[vi][s3].IsEmpty()) cp_lyrics[cp_id] += " ";
						cp_lyrics[cp_id] += ilyr[vi][s3];
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
	xmlfile_loaded = 1;
	return 0;
}

int CGenCA3::CheckXML() {
	for (int m = 1; m < xfi.mea.size(); ++m) {
		for (vi = 0; vi < xfi.voice.size(); ++vi) {
			for (int ni = 0; ni < xfi.note[vi][m].size(); ++ni) {
				float len = xfi.note[vi][m][ni].dur * 0.25 / xfi.note[vi][m][ni].dur_div;
				if (len < 0.125) {
					CString est;
					est.Format("Note length %.3f is shorter than a quaver (0.125). Currently not supported. Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d, note %d of %d",
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
	if (av_cnt > 9) {
		est.Format("%d voices detected in exercise. Maximum 9 voices is supported. Ignoring exercise.",
			av_cnt);
		WriteLogLy(5, est, 0);
		return 1;
	}
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
						est.Format("Measure %zu size is changed from %d to %d inside exercise %d. Ignoring exercise.",
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

	// Check if not enough notes are imported
	if (c_len % npm && av_cnt > 1) {
		est.Format("Exercise %d finishes before measure end. Ignoring exercise.",
			cp_id + 1);
		WriteLogLy(1, est, 0);
		return 1;
	}
	for (v = 0; v < av_cnt; ++v) {
		if (!cc[v][c_len - 1]) {
			est.Format("Voice %d ends before the end of exercise %d. Ignoring exercise.",
				v + 1, cp_id + 1);
			WriteLogLy(1, est, 0);
			return 1;
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
		//est.Format("Counterpoint %d starts not on downbeat. Please check counterpoint.", cp_id + 1);
		//WriteLog(1, est);
		//return 1;
	}

	ep2 = c_len;
	return 0;
}

int CGenCA3::GetCPSpecies() {
	CString st, est;
	if (av_cnt == 1) {
		vsp.resize(av_cnt);
		vsp[0] = 0;
		return 0;
	}
	LoadGlobalSpecies(conf_species);
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
			if (sa[i].Left(2) == "sp" && isdigit(sa[i][2])) {
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
			if (sa[i].Left(2) == "sp" && isdigit(sa[i][2])) {
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
		vector<int> cf_v;
		cf_v.resize(av_cnt, 1);
		if (my_sp <= 1) {
			// For species 1 search voice with minimum notes (because counterpoint can contain slurs)
			best_fli = 1000000;
			for (v = av_cnt-1; v >= 0; --v) {
				if (fli_size[v] < best_fli) {
					best_fli = fli_size[v];
					best_v = v;
				}
				for (ls = 0; ls < fli_size[v]; ++ls) {
					if (fli[v][ls] % npm || llen[v][ls] != npm) {
						cf_v[v] = 0;
					}
				}
			}
		}
		else {
			// For species 2-5 search for highest voice containing at least one note starting not on measure start or with length not whole measure or multiple whole measures
			for (v = 0; v < av_cnt; ++v) {
				for (ls = 0; ls < fli_size[v]; ++ls) {
					if (fli[v][ls] % npm || llen[v][ls] % npm) {
						best_v = v;
					}
					if (fli[v][ls] % npm || llen[v][ls] != npm) {
						cf_v[v] = 0;
					}
				}
			}
		}
		int best_cf_v = 0;
		for (v = 0; v < av_cnt; ++v) {
			if (cf_v[v] && v != best_v) {
				best_cf_v = v;
				break;
			}
		}
		vsp.clear();
		vsp.resize(av_cnt, -1);
		vsp[best_v] = my_sp;
		for (v = 0; v < av_cnt; ++v) {
			if (vsp[v] == -1) {
				// Set species 1 for all voices except lowest
				if (v == best_cf_v) vsp[v] = 0;
				// Set CF for lowest voice
				else vsp[v] = 1;
			}
		}
	}
	else if (vsp.size() != av_cnt) {
		est.Format("Check species parameter in config or MusicXML file: %zu voices specified, but there are %d voices in exercise %d. Parameter in MusicXML will have precedence. Ignoring exercise.",
			vsp.size(), av_cnt, cp_id + 1);
		WriteLogLy(1, est, 0);
		return 1;
	}
	return 0;
}

void CGenCA3::Generate() {
	CString st;
	if (error) return;
	ParseRules();
	SetRuleParams();
	// Uncomment to test XX masks in rule comments and subrule comments
	//ReplaceRuleParams();
	if (error) return;
	LoadLyShapes("configs\\ly2\\shapes.csv");
	LoadLyShapeScripts("configs\\ly2\\shape_scripts.csv");
	ValidateShapeText();
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
	if (xfi.ReorderVoices(voices_order_pitch)) {
		WriteLog(1, "Voices reordered due to significant difference in average pitch");
	}
	xfi.ValidateXML();
	if (!xfi.error.IsEmpty()) {
		WriteLog(5, xfi.error);
		error = 9;
		return;
	}
	if (!xfi.warning.IsEmpty()) {
		WriteLogLy(1, xfi.warning, 0);
	}
	if (CheckXML()) return;
	if (XML_to_CP()) return;
	if (conf_species.GetLength() > 1 && conf_species.GetLength() != v_cnt) {
		CString est;
		est.Format("Species in config is for %d voices ('%s'), but there are %d voices in file", conf_species.GetLength(), conf_species, v_cnt);
		WriteLog(0, est);
	}
	for (cp_id = 0; cp_id < cp.size(); ++cp_id) if (!cp_error[cp_id]) {
		st.Format("Analyzing: %d of %zu", cp_id + 1, cp.size());
		SetStatusText(3, st);
		if (GetCP()) continue;
		int real_len = cc[0].size();
		int full_len = floor((real_len + 1) / 8 + 1) * 8;
		GetAnalysisVectors();
		if (GetCPSpecies()) continue;
		if (AnalyseCP()) continue;
		DistributeFlags();
		for (int v = 0; v < v_cnt; ++v) {
			FillPause(step0, full_len, v);
		}
		SendCP();
		SaveLyCP();
		step0 += full_len;
		if (need_exit) break;
	}
	if (!t_sent) {
		WriteLog(5, "Nothing to analyse");
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
	title = CW2A(CA2W(my_config, CP_ACP), CP_UTF8) + " (" +
		CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M") + ")";
	ly_fs.open(dir + "\\" + fname + ".ly");
	read_file_sv("configs\\ly2\\header.ly", sv);
	for (int i = 0; i < sv.size(); ++i) {
		sv[i].Replace("$DEDICATION$", "");
		sv[i].Replace("$TITLE$", "");
		sv[i].Replace("$SUBTITLE$", m_algo_name);
		sv[i].Replace("$SUBSUBTITLE$", title);
		if (ly_page_breaking.IsEmpty()) {
			sv[i].Replace("$PAGE_BREAKING$", "minimal-breaking");
		}
		else {
			sv[i].Replace("$PAGE_BREAKING$", ly_page_breaking);
		}
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
	ly_fs << "\\header {tagline = \\markup \\tiny \\center-column {\"This file was created by MGen CA3 ";
	ly_fs << APP_VERSION << " at " << CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S") << ".\"";
	if (ctools_task_id) ly_fs << " \\with-url #\"http://artportal.su/ctools/job.php?j_id=" << ctools_task_id << "\" \\with-color #(rgb-color 0 0 1) \\underline \"Composer Tools task #" << ctools_task_id << "\" ";
	ly_fs << " #(string-append \"Engraved by LilyPond \" (lilypond-version) \". File: " << my_config << "\")  \" \"";
	ly_fs << "}}\n";
	read_file_sv("configs\\ly2\\footer.ly", sv);
	write_file_sv(ly_fs, sv);
	ly_fs << "\\markup \\tiny { \\vspace #1.5\n Harmonic notation: " + harm_notation_name[harm_notation] + " }\n";
	ly_fs.close();
	ly_saved = 1;
}

void CGenCA3::GetCPKey() {
	CString est;
	fifths = cp_fi[cp_id];
	// Check if fifth index is supported
	if (fifths > 14 || fifths < -14) {
		est.Format("Specified key with %d fifths is not supported. Minimum supported is -14 and maximum supported is 14.",
			fifths);
		WriteLogLy(1, est, 1);
	}
	// Detect major base note
	maj_bn = (fifths * 7 + 12 * 12) % 12;
	// Temporarily set base note to major base note for last chord detection
	bn = maj_bn;
	mode = 0;
	bn_alter = 0;
	BuildPitchConvert();
	// Get diatonic pitch class for chord detection
	GetDiatonic(0, c_len);
	GetPitchClass(0, c_len);
	// Get base note as last note in bass
	for (s = c_len - 1; s >= 0; --s) {
		if (cc[0][s]) {
			//bn = cc[0][s] % 12;
			// Collect notes from all voices
			chn.clear();
			cchn.clear();
			chn.resize(1, empty_chn);
			cchn.resize(1, empty_cchn);
			sp = 0;
			vc = av_cnt;
			vp = 0;
			hs = 0;
			for (v = 0; v < av_cnt; ++v) {
				++chn[0][pc[v][s]];
				++cchn[0][pcc[v][s]];
			}
			int lchm;
			int rat;
			GetHarm(lchm, rat);
			bn = c_cc[lchm + 14] % 12;
			// Get base note alteration
			bn_alter = -1;
			for (v = 0; v < av_cnt; ++v) {
				if ((pcc[v][s] + maj_bn) % 12 == bn) {
					bn_alter = src_alter[v][s];
				}
			}
			// If no root note was found
			if (bn_alter == -1) {
				if (bn == 1 || bn == 3 || bn == 6 || bn == 8 || bn == 10) {
					if (fifths > 0) bn_alter = 1;
					else bn_alter = -1;
				}
				else bn_alter = 0;
			}
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
		est.Format("Specified key was %s, but detected alterations (major I#, II# or VI#) cannot be used in this key. Please check source file. Exercise analysis may be incorrect.",
			GetPrintKey(bn, mode));
		WriteLogLy(1, est, 1);
	}
	// Detect mminor
	if (detected_mminor) {
		if (mode != 9) {
			est.Format("Detected melodic minor alterations (major IV# or V#), but specified key was %s. Please check source file. Exercise analysis may be incorrect.",
				GetPrintKey(bn, mode));
			WriteLogLy(1, est, 1);
		}
		else {
			mminor = 1;
		}
	}
	// TODO: Select better key if alterations do not fit, while preserving bn
}

// Get unique vocal range names
void CGenCA3::GetUniqueVocra() {
	vector<int> vocra_cnt;
	vocra_cnt.resize(5);
	vector<int> vocra_num;
	vocra_num.resize(5);
	for (v = 0; v < av_cnt; ++v) {
		int vr = vocra[v];
		if (!vocra_detected[v] || !vr) {
			continue;
		}
		++vocra_cnt[vr];
	}
	for (v = av_cnt - 1; v >= 0; --v) {
		vname_vocra_unique[v] = "";
		int vr = vocra[v];
		if (!vocra_detected[v] || !vr) {
			continue;
		}
		++vocra_num[vr];
		if (vocra_cnt[vr] > 1) {
			vname_vocra_unique[v].Format("%s-%d", vocra_info[vr].name, vocra_num[vr]);
		}
		else {
			vname_vocra_unique[v] = vocra_info[vr].name;
		}
	}
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
		if (st.Left(7) == "soprano") {
			vocra[v] = vrSop;
			++vocra_used[vrSop];
			vocra_detected[v] = 1;
		}
		if (st.Left(4) == "alto") {
			vocra[v] = vrAlt;
			++vocra_used[vrAlt];
			vocra_detected[v] = 1;
		}
		if (st.Left(5) == "tenor") {
			vocra[v] = vrTen;
			++vocra_used[vrTen];
			vocra_detected[v] = 1;
		}
		if (st.Left(4) == "bass") {
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
		WriteLogLy(1, est, 0);
		return 1;
	}
	ScanVocalRanges();
	for (v = 0; v < av_cnt; ++v) if (!vocra_detected[v]) {
		est.Format("Cannot detect vocal range for counterpoint %d, part %d: %s. Please specify vocal range in instrument name in source file %s",
			cp_id + 1, vid[v] + 1, vname[vid[v]], musicxml_file);
		WriteLogLy(1, est, 0);
	}
	return 0;
}

void CGenCA3::ScanVocalRanges() {
	// Scanned voice
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
	GetUniqueVocra();
	if (!instr_config.size()) {
		// Save loaded instr
		instr_config = instr;
		// Clear instr
		for (int i = 0; i < v_cnt; ++i) instr[i] = 0;
	}
	// Fill instr which is not set
	// This will not overwrite on next cp, but will add instruments that were not set earlier
	for (int i = 0; i < av_cnt; ++i) {
		if (vocra[i] > 0 && !instr[vid[i]]) 
			instr[vid[i]] = instr_config[vocra[i] - 1];
	}
	if (FailSpeciesCombination()) return 1;
	EvaluateCP();
	return 0;
}

int CGenCA3::FailSpeciesCombination() {
	// [sp] Species stats
	vector <int> sps; 
	sps.resize(MAX_SPECIES + 1);
	for (v = 0; v < av_cnt; ++v) {
		++sps[vsp[v]];
		if (!vsp[v] && sps[0] > 1) vsp[v] = 1;
	}
	// Multiple cantus firmus
	if (sps[0] > 1) {
		WriteLogLy(1, "Multiple cantus firmus detected. Replaced with species 1", 1);
	}
	// Species 5 should not be combined with species 2, 3, 4
	if (sps[5] && (sps[2] || sps[3] || sps[4])) {
		//WriteLog(1, "Species 5 usually should not be combined with species 2, 3, or 4");
	}
	return 0;
}
