// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "CF1M.h"
#include "../midifile/MidiFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

CF1M::CF1M() {
}

CF1M::~CF1M() {
}

void CF1M::LoadCantus(CString path)
{
	long long time_start = CGLib::time();
	if (!fileExists(path)) {
		CString est;
		est.Format("Cannot find file %s", path);
		WriteLog(5, est);
		return;
	}
	if (!FileHasHeader(path, "MThd")) {
		CString est;
		est.Format("This file does not have MIDI header: %s", path);
		WriteLog(5, est);
		return;
	}
	MidiFile midifile;
	if (!midifile.read(path)) {
		CString est;
		est.Format("Error reading midi file %s", path);
		WriteLog(5, est);
		return;
	}
	midifile.linkNotePairs();
	//midifile.joinTracks();
	midifile.doTimeAnalysis();

	midifile.absoluteTicks();

	in_ppq = midifile.getTicksPerQuarterNote();
	// ticks per croche
	int tpc = (float)in_ppq / (float)2 / (float)midifile_in_mul;

	vector <float> tempo2;
	long tempo_count = 0;
	int last_step = 0;
	// Load tempo
	for (int track = 0; track < midifile.getTrackCount(); track++) {
		for (int i = 0; i < midifile[track].size(); i++) {
			MidiEvent* mev = &midifile[track][i];
			int pos = round(mev->tick / (float)tpc);
			if (pos >= tempo_count) {
				tempo_count = pos + 1;
				tempo2.resize(tempo_count);
			}
			if (mev->isTempo()) {
				tempo2[pos] = mev->getTempoBPM() * midifile_in_mul;
			}
			if (pos > last_step) last_step = pos;
		}
	}
	// Fill tempo
	for (int z = 1; z < last_step; z++) {
		if (tempo2[z] == 0) tempo2[z] = tempo2[z - 1];
	}

	int cid = 0;
	int nid = 0;
	vector<CString> incom; // Incoming comments
	vector <int> c;
	vector <int> cl;
	vector <float> ct;
	int bad = 0;
	int last_pos = -1;
	CString lyrics_pending;
	for (int track = 0; track < midifile.getTrackCount(); track++) {
		float last_tick = 0;
		for (int i = 0; i<midifile[track].size(); i++) {
			MidiEvent* mev = &midifile[track][i];
			float pos2 = mev->tick;
			int pos = round(mev->tick / (float)tpc);
			//float time = midifile.getTimeInSeconds(mev->tick);
			if (mev->isMetaMessage()) {
				// Lyrics
				if (mev->getMetaType() == 5) {
					CString st;
					st.Empty();
					for (int x = 0; x < mev->size(); x++) {
						st += mev->data()[x];
					}
					// Remove first data items
					st = st.Mid(3);
					st = st.Trim();
					st.MakeLower();
					// Assign lyrics if this position was already sent
					if (pos == last_pos) {
						incom.resize(c.size());
						if (c.size()) {
							if (!incom[c.size() - 1].IsEmpty()) incom[c.size() - 1] += ",";
							incom[c.size() - 1] += st;
						}
						else {
							CString est;
							est.Format("Error assigning lyrics '%s' for already sent position %d", st, pos);
							WriteLog(5, est);
						}
						lyrics_pending.Empty();
					}
					// Else record lyrics
					else {
						lyrics_pending = st;
					}
				}
			}
			if (mev->isNoteOn()) {
				int tick_dur = mev->getTickDuration();
				float nlen2 = tick_dur;
				int nlen = round((mev->tick + tick_dur) / (float)tpc) - pos;
				// Check for pause
				if (pos2 - last_tick > (float)tpc / 2) {
					// Add cantus if it is long
					if (nid >= MIN_CANTUS_SIZE && !bad) {
						cantus.push_back(c);
						cantus_len.push_back(cl);
						cantus_tempo.push_back(ct);
						cantus_incom.push_back(incom);
						//lyrics_pending.Empty();
					}
					else {
						if (nid < MIN_CANTUS_SIZE && nid > 0) {
							CString st;
							st.Format("Melody #%d is shorter (%d steps) than minimum length (%d steps): tick %d, track %d, chan %d, tpc %d (mul %.03f) in file %s",
								cantus.size(), nid, (int)MIN_CANTUS_SIZE, mev->tick, track, mev->getChannel(), tpc, midifile_in_mul, path);
							WriteLog(5, st);
						}
					}
					// Go to next cantus
					nid = 0;
				}
				if (nid == 0) {
					bad = 0;
					// Add new cantus
					cid++;
					c.clear();
					cl.clear();
					ct.clear();
					incom.clear();
				}
				// Add new note
				if ((nid == 0) || (c[nid - 1] != mev->getKeyNumber())) {
					// Check if current note already set
					if (!nlen) {
						if (warning_loadmidi_short < MAX_WARN_MIDI_SHORT) {
							CString st;
							st.Format("Note too short: tick %d, track %d, chan %d, tpc %d (mul %.03f) in file %s. Increasing midifile_in_mul will improve approximation.", mev->tick, track, mev->getChannel(), tpc, midifile_in_mul, path);
							WriteLog(5, st);
							warning_loadmidi_short++;
						}
						bad = 1;
					}
					int nlen = round((mev->tick + tick_dur) / (float)tpc) - pos;
					// Check if note too long
					if (nlen > MAX_LEN) {
						if (warning_loadmidi_long < MAX_WARN_MIDI_LONG) {
							CString st;
							st.Format("Note too long: tick %d, track %d, chan %d, tpc %d (mul %.03f) in file %s. Decreasing midifile_in_mul can help.", mev->tick, track, mev->getChannel(), tpc, midifile_in_mul, path);
							WriteLog(5, st);
							warning_loadmidi_long++;
						}
						nlen = MAX_LEN;
						bad = 1;
					}
					// Avoid repeats
					if (c.size() == 0 || c[c.size() - 1] != mev->getKeyNumber()) {
						c.push_back(mev->getKeyNumber());
						cl.push_back(nlen);
						ct.push_back(tempo2[pos]);
						// Add pending lyrics
						if (!lyrics_pending.IsEmpty()) {
							incom.resize(c.size());
							incom[c.size() - 1] = lyrics_pending;
							lyrics_pending.Empty();
						}
						nid++;
					}
				}
				// Save last time
				last_tick = pos2 + nlen2;
				last_pos = pos;
			}
		}
		// Add cantus if it is long
		if (nid > 5 && !bad) {
			cantus.push_back(c);
			cantus_len.push_back(cl);
			cantus_tempo.push_back(ct);
			cantus_incom.push_back(incom);
			nid = 0;
		}
		else {
			if (nid < MIN_CANTUS_SIZE && nid > 0) {
				CString st;
				st.Format("Melody #%d is shorter (%d steps) than minimum length (%d steps): tick %.0f, track %d, tpc %d (mul %.03f) in file %s. Not loaded.",
					cantus.size(), nid, (int)MIN_CANTUS_SIZE, last_tick, track, tpc, midifile_in_mul, path);
				WriteLog(5, st);
			}
		}
	}
	// Count time
	long long time_stop = CGLib::time();
	CString st;
	st.Format("LoadCantus successfully loaded %d canti (in %lld ms)", cid + 1, time_stop - time_start);
	WriteLog(0, st);
}

