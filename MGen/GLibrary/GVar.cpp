// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "GVar.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

CGVar::CGVar() {
	color_noflag = MakeColor(0, 100, 100, 100);
	// Init constant length arrays
	ngv_min.resize(MAX_VOICE);
	ngv_max.resize(MAX_VOICE);
	instr.resize(MAX_VOICE);
	v_stage.resize(MAX_VOICE);
	v_itrack.resize(MAX_VOICE);
	t_instr.resize(MAX_VOICE);

	show_transpose.resize(MAX_VOICE);
	track_name.resize(MAX_VOICE);
	track_id.resize(MAX_VOICE);
	track_vid.resize(MAX_VOICE);
	itrack.resize(MAX_VOICE);
	// Set instrument
	instr[0] = 5;
	instr[1] = 6;
	nfreq.resize(256);
}

CGVar::~CGVar()
{
}

void CGVar::InitVectors()
{
	ResizeVectors(t_allocated);
	// Init ngv
	for (int v = 0; v < MAX_VOICE; v++) {
		ngv_min[v] = 1000;
		ngv_max[v] = 0;
	}
}

// If info2 is empty, it is not overwritten
void CGVar::AddMelody(int step1, int step2, int v, CString info, CString info2, CString info3) {
	// Find existing mel_info
	int found = 1;
	if (mel_id[step1][v] > -1) {
		for (int i = step1+1; i <= step2; ++i) {
			if (mel_id[i][v] != mel_id[i - 1][v]) {
				found = 0;
				break;
			}
		}
		if (found) {
			// Update existing mel_info
			mel_info[mel_id[step1][v]] = info;
			if (!info2.IsEmpty()) mel_info2[mel_id[step1][v]] = info2;
			if (!info3.IsEmpty()) mel_info3[mel_id[step1][v]] = info3;
			return;
		}
	}
	for (int i = step1; i <= step2; ++i) {
		mel_id[i][v] = mel_info.size();
	}
	mel_info.push_back(info);
	mel_info2.push_back(info2);
	mel_info3.push_back(info3);
}

void CGVar::ResizeVectors(int size, int vsize)
{
	long long time_start = CGLib::time();
	if (!mutex_output.try_lock_for(chrono::milliseconds(5000))) {
		WriteLog(5, "Critical error: ResizeVectors mutex timed out");
	}
	if (vsize == -1) vsize = v_cnt;
	pause.resize(size);
	note.resize(size);
	color.resize(size);
	note_muted.resize(size);
	midifile_out_mul.resize(size, 1);
	len.resize(size);
	coff.resize(size);
	poff.resize(size);
	noff.resize(size);
	tempo.resize(size);
	tempo_src.resize(size);
	stime.resize(size);
	etime.resize(size);
	smst.resize(size);
	smet.resize(size);
	dstime.resize(size);
	detime.resize(size);
 	sstime.resize(size);
	setime.resize(size);
	dyn.resize(size);
	vel.resize(size);
	vib.resize(size);
	vibf.resize(size);
	artic.resize(size);
	filter.resize(size);
	lengroup.resize(size);
	adapt_comment.resize(size);
	midi_ch.resize(size);

	lyrics.resize(size);

	int start = t_allocated;
	// Start from zero if we are allocating first time
	if (size == t_allocated) start = 0;
	if (vsize != v_cnt) start = 0;
	for (int i = start; i < size; i++) {
		pause[i].resize(vsize);
		note[i].resize(vsize);
		color[i].resize(vsize);
		note_muted[i].resize(vsize);
		len[i].resize(vsize);
		coff[i].resize(vsize);
		poff[i].resize(vsize);
		noff[i].resize(vsize);
		dyn[i].resize(vsize);
		vel[i].resize(vsize);
		vib[i].resize(vsize);
		vibf[i].resize(vsize);
		artic[i].resize(vsize);
		filter[i].resize(vsize);
		lengroup[i].resize(vsize);
		lyrics[i].resize(vsize);
		adapt_comment[i].resize(vsize);
		midi_ch[i].resize(vsize);
		dstime[i].resize(vsize);
		detime[i].resize(vsize);
		sstime[i].resize(vsize);
		setime[i].resize(vsize);
		smst[i].resize(vsize, -1);
		smet[i].resize(vsize, -1);
	}

	if (m_algo_id != 2001) {
		ngraph.resize(size, vector<vector<float> >(vsize, vector<float>(ngraph_size)));
		graph.resize(size, vector<vector<float> >(vsize, vector<float>(graph_size)));
		tonic.resize(size);
		minor.resize(size);
		lining.resize(size);
		linecolor.resize(size);
		mel_id.resize(size);
		mark.resize(size);
		mark_color.resize(size);
		comment.resize(size);
		ccolor.resize(size);
		comment2.resize(size);
		nsr1.resize(size);
		nsr2.resize(size);
		nlink.resize(size);
		fsev.resize(size);
		for (int i = start; i < size; i++) {
			ngraph[i].resize(vsize);
			graph[i].resize(vsize);
			tonic[i].resize(vsize);
			minor[i].resize(vsize);
			lining[i].resize(vsize);
			mel_id[i].resize(vsize, -1);
			mark[i].resize(vsize);
			mark_color[i].resize(vsize);
			comment[i].resize(vsize);
			ccolor[i].resize(vsize);
			comment2[i].resize(vsize);
			nsr1[i].resize(vsize);
			nsr2[i].resize(vsize);
			nlink[i].resize(vsize);
			fsev[i].resize(vsize);
		}
	}

	// Count time
	if (debug_level > 1) {
		long long time_stop = CGLib::time();
		CString st;
		st.Format("ResizeVectors from %d to %d steps, from %d to %d voices (in %lld ms)", 
			t_allocated, size, v_cnt, vsize, time_stop - time_start);
		WriteLog(0, st);
	}

	t_allocated = size;
	v_cnt = vsize;
	mutex_output.unlock();
}

