// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// MGenServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MGenServer.h"
#include "Db.h"
#include "../MGen/GLibrary/GLib.h"
#include "EnumWindows.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAX_TRACK 200

// The one and only application object
CWinApp theApp;

using namespace std;

CString ReaperTempFolder = "server\\Reaper\\";

// Global
int maintenance_mode = 0;
volatile int close_flag = 0;
int nRetCode = 0;
CString est;
CString client_host;
vector<CString> errorMessages;
long long time_job0;
CString j_basefile;

// Parameters
int clean_every_minutes = 1;
CString reaperbuf;
CString share;
CString db_driver, db_server, db_port, db_login, db_pass, db_name;
int daw_wait = 200;
int run_minimized = 0;
int screenshots_enabled = 0;
float rms_exp = 10;

// Job
int j_timeout;
int j_timeout2;
int j_engrave = 0;
int j_render = 0;
int j_priority;
int j_autorestart = 0;
CString progress_fname;
CString j_type;
CString f_folder;
CString j_folder;
CString f_name;
CString j_progress;
int f_stems = 0;
int j_stages = 0;

int can_render = 1;
int screenshot_id = 0;
int max_screenshot = 10;

unordered_map <int, map<int, int>> pan_invert; // [stage][track]
unordered_map <int, unordered_map<int, int>> st_used; // [stage][track]
unordered_map <int, float> st_reverb; // [stage]
unordered_map <int, CString> tr_name; // [track]
vector<int> dyn; // [time]
vector <vector<float>> track_dur; // [stage][track]
vector<vector<long>> av_dyn; // [stage][track] Average dynamics
vector<vector<long>> dyn_cnt; // [stage][track] Number of notes in track
float master_db;

// Children
vector <CString> nChild; // Child process name
map <CString, long long> tChild; // Timestamp of last restart
map <CString, int> aChild; // If state process should be automatically restarted on crash
map <CString, int> rChild; // Is state process running?
map <CString, CString> fChild; // Child process folder
map <CString, CString> pChild; // Child process parameter string

// Time
long long render_start = 0;
long long server_start_time = CGLib::time();

// Objects
CDb db;

float gain2dBFS(float gain) {
	return 20 * log10(gain);
}

void InitErrorMessages() {
	errorMessages.resize(1000);
	errorMessages[0] = "OK";
	errorMessages[9] = "MGen detected warnings during run";
	errorMessages[10] = "MGen detected critical errors during run";
	errorMessages[11] = "MGen generator freeze on exit - possible error in generator";
	errorMessages[100] = "GetExitCodeProcess error (for MGen.exe)";
	errorMessages[101] = "MGen process did not exit correctly - possible crash";
	errorMessages[102] = "GetExitCodeProcess error";
}

CString GetErrorMessage(int e) {
	if (e < errorMessages.size()) return errorMessages[e];
	else return "";
}

void GetProgress(CString cn) {
	vector <CString> sv;
	j_progress = "";
	if (cn == "lilypond-windows.exe" && CGLib::fileExists(progress_fname)) {
		CGLib::read_file_sv(progress_fname, sv);
		if (sv.size()) {
			int i = sv.size() - 1;
			// Protect from empty string
			if (sv[i].IsEmpty()) i = max(0, i - 1);
			j_progress.Format("[%zu] %s", sv.size(), sv[i]);
		}
		else j_progress = "";
	}
}

void SendProgress(CString st) {
	if (st == "") return;
	j_progress = st;
	CString q;
	long long timestamp = CGLib::time();
	q.Format("UPDATE jobs SET j_updated=NOW(), j_progress='%s' WHERE j_id='%lld'",
		db.Escape(j_progress), CDb::j_id);
	if (db.Query(q)) {
		nRetCode = 8;
		return;
	}
}

void WriteLog(CString st) {
	db.WriteLog(st);
	if (db.connected && CDb::j_id) {
		SendProgress(st);
	}
}

// Start process, wait a little and check if process exited with error prematurely
// Then report error
int Run(CString fname, CString par, int delay) {
	DWORD ecode;
	SHELLEXECUTEINFO sei = { 0 };
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
	sei.hwnd = NULL;
	sei.lpVerb = NULL;
	sei.lpFile = fname;
	sei.lpParameters = par;
	sei.lpDirectory = NULL;
	if (run_minimized) sei.nShow = SW_SHOWMINNOACTIVE;
	else sei.nShow = SW_SHOWNORMAL;
	sei.hInstApp = NULL;
	ShellExecuteEx(&sei);
	WaitForSingleObject(sei.hProcess, delay);
	if (!GetExitCodeProcess(sei.hProcess, &ecode)) ecode = 102;
	if (ecode != 0 && ecode != STILL_ACTIVE) { // 259
		est.Format("Exit code %d: %s %s", ecode, fname, par);
		WriteLog(est);
		return ecode;
	}
	return 0;
}

void RestartChild(CString cn) {
	if (Run(fChild[cn] + cn, pChild[cn], 200)) nRetCode = 7;
}

HANDLE GetProcessHandle(CString pname) {
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (Process32First(snapshot, &entry) == TRUE) {
		while (Process32Next(snapshot, &entry) == TRUE) {
			if (stricmp(entry.szExeFile, pname) == 0) {
				return OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
				//WaitForSingleObject(hProcess, 100000);
			}
		}
	}
	CloseHandle(snapshot);
	return NULL;
}

void SaveScreenshot() {
	// Rotate screenshot id
	if (screenshots_enabled) {
		screenshot_id = (screenshot_id + 1) % max_screenshot;
		CString st;
		st.Format("screen%d-%d.png", db.server_id, screenshot_id);
		Run("server\\scripts\\nircmd.exe", "savescreenshot " + share + st, 0);
	}
}

