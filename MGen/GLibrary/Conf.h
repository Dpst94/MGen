#pragma once
#include "GIConf.h"
#include "GLib.h"

#define mftSIB 10
#define mftFIN 11
#define mftMUS 12

class CConf :
	public CGLib
{
public:
	CConf();
	~CConf();

	void AddIcf();
	void LoadInstruments(); // Load instruments config
	void LoadInstrument(int i, CString fname);
	void LoadCCName(CString * sName, CString * sValue, CString sSearch, int i);
	void LoadKswGroup(CString * sName, CString * sValue, CString sSearch, int i);
	PmMessage ParseMidiCommand(CString st, int i);
	PmMessage ParseMidiCommand2(CString st, int value, int i);
	void SaveInitCommand(PmMessage msg, int i);
	void LoadInitCommand(CString * sName, CString * sValue, CString sSearch, int i);
	void LoadTechnique(CString * sName, CString * sValue, CString sSearch, int i);
	void LoadInitTechnique(CString * sName, CString * sValue, CString sSearch, int i);
	void LoadMapPitch(CString * sName, CString * sValue, CString sSearch, int i);
	void LoadInstrumentLine(CString st2, CString st3, int i);
	void LoadConfig(CString fname, int load_includes = 1);
	void ProcessConfig();
	void LoadConfigFiles(CString fname, int load_includes);
	void LoadInstrumentLayout();
	void LoadInstrumentLayoutLine(CString & st2, CString & st3);
	void LoadConfigFile(CString fname, int load_includes = 1);
	short CreateVirtualInstrument(int instr_id, int child_id);
	void LoadVarInstr(CString * sName, CString * sValue, char * sSearch, vector<int>& Dest);
	virtual void LoadConfigLine(CString* sN, CString* sV, int idata, float fdata) = 0;

	// Data interface
	int m_loading = 0; // If we are loading saved results
	CString m_current_config; // Current loading config
	int m_algo_id = -1; // Current algorithm id
	CString m_algo_folder; // Current algorithm folder
	CString m_algo_name; // Current algorithm name
	CString m_algo_insts; // Instruments of current algorithm from algorithms.txt
	CString m_config;

	// Configuration parameters
	int t_cnt = 1600; // Timeslot count (eighth notes) to stop generation
	int v_cnt = 1; // Voice count
	int t_allocated = 1600; // Timeslot count to initialize vectors
	int t_send = 60; // Timeslot count to send
	float midifile_in_mul = 1; // Multiply note length with this value when loading
	int shuffle = 0; // If you want to shuffle all canti after generation (can shuffle up to 32000 canti)
	int sleep_ms = 0;
	int adapt_enable = 1;
	int auto_legato = -1;
	int reverb_mix = 30;
	int toload_time = 0;
	int auto_nonlegato = 0; // Setting this to 0 disables nonlegato_freq
	int midi_file_type = 0; // 0 - not specified, 10 - Sibelius, 11 - Finale, 12 - MuseScore
	int grow_notes = -1;
	int master_vol = 100; // Master volume for all tracks. 0 - automatic, 100 - maximum
	int comment_adapt = 1; // If you want to have each adaptation decision commented
	vector <int> show_transpose; // Semitone transpose for showing
	int midifile_export_marks = 0; // Set to export marks (harmony) to midifile lyrics
	int midifile_export_comments = 1; // Set to export comments (violations) to midifile lyrics
	float midifile_out_mul0 = 1; // Multiply note length with this value when saving

	// Instruments
	map<CString, CString> InstDefaultConfig; // Instrument configs by name
	CString instr_layout; // Name of instruments/*.txt file to load instrument layout from
	CString m_config_insts; // String with instrument mapping from config
	vector<int> instr; // Instruments for each voice
	vector<int> v_stage; // Stage for each voice
	vector<int> v_itrack; // Instrument local track number for each voice
	vector<int> t_instr; // Instrument for each result track. This value can be overwritten by instruments sharing same track
	vector<IConf> icf;
	int virt_instr_count = 0;

	// Global config
	int rnd_tempo = 6; // Randomize tempo not greater than this percent
	int rnd_tempo_step = 1; // Maximum difference in tempo between adjacent steps while randomizing

};