void CGVar::SaveVector2C(ofstream & fs, vector< vector<unsigned char> > &v2D, int i) {
	const char* pointer = reinterpret_cast<const char*>(&v2D[i][0]);
	size_t bytes = v_cnt * sizeof(v2D[i][0]);
	fs.write(pointer, bytes);
}

void CGVar::SaveVector2S(ofstream & fs, vector< vector<unsigned short> > &v2D, int i) {
	const char* pointer = reinterpret_cast<const char*>(&v2D[i][0]);
	size_t bytes = v_cnt * sizeof(v2D[i][0]);
	fs.write(pointer, bytes);
}

void CGVar::SaveVector2Color(ofstream & fs, vector< vector<DWORD> > &v2D, int i) {
	size_t bytes = 4;
	for (int v = 0; v < v_cnt; v++) {
		fs.write((char*)&(v2D[i][v]), bytes);
	}
}

void CGVar::SaveVector2ST(ofstream & fs, vector< vector<CString> > &v2D, int i) {
	for (int v = 0; v < v_cnt; v++) {
		unsigned short len = v2D[i][v].GetLength();
		fs.write((char*)&len, sizeof(len));
		if (len != 0) fs.write(v2D[i][v].GetBuffer(), len);
	}
}

void CGVar::SaveVector(ofstream &fs, vector<float> &v) {
	const char* pointer = reinterpret_cast<const char*>(&v[0]);
	size_t bytes = t_generated * sizeof(v[0]);
	fs.write(pointer, bytes);
}

void CGVar::SaveVector(ofstream &fs, vector<DWORD> &v) {
	const char* pointer = reinterpret_cast<const char*>(&v[0]);
	size_t bytes = t_generated * sizeof(v[0]);
	fs.write(pointer, bytes);
}

