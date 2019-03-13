// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "Conf.h"
#include "CsvDb.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

CConf::CConf() {
}

CConf::~CConf() {
}

void CConf::AddIcf() {
	if (icf.size() >= MAX_INSTR) {
		CString est;
		est.Format("Cannot create more instruments than MAX_INSTR (%d). Increase MAX_INSTR if needed", MAX_INSTR);
		WriteLog(5, est);
		return;
	}
	icf.resize(icf.size() + 1);
	icf[icf.size() - 1].KswGroup.resize(128);
	icf[icf.size() - 1].legato_ahead.resize(10);
	icf[icf.size() - 1].ahead_chrom.resize(16);
}

void CConf::LoadConfigFile(CString fname, int load_includes) {
	long long time_start = CGLib::time();
	CString st, st2, st3, iname;
	ifstream fs;
	int instr_id = -1;
	int tr_id = -1;
	int st_id = -1;
	m_current_config = fname;
	// Check file exists
	if (!fileExists(fname)) {
		CString est;
		est.Format("LoadConfigFile cannot find file: %s", fname);
		WriteLog(5, est);
		error = 1000;
		return;
	}
	fs.open(fname);
	char pch[2550];
	int pos = 0;
	int i = 0;
	while (fs.good()) {
		i++;
		// Get line
		fs.getline(pch, 2550);
		st = pch;
		st.Replace("\"", "");
		// Remove unneeded
		pos = st.Find("#");
		// Check if it is first symbol
		if (pos == 0)	st = st.Left(pos);
		pos = st.Find(" #");
		// Check if it is after space
		if (pos > -1)	st = st.Left(pos);
		st.Trim();
		// Load include
		if (load_includes && CheckInclude(st, fname, iname)) {
			LoadConfigFile(iname);
			st.Empty();
		}
		if (error) break;
		pos = st.Find("=");
		if (pos != -1) {
			// Get variable name and value
			st2 = st.Left(pos);
			st3 = st.Mid(pos + 1);
			st2.Trim();
			st3.Trim();
			st2.MakeLower();
			// Load general variables
			int idata = atoi(st3);
			float fdata = atof(st3);
			parameter_found = 0;
			CheckVar(&st2, &st3, "ctools_task_id", &ctools_task_id);
			CheckVar(&st2, &st3, "ctools_file_id", &ctools_file_id);
			CheckVar(&st2, &st3, "v_cnt", &v_cnt, 1, 10);
			CheckVar(&st2, &st3, "t_cnt", &t_cnt, 1);
			CheckVar(&st2, &st3, "t_allocated", &t_allocated, 0);
			CheckVar(&st2, &st3, "t_send", &t_send, 1);
			CheckVar(&st2, &st3, "midifile_in_mul", &midifile_in_mul, 0.00000001);
			CheckVar(&st2, &st3, "midifile_out_mul", &midifile_out_mul0, 0.00000001);
			CheckVar(&st2, &st3, "sleep_ms", &sleep_ms, 0);
			CheckVar(&st2, &st3, "midifile_export_marks", &midifile_export_marks, 0, 1);
			CheckVar(&st2, &st3, "midifile_export_comments", &midifile_export_comments, 0, 1);
			CheckVar(&st2, &st3, "adapt_enable", &adapt_enable, 0, 1);
			CheckVar(&st2, &st3, "autolegato", &auto_legato, 0, 1);
			CheckVar(&st2, &st3, "reverb_mix", &reverb_mix, 0, 100);
			CheckVar(&st2, &st3, "toload_time", &toload_time);
			CheckVar(&st2, &st3, "autononlegato", &auto_nonlegato, 0, 1);
			CheckVar(&st2, &st3, "grownotes", &grow_notes);
			CheckVar(&st2, &st3, "comment_adapt", &comment_adapt, 0, 1);
			CheckVar(&st2, &st3, "mastervolume", &master_vol, 0, 100);
			LoadVar(&st2, &st3, "instr_layout", &instr_layout);
			LoadVar(&st2, &st3, "instruments", &m_config_insts);
			if (st2 == "instruments") {
				LoadInstruments(m_config_insts);
			}
			if (st2 == "unison_mute") {
				++parameter_found;
				int val = atoi(st3);
				for (int ii = 0; ii < icf.size(); ++ii) {
					icf[ii].unis_mute = val;
				}
			}
			// Load midi file type
			if (st2 == "midifiletype") {
				++parameter_found;
				if (st3 == "Sibelius") {
					midi_file_type = mftSIB;
					grow_notes = 0;
				}
				if (st3 == "Finale") {
					midi_file_type = mftFIN;
					grow_notes = 0;
				}
				if (st3 == "MuseScore") {
					midi_file_type = mftMUS;
					grow_notes = 2;
				}
			}
			//LoadVarInstr(&st2, &st3, "instruments", instr);
			LoadVectorPar(&st2, &st3, "show_transpose", show_transpose, 0);
			// Load instrument layout overrides
			LoadInstrumentLayoutLine(st2, st3);
			// Load instrument id
			if (st2.Find(":") > 0) {
				++parameter_found;
				instr_id = -1;
				tr_id = -1;
				st_id = -1;
				CString ipath = st2.Left(st2.Find(":"));
				ipath.Replace("\"", "");
				ipath.Trim();
				vector<CString> sa;
				Tokenize(ipath, sa, "/");
				// Load instrument group
				if (sa.size() == 1) {
					int found = 0;
					for (int i = 0; i < icf.size(); ++i) {
						if (!ipath.CompareNoCase(icf[i].group)) {
							instr_id = i;
							++found;
						}
					}
					if (found > 1)
						WriteLog(5, "Instrument group " + ipath +
							" is ambiguous. Please add instrument config name after slash in file " + fname);
					// If instrument is found but is not loaded - load
					if (!icf[instr_id].loaded)
						LoadInstruments(icf[instr_id].group);
				}
				// Load instrument
				else {
					for (int i = 0; i < icf.size(); ++i) {
						if (!sa[0].CompareNoCase(icf[i].group) && !sa[1].CompareNoCase(icf[i].name)) {
							instr_id = i;
							// If instrument is found but not loaded - load
							if (!icf[i].loaded)
								LoadInstruments(sa[0] + "/" + sa[1]);
							break;
						}
					}
					// Load instrument if it was not previously loaded - and search again
					if (instr_id == -1) {
						LoadInstruments(sa[0] + "/" + sa[1]);
						for (int i = 0; i < icf.size(); ++i) {
							if (!sa[0].CompareNoCase(icf[i].group) && !sa[1].CompareNoCase(icf[i].name)) {
								instr_id = i;
								break;
							}
						}
					}
				}
				if (instr_id != -1 && sa.size() > 2) {
					tr_id = atoi(sa[2]);
					// Find child
					if (icf[instr_id].child.find(tr_id) != icf[instr_id].child.end()) {
						instr_id = icf[instr_id].child[tr_id];
					}
					else {
						instr_id = CreateVirtualInstrument(instr_id, tr_id);
					}
				}
				if (instr_id != -1 && sa.size() == 4) {
					st_id = atoi(sa[3]);
					// Find child
					if (icf[instr_id].child.find(st_id) != icf[instr_id].child.end()) {
						instr_id = icf[instr_id].child[st_id];
					}
					else {
						instr_id = CreateVirtualInstrument(instr_id, st_id);
					}
				}
				if (instr_id == -1) {
					WriteLog(1, "Cannot find instrument (ignoring) " + ipath);
				}
				else {
					// Check if instrument was overridden
					if (icf[instr_id].child.size()) {
						WriteLog(5, "After overriding instrument track or stage config, you cannot reconfigure whole instrument at line " + st + " in file " + fname);
					}
				}
				// Load instrument overrides
				if (instr_id > -1) {
					CString st4 = st2.Mid(st2.Find(":") + 1);
					st4.Trim();
					LoadInstrumentLine(st4, st3, instr_id);
				}
			}
			// Load algorithm-specific variables if we are not loading saved results
			if (!m_loading) {
				LoadConfigLine(&st2, &st3, idata, fdata);
				if (!parameter_found) {
					WriteLog(5, "Unrecognized parameter '" + st2 + "' = '" + st3 + "' in file " + fname);
				}
			}
			if (error) break;
		}
		else {
			if (!st.IsEmpty()) {
				WriteLog(5, "No equal sign in line, which is not a comment: '" + st + "' in file " + fname);
			}
		}
	}
	fs.close();
	CString est;
	long long time_stop = CGLib::time();
	est.Format("LoadConfigFile loaded %d lines from %s in %lld ms", i, fname, time_stop - time_start);
	WriteLog(0, est);
}

