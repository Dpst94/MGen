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

#define MAX_TRACK 1000

// The one and only application object
CWinApp theApp;

using namespace std;

CString ReaperTempFolder = "server\\Reaper\\";

// Global
volatile int close_flag = 0;
int nRetCode = 0;
CString est;
CString client_host;
vector<CString> errorMessages;
long long time_job0;
CString j_basefile;

// Parameters
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

map <int, map<int, int>> st_used; // [stage][track]
map <int, float> st_reverb; // [stage]
map <int, CString> tr_name; // [track]
map <int, vector<int>> dyn; // [track][time]

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

void InitErrorMessages() {
	errorMessages.resize(1000);
	errorMessages[0] = "OK";
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
			j_progress.Format("[%d] %s", sv.size(), sv[i]);
		}
		else j_progress = "";
	}
}

void SendProgress(CString st) {
	if (st == "") return;
	j_progress = st;
	CString q;
	long long timestamp = CGLib::time();
	q.Format("UPDATE jobs SET j_updated=NOW(), j_progress='%s' WHERE j_id='%ld'",
		db.Escape(j_progress), CDb::j_id);
	db.Query(q);
}

void WriteLog(CString st) {
	db.WriteLog(st);
	if (db.db.IsOpen() && CDb::j_id) {
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
	q.Format("REPLACE INTO s_status VALUES('%d',NOW(),'%s','%ld','%lld','%lld','%lld','%lld','%lld','%ld','%d')",
		CDb::server_id, client_host, GetTickCount() / 1000, (timestamp - server_start_time) / 1000,
		rChild["Reaper.exe"] ? (timestamp - tChild["Reaper.exe"]) / 1000 : -1,
		rChild["AutoHotkey.exe"] ? (timestamp - tChild["AutoHotkey.exe"]) / 1000 : -1,
		rChild["MGen.exe"] ? (timestamp - tChild["MGen.exe"]) / 1000 : -1,
		rChild["lilypond-windows.exe"] ? (timestamp - tChild["lilypond-windows.exe"]) / 1000 : -1,
		CDb::j_id, (screenshot_id + max_screenshot - 1) % max_screenshot);
	CGLib::OverwriteFile("server\\status.txt", q);
	db.Query(q);
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
				nRetCode = 3;
				return;
			}
		}
	}
}

void LoadConfig()
{
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
			CGLib::LoadVar(&st2, &st3, "reaperbuf", &reaperbuf);
			CGLib::LoadVar(&st2, &st3, "db_driver", &db_driver);
			CGLib::CheckVar(&st2, &st3, "daw_wait", &daw_wait, 0, 6000);
			CGLib::CheckVar(&st2, &st3, "run_minimized", &run_minimized, 0, 1);
			CGLib::CheckVar(&st2, &st3, "screenshots_enabled", &screenshots_enabled, 0, 1);
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
	if (db.Connect(db_driver, db_server, db_port, db_name, db_login, db_pass)) {
		nRetCode = 4;
	}
	return nRetCode;
}

int FinishJob(int res, CString st) {
	CString q;
	q.Format("UPDATE jobs SET j_updated=NOW(), j_duration=TIMESTAMPDIFF(SECOND, j_started, NOW()), j_finished=NOW(), j_state=3, j_result='%d', j_progress='%s' WHERE j_id='%ld'",
		res, db.Escape(st), CDb::j_id);
	db.Query(q);
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
	if (!CGLib::fileExists(share + j_folder + j_basefile + ".csv")) return;
	CGLib::read_file_sv(share + j_folder + j_basefile + ".csv", sv);
	for (int i = 1; i < sv.size(); ++i) {
		sa.clear();
		CGLib::Tokenize(sv[i], sa, ";");
		if (sa.size() < 7) continue;
		//WriteLog(sa[5] + "/" + sa[6]);
		st_used[atoi(sa[5])][atoi(sa[6])] = 1;
		st_reverb[atoi(sa[5])] = atoi(sa[10]);
		tr_name[atoi(sa[6])] = sa[3] + "/" + sa[4];
	}
}