void CGVar::SaveResults(CString dir, CString fname)
{
	long long time_start = CGLib::time();
	CreateDirectory(dir, NULL);
	ofstream fs;
	fs.open(dir + "\\" + fname + ".mgr", std::ofstream::binary);
	if (t_generated > 0) {
		for (size_t i = 0; i < t_generated; i++) {
			SaveVector2C(fs, pause, i);
			SaveVector2C(fs, note, i);
			SaveVector2S(fs, len, i);
			SaveVector2S(fs, coff, i);
			SaveVector2C(fs, dyn, i);
			//SaveVector2ST(fs, comment, i);
			SaveVector2Color(fs, color, i);
		}
		SaveVector(fs, tempo);
		// Added in version 1.6
		if (lining.size()) for (size_t i = 0; i < t_generated; i++) {
			SaveVector2C(fs, lining, i);
		}
		if (linecolor.size()) SaveVector(fs, linecolor);
		// Added in version 1.6.1
		if (tonic.size()) for (size_t i = 0; i < t_generated; i++) {
			SaveVector2C(fs, tonic, i);
		}
		// Added in MGR version 1.9
		if (minor.size()) for (size_t i = 0; i < t_generated; i++) {
			SaveVector2C(fs, minor, i);
		}
	}
	fs.close();
	// Save strings
	CString st;
	fs.open(dir + "\\" + fname + ".txt");
	fs << "save_format_version = " << MGR_VERSION << " # This is version of format used to save these files\n";
	fs << "m_config = " + m_config + " # Name of config file used for generation\n";
	st.Format("m_algo_id = %d\n", m_algo_id);
	fs << st;
	st.Format("t_cnt = %d # Number of steps I had to generate\n", t_cnt);
	fs << st;
	st.Format("v_cnt = %d # Number of voices\n", v_cnt);
	fs << st;
	st.Format("t_generated = %d # Number of steps I generated\n", t_generated);
	fs << st;
	st.Format("t_sent = %d # Number of steps I finished and sent (can be less than generated)\n", t_sent);
	fs << st;
	st.Format("t_send = %d # Send window\n", t_send);
	fs << st;
	st.Format("ng_min = %d # Minimum generated note\n", ng_min);
	fs << st;
	st.Format("ng_max = %d # Maximum generated note\n", ng_max);
	fs << st;
	st.Format("tg_min = %f # Minimum generated tempo\n", tg_min);
	fs << st;
	st.Format("tg_max = %f # Maximum generated tempo\n", tg_max);
	fs << st;
	st.Format("gen_start_time = %lld # Unix timestamp of generation start\n", gen_start_time);
	fs << st;
	st.Format("time_stopped = %lld # Unix timestamp of generation finish\n", time_stopped);
	fs << st;
	st.Format("duration = %.0f # Length of total playback in ms\n", etime[max(0, t_generated - 1)]);
	fs << st;
	st.Format("need_exit = %d\n", need_exit);
	fs << st;
	fs.close();
	// Save logs
	if (mutex_log.try_lock_for(chrono::milliseconds(500))) {
		fs.open(dir + "\\log-debug.log");
		for (int i = 0; i < logs[0].size(); i++) fs << logs[0][i] << "\n";
		for (int i = 0; i < log_buffer[0].size(); i++) fs << log_buffer[0][i] << "\n";
		fs.close();
		fs.open(dir + "\\log-warning.log");
		for (int i = 0; i < logs[1].size(); i++) fs << logs[1][i] << "\n";
		for (int i = 0; i < log_buffer[1].size(); i++) fs << log_buffer[1][i] << "\n";
		fs.close();
		fs.open(dir + "\\log-perf.log");
		for (int i = 0; i < logs[2].size(); i++) fs << logs[2][i] << "\n";
		for (int i = 0; i < log_buffer[2].size(); i++) fs << log_buffer[2][i] << "\n";
		fs.close();
		fs.open(dir + "\\log-algorithm.log");
		for (int i = 0; i < logs[3].size(); i++) fs << logs[3][i] << "\n";
		for (int i = 0; i < log_buffer[3].size(); i++) fs << log_buffer[3][i] << "\n";
		fs.close();
		fs.open(dir + "\\log-midi.log");
		for (int i = 0; i < logs[4].size(); i++) fs << logs[4][i] << "\n";
		for (int i = 0; i < log_buffer[4].size(); i++) fs << log_buffer[4][i] << "\n";
		fs.close();
		fs.open(dir + "\\log-confirm.log");
		for (int i = 0; i < logs[6].size(); i++) fs << logs[6][i] << "\n";
		for (int i = 0; i < log_buffer[6].size(); i++) fs << log_buffer[6][i] << "\n";
		fs.close();
		fs.open(dir + "\\log-sasemu.log");
		for (int i = 0; i < logs[7].size(); i++) fs << logs[7][i] << "\n";
		for (int i = 0; i < log_buffer[7].size(); i++) fs << log_buffer[7][i] << "\n";
		fs.close();
		fs.open(dir + "\\log-corack.log");
		for (int i = 0; i < logs[8].size(); i++) fs << logs[8][i] << "\n";
		for (int i = 0; i < log_buffer[8].size(); i++) fs << log_buffer[8][i] << "\n";
		fs.close();
		mutex_log.unlock();
	}
	// Count time
	long long time_stop = CGLib::time();
	CString est;
	est.Format("Saved results to files in %lld ms", time_stop - time_start);
	WriteLog(0, est);
}

void CGVar::ExportVectorsCSV(CString dir, CString fname)
{
	long long time_start = CGLib::time();
	CString st;
	CreateDirectory(dir, NULL);
	ofstream fs;
	fs.open(dir + "\\" + fname + ".csv");
	fs << "Step;Tempo;Stime;Etime;";
	for (int v = 0; v < v_cnt; ++v) {
		st.Format("%d", v);
		fs << "Pause" + st + ";Note" + st + ";Len" + st + ";Dyn" + st +
			";Coff" + st + ";Poff" + st + ";Noff" + st + ";STick" + st + ";ETick" + st + ";Comment" + st + ";AComment" + st + ";Color" + st
			+ ";Lining" + st + ";Tonic" + st + ";Mode" + st + ";Dstime" + st + ";Detime" + st + ";Lengroup" + st + ";Articulation" + st +
			";Vib" + st + ";Vibf" + st + ";";
	}
	fs << "\n";
	if (t_generated > 0) {
		for (int i = 0; i < t_generated; ++i) {
			fs << i << ";";
			fs << tempo[i] << ";";
			fs << stime[i] << ";";
			fs << etime[i] << ";";
			for (int v = 0; v < v_cnt; ++v) {
				fs << (int)pause[i][v] << ";";
				fs << (int)note[i][v] << ";";
				fs << (int)len[i][v] << ";";
				fs << (int)dyn[i][v] << ";";
				fs << (int)coff[i][v] << ";";
				fs << (int)poff[i][v] << ";";
				fs << (int)noff[i][v] << ";";
				fs << (int)smst[i][v] << ";";
				fs << (int)smet[i][v] << ";";
				st.Empty();
				for (int x = 0; x < comment[i][v].size(); ++x) st += comment[i][v][x];
				if (st.Left(1) == "\n") st = st.Right(st.GetLength() - 1);
				fs << "\"" << st << "\";";
				fs << adapt_comment[i][v] << ";";
				fs << color[i][v] << ";";
				fs << (int)lining[i][v] << ";";
				fs << (int)tonic[i][v] << ";";
				fs << (int)minor[i][v] << ";";
				fs << dstime[i][v] << ";";
				fs << detime[i][v] << ";";
				fs << (int)lengroup[i][v] << ";";
				fs << ArticName[artic[i][v]] << ";";
				fs << (int)vib[i][v] << ";";
				fs << (int)vibf[i][v] << ";";
			}
			fs << "\n";
		}
	}
	fs.close();
	// Count time
	long long time_stop = CGLib::time();
	CString est;
	est.Format("Saved CSV vectors to file %s\\%s.csv in %lld ms", dir, fname, time_stop - time_start);
	WriteLog(0, est);
}