short CConf::CreateVirtualInstrument(int instr_id, int child_id) {
	// Allocate new virtual instrument
	++virt_instr_count;
	AddIcf();
	int instr_id2 = icf.size() - 1;
	// Copy instrument config
	icf[instr_id2] = icf[instr_id];
	// Remove childs after copying
	icf[instr_id2].child.clear();
	// Save parent
	icf[instr_id2].parent = instr_id;
	// Save child
	icf[instr_id].child[child_id] = instr_id2;
	return instr_id2;
}

void CConf::LoadConfig(CString fname, int load_includes) {
	CString st2;
	// Load instruments layout
	if (instr_layout.IsEmpty()) instr_layout = "Default";
	LoadInstrumentLayout();
	// Load default instrument
	LoadInstruments("");
	// Load configs again, overwriting instrument parameters
	LoadConfigFiles(fname, load_includes);
	// After loading global mapping of voices to instruments, load algorithm-specific mapping
	st2 = "instruments";
	LoadVarInstr(&st2, &m_config_insts, "instruments", instr);
	// Process configs
	ProcessConfig();
}

void CConf::ProcessConfig() {
	int max_vol = 100;
	for (int ii = 0; ii < icf.size(); ++ii) if (icf[ii].used) {
		if (icf[ii].vol > max_vol) max_vol = icf[ii].vol;
	}
	float max_db = (max_vol - 100) / 5.0;
	CString est;
	est.Format("Detected maximum dB overshoot: %f", max_db);
	WriteLog(0, est);
	est = "";
	for (int ii = 0; ii < icf.size(); ++ii) {
		icf[ii].db = vol2db(icf[ii].vol, icf[ii].vol_default, icf[ii].db_max, icf[ii].db_coef);
		icf[ii].db_compressed = icf[ii].db - max_db;
		if (icf[ii].used) {
			CString st;
			st.Format("%d. %s: %.1f (vol %d, cc %d)\n", ii, icf[ii].group, icf[ii].db_compressed,
				icf[ii].vol,
				db2cc(icf[ii].db_compressed, icf[ii].vol_default, icf[ii].db_max, icf[ii].db_coef));
			est += st;
		}
	}
	WriteLog(0, "Instruments db: " + est);
}

void CConf::LoadConfigFiles(CString fname, int load_includes) {
	long long time_start = CGLib::time();
	LoadConfigFile(fname, load_includes);
	// Load autotest config
	if (m_testing == 1) {
		CString fname2 = "autotest\\configs\\" + m_algo_folder + ".pl";
		if (fileExists(fname2)) LoadConfigFile(fname2, load_includes);
	}
	if (m_testing == 2) {
		CString fname2 = "configs\\server\\" + m_algo_folder + ".pl";
		if (fileExists(fname2)) LoadConfigFile(fname2, load_includes);
	}
	// Log
	long long time_stop = CGLib::time();
	CString est;
	est.Format("LoadConfigFiles loaded %s in %lld ms", fname, time_stop - time_start);
	WriteLog(0, est);
}

