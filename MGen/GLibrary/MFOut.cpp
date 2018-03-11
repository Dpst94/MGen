// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "MFOut.h"

#include "../midifile/MidiFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

MFOut::MFOut() {
}

MFOut::~MFOut() {
}

void MFOut::ExportAdaptedMidi(CString dir, CString fname) {
	long long time_start = CGLib::time();
	// If generation finished set last run
	midi_last_run = 1;
	StopMIDI();
	amidi_export = 1;
	midifile_buf.clear();
	midifile_buf.resize(MAX_STAGE);
	for (int i = 0; i < MAX_STAGE; ++i) {
		midifile_buf[i].resize(MAX_VOICE);
	}
	SendMIDI(midi_sent, t_sent);
	// Get maximum stage
	int stage_max = vmax(v_stage) + 1;
	vector <int> stracks_cnt;
	stracks_cnt.resize(MAX_STAGE);
	int tracks_cnt = 0;
	// Get maximum used track
	for (int sta = 0; sta < stage_max; ++sta) {
		for (int tr = MAX_VOICE - 1; tr >= 0; --tr) {
			if (midifile_buf[sta][tr].size()) {
				if (!stracks_cnt[sta]) stracks_cnt[sta] = tr + 1;
				++tracks_cnt;
			}
		}
	}
	if (!tracks_cnt) return;
	// Save to file
	MidiFile midifile;
	vector <MidiFile> smidifile;
	smidifile.resize(stage_max);
	float tps = 200;                // ticks per second
	float spq = 0.5;              // Seconds per quarter note
	int tpq = tps / spq;          // ticks per quarter note
	int track = 0;
	int strack = 0; // stage track
	int channel = 0;
	int tick = 0;
	int type, data1, data2;
	CString st2;
	// Add some expression track (track 0) messages:
	string st = fname;
	for (int sta = 0; sta < stage_max; ++sta) {
		smidifile[sta].setTicksPerQuarterNote(tpq);
		smidifile[sta].addTracks(stracks_cnt[sta] + 1);
		smidifile[sta].addTrackName(0, 0, st);
		smidifile[sta].addTempo(track, 0, 60 / spq);
	}
	midifile.addTracks(tracks_cnt + 1);
	midifile.setTicksPerQuarterNote(tpq);
	midifile.addTrackName(0, 0, st);
	midifile.addTempo(track, 0, 60 / spq);
	track = 0;
	for (int sta = 0; sta < stage_max; ++sta) {
		for (int tr = 0; tr < MAX_VOICE; ++tr) {
			// 1 = note is ON
			vector< vector <int> > note_on = vector<vector<int>>(16, vector<int>(128));
			if (!midifile_buf[sta][tr].size()) continue;
			// Sort by timestamp before sending
			qsort(midifile_buf[sta][tr].data(), midifile_buf[sta][tr].size(), sizeof(PmEvent), PmEvent_comparator);
			// Convert DAW track to midi file track number
			strack = tr + 1;
			++track;
			// Send instrument name
			CString st;
			if (sta == 0) st = icf[t_instr[tr]].group;
			else st.Format("%s %d", icf[t_instr[tr]].group, sta);
			string st2 = st;
			midifile.addTrackName(track, 0, st2);
			smidifile[sta].addTrackName(strack, 0, st2);
			//midifile.addPatchChange(track, 0, channel, 0); // 0=piano, 40=violin, 70=bassoon
			//smidifile[sta].addPatchChange(strack, 0, channel, 0); // 0=piano, 40=violin, 70=bassoon
			long long ts;
			long long max_ts = 0;
			for (int i = 0; i < midifile_buf[sta][tr].size(); i++) {
				ts = max(0, midifile_buf[sta][tr][i].timestamp);
				if (toload_time && ts > toload_time * 1000) continue;
				if (ts > max_ts) max_ts = ts;
				tick = ts / 1000.0 / spq * tpq;
				type = Pm_MessageStatus(midifile_buf[sta][tr][i].message) & 0xF0;
				data1 = Pm_MessageData1(midifile_buf[sta][tr][i].message);
				data2 = Pm_MessageData2(midifile_buf[sta][tr][i].message);
				channel = Pm_MessageStatus(midifile_buf[sta][tr][i].message) & 0x0F;
				if (type == MIDI_NOTEON) {
					if (data2) {
						midifile.addNoteOn(track, tick, channel, data1, data2);
						smidifile[sta].addNoteOn(strack, tick, channel, data1, data2);
						note_on[channel][data1] = 1;
					}
					else {
						midifile.addNoteOff(track, tick, channel, data1, 0);
						smidifile[sta].addNoteOff(strack, tick, channel, data1, 0);
						note_on[channel][data1] = 0;
					}
				}
				if (type == MIDI_CC) {
					midifile.addController(track, tick, channel, data1, data2);
					smidifile[sta].addController(strack, tick, channel, data1, data2);
				}
			}
			// Send note off
			if (toload_time) ts = toload_time * 1000;
			tick = ts / 1000.0 / spq * tpq;
			for (int c = 0; c < 16; ++c) {
				for (int i = 0; i < 128; ++i) if (note_on[c][i]) {
					midifile.addNoteOff(track, tick, c, i, 0);
					smidifile[sta].addNoteOff(strack, tick, c, i, 0);
				}
			}
			// Send tail
			ts = max_ts + EXPORT_MIDI_TAIL;
			if (toload_time) ts = toload_time * 1000 + EXPORT_MIDI_TAIL;
			tick = ts / 1000.0 / spq * tpq;
			smidifile[sta].addNoteOff(strack, tick, 0, 0, 0);
			midifile.addNoteOff(track, tick, 0, 0, 0);
		}
	}
	for (int sta = 0; sta < stage_max; ++sta) {
		smidifile[sta].sortTracks();         // ensure tick times are in correct order
		st2.Format(dir + "\\" + fname + "_%d.midi", sta);
		smidifile[sta].write(st2);
	}
	midifile.sortTracks();         // ensure tick times are in correct order
	midifile.write(dir + "\\" + fname + ".midi");

	amidi_export = 0;
	// Log
	long long time_stop = CGLib::time();
	CString est;
	est.Format("Exported adapted midi file %s in %lld ms",
		as_fname, time_stop - time_start);
	WriteLog(0, est);
}