void CGVar::LoadVector2C(ifstream& fs, vector< vector<unsigned char> > &v2D, int i) {
	v2D[i].resize(v_cnt);
	char* pointer = reinterpret_cast<char*>(&(v2D[i][0]));
	size_t bytes = v_cnt * sizeof(v2D[i][0]);
	fs.read(pointer, bytes);
	int read_count = fs.gcount();
	if (read_count != bytes && warning_loadvectors < MAX_WARN_LOADVECTORS) {
		CString est;
		est.Format("LoadVector2C: Error reading %d bytes from binary file (got %d bytes instead) at step %d", bytes, read_count, i);
		WriteLog(5, est);
		warning_loadvectors++;
	}
}

void CGVar::LoadVector2S(ifstream& fs, vector< vector<unsigned short> > &v2D, int i) {
	v2D[i].resize(v_cnt);
	char* pointer = reinterpret_cast<char*>(&(v2D[i][0]));
	size_t bytes = v_cnt * sizeof(v2D[i][0]);
	fs.read(pointer, bytes);
	int read_count = fs.gcount();
	if (read_count != bytes && warning_loadvectors < MAX_WARN_LOADVECTORS) {
		CString est;
		est.Format("LoadVector2S: Error reading %d bytes from binary file (got %d bytes instead) at step %d", bytes, read_count, i);
		WriteLog(5, est);
		warning_loadvectors++;
	}
}

void CGVar::LoadVector2Color(ifstream & fs, vector< vector<DWORD> > &v2D, int i) {
	size_t bytes = 4;
	for (int v = 0; v < v_cnt; v++) {
		fs.read((char*)&(v2D[i][v]), bytes);
		int read_count = fs.gcount();
		if (read_count != bytes && warning_loadvectors < MAX_WARN_LOADVECTORS) {
			CString est;
			est.Format("LoadVector2Color: Error reading %d bytes from binary file (got %d bytes instead) at step %d", bytes, read_count, i);
			WriteLog(5, est);
			warning_loadvectors++;
		}
	}
}

void CGVar::LoadVector2ST(ifstream & fs, vector< vector<CString> > &v2D, int i) {
	for (int v = 0; v < v_cnt; v++) {
		WORD len = 0;
		int bytes = sizeof(len);
		char buf[5000];
		fs.read((char*)&len, bytes);
		int read_count = fs.gcount();
		if (read_count != bytes && warning_loadvectors < MAX_WARN_LOADVECTORS) {
			CString est;
			est.Format("LoadVector2ST len: Error reading %d bytes from binary file (got %d bytes instead) at step %d: %d", bytes, read_count, i, len);
			WriteLog(5, est);
			warning_loadvectors++;
		}
		if (len != 0) {
			bytes = len;
			fs.read((char*)&buf, bytes);
			v2D[i][v] = CString(buf, bytes);
			string st = v2D[i][v];
			int read_count = fs.gcount();
			if (read_count != bytes && warning_loadvectors < MAX_WARN_LOADVECTORS) {
				CString est;
				est.Format("LoadVector2ST: Error reading %d bytes from binary file (got %d bytes instead) at step %d: %s", bytes, read_count, i, v2D[i][v]);
				WriteLog(5, est);
				warning_loadvectors++;
			}
			if (st.find_first_not_of(" abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890`-=[]\\';/.,~!@#$%^&*()_+|{}:<>?1234567890") != string::npos && warning_loadvectors < MAX_WARN_LOADVECTORS) {
				CString est;
				est.Format("LoadVector2ST: String contains unprintable characters at step %d: %s", i, v2D[i][v]);
				WriteLog(5, est);
				warning_loadvectors++;
			}
		}
	}
}

void CGVar::LoadVector(ifstream &fs, vector<float> &v) {
	v.clear();
	v.resize(t_generated);
	char* pointer = reinterpret_cast<char*>(&v[0]);
	size_t bytes = t_generated * sizeof(v[0]);
	fs.read(pointer, bytes);
	int read_count = fs.gcount();
	if (read_count != bytes && warning_loadvectors < MAX_WARN_LOADVECTORS) {
		CString est;
		est.Format("LoadVector float: Error reading %d bytes from binary file (got %d bytes instead)", bytes, read_count);
		WriteLog(5, est);
		warning_loadvectors++;
	}
}