void CConf::LoadInstrumentLayout()
{
	CString fname = "instruments\\" + instr_layout + ".txt";
	long long time_start = CGLib::time();
	// Check file exists
	if (!fileExists(fname)) {
		CString est;
		est.Format("LoadInstrumentLayout cannot find file: %s", fname);
		WriteLog(5, est);
		return;
	}
	ifstream fs;
	fs.open(fname);
	CString st, st2, st3, iclass;
	char pch[2550];
	int pos = 0;
	int x = 0;
	int ii = 0;
	// Clear instrument configs
	icf.clear();
	while (fs.good()) {
		++x;
		fs.getline(pch, 2550);
		st = pch;
		// Remove unneeded
		pos = st.Find("#");
		// Check if it is first symbol
		if (pos == 0)	st = st.Left(pos);
		pos = st.Find(" #");
		// Check if it is after space
		if (pos > -1)	st = st.Left(pos);
		st.Trim();
		pos = 0;
		if (st.Find("|") != -1) {
			st2 = st.Tokenize("|", pos);
			st2.Trim();
			// Get instrument class
			if (st2.Find("/") != -1) {
				iclass = st2.Left(st2.Find("/"));
				st2 = st2.Mid(st2.Find("/") + 1);
			}
			else iclass.Empty();
			for (int x = 0; x < icf.size(); ++x) {
				if (icf[x].group == st2) {
					WriteLog(5, "Instrument layout should contain unique instrument groups. Detected duplicate: " + st2);
				}
			}
			AddIcf();
			ii = icf.size() - 1;
			icf[ii].iclass = iclass;
			icf[ii].group = st2;
			icf[ii].default_instr = ii;
			st2 = st.Tokenize("|", pos);
			st2.Trim();
			icf[ii].name = st2;
			st2 = st.Tokenize("|", pos);
			st2.Trim();
			icf[ii].channel = atoi(st2);
			st2 = st.Tokenize("|", pos);
			st2.Trim();
			icf[ii].track = atoi(st2);
			st2 = st.Tokenize("|", pos);
			st2.Trim();
			if (st2 == "+") icf[ii].port = 1;
			// Set default mapping
			instr[ii] = ii;
			InstDefaultConfig[icf[ii].group] = icf[ii].name;
		}
		pos = st.Find("=");
		if (pos != -1) {
			st2 = st.Left(pos);
			st3 = st.Mid(pos + 1);
			st2.Trim();
			st3.Trim();
			st2.MakeLower();
			parameter_found = 0;
			LoadInstrumentLayoutLine(st2, st3);
			if (!parameter_found) {
				WriteLog(5, "Unrecognized parameter '" + st2 + "' = '" + st3 + "' in file " + fname);
			}
		}
	}
	// 
	for (int i = 0; i < MAX_VOICE; i++) instr[i] = 0;
	fs.close();
	if (icf.size() == 0) {
		WriteLog(5, "No instruments loaded from " + fname);
	}
	// Log
	long long time_stop = CGLib::time();
	CString est;
	est.Format("LoadInstrumentLayout loaded %d lines from " + fname + " in %lld ms", x, time_stop - time_start);
	WriteLog(0, est);
}

void CConf::LoadInstrumentLayoutLine(CString &st2, CString &st3) {
	CheckVar(&st2, &st3, "rnd_tempo", &rnd_tempo);
	CheckVar(&st2, &st3, "rnd_tempo_step", &rnd_tempo_step);
	CheckVar(&st2, &st3, "rnd_tempo_slow", &rnd_tempo_slow);
}

void CConf::LoadVarInstr(CString * sName, CString * sValue, char* sSearch, vector<int> & Dest) {
	if (*sName == sSearch) {
		int pos = 0, ii = 0;
		CString st;
		for (int v = 0; v<MAX_VOICE; v++) {
			st = sValue->Tokenize(",", pos);
			st.Trim();
			if (st.IsEmpty()) break;
			int found = 0;
			if (st.Find("/") == -1) {
				for (ii = 0; ii < icf.size(); ii++) {
					if (icf[ii].group == st) {
						++found;
						Dest[v] = ii;
						++icf[ii].used;
						break;
					}
				}
			}
			// Load particular config
			else {
				CString gname = st.Left(st.Find("/"));
				CString cname = st.Mid(st.Find("/") + 1);
				for (ii = 0; ii < icf.size(); ii++) {
					if (icf[ii].group == gname && icf[ii].name == cname) {
						++found;
						Dest[v] = ii;
						++icf[ii].used;
						break;
					}
				}
			}
			if (!found) {
				CString est;
				est.Format("Cannot find any instrument named %s (%d) in layout %s. Mapped to default instrument %s/%s (%d)",
					st, v, instr_layout, icf[0].group, icf[0].name, 0);
				WriteLog(5, est);
			}
			else {
				// Calculate instrument config for voice
				if (icf[ii].child.find(v + 1) != icf[ii].child.end()) {
					ii = icf[ii].child[v + 1];
					instr[v] = ii;
					++icf[ii].used;
				}
			}
		}
	}
}

void CConf::LoadInstruments(CString ist) {
	long long time_start = CGLib::time();
	CString fname, cname;
	int found_default, ii2;
	// Build instruments vector
	vector<CString> iag, iac;
	Tokenize(ist, iag, ",");
	iac.resize(iag.size());
	for (int i = 0; i < iag.size(); ++i) {
		iag[i].Trim();
		if (iag[i].Find("/") != -1) {
			iac[i] = iag[i].Mid(iag[i].Find("/") + 1);
			iag[i] = iag[i].Left(iag[i].Find("/"));
		}
	}
	for (int ii = 0; ii < icf.size(); ++ii) if (icf[ii].default_instr == ii) {
		CFileFind finder;
		CString strWildcard = "instruments\\" + icf[ii].group + "\\*.*";
		BOOL bWorking = finder.FindFile(strWildcard);
		found_default = 0;
		while (bWorking) {
			bWorking = finder.FindNextFile();
			if (finder.IsDots()) continue;
			fname = finder.GetFileName();
			if (fname[0] == '_') continue;
			if (fname.Right(3) != ".pl") continue;
			cname = fname.Left(fname.GetLength() - 3);
			if (cname.Find(".") != -1) {
				cname = cname.Mid(cname.Find(".") + 1);
			}
			// Is it default config?
			if (icf[ii].name == cname) {
				ii2 = ii;
				found_default = 1;
			}
			// If config was already loaded
			int found_loaded = 0;
			for (int i = 0; i < icf.size(); ++i) {
				// Correct group and not loaded
				if (icf[i].group == icf[ii].group && cname == icf[i].name && icf[i].loaded) {
					found_loaded = 1;
					break;
				}
			}
			if (found_loaded) continue;
			// Load if this is default config of default instrument
			int need_load = 0;
			if (icf[ii].name == cname && !ii) {
				need_load = 1;
			}
			else {
				for (int i = 0; i < iag.size(); ++i) {
					if (!iag[i].CompareNoCase(icf[ii].group)) {
						// Find default config
						if (iac[i] == "" && icf[ii].name == cname) {
							need_load = 1;
							break;
						}
						// Find non-default config
						else if (!iac[i].CompareNoCase(cname)) {
							need_load = 1;
							break;
						}
					}
				}
			}
			if (!need_load) continue;
			//WriteLog(1, "Loading instrument " + icf[ii].group + "/" + cname);
			if (icf[ii].name != cname) {
				// If not default, create copy of current config
				AddIcf();
				ii2 = icf.size() - 1;
				icf[ii2] = icf[ii];
			}
			icf[ii2].name = cname;
			icf[ii2].fname = fname;
			icf[ii].configs_count++;
			LoadInstrument(ii2, "instruments\\" + icf[ii2].group + "\\" + icf[ii2].fname);
		}
		finder.Close();
		if (!found_default) {
			WriteLog(5, "Not found file for default instrument config " + icf[ii].group + "/" + icf[ii].name);
		}
	}
	// Log
	long long time_stop = CGLib::time();
	CString est;
	est.Format("LoadInstruments loaded %d instruments (%s) in %lld ms", icf.size(), ist, time_stop - time_start);
	WriteLog(0, est);
}