void SendStatus() {
	CString q;
	long long timestamp = CGLib::time();
	long long passed = (timestamp - server_start_time) / 1000;
	q.Format("SELECT session, last_update, TIMESTAMPDIFF(SECOND, last_update, NOW()) as tdiff FROM s_status WHERE s_id='%d'", CDb::server_id);
	if (db.Fetch(q)) {
		nRetCode = 8;
		return;
	}
	if (db.result.size()) {
		long long session2 = db.GetLongLong("session");
		long long tdiff = db.GetLongLong("tdiff");
		if (session2 != CDb::session_id && tdiff < passed) {
			est.Format("Warning! Multiple servers with same server_id '%d' detected. Current server has session '%lld', which started %lld second ago. Another server with session '%lld' updated status at %s (%lld seconds ago)",
				CDb::server_id, CDb::session_id, passed, session2, db.GetSt("last_update"), tdiff);
			WriteLog(est);
		}
	}
	q.Format("REPLACE INTO s_status VALUES('%d','%lld',NOW(),'%s','%ld','%lld','%lld','%lld','%lld','%lld','%lld','%d','%d')",
		CDb::server_id, CDb::session_id, client_host, GetTickCount() / 1000, (timestamp - server_start_time) / 1000,
		rChild["Reaper.exe"] ? (timestamp - tChild["Reaper.exe"]) / 1000 : -1,
		rChild["AutoHotkey.exe"] ? (timestamp - tChild["AutoHotkey.exe"]) / 1000 : -1,
		rChild["MGen.exe"] ? (timestamp - tChild["MGen.exe"]) / 1000 : -1,
		rChild["lilypond-windows.exe"] ? (timestamp - tChild["lilypond-windows.exe"]) / 1000 : -1,
		CDb::j_id, (screenshot_id + max_screenshot - 1) % max_screenshot,
		maintenance_mode);
	CGLib::OverwriteFile("server\\status.txt", q);
	if (db.Query(q)) {
		nRetCode = 8;
		return;
	}
}

void KillProcessByName(const char *filename) {
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(pEntry);
	BOOL hRes = Process32First(hSnapShot, &pEntry);
	while (hRes)
	{
		if (strcmp(pEntry.szExeFile, filename) == 0)
		{
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
				(DWORD)pEntry.th32ProcessID);
			if (hProcess != NULL)
			{
				TerminateProcess(hProcess, 9);
				CloseHandle(hProcess);
			}
		}
		hRes = Process32Next(hSnapShot, &pEntry);
	}
	CloseHandle(hSnapShot);
}

void CheckChildren(int restart) {
	int need_wait = 0;
	for (int c = 0; c < nChild.size(); ++c) {
		CString cn = nChild[c];
		if (!can_render) {
			if (cn == "Reaper.exe") continue;
			if (cn == "AutoHotkey.exe") continue;
		}
		if (cn == "MGenClean.exe" && CGLib::time() - server_start_time < 10000) continue;
		HANDLE hProcess = GetProcessHandle(cn);
		if (hProcess == NULL) {
			rChild[cn] = 0;
			if (restart && aChild[cn]) {
				WriteLog("Restarting process " + cn + "...");
				RestartChild(cn);
				tChild[cn] = CGLib::time();
				if (cn == "Reaper.exe") need_wait = 1;
			}
		}
		else {
			rChild[cn] = 1;
			CloseHandle(hProcess);
		}
	}
	if (need_wait) {
		est.Format("Waiting for %d seconds for virtual instruments to load",
			daw_wait);
		WriteLog(est);
		for (int i = 0; i < daw_wait; ++i) {
			CheckChildren(0);
			SaveScreenshot();
			SendStatus();
			Sleep(1000);
		}
	}
}

// Start process, wait for some time. If process did not finish, this is an error
int RunTimeout(CString path, CString par, int delay, int check_exit_code=1) {
	CString fname = CGLib::fname_from_path(path);
	DWORD ecode;
	SHELLEXECUTEINFO sei = { 0 };
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
	sei.hwnd = NULL;
	sei.lpVerb = NULL;
	sei.lpFile = path;
	sei.lpParameters = par;
	sei.lpDirectory = NULL;
	if (run_minimized) sei.nShow = SW_SHOWMINNOACTIVE;
	else sei.nShow = SW_SHOWNORMAL;
	sei.hInstApp = NULL;
	ShellExecuteEx(&sei);
	long long start_timestamp = CGLib::time();
	while (WaitForSingleObject(sei.hProcess, 500) == WAIT_TIMEOUT) {
		CheckChildren(0);
		GetProgress(fname);
		SendProgress(j_progress);
		SaveScreenshot();
		SendStatus();
		if (CGLib::time() - start_timestamp > delay) {
			WriteLog(path + " " + par + ": Timeout waiting for process");
			return 100;
		}
	}
	if (!GetExitCodeProcess(sei.hProcess, &ecode)) ecode = 102;
	if (ecode != 0 && ecode != STILL_ACTIVE) { // 259
		if (check_exit_code) {
			est.Format("Exit code %d: %s %s", ecode, path, par);
			WriteLog(est);
		}
		return ecode;
	}
	return 0;
}

void CheckChildrenPath() {
	for (int c = 0; c < nChild.size(); ++c) {
		CString cn = nChild[c];
		if (!CGLib::fileExists(fChild[cn] + cn)) {
			if (cn == "Reaper.exe" || cn == "AutoHotkey.exe") {
				can_render = 0;
				WriteLog("Cannot render, because file not found: " + fChild[cn] + cn);
			}
			else {
				WriteLog("Not found program file: " + fChild[cn] + cn);
				//nRetCode = 3;
				return;
			}
		}
	}
}