void CGVar::LoadVector(ifstream &fs, vector<unsigned char> &v) {
	v.clear();
	v.resize(t_generated);
	char* pointer = reinterpret_cast<char*>(&v[0]);
	size_t bytes = t_generated * sizeof(v[0]);
	fs.read(pointer, bytes);
	int read_count = fs.gcount();
	if (read_count != bytes && warning_loadvectors < MAX_WARN_LOADVECTORS) {
		CString est;
		est.Format("LoadVector uchar: Error reading %d bytes from binary file (got %d bytes instead)", bytes, read_count);
		WriteLog(5, est);
		warning_loadvectors++;
	}
}

void CGVar::LoadVector(ifstream &fs, vector<DWORD> &v) {
	v.clear();
	v.resize(t_generated);
	char* pointer = reinterpret_cast<char*>(&v[0]);
	size_t bytes = t_generated * sizeof(v[0]);
	fs.read(pointer, bytes);
	int read_count = fs.gcount();
	if (read_count != bytes && warning_loadvectors < MAX_WARN_LOADVECTORS) {
		CString est;
		est.Format("LoadVector Color: Error reading %d bytes from binary file (got %d bytes instead)", bytes, read_count);
		WriteLog(5, est);
		warning_loadvectors++;
	}
}

void CGVar::LoadResultMusic(CString dir, CString fname)
{
	long long time_start = CGLib::time();
	ifstream fs;
	CString path;
	// Load binary
	path = dir + "\\" + fname + ".mgr";
	if (!fileExists(path)) {
		CString est;
		est.Format("Cannot find file %s", path);
		WriteLog(5, est);
		return;
	}
	fs.open(path, ifstream::binary);
	fs.unsetf(ios::skipws);
	if (t_generated != 0) {
		for (size_t i = 0; i < t_generated; i++) {
			LoadVector2C(fs, pause, i);
			LoadVector2C(fs, note, i);
			LoadVector2S(fs, len, i);
			LoadVector2S(fs, coff, i);
			LoadVector2C(fs, dyn, i);
			//LoadVector2ST(fs, comment, i);
			LoadVector2Color(fs, color, i);
		}
		LoadVector(fs, tempo);
		// Added with version 1.6
		if (fs.peek() != EOF) {
			for (size_t i = 0; i < t_generated; i++) {
				LoadVector2C(fs, lining, i);
			}
			LoadVector(fs, linecolor);
		}
		// Added in version 1.6.1
		if (fs.peek() != EOF) {
			for (size_t i = 0; i < t_generated; i++) {
				LoadVector2C(fs, tonic, i);
			}
		}
		// Added in version 1.9
		if (fs.peek() != EOF) {
			for (size_t i = 0; i < t_generated; i++) {
				LoadVector2C(fs, minor, i);
			}
		}
	}
	CountOff(0, t_generated - 1);
	CountTime(0, t_generated - 1);
	UpdateNoteMinMax(0, t_generated - 1);
	UpdateTempoMinMax(0, t_generated - 1);
	fs.close();
	// Count time
	long long time_stop = CGLib::time();
	CString est;
	est.Format("Loaded result music from folder %s in %lld ms", dir, time_stop - time_start);
	WriteLog(0, est);
	ValidateVectors(0, t_generated - 1);
}

void CGVar::ValidateVectors2(int step1, int step2) {
	long long time_start = CGLib::time();
	CString st;
	for (int i = step1; i <= step2; i++) {
		for (int v = 0; v < v_cnt; v++) {
			// Check vel is zero
			if (!vel[i][v] && !pause[i][v] && m_algo_id != 112 && m_algo_id != 121 && warning_valid < MAX_WARN_VALID) {
				st.Format("Validation failed at step %d voice %d: velocity cannot be zero", i, v);
				WriteLog(5, st);
				warning_valid++;
			}
			// Check vel is above 127
			if (vel[i][v] > 127 && !pause[i][v] && warning_valid < MAX_WARN_VALID) {
				st.Format("Validation failed at step %d voice %d: velocity cannot be above 127", i, v);
				WriteLog(5, st);
				warning_valid++;
			}
		}
	}
	// Count time
	if (debug_level > 1) {
		long long time_stop = CGLib::time();
		CString est;
		est.Format("Post-validated vectors steps %d-%d in %lld ms", step1, step2, time_stop - time_start);
		WriteLog(0, est);
	}
}