void CConf::LoadInstrument(int i, CString fname)
{
	CString st, st2, st3, iname;
	long long time_start = CGLib::time();
	ifstream fs;
	if (!fileExists(fname)) {
		WriteLog(5, "Cannot find instrument config file " + fname);
		return;
	}
	fs.open(fname);
	char pch[2550];
	int pos = 0;
	int x = 0;
	while (fs.good()) {
		x++;
		fs.getline(pch, 2550);
		st = pch;
		st.Replace("\"", "");
		// Remove comments
		pos = st.Find("#");
		// Check if it is first symbol
		if (pos == 0)	st = st.Left(pos);
		pos = st.Find(" #");
		// Check if it is after space
		if (pos > -1)	st = st.Left(pos);
		st.Trim();
		// Load include
		if (CheckInclude(st, fname, iname)) {
			LoadInstrument(i, iname);
			st.Empty();
		}
		// Find equals
		pos = st.Find("=");
		if (pos != -1) {
			st2 = st.Left(pos);
			st3 = st.Mid(pos + 1);
			st2.Trim();
			st3.Trim();
			st2.MakeLower();
			// Initialize loading
			LoadInstrumentLine(st2, st3, i);
			if (!parameter_found) {
				WriteLog(5, "Unrecognized parameter '" + st2 + "' = '" + st3 + "' in file " + fname);
			}
			//CConf::LoadVar(&st2, &st3, "save_format_version", &save_format_version);
		}
		else {
			if (!st.IsEmpty()) {
				WriteLog(5, "No equal sign in line, which is not a comment: '" + st + "' in file " + fname);
			}
		}
	} // while (fs.good())
	fs.close();
	icf[i].loaded = 1;
	// Log
	long long time_stop = CGLib::time();
	CString est;
	est.Format("LoadInstrument (%s) loaded %d lines from " + fname + " in %lld ms", fname, x, time_stop - time_start);
	//WriteLog(0, est);
}

void CConf::LoadCCName(CString *sName, CString *sValue, CString sSearch, int i) {
	if (*sName != sSearch) return;
	++parameter_found;
	CString st = *sValue;
	st.Trim();
	int pos = st.Find(":");
	if (pos < 1) {
		WriteLog(5, "Cannot find colon in config line " + *sName + " = " + *sValue);
		error = 100;
		return;
	}
	CString st1 = st.Left(pos);
	CString st2 = st.Mid(pos + 1);
	st1.Trim();
	st2.Trim();
	//WriteLog(1, "Detected '" + st1 + "' -> '" + st2 + "'");
	char cc_id = atoi(st1);
	if (!cc_id && st1 != "0") {
		WriteLog(5, "Wrong format for CC id in config line: " + *sName + " = " + *sValue);
		error = 101;
	}
	if (icf[i].NameToCC.find(st2) != icf[i].NameToCC.end() && icf[i].NameToCC[st2] != cc_id) {
		WriteLog(5, "Duplicate CC name for different CC id in config line: " + *sName + " = " + *sValue);
		error = 102;
	}
	if (icf[i].NameToKsw.find(st2) != icf[i].NameToKsw.end()) {
		WriteLog(5, "You cannot use CC name same as KSW name in config line: " + *sName + " = " + *sValue);
		error = 103;
	}
	icf[i].CCToName[cc_id] = st2;
	icf[i].NameToCC[st2] = cc_id;
}

void CConf::LoadKswGroup(CString *sName, CString *sValue, CString sSearch, int i) {
	if (*sName != sSearch) return;
	++parameter_found;
	CString st3 = *sValue;
	st3.Trim();
	vector <CString> sa;
	Tokenize(st3, sa, ",");
	for (int x = 0; x < sa.size(); ++x) {
		CString st = sa[x];
		st.Trim();
		int pos = st.Find(":");
		if (pos < 1) {
			WriteLog(5, "Cannot find colon in config line " + *sName + " = " + *sValue);
			error = 100;
			return;
		}
		CString st1 = st.Left(pos);
		CString st2 = st.Mid(pos + 1);
		st1.Trim();
		st2.Trim();
		//WriteLog(1, "Detected '" + st1 + "' -> '" + st2 + "'");
		char cnote = GetNoteI(st1);
		if (icf[i].NameToKsw.find(st2) != icf[i].NameToKsw.end() && icf[i].NameToKsw[st2] != cnote) {
			WriteLog(5, "Duplicate KSW name for different notes in config line: " + *sName + " = " + *sValue);
			error = 102;
		}
		if (icf[i].NameToCC.find(st2) != icf[i].NameToCC.end()) {
			WriteLog(5, "You cannot use KSW name same as CC name in config line: " + *sName + " = " + *sValue);
			error = 103;
		}
		icf[i].KswToName[cnote] = st2;
		icf[i].NameToKsw[st2] = cnote;
		icf[i].KswGroup[cnote] = icf[i].ksw_group_count;
	}
	++icf[i].ksw_group_count;
}