void LoadConfig() {
	TCHAR buffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, buffer);
	CString current_dir = string(buffer).c_str();
	WriteLog("Started MGenServer in current dir: " + current_dir );

	CString st, st2, st3, cur_child;
	ifstream fs;
	CString fname = "server\\server.pl";
	// Check file exists
	if (!CGLib::fileExists(fname)) {
		WriteLog("LoadConfig cannot find file: " + fname);
		nRetCode = 3;
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
		pos = st.Find("=");
		if (pos != -1) {
			// Get variable name and value
			st2 = st.Left(pos);
			st3 = st.Mid(pos + 1);
			st2.Trim();
			st3.Trim();
			st2.MakeLower();
			// Load general variables
			int idata = atoi(st2);
			float fdata = atof(st3);
			CGLib::parameter_found = 0;
			if (st2 == "childprocess") {
				nChild.push_back(st3);
				aChild[st3] = 0;
				rChild[st3] = 0;
				fChild[st3].Empty();
				pChild[st3].Empty();
				tChild[st3] = CGLib::time();
				cur_child = st3;
				++CGLib::parameter_found;
			}
			CGLib::CheckVar(&st2, &st3, "server_id", &CDb::server_id, 0, 1000000);
			if (aChild.size()) {
				CGLib::CheckVar(&st2, &st3, "childrestart", &aChild[cur_child], 0, 1);
				CGLib::LoadVar(&st2, &st3, "childpath", &fChild[cur_child]);
				CGLib::LoadVar(&st2, &st3, "childparams", &pChild[cur_child]);
			}
			CGLib::CheckVar(&st2, &st3, "clean_every_minutes", &clean_every_minutes);
			CGLib::LoadVar(&st2, &st3, "reaperbuf", &reaperbuf);
			CGLib::LoadVar(&st2, &st3, "db_driver", &db_driver);
			CGLib::CheckVar(&st2, &st3, "daw_wait", &daw_wait, 0, 6000);
			CGLib::CheckVar(&st2, &st3, "run_minimized", &run_minimized, 0, 1);
			CGLib::CheckVar(&st2, &st3, "screenshots_enabled", &screenshots_enabled, 0, 1);
			CGLib::CheckVar(&st2, &st3, "maintenance_mode", &maintenance_mode, 0, 1);
			CGLib::LoadVar(&st2, &st3, "share", &share);
			CGLib::LoadVar(&st2, &st3, "db_server", &db_server);
			CGLib::LoadVar(&st2, &st3, "db_port", &db_port);
			CGLib::LoadVar(&st2, &st3, "db_login", &db_login);
			CGLib::LoadVar(&st2, &st3, "db_pass", &db_pass);
			CGLib::LoadVar(&st2, &st3, "db_name", &db_name);
			if (!CGLib::parameter_found) {
				WriteLog("Unrecognized parameter '" + st2 + "' = '" + st3 + "' in file " + fname);
			}
			if (nRetCode) break;
		}
	}
	fs.close();
	est.Format("LoadConfig loaded %d lines from %s", i, fname);
	WriteLog(est);
	// Check config
	if (!CGLib::dirExists(share)) {
		WriteLog("Shared folder not found: " + share);
		nRetCode = 6;
	}
}

int PauseClose() {
	est.Format("Server is exiting with return code %d", nRetCode);
	WriteLog(est);
	//cout << "Press any key to continue... ";
	//_getch();
	return nRetCode;
}

int Connect() {
	if (db.Connect(db_server, db_port, db_name, db_login, db_pass)) {
		nRetCode = 4;
	}
	return nRetCode;
}

void RotateConfig() {
	CString fname, fname2;
	for (int i = 10; i > 0; --i) {
		fname.Format(share + j_folder + "config-log\\config_%d.pl", i - 1);
		fname2.Format(share + j_folder + "config-log\\config_%d.pl", i);
		if (!CGLib::fileExists(fname)) continue;
		CGLib::copy_file(fname, fname2);
	}
}

int FinishJob(int res, CString st) {
	// Prevent changing database if there is db error (need to restart)
	if (nRetCode) return res;
	int state = 3;
	// Queue autorestarting job after finish
	if (j_autorestart && !res) state = 1;
	RotateConfig();
	CString q;
	q.Format("UPDATE jobs SET j_updated=NOW(), j_duration=TIMESTAMPDIFF(SECOND, j_started, NOW()), j_finished=NOW(), j_state='%d', j_result='%d', j_progress='%s', j_cleaned=0, j_size='%llu' WHERE j_id='%lld'",
		state, res, db.Escape(st), CGLib::FolderSize(share + j_folder), CDb::j_id);
	if (db.Query(q)) {
		nRetCode = 8;
		return res;
	}
	WriteLog(st);
	return res;
}

int SendKeyToWindowClass(CString wClass, short vk) {
	HWND hWindow = FindWindow(wClass, NULL);
	if (hWindow) {
		PostMessage(hWindow, WM_KEYDOWN, VK_F12, 0);
		return 0;
	}
	else return 1;
}

void LoadVoices() {
	vector <CString> sv, sa;
	tr_name.clear();
	st_used.clear();
	st_reverb.clear();
	pan_invert.clear();
	if (!CGLib::fileExists(share + j_folder + j_basefile + ".csv")) return;
	CGLib::read_file_sv(share + j_folder + j_basefile + ".csv", sv);
	for (int i = 1; i < sv.size(); ++i) {
		sa.clear();
		CGLib::Tokenize(sv[i], sa, ";");
		if (sa.size() < 7) continue;
		int tr = atoi(sa[6]);
		int sta = atoi(sa[5]);
		//WriteLog(sa[5] + "/" + sa[6]);
		st_used[sta][tr] = 1;
		st_reverb[sta] = atoi(sa[10]);
		tr_name[tr] = sa[3] + "/" + sa[4];
		pan_invert[sta][tr] = atoi(sa[12]);
	}
}

void MakeRenderLua(int sta) {
	vector<CString> sv;
	ofstream fs;
	CString reverb_mix_st, pan_invert_st;
	fs.open("server\\scripts\\render-midi.lua");
	CGLib::read_file_sv("server\\scripts\\templates\\render-midi.lua", sv);
	reverb_mix_st.Format("%.2f", st_reverb[sta] / 100.0);
	// Make pan invert string
	for (auto it = pan_invert[sta].begin(); it != pan_invert[sta].end(); it++) {
		CString st;
		if (it->second) {
			st.Format("reaper.SetMediaTrackInfo_Value(reaper.GetTrack(0, %d), \"D_DUALPANL\", 1)\n",
				it->first + 2);
			pan_invert_st += st;
			st.Format("reaper.SetMediaTrackInfo_Value(reaper.GetTrack(0, %d), \"D_DUALPANR\", -1)\n",
				it->first + 2);
			pan_invert_st += st;
		}
		else {
			st.Format("reaper.SetMediaTrackInfo_Value(reaper.GetTrack(0, %d), \"D_DUALPANL\", -1)\n",
				it->first + 2);
			pan_invert_st += st;
			st.Format("reaper.SetMediaTrackInfo_Value(reaper.GetTrack(0, %d), \"D_DUALPANR\", 1)\n",
				it->first + 2);
			pan_invert_st += st;
		}
	}
	for (int i = 0; i < sv.size(); ++i) {
		sv[i].Replace("$REVERB_MIX$", reverb_mix_st);
		sv[i].Replace("$PAN_INVERT$", pan_invert_st);
		fs << sv[i] << "\n";
	}
	fs.close();
}

void LoadTrackLength(int sta, int tr, CString fname) {
	vector<CString> sv;
	CString st;
	CGLib::read_file_sv(fname, sv);
	track_dur.resize(max(track_dur.size(), sta + 1));
	track_dur[sta].resize(max(track_dur[sta].size(), tr + 1));
	for (int i = 0; i < sv.size(); ++i) {
		st = sv[i];
		if (st.Left(16) != "format.duration=") continue;
		st = st.Mid(16);
		st.Replace("\"", "");
		track_dur[sta][tr] = atof(st);
	}
}