void CGVar::ValidateVectors(int step1, int step2) {
	long long time_start = CGLib::time();
	CString st;
	// Calculate source time where it was not set
	CountSTime(step1, step2);
	// Check first step
	if (!step1) for (int v = 0; v < v_cnt; v++) {
		if (coff[0][v] && warning_valid < MAX_WARN_VALID) {
			st.Format("Validation failed at step 0 voice %d: coff must be zero", v);
			WriteLog(5, st);
			warning_valid++;
		}
	}
	for (int i = step1; i <= step2; i++) {
		for (int v = 0; v < v_cnt; v++) {
			if (i > 0) {
				// Check len, pause and note do not change with coff > 0, while coff increases, poff increases, noff decreases
				if (coff[i][v]) {
					if (pause[i][v] != pause[i - 1][v] && warning_valid < MAX_WARN_VALID) {
						st.Format("Validation failed at step %d voice %d: pause change requires coff=0", i, v);
						WriteLog(5, st);
						warning_valid++;
					}
					if (note[i][v] != note[i - 1][v] && warning_valid < MAX_WARN_VALID) {
						st.Format("Validation failed at step %d voice %d: note change requires coff=0", i, v);
						WriteLog(5, st);
						warning_valid++;
					}
					if (len[i][v] != len[i - 1][v] && warning_valid < MAX_WARN_VALID) {
						st.Format("Validation failed at step %d voice %d: len change requires coff=0", i, v);
						WriteLog(5, st);
						warning_valid++;
					}
					if (coff[i][v] != coff[i - 1][v] + 1 && warning_valid < MAX_WARN_VALID) {
						st.Format("Validation failed at step %d voice %d: coff must increase by 1 each step inside note/pause", i, v);
						WriteLog(5, st);
						warning_valid++;
					}
					if (coff[i][v] >= len[i][v] && warning_valid < MAX_WARN_VALID) {
						st.Format("Validation failed at step %d voice %d: coff must be less than len", i, v);
						WriteLog(5, st);
						warning_valid++;
					}
					if (poff[i][v] != poff[i - 1][v] + 1 && i != coff[i][v] && warning_valid < MAX_WARN_VALID) {
						st.Format("Validation failed at step %d voice %d: poff must increase by 1 each step inside note/pause", i, v);
						WriteLog(5, st);
						warning_valid++;
					}
					if (noff[i][v] != noff[i - 1][v] - 1 && warning_valid < MAX_WARN_VALID) {
						st.Format("Validation failed at step %d voice %d: noff must decrease by 1 each step inside note/pause", i, v);
						WriteLog(5, st);
						warning_valid++;
					}
				}
				// Check poff and noff point at correct positions
				else {
					if (noff[i - 1][v] != 1 && warning_valid < MAX_WARN_VALID) {
						st.Format("Validation failed at step %d voice %d: noff must equal 1 one step before next note/pause", i, v);
						WriteLog(5, st);
						warning_valid++;
					}
					if (poff[i][v] != len[i-1][v] && warning_valid < MAX_WARN_VALID) {
						st.Format("Validation failed at step %d voice %d: poff must equal len of previous note at first step of note/pause", i, v);
						WriteLog(5, st);
						warning_valid++;
					}
				}
			}
			// Check len is not zero
			if (!len[i][v] && warning_valid < MAX_WARN_VALID) {
				st.Format("Validation failed at step %d voice %d: len cannot be zero", i, v);
				WriteLog(5, st);
				warning_valid++;
			}
			// Check key is correct
			if (tonic.size() && (tonic[i][v]<0 || tonic[i][v]>11) && warning_valid < MAX_WARN_VALID) {
				st.Format("Validation failed at step %d voice %d: tonic must be in range 0-11", i, v);
				WriteLog(5, st);
				warning_valid++;
			}
			// Check mode is correct
			if (minor.size() && (minor[i][v]<0 || minor[i][v]>1) && warning_valid < MAX_WARN_VALID) {
				st.Format("Validation failed at step %d voice %d: minor must be in range 0-1", i, v);
				WriteLog(5, st);
				warning_valid++;
			}
			// Check sstime is not zero
			if (!sstime[i][v] && i > 0 && warning_valid < MAX_WARN_VALID) {
				st.Format("Validation failed at step %d voice %d: sstime can be zero only at first step", i, v);
				WriteLog(5, st);
				warning_valid++;
			}
			// Check setime is not zero
			if (!setime[i][v] && i > 0 && warning_valid < MAX_WARN_VALID) {
				st.Format("Validation failed at step %d voice %d: setime cannot be zero", i, v);
				WriteLog(5, st);
				warning_valid++;
			}
			/*
			// Do not check because sstime has meaning only for first step of note, setime has meaning only for last step of note
			// Comparing sstime and setime of the same step does not mean anything
			// Check setime is greater than sstime
			if (setime[i][v] <= sstime[i][v] && warning_valid < MAX_WARN_VALID) {
				st.Format("Validation failed at step %d voice %d: setime must be greater than sstime", i, v);
				WriteLog(5, st);
				warning_valid++;
			}
			*/
		}
		// Check tempo is not zero
		if (!tempo[i] && warning_valid < MAX_WARN_VALID) {
			st.Format("Validation failed at step %d: tempo cannot be zero", i);
			WriteLog(5, st);
			warning_valid++;
		}
		// Check stime is not zero
		if (!stime[i] && i > 0 && warning_valid < MAX_WARN_VALID) {
			st.Format("Validation failed at step %d: stime can be zero only at first step", i);
			WriteLog(5, st);
			warning_valid++;
		}
		// Check etime is not zero
		if (!etime[i] && i > 0 && warning_valid < MAX_WARN_VALID) {
			st.Format("Validation failed at step %d: etime cannot be zero", i);
			WriteLog(5, st);
			warning_valid++;
		}
		// Check etime is greater than stime
		if (etime[i] <= stime[i] && warning_valid < MAX_WARN_VALID) {
			st.Format("Validation failed at step %d: etime must be greater than stime", i);
			WriteLog(5, st);
			warning_valid++;
		}
	}
	// Count time
	if (debug_level > 1) {
		long long time_stop = CGLib::time();
		CString est;
		est.Format("Validated vectors steps %d-%d in %lld ms", step1, step2, time_stop - time_start);
		WriteLog(0, est);
	}
}