void MakeRenderLua(int sta) {
	vector<CString> sv;
	ofstream fs;
	CString title;
	CString st;
	fs.open("server\\scripts\\render-midi.lua");
	CGLib::read_file_sv("server\\scripts\\templates\\render-midi.lua", sv);
	st.Format("%.2f", st_reverb[sta] / 100.0);
	for (int i = 0; i < sv.size(); ++i) {
		sv[i].Replace("$REVERB_MIX$", st);
		fs << sv[i] << "\n";
	}
	fs.close();
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
	par.Format("-y -i \"%s\" -filter_complex showwavespic=s=1050x120 -frames:v 1 \"%s_.png\"",
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

	par.Format("-show_format -of flat -i \"%s\" > \"%s\".inf 2>&1",
		share + j_folder + fname2,
		share + j_folder + fname3);
	ret = RunTimeout(fChild["ffmpeg.exe"] + "ffprobe.exe", par, 30000, 0);
	if (!CGLib::fileExists(share + j_folder + fname3 + ".inf")) {
		est.Format("File not found: " + share + j_folder + fname3 + ".inf");
	}

	// Get volume
	CImage img;
	HRESULT res = img.Load(share + j_folder + fname3 + ".png");
	if (res == S_OK && tr >= 0) { 
		//st.Format("Get volume %d:%d " + fname3, sta, tr);
		//WriteLog(st);
		ofstream fs;
		CreateDirectory(share + j_folder + "waveform", NULL);
		fs.open(share + j_folder + "waveform\\" + fname3 + ".csv");
		int ihei = img.GetHeight();
		int iwid = img.GetWidth();
		dyn[tr + sta * MAX_TRACK].resize(iwid);
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
			dyn[tr + sta * MAX_TRACK][x] = c;
			//fs.write(reinterpret_cast<const char*>(&c), 1);
		}
		fs.close();
		//WriteLog("Volume recorded");
	}
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

void ProcessDyn() {
	CString st;
	int tr, tr2, sta, sta2;
	int xmax = 0;
	ofstream fs;
	CreateDirectory(share + j_folder + "waveform", NULL);
	fs.open(share + j_folder + "waveform\\volume-analysis.csv");
	fs << "Stage;Track;VolCorrect;Track name;Comment\n";
	for (map<int, vector<int>>::iterator it = dyn.begin(); it != dyn.end(); ++it) {
		tr = it->first % MAX_TRACK;
		sta = it->first / MAX_TRACK;
		xmax = it->second.size();
		CString vol_comment;
		//st.Format("Process dynamics for track %d:%d", sta, tr);
		//WriteLog(st);
		double correct = 0;
		int sum_common = 0;
		for (map<int, vector<int>>::iterator it2 = dyn.begin(); it2 != dyn.end(); ++it2) {
			tr2 = it2->first % MAX_TRACK;
			sta2 = it2->first / MAX_TRACK;
			// Do not compare track to itself
			if (tr2 == tr && sta == sta2) continue;
			// Do not compare tracks with different size;
			if (xmax != it->second.size()) continue;
			int common = 0;
			double esum = 0;
			double esum2 = 0;
			for (int x = 0; x < xmax; ++x) {
				// Do not compare zero values
				if (!it->second[x]) continue;
				if (!it2->second[x]) continue;
				++common;
				esum += pow(it->second[x], rms_exp);
				esum2 += pow(it2->second[x], rms_exp);
			}
			if (common && esum > 0) {
				double rms = pow(esum / common, 1 / rms_exp);
				double rms2 = pow(esum2 / common, 1 / rms_exp);
				if (rms > 0) {
					correct += pow(common, 1 / 4) * (rms2 - rms) / rms;
					sum_common += pow(common, 1 / 4);
					st.Format("%d:%d %.3lf-%.3lf [%d], ", sta, tr, sta2, tr2, rms, rms2, common);
					vol_comment += st;
					//WriteLog(st);
				}
			}
		}
		if (sum_common) correct = correct / sum_common / 3 * 100;
		//st.Format("Suggested volume correction for track %d:%d: %.0lf%%", sta, tr, correct);
		//WriteLog(st);
		st.Format("%d;%d;%.0lf;%s;%s", sta, tr, correct, tr_name[tr], vol_comment);
		fs << st << "\n";
	}
	fs.close();
}

int RunRender() {
	if (!j_render) return 0;
	
	LoadVoices();
	dyn.clear();
	DeleteFile(reaperbuf + "stage.temp");
	for (int sta = j_stages - 1; sta >= 0; --sta) {
		if (RunRenderStage(sta)) return 1;
	}
	// Process dynamics
	ProcessDyn();

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
	CGLib::CleanFolder(share + j_folder + "*.inf");
	CGLib::CleanFolder(share + j_folder + "*.csv");
	CGLib::CleanFolder(share + j_folder + "*.png");
	CGLib::CleanFolder(share + j_folder + "*.midi");
	// Copy config and midi file
	CreateDirectory("server\\cache", NULL);
	CGLib::copy_file(fname_pl, fname_pl2);
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
	int ret = RunTimeout(fChild["MGen.exe"] + "MGen.exe", par, j_timeout2 * 1000);
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
	if (!CGLib::fileExists(as_dir + "\\" + as_fname + ".ly")) {
		est = "Autosave file not found: " + as_dir + "\\" + as_fname + ".ly";
		return FinishJob(1, est);
	}
	// Copy results
	CGLib::copy_file(as_dir + "\\" + as_fname + ".txt", share + j_folder + j_basefile + ".txt");
	CGLib::copy_file(as_dir + "\\" + as_fname + ".csv", share + j_folder + j_basefile + ".csv");
	CGLib::copy_file(as_dir + "\\log-warning.log", share + j_folder + "log-warning.log");
	CGLib::copy_file(as_dir + "\\log-debug.log", share + j_folder + "log-debug.log");
	CGLib::copy_file(as_dir + "\\log-algorithm.log", share + j_folder + "log-algorithm.log");
	CGLib::copy_file(as_dir + "\\" + as_fname + ".ly", share + j_folder + j_basefile + ".ly");
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
	if (ret) {
		est.Format("Error during algorithm run: %d - %s", ret, GetErrorMessage(ret));
		return FinishJob(1, est);
	}
	if (!CGLib::fileExists("autotest\\exit.log")) {
		est.Format("Algorithm process did not exit correctly - possible crash");
		return FinishJob(1, est);
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
		ret = RunTimeout(fChild["lilypond-windows.exe"] + "lilypond-windows.exe",
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
	if (RunRender()) return 1;
	est.Format("Success in %lld seconds", 
		(CGLib::time() - time_job0) / 1000);
	return FinishJob(0, est);
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
	db.Query("LOCK TABLES jobs WRITE, files WRITE, j_logs WRITE, s_status WRITE, users WRITE");
	int err;
	if (can_render) {
		err = db.Fetch("SELECT * FROM jobs "
			"LEFT JOIN files USING (f_id) "
			"WHERE j_state=1 ORDER BY j_priority, j_id LIMIT 1");
	}
	else {
		err = db.Fetch("SELECT * FROM jobs "
			"LEFT JOIN files USING (f_id) "
			"WHERE j_state=1 AND j_class<2 ORDER BY j_priority, j_id LIMIT 1");
	}
	if (!err && !db.rs.IsEOF()) {
		// Load job
		CDb::j_id = db.GetInt("j_id");
		j_priority = db.GetInt("j_priority");
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
		// Take job
		q.Format("UPDATE jobs SET j_started=NOW(), j_updated=NOW(), s_id='%d', j_state=2, j_progress='Job assigned' WHERE j_id='%ld'",
			CDb::server_id, CDb::j_id);
		db.Query(q);
		db.Query("UNLOCK TABLES");
		// Log
		est.Format("Taking job #%ld: %s, %s%s (priority %d)", 
			CDb::j_id, j_type, j_folder, f_name, j_priority);
		WriteLog(est);
		// Update status
		SaveScreenshot();
		SendStatus();
		RunJob();
		SendStatus();
	}
	else {
		db.Query("UNLOCK TABLES");
	}
}

void Init() {
	// On start, reset all jobs that did not finish correctly
	CString q;
	q.Format("SELECT COUNT(*) as cnt FROM jobs WHERE s_id='%d' AND j_state=2", CDb::server_id);
	db.Fetch(q);
	if (!db.rs.IsEOF()) {
		db.GetFields();
		int cnt = db.GetInt("cnt");
		if (cnt) {
			est.Format("Detected and cleared %d jobs that did not finish correctly on this server #%d",
				cnt, CDb::server_id);
			WriteLog(est);
		}
	}
	q.Format("UPDATE jobs SET j_updated=NOW(), j_state=1 WHERE s_id='%d' AND j_state=2", CDb::server_id);
	db.Query(q);
	// Get client hostname
	db.Fetch("SELECT SUBSTRING_INDEX(host,':',1) as 'ip' from information_schema.processlist WHERE ID=connection_id()");
	if (!db.rs.IsEOF()) {
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
	CheckChildrenPath();
	if (Connect()) return PauseClose();
	if (nRetCode) {
		return PauseClose();
	}
	CleanChildren();
	Init();
	for (;;) {
		CheckChildren(1);
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