void AnalyseWaveform(int sta, CString fname2) {
	CString st;
	st.Format("Stage %d/%d: Waveform and information analysis", j_stages - sta, j_stages);
	SendProgress(st);

	// Remove extension
	CString fname3 = fname2;
	if (fname3.Find(".mp3") == fname3.GetLength() - 4) {
		fname3 = fname3.Left(fname3.GetLength() - 4);
	}

	// Get track number
	st = fname3;
	if (st.Find(j_basefile) == 0) st.Delete(0, j_basefile.GetLength());
	if (st[0] == '-') st.Delete(0);
	if (st.Find('-') != -1) st.Delete(st.Find('-'), st.GetLength());
	int tr = -1;
	if (IsCharAlphaNumeric(st[0]) && !IsCharAlpha(st[0])) tr = atoi(st) - 3;

	// Create waveform graphic
	CString par;
	par.Format("-y -i \"%s\" -filter_complex showwavespic=s=1350x120 -frames:v 1 \"%s_.png\"",
		share + j_folder + fname2,
		share + j_folder + fname3);
	int ret = RunTimeout(fChild["ffmpeg.exe"] + "ffmpeg.exe",
		par, 30000);
	if (ret) {
		est.Format("Error during running waveformer: %d", ret);
	}
	if (!CGLib::fileExists(share + j_folder + fname3 + "_.png")) {
		est.Format("File not found: " + share + j_folder + fname3 + "_.png");
	}

	par.Format("-y -i \"%s\" -filter_complex showwavespic=s=8000x800 -frames:v 1 \"%s.png\"",
		share + j_folder + fname2,
		share + j_folder + fname3);
	ret = RunTimeout(fChild["ffmpeg.exe"] + "ffmpeg.exe",
		par, 30000);
	if (ret) {
		est.Format("Error during running waveformer: %d", ret);
	}
	if (!CGLib::fileExists(share + j_folder + fname3 + ".png")) {
		est.Format("File not found: " + share + j_folder + fname3 + ".png");
	}

	/*
	CreateDirectory(share + j_folder + "inf", NULL);
	par.Format("-show_format -of flat -i \"%s\" > \"%s.inf\" 2>&1",
		share + j_folder + fname2,
		share + j_folder + "inf\\" + fname3);
	ret = RunTimeout("cmd.exe", "/c " + fChild["ffmpeg.exe"] + "ffprobe.exe " + par, 30000, 0);
	if (!CGLib::fileExists(share + j_folder + "inf\\" + fname3 + ".inf")) {
		est.Format("File not found: " + share + j_folder + "inf\\" + fname3 + ".inf");
		WriteLog(est);
	}
	else {
		LoadTrackLength(sta, tr, share + j_folder + "inf\\" + fname3 + ".inf");
	}
	*/

	// Get volume
	CImage img;
	HRESULT res = img.Load(share + j_folder + fname3 + ".png");
	if (res == S_OK && tr == -1) {
		//st.Format("Get volume %d:%d " + fname3, sta, tr);
		//WriteLog(st);
		ofstream fs;
		CreateDirectory(share + j_folder + "waveform", NULL);
		fs.open(share + j_folder + "waveform\\" + fname3 + ".csv");
		int ihei = img.GetHeight();
		int iwid = img.GetWidth();
		dyn.resize(iwid);
		COLORREF clr;
		COLORREF white = RGB(255, 255, 255);
		for (int x = 0; x < iwid; ++x) {
			int started = 0;
			int low = -1;
			int high = -1;
			for (int y = ihei / 2; y < ihei; ++y) {
				clr = img.GetPixel(x, y);
				if (clr == CLR_INVALID) continue;
				if (clr == 0) {
					high = y;
					break;
				}
			}
			//CString st;
			//st.Format("Dyn: %d %d %d", x, low, high);
			//WriteLog(st);
			unsigned char c = 0;
			if (high > ihei / 2) {
				c = ((high - ihei / 2) * 255) / (ihei / 2);
			}
			fs << (int)c << "\n";
			dyn[x] = c;
			//fs.write(reinterpret_cast<const char*>(&c), 1);
		}
		fs.close();
		//WriteLog("Volume recorded");
	}
	/*
	if (res == S_OK && tr >= 0) { 
		//st.Format("Get volume %d:%d " + fname3, sta, tr);
		//WriteLog(st);
		ofstream fs;
		CreateDirectory(share + j_folder + "waveform", NULL);
		fs.open(share + j_folder + "waveform\\" + fname3 + ".csv");
		int ihei = img.GetHeight();
		int iwid = img.GetWidth();
		dyn.resize(max(dyn.size(), sta + 1));
		dyn[sta].resize(max(dyn[sta].size(), tr + 1));
		dyn[sta][tr].resize(iwid);
		COLORREF clr;
		COLORREF white = RGB(255, 255, 255);
		for (int x = 0; x < iwid; ++x) {
			int started = 0;
			int low = -1;
			int high = -1;
			for (int y = ihei / 2; y < ihei; ++y) {
				clr = img.GetPixel(x, y);
				if (clr == CLR_INVALID) continue;
				if (clr == 0) {
					high = y;
					break;
				}
			}
			//CString st;
			//st.Format("Dyn: %d %d %d", x, low, high);
			//WriteLog(st);
			unsigned char c = 0;
			if (high > ihei / 2) {
				c = ((high - ihei / 2) * 255) / (ihei / 2);
			}
			fs << (int)c << "\n";
			dyn[sta][tr][x] = c;
			//fs.write(reinterpret_cast<const char*>(&c), 1);
		}
		fs.close();
		//WriteLog("Volume recorded");
	}
	*/
}

int GetMasterVolume(float &mdb) {
	mdb = 0;
	if (!dyn.size()) return 0;
	long av_dyn = 0;
	int max_dyn = 0;
	float rms_dyn = 0;
	for (int i = 0; i < dyn.size(); ++i) {
		if (dyn[i] > max_dyn) max_dyn = dyn[i];
		av_dyn += dyn[i];
		rms_dyn += pow(dyn[i], 2);
	}
	av_dyn = av_dyn / dyn.size();
	rms_dyn = pow(rms_dyn, 0.5);
	mdb = -gain2dBFS(max_dyn / 400.0);
	return 0;
}