void MFOut::SaveMidi(CString dir, CString fname) {
	MidiFile midifile;
	midifile.addTracks(v_cnt);    // Add another two tracks to the MIDI file
	int tpq = 120;                // ticks per quarter note
	int tpc = 60; // ticks per croche
	midifile.setTicksPerQuarterNote(tpq);
	int track = 0;
	int channel = 0;
	int pos = tpq * 4;
	// Add some expression track (track 0) messages:
	string st = fname;
	midifile.addTrackName(track, 0, st);
	// Save tempo
	midifile.addTempo(track, 0, tempo[0]);
	for (int i = 0; i < t_generated; i++) {
		midifile.addTempo(track, pos, tempo[i] * midifile_out_mul[i]);
		pos += tpc * midifile_out_mul[i];
	}
	// Save notes
	for (int v = 0; v < v_cnt; v++) {
		track = v + 1;
		if (track_id[v]) track = track_id[v];
		channel = v % 16;
		string st = icf[instr[v]].group;
		// Replace piano with other instrument, because otherways it generates two-stave track in Sibelius
		if (st == "Piano") st = "Vibraphone";
		midifile.addTrackName(track, 0, st);
		//if (ngv_min[v] < 57) midifile.addPatchChange(track, 0, channel, bass_program[v]); // 0=piano, 40=violin, 70=bassoon
		midifile.addPatchChange(track, 0, channel, 0); // 0=piano, 40=violin, 70=bassoon
		pos = tpq * 4;
		for (int i = 0; i < t_generated; i++) {
			if (pause[i][v]) {
				pos += tpc * midifile_out_mul[i];
				continue;
			}
			midifile.addNoteOn(track, pos, channel, note[i][v], dyn[i][v]);
			midifile.addNoteOff(track, pos + tpc*midifile_out_mul[i] * (len[i][v]) - 1, channel, note[i][v], 0);
			if (midifile_export_comments && !comment2[i][v].IsEmpty()) {
				string st;
				st = comment2[i][v];
				midifile.addLyric(track, pos, st);
			}
			if (midifile_export_marks && !mark[i][v].IsEmpty()) {
				string st;
				st = mark[i][v];
				midifile.addLyric(track, pos, st);
			}
			if (noff[i][v] == 0) break;
			pos += tpc * midifile_out_mul[i] * noff[i][v];
			i += noff[i][v] - 1;
		}
	}
	midifile.sortTracks();         // ensure tick times are in correct order
	midifile.write(dir + "\\" + fname + ".mid");
}