void CGVar::LoadResultLogs(CString dir, CString fname)
{
	ifstream fs;
	CString st, path;
	int pos, i;
	char pch[2550];
	// Load logs
	path = dir + "\\log-algorithm.log";
	if (!fileExists(path)) {
		CString est;
		est.Format("Cannot find file %s", path);
		WriteLog(5, est);
		return;
	}
	fs.open(path);
	pos = 0;
	i = 0;
	while (fs.good()) {
		fs.getline(pch, 2550);
		st = pch;
		if (!st.IsEmpty()) WriteLog(3, st);
		if (++i > MAX_LOAD_LOG) break;
	}
	fs.close();
	// Load logs
	path = dir + "\\log-debug.log";
	if (!fileExists(path)) {
		CString est;
		est.Format("Cannot find file %s", path);
		WriteLog(5, est);
		return;
	}
	fs.open(path);
	pos = 0;
	i = 0;
	while (fs.good()) {
		fs.getline(pch, 2550);
		st = pch;
		if (!st.IsEmpty()) WriteLog(0, st);
		if (++i > MAX_LOAD_LOG) break;
	}
	fs.close();
	// Load logs
	path = dir + "\\log-warning.log";
	if (!fileExists(path)) {
		CString est;
		est.Format("Cannot find file %s", path);
		WriteLog(5, est);
		return;
	}
	fs.open(path);
	pos = 0;
	i = 0;
	while (fs.good()) {
		fs.getline(pch, 2550);
		st = pch;
		if (!st.IsEmpty()) WriteLog(1, st);
		if (++i > MAX_LOAD_LOG) break;
	}
	fs.close();
}

void CGVar::LoadResults(CString dir, CString fname)
{
	long long time_start = CGLib::time();
	ifstream fs;
	CString st, st2, st3, path;
	int pos;
	char pch[2550];
	// Load strings
	path = dir + "\\" + fname + ".txt";
	if (!fileExists(path)) {
		CString est;
		est.Format("Cannot find file %s", path);
		WriteLog(5, est);
		return;
	}
	fs.open(path);
	pos = 0;
	int itemp;
	float ftemp;
	while (fs.good()) {
		fs.getline(pch, 2550);
		st = pch;
		pos = st.Find("#");
		if (pos != -1) st = st.Left(pos);
		st.Trim();
		pos = st.Find("=");
		if (pos != -1) {
			st2 = st.Left(pos);
			st3 = st.Mid(pos + 1);
			st2.Trim();
			st3.Trim();
			st2.MakeLower();
			parameter_found = 0;
			CheckVar(&st2, &st3, "ng_min", &itemp);
			CheckVar(&st2, &st3, "ng_max", &itemp);
			CheckVar(&st2, &st3, "tg_min", &ftemp);
			CheckVar(&st2, &st3, "tg_max", &ftemp);
			CheckVar(&st2, &st3, "t_cnt", &t_cnt);
			CheckVar(&st2, &st3, "m_algo_id", &m_algo_id);
			CheckVar(&st2, &st3, "v_cnt", &v_cnt);
			CheckVar(&st2, &st3, "t_generated", &t_generated);
			CheckVar(&st2, &st3, "t_sent", &t_sent);
			CheckVar(&st2, &st3, "t_send", &t_send);
			CheckVar(&st2, &st3, "need_exit", &need_exit);
			LoadVar(&st2, &st3, "gen_start_time", &gen_start_time);
			LoadVar(&st2, &st3, "time_stopped", &time_stopped);
			LoadVar(&st2, &st3, "m_config", &m_config);
			LoadVar(&st2, &st3, "save_format_version", &save_format_version);
			if (!parameter_found) {
				WriteLog(5, "Unrecognized parameter '" + st2 + "' = '" + st3 + "' in file " + path);
			}
		}
		else {
			if (!st.IsEmpty()) {
				WriteLog(5, "No equal sign in line, which is not a comment: '" + st + "' in file " + path);
			}
		}
	}
	fs.close();
	// Allocate
	t_allocated = t_generated;
	InitVectors();
	// Count time
	long long time_stop = CGLib::time();
	CString est;
	est.Format("Loaded result info from folder %s in %lld ms", dir, time_stop - time_start);
	WriteLog(0, est);
}

// Calculate noff, poff
void CGVar::CountOff(int step1, int step2)
{
	for (int i = step1; i <= step2; i++) {
		for (int v = 0; v < v_cnt; v++) {
			noff[i][v] = len[i][v] - coff[i][v];
			if (i - coff[i][v] - 1 >= 0) poff[i][v] = len[i - coff[i][v] - 1][v] + coff[i][v];
			else poff[i][v] = 0;
		}
	}
}