int RunMaster() {
	GetMasterVolume(master_db);
	return 0;
}

int RunRenderStage(int sta) {
	vector <CString> sv;
	CString st;
	CString sta_st, sta_st2;
	sta_st.Format("%d", sta);
	sta_st2.Format("%d", j_stages - sta);
	if (!rChild["Reaper.exe"]) {
		return FinishJob(1, "Cannot render because DAW is not running");
	}
	if (!rChild["AutoHotkey.exe"]) {
		return FinishJob(1, "Cannot render because ahk is not running");
	}
	// Clean folder
	CreateDirectory(reaperbuf, NULL);
	CGLib::CleanFolder(reaperbuf + "*.mp3");
	CGLib::CleanFolder(reaperbuf + "*.reapeaks");
	CGLib::CleanFolder(reaperbuf + "*.reaindex");
	DeleteFile(reaperbuf + "progress.txt");
	DeleteFile(reaperbuf + "input.mid");
	DeleteFile(reaperbuf + "windows.log"); 
	DeleteFile(reaperbuf + "finished.txt");	
	rename(reaperbuf + "stage.temp", reaperbuf + "stage.mp3");
	// Copy files
	CGLib::copy_file(share + j_folder + j_basefile + "_" + sta_st + ".midi", reaperbuf + "input.mid");
	// Prepare script
	MakeRenderLua(sta);
	// Start render
	est.Format("Starting render stage #" + sta_st2 + " after %d seconds...",
		(CGLib::time() - time_job0) / 1000);
	WriteLog(est);
	if (SendKeyToWindowClass("REAPERwnd", VK_F12)) {
		return FinishJob(1, "Error sending message to DAW window");
	}
	// Wait for finish
	render_start = CGLib::time();
	for (;;) {
		CheckChildren(1);
		SaveScreenshot();
		SendStatus();
		Sleep(1000);
		// Check if Reaper or AutoHotkey restarted
		if (tChild["Reaper.exe"] > render_start || tChild["AutoHotkey.exe"] > render_start) {
			return FinishJob(1, "Child restarted during render. Please restart task");
		}
		// Check if progress exists
		if (CGLib::fileExists(reaperbuf + "progress.txt")) {
			CGLib::read_file_sv(reaperbuf + "progress.txt", sv);
			if (sv.size()) {
				st.Format("Stage %d/%d: ", j_stages - sta, j_stages);
				SendProgress(st + sv[0]);
			}
		}
		// Check if no progress for long time
		else if (CGLib::time() - render_start > 1000 * 1000) {
			CGLib::copy_file(reaperbuf + "windows.log", share + j_folder + "log-daw_" + sta_st + ".log");
			return FinishJob(1, "Render showed no progress during 1000 seconds");
		}
		// Check if reascript finished
		if (CGLib::fileExists(reaperbuf + "finished.txt")) break;
		// Check for timeout
		if (CGLib::time() - render_start > j_render * 1000) {
			est.Format("Render timed out with %d seconds. Please increase render timeout or decrease music length",
				j_render);
			CGLib::copy_file(reaperbuf + "windows.log", share + j_folder + "log-daw_" + sta_st + ".log");
			return FinishJob(1, est);
		}
	}
	CGLib::copy_file(reaperbuf + "windows.log", share + j_folder + "log-daw_" + sta_st + ".log");
	// No output file
	if (!CGLib::fileExists(reaperbuf + "output-00-master.mp3")) {
		return FinishJob(1, "Output file output-00-master.mp3 does not exist");
	}
	// Zero length file
	if (CGLib::FileSize(reaperbuf + "output-00-master.mp3") <= 0) {
		return FinishJob(1, "Output file output-00-master.mp3 is too small");
	}
	if (sta) {
		if (f_stems) {
			CGLib::copy_file(reaperbuf + "output-00-master.mp3", share + j_folder + j_basefile + "_" + sta_st + ".mp3");
			AnalyseWaveform(sta, j_basefile + "_" + sta_st + ".mp3");
		}
	}
	else {
		CGLib::copy_file(reaperbuf + "output-00-master.mp3", share + j_folder + j_basefile + ".mp3");
		AnalyseWaveform(sta, j_basefile + ".mp3");
	}
	// Copy stems
	if (f_stems) {
		CFileFind finder;
		int track;
		BOOL bWorking = finder.FindFile(reaperbuf + "*.mp3");
		CString fname, fname2;
		while (bWorking) {
			bWorking = finder.FindNextFile();
			if (finder.IsDots()) continue;
			fname = finder.GetFileName();
			// Do not copy stage and master
			if (fname == "stage.mp3") continue;
			if (fname == "output-00-master.mp3") continue;
			fname2 = fname.Left(fname.GetLength() - 4);
			fname2 = fname2.Right(fname2.GetLength() - 7);
			if (fname2.Find("-") != -1) {
				track = atoi(fname2.Left(fname2.Find("-"))) - 3;
				if (!st_used[sta][track]) continue;
				//fname2 = fname2.Mid(fname2.Find("-") + 1);
			}
			fname2 = j_basefile + "-" + fname2 + "_" + sta_st + ".mp3";
			CheckChildren(1);
			SaveScreenshot();
			SendStatus();
			SendProgress("Copying file " + fname2);
			//WriteLog(finder.GetFilePath() + ": " + fname + " -> " + fname2);
			CGLib::copy_file(finder.GetFilePath(),
				share + j_folder + fname2);
			AnalyseWaveform(sta, fname2);
		}
		finder.Close();
	}
	DeleteFile(reaperbuf + "stage.mp3");
	rename(reaperbuf + "output-00-master.mp3", reaperbuf + "stage.temp");
	return 0;
}