PmMessage CConf::ParseMidiCommand(CString st, int i) {
	st.Trim();
	// Remove value for ksw velocity or CC value
	int value = -1;
	// Get value if specified
	int pos = st.Find(":");
	if (pos > 0) {
		CString st1 = st.Left(pos);
		CString st2 = st.Mid(pos + 1);
		st1.Trim();
		st2.Trim();
		st = st1;
		value = atoi(st2);
	}
	return ParseMidiCommand2(st, value, i);
}

PmMessage CConf::ParseMidiCommand2(CString st, int value, int i) {
	if (icf[i].NameToCC.find(st) != icf[i].NameToCC.end()) {
		// Default value if not specified
		if (value == -1) value = 100;
		int id = icf[i].NameToCC[st];
		//WriteLog(1, "Accepted InitCommand for CC: " + *sName + " = " + *sValue);
		return Pm_Message(MIDI_CC, id, value);
	}
	if (icf[i].NameToKsw.find(st) != icf[i].NameToKsw.end()) {
		// Default value if not specified
		if (value == -1) value = 101;
		int id = icf[i].NameToKsw[st];
		//WriteLog(1, "Accepted InitCommand for KSW: " + *sName + " = " + *sValue);
		return Pm_Message(MIDI_NOTEON, id, value);
	}
	return 0;
}

void CConf::SaveInitCommand(PmMessage msg, int i) {
	// Clear whole group
	if (Pm_MessageStatus(msg) == MIDI_NOTEON) {
		int id = Pm_MessageData1(msg);
		int gr = icf[i].KswGroup[id];
		auto it = icf[i].InitCommands.begin();
		while (it != icf[i].InitCommands.end()) {
			if (Pm_MessageStatus(*it) == MIDI_NOTEON && icf[i].KswGroup[Pm_MessageData1(*it)] == gr) {
				it = icf[i].InitCommands.erase(it);
			}
			else ++it;
		}
	}
	if (Pm_MessageStatus(msg) == MIDI_CC) {
		int id = Pm_MessageData1(msg);
		auto it = icf[i].InitCommands.begin();
		while (it != icf[i].InitCommands.end()) {
			if (Pm_MessageStatus(*it) == MIDI_CC && Pm_MessageData1(*it) == id) {
				it = icf[i].InitCommands.erase(it);
			}
			else ++it;
		}
	}
	icf[i].InitCommands.push_back(msg);
}

void CConf::LoadInitCommand(CString *sName, CString *sValue, CString sSearch, int i) {
	if (*sName != sSearch) return;
	++parameter_found;
	CString st = *sValue;
	PmMessage msg = ParseMidiCommand(st, i);
	if (!msg) {
		WriteLog(5, "Unknown name. Please first bind CC name or KSW name in instrument config: " + *sName + " = " + *sValue);
		return;
	}
	SaveInitCommand(msg, i);
}

void CConf::LoadTechnique(CString *sName, CString *sValue, CString sSearch, int i) {
	if (*sName != sSearch) return;
	++parameter_found;
	CString st = *sValue;
	st.Trim();
	int pos = st.Find(";");
	if (pos < 1) {
		WriteLog(5, "Cannot find semicolon in config line " + *sName + " = " + *sValue);
		error = 100;
		return;
	}
	CString st1 = st.Left(pos);
	CString st2 = st.Mid(pos + 1);
	st1.Trim();
	st2.Trim();
	vector<CString> sa;
	Tokenize(st2, sa, "+");
	int id;
	// Find tech
	if (icf[i].NameToTech.find(st1) != icf[i].NameToTech.end()) {
		id = icf[i].NameToTech[st1];
	}
	else {
		id = icf[i].tech.size();
		icf[i].tech.resize(id + 1);
		// Bind
		icf[i].TechToName[id] = st1;
		icf[i].NameToTech[st1] = id;
	}
	icf[i].tech[id].clear();
	for (int x = 0; x < sa.size(); ++x) {
		CString st = sa[x];
		st.Trim();
		PmMessage msg = ParseMidiCommand(st, i);
		// Push value
		icf[i].tech[id].push_back(msg);
	}
}

void CConf::LoadInitTechnique(CString *sName, CString *sValue, CString sSearch, int i) {
	if (*sName != sSearch) return;
	++parameter_found;
	CString st = *sValue;
	if (icf[i].NameToTech.find(st) == icf[i].NameToTech.end()) {
		WriteLog(5, "Unknown name. Please first initialize technique name in instrument config: " + *sName + " = " + *sValue);
		return;
	}
	int id = icf[i].NameToTech[st];
	for (auto const& it : icf[i].tech[id]) {
		SaveInitCommand(it, i);
	}
}

void CConf::LoadMapPitch(CString *sName, CString *sValue, CString sSearch, int i) {
	if (*sName != sSearch) return;
	++parameter_found;
	CString st = *sValue;
	vector<CString> sa;
	Tokenize(st, sa, ",");
	if (sa.size() != 2 && sa.size() != 3) {
		WriteLog(5, "Wrong format in instrument config: " + *sName + " = " + *sValue);
		return;
	}
	sa[0].Trim();
	sa[1].Trim();
	int n1 = GetNoteI(sa[0]);
	int n2 = GetNoteI(sa[1]);
	if (sa.size() == 3) {
		sa[2].Trim();
		int n3 = GetNoteI(sa[2]);
		icf[i].map_tremolo[n1] = n3;
	}
	icf[i].map_pitch[n1] = n2;
}

