// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#pragma once
#include "GAdapt.h"

#define MAX_WARN_MIDI_AHEAD 5
// Maximum delay (ms) between transition and keyswitch
#define MAX_TRANS_DELAY 10
// Maximum time (ms) allowed to move note and linked events (ks/cc) left
#define MAX_AHEAD 1000
// Maximum number of init presteps
#define INIT_PRESTEPS 2
// Length of init prestep in ms
#define INIT_PRESTEP 30

// PortMIDI
#define MIN_MIDI_BUF_MSEC 6000
#define MAX_MIDI_BUF_MSEC 12000
#define MIDI_BUF_PROTECT 500 // Number of ms to postpone playback on start
#define TIME_START Pt_Start(1, 0, 0) /* timer started w/millisecond accuracy */

class MPort :
	public CGAdapt
{
public:
	MPort();
	~MPort();

	CMidiOut *mo;
	void StartMIDI(int midi_device_i, int from);
	void LogInstruments();
	void CheckDstime(int i, int v);
	void GetMidiPrePause();
	void SendMIDI(int step1, int step2);
	void SendPedalCC(int step1, int step2, int ii, int v);
	void InterpolateCC(int CC, float rnd, int step1, int step2, vector<vector<unsigned char>>& dv, int ii, int v);
	void StopMIDI();
	int GetPlayStep();

	int midi_sent = 0; // Steps already sent to midi
	long long midi_sent_t = 0; // Timestamp of last event sent to midi in previous SendMIDI
	long long midi_sent_t2 = 0; // Timestamp of last event sent to midi in current SendMIDI
	long long midi_sent_t3 = 0; // Timestamp of last event postponed in current SendMIDI
	PmMessage midi_sent_msg = 0; // Last event sent to midi in previous SendMIDI
	PmMessage midi_sent_msg2 = 0; // Last event sent to midi in current SendMIDI
	PmMessage midi_sent_msg3 = 0; // Last event postponed in current SendMIDI
	float midi_prepause = 0; // How long pause is needed before music
	int midi_current_step = 0; // Current step processed by SendMIDI (for logs)
	long long midi_start_time = 0; // Time when midi started to play
	int midi_last_run = 0; // If current SendMIDI is last
	int midi_first_run = 0; // If current SendMIDI is first
	int buf_underrun = 0; // Shows if current playback had an issue with buffer underrun
	int midi_play_step = 0; // Current step being played by midi
	vector<short> last_cc;

protected:
	vector<vector<vector<PmEvent>>> midifile_buf;
	int amidi_export = 0; // Exporting adapted midi
	int warning_ahead = 0;

	// PortMIDI internal
	void AddMidiEvent(long long timestamp, int mm_type, int data1, int data2);
	void AddNoteOn(long long timestamp, int data1, int data2);
	void AddKsOn(long long timestamp, int data1, int data2);
	void AddNoteOff(long long timestamp, int data1, int data2);
	void AddKsOff(long long timestamp, int data1, int data2);
	void AddCC(long long timestamp, int data1, int data2);
	void AddKs(long long stimestamp, int ks);
	void AddTransitionCC(int i, long long stimestamp, int CC, int value1, int value2);
	void AddTransitionKs(int i, long long timestamp, int ks);
	static int PmEvent_comparator(const void *v1, const void *v2);

	// Information for current note in SendMIDI
	vector <PmEvent> midi_buf;
	vector <PmEvent> midi_buf_next; // Buffer for next SendMIDI run
	long long midi_buf_lim = 0; // After this timestamp information goes to midi_buf_next
	int midi_channel = 0;
	int midi_channel_saved = 0;
	int midi_track = 0;
	int midi_stage = 0;
	int midi_voice = 0;
	int midi_sending_buf_next = 0;
};