/*
void ProcessDyn2() {
	CString st;
	vector<CString> ast;
	ifstream ifs;
	ifs.open(share + j_folder + "notes\\notes.csv");
	char pch[2550];
	int pos = 0;
	int i = 0;
	// Skip first header line
	ifs.getline(pch, 2550);
	while (ifs.good()) {
		i++;
		// Get line
		ifs.getline(pch, 2550);
		st = pch;
		CGLib::Tokenize(st, ast, ";");
		// Do not process wrong lines
		if (ast.size() != 4) continue;
		int sta = atoi(ast[0]);
		int tr = atoi(ast[1]);
		// Start time and end time
		float sti = atoi(ast[2]);
		float eti = atoi(ast[3]);
		// Skip non-existent stage or track
		if (track_dur.size() >= sta || dyn.size() >= sta) continue;
		if (track_dur[sta].size() >= tr || dyn[sta].size() >= tr) continue;
		if (!track_dur[sta][tr] || !dyn[sta][tr].size()) continue;
		// Calculate image steps
		int is1 = sti * 8000.0 / track_dur[sta][tr];
		int is2 = eti * 8000.0 / track_dur[sta][tr];
		// Find highest dynamics
		int max_dyn = 0;
		for (int i = is1; i <= is2; ++i) {
			if (dyn[sta][tr][i] > max_dyn) {
				max_dyn = dyn[sta][tr][i];
			}
		}
		// Skip zero dynamics
		if (!max_dyn) continue;
		av_dyn.resize(max(av_dyn.size(), sta + 1));
		av_dyn[sta].resize(max(av_dyn[sta].size(), tr + 1));
		dyn_cnt.resize(max(dyn_cnt.size(), sta + 1));
		dyn_cnt[sta].resize(max(dyn_cnt[sta].size(), tr + 1));
		// Append dynamics
		av_dyn[sta][tr] += max_dyn;
		++dyn_cnt[sta][tr];
	}
	ofstream fs;
	fs.open(share + j_folder + "waveform\\volume-analysis.csv");
	fs << "Stage;Track;VolCorrect;Track name;Comment\n";
	for (int sta = 0; sta < dyn.size(); ++sta) {
		// Skip empty stages
		if (!dyn[sta].size()) continue;
		if (av_dyn.size() <= sta) continue;
		if (!av_dyn[sta].size()) continue;
		for (int tr = 0; tr < dyn[sta].size(); ++tr) {
			// Skip empty tracks
			if (!dyn[sta][tr].size()) continue;
			if (av_dyn[sta].size() <= tr) continue;
			CString vol_comment;
			vol_comment.Format("%ld notes, %.0f ms track duration", dyn_cnt[sta][tr], track_dur[sta][tr]);
			st.Format("%d;%d;%.0lf;%s;%s", sta, tr, av_dyn[sta][tr] / dyn_cnt[sta][tr], tr_name[tr], vol_comment);
			fs << st << "\n";
		}
	}
}

void ProcessDyn() {
	CString st;
	int tr, tr2, sta, sta2;
	int xmax = 0;
	ofstream fs;
	fs.open(share + j_folder + "waveform\\volume-analysis.csv");
	fs << "Stage;Track;VolCorrect;Track name;Comment\n";
	for (sta = 0; sta < dyn.size(); ++sta) {
		// Skip empty stages
		if (!dyn[sta].size()) continue;
		for (tr = 0; tr < dyn[sta].size(); ++tr) {
			// Skip empty tracks
			if (!dyn[sta][tr].size()) continue;
			xmax = dyn[sta][tr].size();
			CString vol_comment;
			//st.Format("Process dynamics for track %d:%d", sta, tr);
			//WriteLog(st);
			double correct = 0;
			int sum_common = 0;
			for (sta2 = 0; sta2 < dyn.size(); ++sta2) {
				// Skip empty stages
				if (!dyn[sta2].size()) continue;
				for (tr2 = 0; tr2 < dyn[sta2].size(); ++tr2) {
					// Skip empty tracks
					if (!dyn[sta2][tr2].size()) continue;
					// Do not compare track to itself
					if (tr2 == tr && sta == sta2) continue;
					int xmax2 = dyn[sta2][tr2].size();
					// Do not compare tracks with different size;
					if (xmax != xmax2) continue;
					int common = 0;
					double esum = 0;
					double esum2 = 0;
					for (int x = 0; x < xmax; ++x) {
						// Do not compare zero values
						if (!dyn[sta][tr][x]) continue;
						if (!dyn[sta2][tr2][x]) continue;
						++common;
						// Calculate sums of powered values 
						esum += pow(dyn[sta][tr][x], rms_exp);
						esum2 += pow(dyn[sta2][tr2][x], rms_exp);
					}
					if (common && esum > 0) {
						// Get rms values of both tracks
						double rms = pow(esum / common, 1 / rms_exp);
						double rms2 = pow(esum2 / common, 1 / rms_exp);
						if (rms > 0) {
							correct += pow(common, 1 / 4) * (rms2 - rms) / rms;
							sum_common += pow(common, 1 / 4);
							st.Format("%d:%d %.3lf-%.3lf [%d], ", sta2, tr2, rms, rms2, common);
							vol_comment += st;
							//WriteLog(st);
						}
					}
				}
			}
			if (sum_common) correct = correct / sum_common / 3 * 100;
			//st.Format("Suggested volume correction for track %d:%d: %.0lf%%", sta, tr, correct);
			//WriteLog(st);
			st.Format("%d;%d;%.0lf;%s;%s", sta, tr, correct, tr_name[tr], vol_comment);
			fs << st << "\n";
		}
	}
	fs.close();
}
*/

int RunRender() {
	if (!j_render) return 0;
	
	LoadVoices();
	//dyn.clear();
	//track_dur.clear();
	DeleteFile(reaperbuf + "stage.temp");
	for (int sta = j_stages - 1; sta >= 0; --sta) {
		if (RunRenderStage(sta)) return 1;
	}
	// Process dynamics
	RunMaster();
	//ProcessDyn();

	// Clean temporary files
	CGLib::CleanFolder(ReaperTempFolder + "*.wav");
	CGLib::CleanFolder(ReaperTempFolder + "*.reapeaks");

	return 0;
}

