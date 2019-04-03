#pragma once
#include "Conf.h"

// Maximum size of progress
#define MAX_PROGRESS 1000

// Maximum number of lines to load from saved logs
#define MAX_LOAD_LOG 10

#define MAX_WARN_LOADVECTORS 10
#define MAX_WARN_VALID 10
// Maximum allowed note length. Must be at least two times lower than 65 535 for poff to work
#define MAX_LEN 16384 

class CGVar :
	public CConf
{
public:
	CGVar();
	~CGVar();

	void InitVectors();
	void SaveResults(CString dir, CString fname);
	void ExportVectorsCSV(CString dir, CString fname);
	void LoadResults(CString dir, CString fname);
	void LoadResultLogs(CString dir, CString fname);
	void LoadResultMusic(CString dir, CString fname);

	// Warnings
	int warning_loadvectors = 0;
	int warning_valid = 0;


	// Data interface
	CString as_fname; // Directory of autosave document
	CString as_dir; // Directory of autosave document
	CString save_format_version; // Version of save format loaded

  // Main variables
	int ly_debugexpect = 0; // Export unexpected and unconfirmed flags to LY
	vector<int> progress; // Show progress
	int progress_size = 0; // Effective progress size to use
	float m_pspeed = 100; // Playback speed in percent
	long long gen_start_time = 0; // Time in milliseconds when generation started
	long long time_stopped = 0; // Time in milliseconds when generation stopped
	int t_generated = 0; // Timeslots generated
	int t_adapted = 0; // Timeslot count adapted
	int t_sent = 0; // Timeslot count sent to mainframe
	int ng_min = 1000; // Minimum generated note
	int ng_max = 0; // Maximum generated note
	int ng_min2 = 1000; // Minimum generated note (with note scan range)
	int ng_max2 = 0; // Maximum generated note (with note scan range)
	vector<int> ngv_min; // Minimum generated note per voice
	vector<int> ngv_max; // Maximum generated note per voice
	float tg_min = 1000; // Minimum generated tempo
	float tg_max = 0; // Maximum generated tempo
	float basic_tempo = 100; // Basic tempo
	float midifile_out_mul2 = 1; // Multiply note length with this value when saving

	// Output
	vector<float> midifile_out_mul; // [s] Multiply note length with this value when saving
	vector <int> nfreq; // Note frequency
	vector< vector <unsigned char> > pause; // 1 = pause, 0 = note
	vector< vector <unsigned char> > note; // Note values (midi)
	vector< vector <unsigned char> > note_muted; // Note values (midi)
	vector< vector <unsigned short> > len; // Note length in timeslots
	vector< vector <unsigned short> > coff; // Offset of current note start backward (0 = first timeslot of note)
	vector< vector <unsigned short> > poff; // Offset of previous note start (backward)
	vector< vector <unsigned short> > noff; // Offset of next note start (forward)
	vector< vector <unsigned char> > tonic; // Key tonic of current note (3 = D#)
	vector< vector <unsigned char> > minor; // Key minor indicator of current note (0 = major, 1 = minor)
	vector< vector <unsigned char> > dyn; // Dynamics (velocity for piano)
	vector< vector <unsigned char> > vel; // Velocity of midi notes
	vector< vector <unsigned char> > vib; // Vibrato intensity
	vector< vector <unsigned char> > vibf; // Vibrato frequency
	vector< vector <unsigned char> > artic; // Articulations
	vector< vector <unsigned char> > filter; // Sound filter
	vector< vector <map<short, char>> > nlink; // [i][v] Link to other note for LY
	vector< vector <map<short, char>> > fsev; // [i][v] Flag severity for LY
	vector<short> sstep; // [i] Source step in counterpoint / cantus
	vector< vector <vector<float>> > ngraph; // [s][v][] Graph using chromatic scale
	vector< vector <vector<float>> > graph; // [s][v][] Graph using arbitrary scale
	vector<CString> graph_name; // [] Graph name
	vector<float> graph_max; // [] Maximum graph value
	vector<float> graph_scale; // [] Maximum graph scale (semitones per unit)
	int ngraph_size = 0; // Number of ngraphs
	int graph_size = 0; // Number of graphs
	vector< vector <int> > mel_id; // [i][v] Link from note step to melody id
	vector< vector <unsigned char> > lining; // Visual lining pattern
	vector< vector <CString> > mark; // Mark on note
	vector<CString> mel_info; // Information about melody
	vector<CString> mel_info2; // Information about melody
	vector<CString> mel_info3; // Information about melody
	vector< vector <DWORD> > mark_color; // Mark color
	vector< vector <unsigned char> > midi_ch; // Midi channel of each note
	vector< DWORD > linecolor; // Shows color of bar line if not zero
	vector< vector <char> > lengroup; // How many notes left until last in lengroup
	vector< vector <CString> > lyrics; // Imported MIDI lyrics
	vector< vector <vector<CString>> > comment; // Comment for note
	vector< vector <vector<DWORD>> > ccolor; // Comment color for note
	vector< vector <CString> > comment2; // Comment for note (shorter)
	vector< vector <char> > nsr1; // Note scan range
	vector< vector <char> > nsr2; // Note scan range
	vector< vector <CString> > adapt_comment; // Adaptation comment for note
	vector< vector <DWORD> > color; // Note color (rgb ignored if all zero; alpha ignored if zero)
	vector<float> tempo; // Tempo
	vector<float> tempo_src; // Source tempo before randomization
	vector<float> stime; // Time of current step start in ms
	vector<float> etime; // Time of current step ending in ms
	vector< vector <DWORD> > smst; // Source midi start tick
	vector< vector <DWORD> > smet; // Source midi end tick
	vector< vector <float> > dstime; // Delta of current step start in ms for playback
	vector< vector <float> > detime; // Delta of current step ending in ms for playback
	vector< vector <float> > sstime; // Time of current step start in ms (exact in source midi file)
	vector< vector <float> > setime; // Time of current step end in ms (exact in source midi file)
	vector<CString> track_name; // Track names from midi file for each voice
	vector<int> track_id; // Track id from midi file for each voice
	vector<int> track_vid; // Voice id inside track from midi file for each voice
	vector<int> itrack; // Instrument local track number for each track
	int stages_calculated = 0;

	// CA1
	CString midi_file;

	// CA3
	CString musicxml_file;
	float voices_order_pitch = 0; // Difference in average pitch between adjacent voices to initiate voice swap. Set to 0 to disable swap

protected:
	void ValidateVectors(int step1, int step2);
	void ValidateVectors2(int step1, int step2);
	// File operations
	void SaveVector2C(ofstream & fs, vector<vector<unsigned char>>& v2D, int i);
	void SaveVector2S(ofstream & fs, vector<vector<unsigned short>>& v2D, int i);
	void SaveVector2Color(ofstream & fs, vector<vector<DWORD>>& v2D, int i);
	void SaveVector2ST(ofstream & fs, vector<vector<CString>>& v2D, int i);
	void SaveVector(ofstream &fs, vector<float> &v);
	void SaveVector(ofstream &fs, vector<DWORD> &v);
	void LoadVector2C(ifstream & fs, vector<vector<unsigned char>>& v2D, int i);
	void LoadVector2S(ifstream & fs, vector<vector<unsigned short>>& v2D, int i);
	void LoadVector2Color(ifstream & fs, vector<vector<DWORD>>& v2D, int i);
	void LoadVector2ST(ifstream & fs, vector<vector<CString>>& v2D, int i);
	void LoadVector(ifstream & fs, vector<DWORD>& v);
	void LoadVector(ifstream & fs, vector<float>& v);
	void LoadVector(ifstream & fs, vector<unsigned char>& v);
	// Helper functions for child generators
	void CountOff(int step1, int step2);
	void FixLen(int step1, int step2);
	void CountTime(int step1, int step2);
	void CountSTime(int step1, int step2);
	void CopyVoice(int v1, int v2, int step1, int step2, int interval);
	void UpdateNoteMinMax(int step1, int step2, int final_run = 1);
	void UpdateTempoMinMax(int step1, int step2);
	void AddNote(int pos, int v, char note2, int len2, int dyn2);
	void RegisterGraph(CString name, float scale);
	int GetPrevNote(int i, int v);
	int GetNextNote(int i, int v);
	inline void SetProgress(int i, int value);
	void AddMelody(int step1, int step2, int v, CString info, CString info2="", CString info3="");
	inline void ResizeVectors(int size, int vsize = -1);

	DWORD color_noflag; // Color for notes with no flags

	// Warnings
	int warn_progress = 0; // Progress too long
};

void CGVar::SetProgress(int i, int value) {
	if (i > MAX_PROGRESS) {
		if (!warn_progress) {
			WriteLog(5, "Exceeded progress size. Please increase progress size or check algorithm.");
			++warn_progress;
		}
		return;
	}
	progress[i] = value;
}