// Find notes that have wrong len and cut them
void CGVar::FixLen(int step1, int step2)
{
	int real_len;
	for (int v = 0; v < v_cnt; v++) {
		// Calculate real length
		real_len = 1;
		for (int i = step1; i <= step2; i++) {
			// If this is not note start, increase real_length
			if (coff[i][v]) {
				++real_len;
			}
			// If this is note start, recalculate length of previous note
			else {
				// Has to be not first note
				if (i > 0) {
					// Does len differ?
					if (real_len != len[i-1][v]) {
						for (int x = i - real_len; x < i; ++x) {
							if (x >= 0) len[x][v] = real_len;
						}
					}
				}
				real_len = 1;
			}
		}
	}
}

void CGVar::CountTime(int step1, int step2) {
	for (int i = step1; i <= step2; i++) {
		if (i > 0) stime[i] = stime[i - 1] + 30000.0 / tempo[i - 1];
		else stime[i] = 0;
		etime[i] = stime[i] + 30000.0 / tempo[i];
	}
}

void CGVar::CountSTime(int step1, int step2) {
	for (int v = 0; v < v_cnt; v++) {
		for (int i = step1; i <= step2; i++) {
			if (!sstime[i][v]) sstime[i][v] = stime[i];
			if (!setime[i][v]) setime[i][v] = etime[i];
		}
	}
}

void CGVar::CopyVoice(int v1, int v2, int step1, int step2, int interval)
{
	for (int i = step1; i <= step2; i++) {
		pause[i][v2] = pause[i][v1];
		if (!pause[i][v1]) note[i][v2] = note[i][v1] + interval;
		len[i][v2] = len[i][v1];
		dyn[i][v2] = dyn[i][v1];
		coff[i][v2] = coff[i][v1];
		poff[i][v2] = poff[i][v1];
		noff[i][v2] = noff[i][v1];
	}
}

void CGVar::UpdateNoteMinMax(int step1, int step2, int final_run)
{
	for (int i = step1; i <= step2; i++) {
		for (int v = 0; v < v_cnt; v++) if ((pause[i][v] == 0) && (note[i][v] != 0)) {
			// Global minimax includes show_transpose, because it is used only for visualization
			if (ng_min > note[i][v] + show_transpose[v]) ng_min = note[i][v] + show_transpose[v];
			if (ng_max < note[i][v] + show_transpose[v]) ng_max = note[i][v] + show_transpose[v];
			// Calculate range including note scan range
			if (ng_min2 > note[i][v] + show_transpose[v]) ng_min2 = note[i][v] + show_transpose[v];
			if (ng_max2 < note[i][v] + show_transpose[v]) ng_max2 = note[i][v] + show_transpose[v];
			if (nsr1.size()) {
				if (nsr1[i][v] && ng_min2 > nsr1[i][v] + show_transpose[v]) 
					ng_min2 = nsr1[i][v] + show_transpose[v];
				if (nsr2[i][v] && ng_max2 < nsr2[i][v] + show_transpose[v]) 
					ng_max2 = nsr2[i][v] + show_transpose[v];
			}
			// Voice minimax does not include show_transpose, because it is used for Adaptation
			if (ngv_min[v] > note[i][v]) ngv_min[v] = note[i][v];
			if (ngv_max[v] < note[i][v]) ngv_max[v] = note[i][v];
			if (final_run) {
				// Count note frequency
				++nfreq[note[i][v] + show_transpose[v]];
				// Count graph maximum
				if (graph.size()) {
					for (int n = 0; n < graph_size; ++n) {
						if (graph_max[n] < graph[i][v][n]) graph_max[n] = graph[i][v][n];
					}
				}
			}
		}
	}
}

void CGVar::UpdateTempoMinMax(int step1, int step2)
{
	for (int i = step1; i <= step2; i++) {
		if (tg_min > tempo[i]) tg_min = tempo[i];
		if (tg_max < tempo[i]) tg_max = tempo[i];
	}
}

void CGVar::AddNote(int pos, int v, char note2, int len2, int dyn2) {
	if (pos + len2 >= t_allocated) ResizeVectors(max(pos + len2 + 1, t_allocated * 2));
	for (int i = 0; i < len2; i++) {
		note[pos + i][v] = note2;
		len[pos + i][v] = len2;
		coff[pos + i][v] = i;
		dyn[pos + i][v] = dyn2;
		pause[pos + i][v] = 0;
	}
}

// Fill pause from start step to (start+length) step inclusive
void CGVar::FillPause(int start, int length, int v) {
	if (start + length >= t_allocated) ResizeVectors(max(start + length + 1, t_allocated * 2));
	for (int x = start; x <= start + length; ++x) {
		pause[x][v] = 1;
		note[x][v] = 0;
		len[x][v] = 1;
		coff[x][v] = 0;
		vel[x][v] = 0;
		if (tonic.size()) {
			tonic[x][v] = tonic_cur;
			minor[x][v] = minor_cur;
			comment[x][v].clear();
			comment2[x][v].Empty();
		}
		midifile_out_mul[x] = midifile_out_mul0 * midifile_out_mul2;
	}
}

// Adds new graph, 
void CGVar::RegisterGraph(CString name, float scale) {
	graph_name.push_back(name);
	graph_scale.push_back(scale);
	graph_max.push_back(0);
	++graph_size;
}