int RunJobMGen() {
	j_stages = 0;
	time_job0 = CGLib::time();
	j_basefile = CGLib::bname_from_path(f_name);
	CString sta2;
	CString fname = share + f_folder + f_name;
	CString fname2 = "server\\cache\\" + f_name;
	CString fname_pl = share + j_folder + j_basefile + ".pl";
	CString fname_pl2 = "configs\\Gen" + j_type + "\\sv_" + j_basefile + ".pl";
	// Check input file exists
	if (!CGLib::fileExists(fname_pl)) {
		est = "File not found: " + fname_pl;
		return FinishJob(1, est);
	}
	// Clean folder
	CGLib::CleanFolder(share + j_folder + "*.mp3");
	CGLib::CleanFolder(share + j_folder + "*.pdf");
	CGLib::CleanFolder(share + j_folder + "*.ly");
	CGLib::CleanFolder(share + j_folder + "*.txt");
	//CGLib::CleanFolder(share + j_folder + "inf\\*.inf");
	CGLib::CleanFolder(share + j_folder + "*.csv");
	CGLib::CleanFolder(share + j_folder + "waveform\\*.csv");
	CGLib::CleanFolder(share + j_folder + "*.png");
	CGLib::CleanFolder(share + j_folder + "*.midi");
	// Copy config and midi file
	CreateDirectory("server\\cache", NULL);
	CreateDirectory(share + j_folder + "config-log", NULL);
	CGLib::copy_file(fname_pl, fname_pl2);
	CGLib::copy_file(fname_pl, share + j_folder + "config-log\\config_0.pl");
	CGLib::AppendLineToFile(share + j_folder + "config-log\\config_0.pl", 
		"# " + CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S") +
		" config snapshot at task start\n");
	CGLib::copy_file(fname, fname2);
	// Delete log
	DeleteFile("log\\autosave.txt");
	DeleteFile("autotest\\exit.log");
	SendProgress("Running algorithm");
	// Run MGen
	CString par;
	par.Format("-job=%d %s", j_timeout, fname_pl2);
	tChild["MGen.exe"] = CGLib::time();
	//WriteLog("Starting MGen.exe " + par);
	int mgen_ret = RunTimeout(fChild["MGen.exe"] + "MGen.exe", par, j_timeout2 * 1000);
	SendProgress("Analysing algorithm run results");
	// Get autosave
	CString as_fname, as_dir;
	if (!CGLib::fileExists("log\\autosave.txt")) {
		est = "File not found: log\\autosave.txt";
		return FinishJob(1, est);
	}
	vector <CString> sv;
	CGLib::read_file_sv("log\\autosave.txt", sv);
	if (sv.size() != 3) {
		est = "Wrong row count in file: log\\autosave.txt";
		return FinishJob(1, est);
	}
	as_dir = sv[0];
	as_fname = sv[1];
	if (!CGLib::fileExists(as_dir + "\\" + as_fname + ".txt")) {
		est = "Autosave file not found: " + as_dir + "\\" + as_fname + ".txt";
		return FinishJob(1, est);
	}
	// Copy results
	CGLib::copy_file(as_dir + "\\" + as_fname + ".txt", share + j_folder + j_basefile + ".txt");
	CGLib::copy_file(as_dir + "\\" + as_fname + ".csv", share + j_folder + j_basefile + ".csv");
	CGLib::copy_file(as_dir + "\\log-warning.log", share + j_folder + "log-warning.log");
	CGLib::copy_file(as_dir + "\\log-debug.log", share + j_folder + "log-debug.log");
	CGLib::copy_file(as_dir + "\\log-algorithm.log", share + j_folder + "log-algorithm.log");
	CGLib::copy_file(as_dir + "\\" + as_fname + ".ly", share + j_folder + j_basefile + ".ly");
	CGLib::copy_folder(as_dir + "\\noteinfo", share + j_folder + "noteinfo", "*.*", true);
	CGLib::copy_file(as_dir + "\\" + as_fname + ".midi", share + j_folder + j_basefile + ".midi");
	// Get j_stages 
	j_stages = 0;
	for (int sta = 0; sta < MAX_STAGE; ++sta) {
		sta2.Format("%d", sta);
		if (!CGLib::fileExists(as_dir + "\\" + as_fname + "_" + sta2 + ".midi"))
			break;
		j_stages = sta + 1;
		CGLib::copy_file(as_dir + "\\" + as_fname + "_" + sta2 + ".midi", 
			share + j_folder + j_basefile + "_" + sta2 + ".midi");
	}
	long long time_job1 = CGLib::time();
	// Run lilypond
	if (j_engrave) {
		est.Format("Starting engraver after %lld seconds...", 
			(CGLib::time() - time_job0) / 1000);
		WriteLog(est);
		par =
			"-dgui " +
			share + j_folder + j_basefile + ".ly";
		tChild["lilypond-windows.exe"] = CGLib::time();
		progress_fname = share + j_folder + j_basefile + ".log";
		int ret = RunTimeout(fChild["lilypond-windows.exe"] + "lilypond-windows.exe",
			par, j_engrave * 1000);
		if (ret) {
			est.Format("Error during running engraver: %d", ret);
			return FinishJob(1, est);
		}
		if (!CGLib::fileExists(share + j_folder + j_basefile + ".pdf")) {
			est.Format("File not found: " + share + j_folder + j_basefile + ".pdf");
			return FinishJob(1, est);
		}
	}
	if (mgen_ret > 9) {
		est.Format("Error during algorithm run: %d - %s", mgen_ret, GetErrorMessage(mgen_ret));
		return FinishJob(2, est);
	}
	if (!CGLib::fileExists("autotest\\exit.log")) {
		est.Format("Algorithm process did not exit correctly - possible crash");
		return FinishJob(1, est);
	}
	if (RunRender()) return 1;
	// Check if there were warnings
	if (mgen_ret) {
		est.Format("Warnings during algorithm run: %d - %s", mgen_ret, GetErrorMessage(mgen_ret));
		return FinishJob(3, est);
	}
	else {
		est.Format("Success in %lld seconds",
			(CGLib::time() - time_job0) / 1000);
		return FinishJob(0, est);
	}
}

int RunJob() {
	// Check that folder exists
	if (!CGLib::dirExists(share + f_folder)) {
		est = "Folder not found: " + share + f_folder;
		WriteLog(est);
		return FinishJob(1, est);
	}
	// Check that folder exists
	if (!CGLib::dirExists(share + j_folder)) {
		est = "Folder not found: " + share + j_folder;
		WriteLog(est);
		return FinishJob(1, est);
	}
	// Create job folder
	/*
	vector <CString> sv;
	CString path;
	CGLib::Tokenize(j_folder, sv, "\\");
	for (int i = 0; i < sv.size(); ++i) {
		if (sv[i].IsEmpty()) break;
		if (!path.IsEmpty()) path += "\\";
		path += sv[i];
		CreateDirectory(share + path, NULL);
	}
	*/
	RunJobMGen();
	CDb::j_id = 0;
	return 0;
}