void CConf::LoadInstrumentLine(CString st2, CString st3, int i) {
	parameter_found = 0;
	LoadVar(&st2, &st3, "library", &icf[i].lib);
	CheckVar(&st2, &st3, "pan", &icf[i].pan, 0, 100);
	CheckVar(&st2, &st3, "volume", &icf[i].vol, 0, 200);
	CheckVar(&st2, &st3, "volume_default", &icf[i].vol_default, 0, 127);
	CheckVar(&st2, &st3, "db_max", &icf[i].db_max);
	CheckVar(&st2, &st3, "db_coef", &icf[i].db_coef);
	CheckVar(&st2, &st3, "ks1", &icf[i].ks1);
	LoadNote(&st2, &st3, "n_min", &icf[i].nmin);
	LoadNote(&st2, &st3, "n_max", &icf[i].nmax);
	LoadNote(&st2, &st3, "import_min", &icf[i].import_min);
	LoadNote(&st2, &st3, "import_max", &icf[i].import_max);
	LoadNote(&st2, &st3, "replacepitch", &icf[i].replace_pitch);
	LoadNote(&st2, &st3, "trem_replace", &icf[i].trem_replace);
	CheckVar(&st2, &st3, "trem_transpose", &icf[i].trem_transpose, -127, 127);
	CheckVar(&st2, &st3, "trem_chan", &icf[i].trem_chan, 1, 16);
	CheckVar(&st2, &st3, "trem_lock", &icf[i].trem_lock, 0, 1);
	CheckVar(&st2, &st3, "trem_maxint", &icf[i].trem_maxint, 0, 127);
	CheckVar(&st2, &st3, "bow_lock", &icf[i].bow_lock, 0, 2);
	LoadCCName(&st2, &st3, "cc_name", i);
	LoadKswGroup(&st2, &st3, "kswgroup", i);
	LoadTechnique(&st2, &st3, "technique", i);
	//LoadInitCommand(&st2, &st3, "initcommand", i);
	LoadInitTechnique(&st2, &st3, "inittechnique", i);
	LoadMapPitch(&st2, &st3, "mappitch", i);
	CheckVar(&st2, &st3, "t_min", &icf[i].tmin);
	CheckVar(&st2, &st3, "t_max", &icf[i].tmax);
	CheckVar(&st2, &st3, "poly", &icf[i].poly);
	CheckVar(&st2, &st3, "single_stage", &icf[i].single_stage, 0, 1);
	CheckVar(&st2, &st3, "type", &icf[i].type);
	CheckVar(&st2, &st3, "channel", &icf[i].channel, 1, 16);
	CheckVar(&st2, &st3, "channels", &icf[i].channels, 1, 16);
	CheckVar(&st2, &st3, "channels_dyn", &icf[i].channels_dyn, 1, 16);
	CheckVar(&st2, &st3, "legato_long_minlen", &icf[i].legato_long_minlen);
	CheckVar(&st2, &st3, "vel_legato_long", &icf[i].vel_legato_long);
	CheckVar(&st2, &st3, "unison_mute", &icf[i].unis_mute, 0, 1);
	CheckVar(&st2, &st3, "unison_dyn_mul", &icf[i].unis_dyn_mul, 0);
	CheckRange(&st2, &st3, "dyn_range", &icf[i].dyn_range1, &icf[i].dyn_range2, 0, 100);
	CheckRange(&st2, &st3, "acc_range", &icf[i].acc_range1, &icf[i].acc_range2, 1, 127);
	CheckRange(&st2, &st3, "vib_bell_dur", &icf[i].vib_bell_mindur, &icf[i].vib_bell_dur);
	CheckRange(&st2, &st3, "vib_bell_top", &icf[i].vib_bell_top1, &icf[i].vib_bell_top2);
	CheckRange(&st2, &st3, "vibf_bell_top", &icf[i].vibf_bell_top1, &icf[i].vibf_bell_top2);
	CheckRange(&st2, &st3, "vib_bell", &icf[i].vib_bell1, &icf[i].vib_bell2);
	CheckRange(&st2, &st3, "vibf_bell", &icf[i].vibf_bell1, &icf[i].vibf_bell2);
	CheckRange(&st2, &st3, "vib_dyn", &icf[i].vib_dyn1, &icf[i].vib_dyn2, 0, 127);
	CheckRange(&st2, &st3, "vib_sbell_top", &icf[i].vib_sbell_top1, &icf[i].vib_sbell_top2);
	CheckRange(&st2, &st3, "vibf_sbell_top", &icf[i].vibf_sbell_top1, &icf[i].vibf_sbell_top2);
	CheckRange(&st2, &st3, "vib_sbell", &icf[i].vib_sbell1, &icf[i].vib_sbell2);
	CheckRange(&st2, &st3, "vibf_sbell", &icf[i].vibf_sbell1, &icf[i].vibf_sbell2);
	CheckRange(&st2, &st3, "vib_sdyn", &icf[i].vib_sdyn1, &icf[i].vib_sdyn2, 0, 127);
	CheckVar(&st2, &st3, "vib_bell_freq", &icf[i].vib_bell_freq);
	CheckVar(&st2, &st3, "vib_bell_exp", &icf[i].vib_bell_exp);
	CheckVar(&st2, &st3, "vibf_bell_exp", &icf[i].vibf_bell_exp);
	CheckVar(&st2, &st3, "vib_bell_rexp", &icf[i].vib_bell_rexp);
	CheckVar(&st2, &st3, "vibf_bell_rexp", &icf[i].vibf_bell_rexp);
	CheckVar(&st2, &st3, "vib_sbell_freq", &icf[i].vib_sbell_freq);
	CheckVar(&st2, &st3, "vib_sbell_exp", &icf[i].vib_sbell_exp);
	CheckVar(&st2, &st3, "vibf_sbell_exp", &icf[i].vibf_sbell_exp);
	CheckVar(&st2, &st3, "vib_sbell_rexp", &icf[i].vib_sbell_rexp);
	CheckVar(&st2, &st3, "vibf_sbell_rexp", &icf[i].vibf_sbell_rexp);
	CheckVar(&st2, &st3, "rnd_vel", &icf[i].rnd_vel);
	CheckVar(&st2, &st3, "rnd_vel_repeat", &icf[i].rnd_vel_repeat);
	CheckVar(&st2, &st3, "rnd_dyn", &icf[i].rnd_dyn);
	CheckVar(&st2, &st3, "rnd_dyn_slow", &icf[i].rnd_dyn_slow, 1);
	CheckVar(&st2, &st3, "rnd_vib", &icf[i].rnd_vib);
	CheckVar(&st2, &st3, "rnd_vib_slow", &icf[i].rnd_vib_slow, 1);
	CheckVar(&st2, &st3, "rnd_vibf", &icf[i].rnd_vibf);
	CheckVar(&st2, &st3, "rnd_vibf_slow", &icf[i].rnd_vibf_slow, 1);
	CheckVar(&st2, &st3, "splitpo_pent_minint", &icf[i].splitpo_pent_minint);
	CheckVar(&st2, &st3, "cc_vib", &icf[i].CC_vib);
	CheckVar(&st2, &st3, "cc_vibf", &icf[i].CC_vibf);
	CheckVar(&st2, &st3, "cc_steps", &icf[i].CC_steps);
	CheckVar(&st2, &st3, "cc_dynamics", &icf[i].CC_dyn);
	CheckVar(&st2, &st3, "cc_ma", &icf[i].CC_ma, 3);
	CheckVar(&st2, &st3, "cc_retrigger", &icf[i].CC_retrigger);
	CheckVar(&st2, &st3, "retrigger_freq", &icf[i].retrigger_freq);
	CheckVar(&st2, &st3, "max_slur_count", &icf[i].max_slur_count);
	CheckVar(&st2, &st3, "max_slur_interval", &icf[i].max_slur_interval);
	CheckVar(&st2, &st3, "nonlegato_maxgap", &icf[i].nonlegato_maxgap);
	CheckVar(&st2, &st3, "nonlegato_mingap", &icf[i].nonlegato_mingap);
	CheckVar(&st2, &st3, "retrigger_mingap", &icf[i].retrigger_mingap);
	CheckVar(&st2, &st3, "auto_legato", &icf[i].auto_legato);
	CheckVar(&st2, &st3, "reverb_mix", &icf[i].reverb_mix);
	CheckVar(&st2, &st3, "fix_transpose", &icf[i].fix_transpose);
	CheckVar(&st2, &st3, "pedal_import", &icf[i].pedal_import);
	CheckVar(&st2, &st3, "trem_maxlen", &icf[i].trem_maxlen);
	CheckRange(&st2, &st3, "trem_dyn_range", &icf[i].trem_dyn_range1, &icf[i].trem_dyn_range2, 0, 100);
	CheckVar(&st2, &st3, "trem_min_repeats", &icf[i].trem_min_repeats);
	CheckVar(&st2, &st3, "all_ahead", &icf[i].all_ahead);
	LoadVectorPar(&st2, &st3, "legato_ahead", icf[i].legato_ahead);
	LoadVectorPar(&st2, &st3, "ahead_chrom", icf[i].ahead_chrom);
	CheckVar(&st2, &st3, "leg_pdur", &icf[i].leg_pdur);
	CheckVar(&st2, &st3, "leg_cdur", &icf[i].leg_cdur);
	CheckVar(&st2, &st3, "legato_ahead_exp", &icf[i].legato_ahead_exp);
	CheckVar(&st2, &st3, "splitpo_freq", &icf[i].splitpo_freq);
	CheckVar(&st2, &st3, "splitpo_mindur", &icf[i].splitpo_mindur);
	CheckVar(&st2, &st3, "gliss_mindur", &icf[i].gliss_mindur);
	CheckVar(&st2, &st3, "nonlegato_minlen", &icf[i].nonlegato_minlen);
	CheckVar(&st2, &st3, "divisi_auto", &icf[i].divisi_auto, 0, 1);
	CheckVar(&st2, &st3, "stac_auto", &icf[i].stac_auto, 0, 1);
	CheckVar(&st2, &st3, "stac_ahead", &icf[i].stac_ahead);
	CheckVar(&st2, &st3, "stac_chan", &icf[i].stac_chan, 1, 16);
	CheckVar(&st2, &st3, "stac_maxlen", &icf[i].stac_maxlen);
	CheckRange(&st2, &st3, "stac_dyn_range", &icf[i].stac_dyn_range1, &icf[i].stac_dyn_range2, 1, 100);
	CheckVar(&st2, &st3, "pizz_import", &icf[i].pizz_import, 0, 1);
	CheckVar(&st2, &st3, "pizz_ahead", &icf[i].pizz_ahead);
	CheckVar(&st2, &st3, "pizz_chan", &icf[i].pizz_chan, 1, 16);
	CheckVar(&st2, &st3, "mute_lock", &icf[i].mute_lock, 0, 1);
	CheckVar(&st2, &st3, "mute_import", &icf[i].mute_import, 0, 1);
	CheckVar(&st2, &st3, "trem_import", &icf[i].trem_import, 0, 1);
	CheckVar(&st2, &st3, "trem_len", &icf[i].trem_len, 0);
	CheckVar(&st2, &st3, "trem_end", &icf[i].trem_end, 0, 2);
	CheckVar(&st2, &st3, "spic_import", &icf[i].spic_import, 0, 1);
	CheckVar(&st2, &st3, "stac_import", &icf[i].stac_import, 0, 1);
	CheckVar(&st2, &st3, "marc_import", &icf[i].marc_import, 0, 1);
	CheckVar(&st2, &st3, "tasto_import", &icf[i].tasto_import, 0, 1);
	CheckRange(&st2, &st3, "pizz_dyn_range", &icf[i].pizz_dyn_range1, &icf[i].pizz_dyn_range2, 1, 100);
	CheckVar(&st2, &st3, "mute_predelay", &icf[i].mute_predelay);
	CheckVar(&st2, &st3, "nonlegato_freq", &icf[i].nonlegato_freq);
	CheckVar(&st2, &st3, "lengroup2", &icf[i].lengroup2);
	CheckVar(&st2, &st3, "lengroup3", &icf[i].lengroup3);
	CheckVar(&st2, &st3, "lengroup4", &icf[i].lengroup4);
	CheckVar(&st2, &st3, "lengroup_edt1", &icf[i].lengroup_edt1);
	CheckVar(&st2, &st3, "lengroup_edt2", &icf[i].lengroup_edt2);
	CheckRange(&st2, &st3, "rand_pos", &icf[i].rand_start, &icf[i].rand_end);
	CheckRange(&st2, &st3, "rand_pos_max", &icf[i].rand_start_max, &icf[i].rand_end_max);
	CheckVar(&st2, &st3, "rand_start", &icf[i].rand_start);
	CheckVar(&st2, &st3, "rand_end", &icf[i].rand_end);
	CheckVar(&st2, &st3, "rand_start_max", &icf[i].rand_start_max);
	CheckVar(&st2, &st3, "rand_end_max", &icf[i].rand_end_max);
	CheckVar(&st2, &st3, "lengroup_edt2", &icf[i].lengroup_edt2);
	CheckVar(&st2, &st3, "lengroup_edt2", &icf[i].lengroup_edt2);
	CheckVar(&st2, &st3, "retrigger_min_len", &icf[i].retrigger_min_len);
	CheckVar(&st2, &st3, "retrigger_rand_end", &icf[i].retrigger_rand_end);
	CheckVar(&st2, &st3, "retrigger_rand_max", &icf[i].retrigger_rand_max);
	CheckVar(&st2, &st3, "harsh_acc_freq", &icf[i].harsh_acc_freq);
	CheckVar(&st2, &st3, "harsh_acc_vel", &icf[i].harsh_acc_vel);
	CheckVar(&st2, &st3, "slow_acc_vel", &icf[i].slow_acc_vel);
	CheckVar(&st2, &st3, "slow_acc_minlen", &icf[i].slow_acc_minlen);
	CheckVar(&st2, &st3, "gliss_leg_vel", &icf[i].gliss_leg_vel);
	CheckVar(&st2, &st3, "gliss_minlen", &icf[i].gliss_minlen);
	CheckVar(&st2, &st3, "gliss_freq", &icf[i].gliss_freq);
	CheckRange(&st2, &st3, "bell_mindur", &icf[i].bell_mindur, &icf[i].bell_mindur2);
	CheckVar(&st2, &st3, "max_ahead_note", &icf[i].max_ahead_note);
	CheckRange(&st2, &st3, "bell_mul", &icf[i].bell_start_mul, &icf[i].bell_end_mul);
	CheckRange(&st2, &st3, "bell_len", &icf[i].bell_start_len, &icf[i].bell_end_len);
	CheckRange(&st2, &st3, "bell_vel", &icf[i].bell_start_vel, &icf[i].bell_end_vel);
	CheckVar(&st2, &st3, "rbell_freq", &icf[i].rbell_freq);
	CheckRange(&st2, &st3, "rbell_pos", &icf[i].rbell_pos1, &icf[i].rbell_pos2);
	CheckRange(&st2, &st3, "rbell_dur", &icf[i].rbell_mindur, &icf[i].rbell_dur);
	CheckRange(&st2, &st3, "rbell_mul", &icf[i].rbell_mul, &icf[i].rbell_mul2);
	CheckVar(&st2, &st3, "end_sfl_dur", &icf[i].end_sfl_dur);
	CheckVar(&st2, &st3, "end_sfl_freq", &icf[i].end_sfl_freq);
	CheckVar(&st2, &st3, "end_pbd_dur", &icf[i].end_pbd_dur);
	CheckVar(&st2, &st3, "end_pbd_freq", &icf[i].end_pbd_freq);
	CheckVar(&st2, &st3, "end_vib2_dur", &icf[i].end_vib2_dur);
	CheckVar(&st2, &st3, "end_vib2_freq", &icf[i].end_vib2_freq);
	CheckVar(&st2, &st3, "end_vib_dur", &icf[i].end_vib_dur);
	CheckVar(&st2, &st3, "end_vib_freq", &icf[i].end_vib_freq);
	if (st2 == "mute_activate") {
		++parameter_found;
		if (icf[i].NameToTech.find(st3) != icf[i].NameToTech.end()) {
			icf[i].mute_activate = icf[i].NameToTech[st3];
		}
		else WriteLog(5, "Unknown technique specified for mute_selected: " + st3);
	}
	if (st2 == "mute_deactivate") {
		++parameter_found;
		if (icf[i].NameToTech.find(st3) != icf[i].NameToTech.end()) {
			icf[i].mute_deactivate = icf[i].NameToTech[st3];
		}
		else WriteLog(5, "Unknown technique specified for mute_selected: " + st3);
	}
	if (st2 == "trem_activate") {
		++parameter_found;
		if (icf[i].NameToTech.find(st3) != icf[i].NameToTech.end()) {
			icf[i].trem_activate = icf[i].NameToTech[st3];
		}
		else WriteLog(5, "Unknown technique specified for trem_selected: " + st3);
	}
	if (st2 == "trem_deactivate") {
		++parameter_found;
		if (icf[i].NameToTech.find(st3) != icf[i].NameToTech.end()) {
			icf[i].trem_deactivate = icf[i].NameToTech[st3];
		}
		else WriteLog(5, "Unknown technique specified for trem_selected: " + st3);
	}
	// Parse command
	PmMessage msg = ParseMidiCommand2(st2, atoi(st3), i);
	if (!msg && !parameter_found) {
		//WriteLog(5, "Unknown name. Please first bind CC name or KSW name in instrument config: " + st2 + " = " + st3);
		return;
	}
	if (msg) {
		if (parameter_found) {
			WriteLog(5, "Command name conflicts with config parameter name: " + st2 + " = " + st3);
			return;
		}
		++parameter_found;
		SaveInitCommand(msg, i);
	}
}

void CConf::ReplaceCsvDb(CString path1, CString path2, CString filter, CString value) {
	CCsvDb cdb, cdb2;
	CString est;
	// Load corrected table
	est = cdb.Open(path1);
	if (est != "") {
		WriteLog(5, est);
		return;
	}
	est = cdb.Select();
	if (est != "") {
		WriteLog(5, est);
		return;
	}
	// Copy to main table
	est = cdb2.Open(path2);
	if (est != "") {
		WriteLog(5, est);
		return;
	}
	cdb2.filter[filter] = value;
	est = cdb2.Delete();
	if (est != "") {
		WriteLog(5, est);
		return;
	}
	est = cdb2.InsertMultiple(cdb.result);
	if (est != "") {
		WriteLog(5, est);
		return;
	}
}