void TakeJob() {
	CString q, est;
	q = "LOCK TABLES jobs WRITE, files WRITE, j_logs WRITE, s_status WRITE, users WRITE";
	if (db.Query(q)) {
		nRetCode = 8;
		return;
	}
	int err;
	CString cond;
	if (maintenance_mode == 1) cond += " AND u_admin=1 ";
	if (!can_render) {
		cond += " AND j_class<2 ";
	}
	err = db.Fetch("SELECT * FROM jobs "
		"LEFT JOIN files USING (f_id) "
		"LEFT JOIN users ON (jobs.started_u_id = users.u_id) "
		"WHERE j_state=1 " + cond +
		"ORDER BY j_priority, j_id LIMIT 1");
	if (err) {
		nRetCode = 8;
		return;
	}
	if (db.result.size()) {
		// Load job
		CDb::j_id = db.GetLongLong("j_id");
		j_priority = db.GetInt("j_priority");
		j_autorestart = db.GetInt("j_autorestart");
		f_stems = db.GetInt("f_stems");
		j_timeout = db.GetInt("j_timeout");
		j_timeout2 = db.GetInt("j_timeout2");
		j_engrave = db.GetInt("j_engrave");
		j_render = db.GetInt("j_render");
		j_type = db.GetSt("j_type");
		f_folder = db.GetSt("f_folder");
		f_folder.Replace("/", "\\");
		j_folder = db.GetSt("j_folder");
		j_folder.Replace("/", "\\");
		f_name = db.GetSt("f_name");
		// Load defaults
		if (!j_timeout) j_timeout = 600;
		if (!j_timeout2) j_timeout2 = 640;
		// Prevent changing database if there is db error (need to restart)
		if (nRetCode) return;
		// Take job
		q.Format("UPDATE jobs SET j_started=NOW(), j_updated=NOW(), s_id='%d', j_state=2, j_progress='Job assigned', j_changes=0, j_changes_st='' WHERE j_id='%lld'",
			CDb::server_id, CDb::j_id);
		if (db.Query(q)) {
			nRetCode = 8;
			return;
		}
		q = "UNLOCK TABLES";
		if (db.Query(q)) {
			nRetCode = 8;
			return;
		}
		// Log
		est.Format("Taking job #%lld: %s, %s%s (priority %d)", 
			CDb::j_id, j_type, j_folder, f_name, j_priority);
		WriteLog(est);
		// Update status
		SaveScreenshot();
		SendStatus();
		RunJob();
		SendStatus();
	}
	else {
		q = "UNLOCK TABLES";
		if (db.Query(q)) {
			nRetCode = 8;
			return;
		}
	}
}

void Init() {
	db.log_fname = "server\\server.log";
	CString q;
	// Register server session
	q.Format("INSERT INTO sessions (s_id, s_created) VALUES('%d', NOW())", CDb::server_id);
	if (db.Query(q)) {
		nRetCode = 8;
		return;
	}
	CDb::session_id = db.GetInsertId();
	// On start, reset all jobs that did not finish correctly
	q.Format("SELECT COUNT(*) as cnt FROM jobs WHERE s_id='%d' AND j_state=2", CDb::server_id);
	if (db.Fetch(q)) {
		nRetCode = 8;
		return;
	}
	if (db.result.size()) {
		int cnt = db.GetInt("cnt");
		if (cnt) {
			est.Format("Detected and cleared %d jobs that did not finish correctly on this server #%d",
				cnt, CDb::server_id);
			WriteLog(est);
		}
	}
	q.Format("UPDATE jobs SET j_updated=NOW(), j_state=1 WHERE s_id='%d' AND j_state=2", CDb::server_id);
	if (db.Query(q)) {
		nRetCode = 8;
		return;
	}
	// Get client hostname
	q = "SELECT SUBSTRING_INDEX(host,':',1) as 'ip' from information_schema.processlist WHERE ID=connection_id()";
	if (db.Fetch(q)) {
		nRetCode = 8;
		return;
	}
	if (db.result.size()) {
		client_host = db.GetSt("ip");
	}
}

BOOL WINAPI ConsoleHandlerRoutine(DWORD dwCtrlType) {
	if (dwCtrlType == CTRL_CLOSE_EVENT) {
		WriteLog("User initiated server exit");
		close_flag = 1;
		while (close_flag != 2)
			Sleep(100);
		//WriteLog("Exiting server");
		return TRUE;
	}
	return FALSE;
}

void CleanChildren() {
	KillProcessByName("MGen.exe");
	KillProcessByName("lilypond-windows.exe");
	// Send reaper stop render
	Run(fChild["AutoHotkey.exe"] + "AutoHotkey.exe", "server\\scripts\\mgen-kill.ahk", 800);
	/*
	HWND hWindow = FindWindow("REAPERwnd", NULL);
	PostMessage(hWindow, WM_KEYDOWN, VK_ESCAPE, 0);
	if (hWindow) {
		CEnumWindows ahwndChildWindows(hWindow);
		for (int nWindow = 0; nWindow < ahwndChildWindows.Count(); nWindow++) {
			HWND hWnd = ahwndChildWindows.Window(nWindow);
			char pch[1000];
			GetWindowText(hWnd, pch, 1000);
			CString st = pch;
			WriteLog(st);
			PostMessage(hWnd, WM_KEYDOWN, VK_ESCAPE, 0);
		}
	}
	*/
}

int main() {
	HMODULE hModule = ::GetModuleHandle(nullptr);

  if (hModule != nullptr) {
    // initialize MFC and print and error on failure
    if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0)) {
      // TODO: change error code to suit your needs
      wprintf(L"Fatal Error: MFC initialization failed\n");
			return 1;
		}
  }
  else {
    // TODO: change error code to suit your needs
    wprintf(L"Fatal Error: GetModuleHandle failed\n");
		return 2;
	}

	InitErrorMessages();
	BOOL ret = SetConsoleCtrlHandler(ConsoleHandlerRoutine, TRUE);
	LoadConfig();
	if (nRetCode) {
		return PauseClose();
	}
	CheckChildrenPath();
	CleanChildren();
	CheckChildren(1);
	Connect();
	if (nRetCode) return PauseClose();
	Init();
	for (;;) {
		CheckChildren(1);
		long long timestamp = CGLib::time();
		long long passed = (timestamp - server_start_time) / 1000;
		if (passed > 60 * 60) {
			close_flag = 2;
			break;
		}
		if (nRetCode) return PauseClose();
		SaveScreenshot();
		SendStatus();
		if (close_flag == 1) {
			close_flag = 2;
			break;
		}
		TakeJob();
		Sleep(1000);
	}
	return PauseClose();
}
