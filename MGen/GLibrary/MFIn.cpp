// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "MFIn.h"

#include "../midifile/MidiFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

MFIn::MFIn() {
}

MFIn::~MFIn() {
}

void MFIn::LoadMidi(CString path)
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
	//midifile.joinTracks();
	midifile.linkNotePairs();
	midifile.doTimeAnalysis();

	if (midifile.getTrackCount() < 2) {
		CString st;
		st.Format("Detected only %d tracks while loading file %s. Probably MIDI type 0. Splitting midi tracks by channels. Track names are not supported for MIDI type 0 yet.", midifile.getTrackCount(), path);
		WriteLog(1, st);
		midifile.splitTracksByChannel();
		midifile_type = 0;
	}

	midifile.absoluteTicks();
	in_ppq = midifile.getTicksPerQuarterNote();
	int tpc = max(1, (float)in_ppq / (float)2 / (float)midifile_in_mul); // ticks per croche
	vector<int> vlast_step(MAX_VOICE);
	vector<int> vlast_pitch(MAX_VOICE);
	vector<int> voverlap(MAX_VOICE);
	vector<int> vdist(MAX_VOICE);
	CString st, tnames = "", inames = "";
	// Convert track instrument ids to voice instrument ids
	vector<int> instr2 = instr;

	// Autolegato with gaps, if grownotes was not set
	if (midi_file_type == mftFIN && grow_notes == 0 && auto_legato == 1) {
		grow_notes = in_ppq * 0.09;
	}
	if (midi_file_type == mftMUS && grow_notes == 0 && auto_legato == 1) {
		grow_notes = in_ppq * 0.09;
	}
	midifile_loaded = 1;
	int last_step = 0;
	// If there is no tempo in file, set default
	tempo[0] = 120;
	int first_track = -1;
	vector<int> track_firstchan;
	track_firstchan.resize(midifile.getTrackCount(), -1);
	// Load tempo
	for (int track = 0; track < midifile.getTrackCount(); track++) {
		for (int i = 0; i < midifile[track].size(); i++) {
			MidiEvent* mev = &midifile[track][i];
			int pos = round(mev->tick / (float)tpc);
			if (mev->isNoteOn() || mev->isController()) {
				if (track_firstchan[track] == -1)
					track_firstchan[track] = mev->getChannel();
			}
			if (mev->isTempo()) {
				if (pos >= t_allocated) ResizeVectors(max(t_allocated * 2, pos + 1));
				tempo[pos] = mev->getTempoBPM() * midifile_in_mul;
				if (pos > last_step) last_step = pos;
			}
			if (mev->isNoteOn() && first_track == -1) first_track = track;
		}
	}
	if (first_track == -1) first_track = 0;
	// Fill tempo
	for (int z = 1; z <= last_step; z++) {
		if (tempo[z] == 0) tempo[z] = tempo[z - 1];
	}
	int last_step_tempo = last_step;
	UpdateTempoMinMax(0, last_step);
	CountTime(0, last_step);
	last_step = 0;
	int v1 = 0;
	int v2 = 0;
	int v = 0;

	vector<int> mute_active;
	vector<int> marc_active;
	vector<int> pizz_active;
	vector<int> trem_active;
	vector<int> spic_active;
	vector<int> stac_active;
	vector<int> tasto_active;
	mute_active.resize(16);
	marc_active.resize(16);
	pizz_active.resize(16);
	trem_active.resize(16);
	spic_active.resize(16);
	stac_active.resize(16);
	tasto_active.resize(16);

	for (int track = first_track; track < midifile.getTrackCount(); track++) {
		if (need_exit) break;
		int last_cc1_step = -1;
		vfill(mute_active, 0);
		vfill(marc_active, 0);
		vfill(pizz_active, 0);
		vfill(trem_active, 0);
		vfill(spic_active, 0);
		vfill(stac_active, 0);
		vfill(tasto_active, 0);
		if (track > first_track) {
			// Get next free voice
			v1 = v2 + 1;
			// Voice interval = 1
			v2 = v1;
			// Current voice is first voice in interval
			v = v1;
			if (v >= MAX_VOICE) {
				CString st;
				st.Format("Too many voices need to be created for loading file %s. Maximum number of voices %d. Increase MAX_VOICE", path, MAX_VOICE);
				WriteLog(5, st);
				break;
			}
			// Resize vectors for new voice number
			if (v > v_cnt - 1) ResizeVectors(t_allocated, v + 1);
		}
		// Save track id
		track_id[v] = track - first_track + 1;
		track_vid[v] = 0;
		// Convert track instrument to voice instrument
		instr[v] = instr2[track_id[v] - 1];
		for (int i = 0; i<midifile[track].size(); i++) {
			if (need_exit) break;
			MidiEvent* mev = &midifile[track][i];
			int chan = mev->getChannel();
			// Get track names
			if (mev->isMetaMessage()) {
				if (mev->getMetaType() == 0x03) {
					track_name[v].Empty();
					for (int x = 0; x < mev->size(); x++) {
						track_name[v] += mev->data()[x];
					}
					// Remove first data items
					track_name[v] = track_name[v].Mid(3);
					st.Format("%d", v);
					tnames += " \n" + st + "=" + track_name[v];
					// Map track name to instrument name
					for (int i = 0; i < icf.size(); i++) {
						// Exact match
						//if (InstName[i] == track_name[v]) instr[v] = i;
						// Search inside track name
						//else if (track_name[v].Find(InstName[i], 0) != -1) instr[v] = i;
					}
				}
			}
			if (mev->isPatchChange()) {
				// Get program changes for MuseScore files
				if (track_name[v].IsEmpty()) {
					track_name[v] = midi_iname[mev->data()[1]];
				}
				// Get program changes for Sibelius files
				int patch = mev->data()[1];
				if (patch == 59) mute_active[chan] = 1;
				else if (patch == 45) pizz_active[chan] = 1;
				else {
					mute_active[chan] = 0;
					marc_active[chan] = 0;
					pizz_active[chan] = 0;
					trem_active[chan] = 0;
					spic_active[chan] = 0;
					stac_active[chan] = 0;
					tasto_active[chan] = 0;
				}
			}
			if (mev->isController()) {
				int pos = round(mev->tick / (float)tpc);
				int cc = mev->data()[1];
				int val = mev->data()[2];
				if (pos + 1 >= t_allocated) ResizeVectors(max(pos + 1, t_allocated * 2));
				if (cc == 1) {
					// Fill cc1
					if (last_cc1_step > -1) {
						for (int z = last_cc1_step + 1; z < pos; ++z) {
							dyn[z][v1] = dyn[z - 1][v1];
						}
					}
					dyn[pos][v1] = val;
					last_cc1_step = pos;
				}
				if (cc == 64 && icf[instr[v]].pedal_import) {
					if (val > 63) {
						SetBit(filter[pos][v], fPEDAL);
					}
					else {
						SetBit(filter[pos][v], fUNPEDAL);
					}
				}
			}
			if (mev->isNoteOn()) {
				int pos = round(mev->tick / (float)tpc);
				int pitch = mev->getKeyNumber();
				int myvel = mev->getVelocity();
				int tick_dur = mev->getTickDuration();
				if (grow_notes > -1 && icf[instr[v]].poly == 1) tick_dur += grow_notes;
				int nlen = round((mev->tick + tick_dur) / (float)tpc) - pos;
				// Parse keyswitch
				if (pitch < icf[instr[v]].import_min || pitch > icf[instr[v]].import_max) {
					if (pitch < 12) {
						vfill(mute_active, 0);
						vfill(marc_active, 0);
						vfill(pizz_active, 0);
						vfill(trem_active, 0);
						vfill(spic_active, 0);
						vfill(stac_active, 0);
						vfill(tasto_active, 0);
					}
					if (pitch == 2) vfill(mute_active, 1);
					if (pitch == 3) vfill(marc_active, 1);
					if (pitch == 5) vfill(pizz_active, 1);
					if (pitch == 7) vfill(trem_active, 1);
					if (pitch == 9) vfill(spic_active, 1);
					if (pitch == 10) vfill(stac_active, 1);
					if (pitch == 11) vfill(tasto_active, 1);
				}
				// Parse normal note
				else {
					// Check if note too long
					if (nlen > MAX_LEN) {
						if (warning_loadmidi_long < MAX_WARN_MIDI_LONG) {
							CString st;
							st.Format("Note too long and will be cut short at %d track %d tick with %d tpc (mul %.03f) approximated to %d step in file %s. Decrease midifile_in_mul can resolve this situation.", track, mev->tick, tpc, midifile_in_mul, pos, path);
							WriteLog(1, st);
							warning_loadmidi_long++;
						}
						nlen = MAX_LEN;
					}
					if (nlen < 1) nlen = 1;
					// Allocate one more step for note overwrite checking
					if (pos + nlen + 1 >= t_allocated) ResizeVectors(max(pos + nlen + 1, t_allocated * 2));
					// Fill tempo
					if (!tempo[pos + nlen]) {
						for (int z = last_step_tempo + 1; z < pos + nlen + 1; ++z) {
							if (!tempo[z]) tempo[z] = tempo[z - 1];
						}
						// Count new time
						CountTime(last_step_tempo + 1, pos + nlen - 1);
						// Set last step that has tempo
						last_step_tempo = pos + nlen - 1;
					}
					// Fallback
					if (!tempo[pos]) tempo[pos] = 100;
					float delta = (float)(mev->tick - pos*tpc) / (float)tpc * 30000.0 / (float)tempo[pos];
					float delta2 = (float)(mev->tick + tick_dur - (pos + nlen)*tpc) /
						(float)tpc * 30000.0 / (float)tempo[pos + nlen];
					// Find overlaps and distance
					if (icf[instr[v]].poly > 1) {
						for (int x = v1; x <= v2; ++x) {
							// Overlap happens only in case when positions overlap
							if (note[pos][x]) {
								voverlap[x] = 1;
								vdist[x] = 1000;
								// Check if note too short
								if (len[pos][x] < 2) {
									if (warning_loadmidi_short < MAX_WARN_MIDI_SHORT) {
										CString st;
										st.Format("Note %s too short and gets same step with next note %s at %d track, %d tick with %d tpc (mul %.03f) approximated to %d step in file %s. Increasing midifile_in_mul will improve approximation.", GetNoteName(note[pos][x]), GetNoteName(pitch), track, mev->tick, tpc, midifile_in_mul, pos, path);
										WriteLog(1, st);
										warning_loadmidi_short++;
									}
								}
							}
							else {
								voverlap[x] = 0;
								vdist[x] = abs(vlast_pitch[x] - pitch);
							}
						}
						// Find best voice
						int min_vdist = 1000;
						for (int x = v1; x <= v2; ++x) {
							if (vdist[x] < min_vdist) {
								min_vdist = vdist[x];
								v = x;
							}
						}
						// If no voice without overlaps, create new
						if (min_vdist == 1000) {
							v2++;
							v = v2;
							// Copy instrument
							instr[v] = instr[v1];
							if (v >= MAX_VOICE) {
								CString st;
								st.Format("Too many voices need to be created for loading file %s. Maximum number of voices %d. Increase MAX_VOICE", path, MAX_VOICE);
								WriteLog(5, st);
								break;
							}
							track_id[v] = track - first_track + 1;
							track_vid[v] = v - v1;
							track_name[v] = track_name[v1];
						}
					} // if (instr_poly[instr[v]] > 1)
					else {
						// Find lowest voice without note start
						if (icf[instr[v]].divisi_auto) {
							int found = 0;
							for (v = v1; v <= v2; ++v) {
								if (coff[pos][v] || pause[pos][v] || !note[pos][v]) {
									found = 1;
									break;
								}
							}
							// If no voice without overlaps, create new
							if (!found) {
								v2++;
								v = v2;
								// Copy instrument
								instr[v] = instr[v1];
								if (v >= MAX_VOICE) {
									CString st;
									st.Format("Too many voices need to be created for loading file %s. Maximum number of voices %d. Increase MAX_VOICE", path, MAX_VOICE);
									WriteLog(5, st);
									break;
								}
								track_id[v] = track - first_track + 1;
								track_vid[v] = v - v1;
								track_name[v] = track_name[v1];
								// Resize vectors for new voice number
								if (v > v_cnt - 1) ResizeVectors(t_allocated, v + 1);
							}
						}
						// Check if overwriting long overlap
						if (!pause[pos][v] && noff[pos][v]) {
							// Update previous note ending
							if (pos) {
								setime[pos - 1][v] = stime[pos] + delta;
								smet[pos - 1][v] = smet[pos - 1 + noff[pos - 1][v] - 1][v];
							}
							// Using stime/etime here, because this check is approximate
							float ndur = etime[pos + nlen - 1] - stime[pos];
							float ndur2 = etime[pos + noff[pos][v] - 1] - stime[pos - coff[pos][v]];
							// Calculate overlap (abs is protection from bugs)
							float ov = abs(etime[pos + noff[pos][v] - 1] - stime[pos]);
							// Is overlap long?
							if (ov > ndur * MAX_OVERLAP_MONO || ov > ndur2 * MAX_OVERLAP_MONO) if (warning_loadmidi_overlap < MAX_WARN_MIDI_OVERLAP) {
								CString st;
								st.Format("Error: too long overlap (voice %d) %.0f ms at step %d (note lengths %.0f, %.0f ms) in monophonic instrument %s/%s. Probably sending polyphonic instrument to monophonic.",
									v, ov, pos, ndur, ndur2, icf[instr[v]].group, icf[instr[v]].name);
								WriteLog(0, st);
								++warning_loadmidi_overlap;
							}
						}
						// Clear any garbage after this note (can build up due to overwriting a longer note)
						if (len[pos + nlen][v]) {
							// Safe right limit
							for (int z = pos + nlen; z < len.size(); ++z) {
								// Stop clearing if current step is free
								if (!len[z][v]) break;
								// Clear step
								len[z][v] = 0;
								note[z][v] = 0;
								pause[z][v] = 1;
								vel[z][v] = 0;
								coff[z][v] = 0;
							}
						}
					}
					// Resize vectors for new voice number
					if (v > v_cnt - 1) ResizeVectors(t_allocated, v + 1);
					// Search for last note
					if ((pos > 0) && (note[pos - 1][v] == 0)) {
						int last_pause = pos - 1;
						for (int z = pos - 1; z >= 0; z--) {
							if (note[z][v] != 0) break;
							last_pause = z;
						}
						// Set previous pause
						FillPause(last_pause, pos - last_pause - 1, v);
						// Set additional variables
						CountOff(last_pause, pos - 1);
					}
					// Set note steps
					for (int z = 0; z < nlen; z++) {
						note[pos + z][v] = pitch;
						len[pos + z][v] = nlen;
						vel[pos + z][v] = myvel;
						midi_ch[pos + z][v] = chan;
						pause[pos + z][v] = 0;
						coff[pos + z][v] = z;
						if (trem_active[chan] && icf[instr[v]].trem_import) artic[pos + z][v] = aTREM;
						if (pizz_active[chan] && icf[instr[v]].pizz_import)
							artic[pos + z][v] = aPIZZ;
						if (spic_active[chan] && icf[instr[v]].spic_import) artic[pos + z][v] = aSTAC;
						if (stac_active[chan] && icf[instr[v]].stac_import) artic[pos + z][v] = aSTAC;
						if (marc_active[chan] && icf[instr[v]].marc_import) artic[pos + z][v] = aSTAC;
						if (mute_active[chan] && icf[instr[v]].mute_import) SetBit(filter[pos + z][v], fMUTE);
						if (tasto_active[chan] && icf[instr[v]].tasto_import) SetBit(filter[pos + z][v], fTASTO);
						// Load MuseScore articulations
						if (midi_file_type == mftMUS) {
							int dchan = (chan - track_firstchan[track] + 16) % 16;
							if (dchan == 1 && icf[instr[v]].mute_import) SetBit(filter[pos + z][v], fMUTE);
							if (dchan == 1 && icf[instr[v]].pizz_import) {
								artic[pos + z][v] = aPIZZ;
							}
							if (dchan == 2 && icf[instr[v]].trem_import) artic[pos + z][v] = aTREM;
						}
						// Lock tremolo
						if (icf[instr[v]].trem_lock) artic[pos + z][v] = aTREM;
						// Lock mute
						if (icf[instr[v]].mute_lock) SetBit(filter[pos + z][v], fMUTE);
						// Lock bow
						if (icf[instr[v]].bow_lock == 1) {
							SetBit(filter[pos + z][v], fTASTO);
							ClearBit(filter[pos + z][v], fPONT);
						}
						if (icf[instr[v]].bow_lock == 2) {
							SetBit(filter[pos + z][v], fPONT);
							ClearBit(filter[pos + z][v], fTASTO);
						}
					}
					// Set midi ticks
					smst[pos][v] = mev->tick;
					smet[pos + nlen - 1][v] = mev->tick + tick_dur;
					// Set midi delta only to first step of note, because in in-note steps you can get different calculations for different tempo
					//midi_delta[pos][v] = delta;
					sstime[pos][v] = stime[pos] + delta;
					setime[pos + nlen - 1][v] = etime[pos + nlen - 1] + delta2;
					// Set additional variables
					CountOff(pos, pos + nlen - 1);
					UpdateNoteMinMax(pos, pos + nlen - 1);
					if (pos + nlen - 1 > last_step) last_step = pos + nlen - 1;
					if (pos + nlen - 1 > vlast_step[v]) vlast_step[v] = pos + nlen - 1;
					if (t_generated < pos) t_generated = pos;
					// Save last pitch
					vlast_pitch[v] = pitch;
				}
			}
		}
		// If track is empty, create a single pause
		if (!note[0][v] && !pause[0][v] && !len[0][v]) {
			FillPause(0, 1, v);
		}
		// Fill cc1 in first voice
		if (last_cc1_step > -1) {
			for (int z = last_cc1_step + 1; z <= last_step; ++z) {
				dyn[z][v1] = dyn[z - 1][v1];
			}
			last_cc1_step = last_step;
		}
		// Copy cc1 to all voices of current track
		for (int v = v1 + 1; v <= v2; ++v) {
			for (int z = 0; z <= last_step; ++z) {
				dyn[z][v] = dyn[z][v1];
			}
		}
		// Overwrite dynamics with vel where dynamics does not make sense
		for (int v = v1; v <= v2; ++v) {
			for (int z = 0; z <= last_step; ++z) {
				if (artic[z][v] == aPIZZ || !dyn[z][v]) dyn[z][v] = vel[z][v];
			}
		}
	} // for track
	if (need_exit) return;
	// Add closing pauses
	if (last_step + TAIL_STEPS + 1 >= t_allocated)
		ResizeVectors(max(last_step + TAIL_STEPS + 1, t_allocated * 2));
	for (int z = last_step + 1; z <= last_step + TAIL_STEPS; ++z) {
		if (!tempo[z]) tempo[z] = tempo[z - 1];
	}
	// Count new time
	CountTime(last_step + 1, last_step + TAIL_STEPS);
	last_step = last_step + TAIL_STEPS;
	for (int v = 0; v < v_cnt; v++) {
		if (vlast_step[v] < last_step) {
			int len2 = last_step - vlast_step[v];
			FillPause(vlast_step[v] + 1, len2, v);
		}
	}
	//MergeSmallOverlaps(0, last_step);
	// Check length of notes is correct
	FixLen(0, last_step);
	// Set additional variables
	CountOff(0, last_step);
	//CountTime(0, last_step);
	UpdateNoteMinMax(0, last_step);
	UnisonMute(0, last_step);
	//UpdateTempoMinMax(0, last_step);
	// Send last
	t_generated = last_step + 1;
	if (tnames != "") {
		CString est;
		est.Format("MIDI file track names: %s", tnames);
		WriteLog(0, est);
	}
	if (inames != "") {
		CString est;
		est.Format("MIDI file instrument names: %s", inames);
		WriteLog(0, est);
	}
	// Count time
	long long time_stop = CGLib::time();
	CString est;
	est.Format("LoadMidi successfully loaded %d steps (in %lld ms)",
		t_generated, time_stop - time_start);
	WriteLog(0, est);
}

void MFIn::UnisonMute(int step1, int step2) {
	for (int v = 0; v < v_cnt; v++) {
		// Do not check if instrument does not support unison muting
		if (!icf[instr[v]].unis_mute) continue;
		for (int v2 = v + 1; v2 < v_cnt; v2++) {
			if (icf[instr[v]].group != icf[instr[v2]].group) continue;
			if (icf[instr[v]].name != icf[instr[v2]].name) continue;
			for (int i = step1; i <= step2; ++i) {
				if (coff[i][v]) continue;
				if (coff[i][v2]) continue;
				if (note_muted[i][v]) continue;
				if (pause[i][v]) continue;
				if (pause[i][v2]) continue;
				if (note[i][v] != note[i][v2]) continue;
				if (len[i][v] != len[i][v2]) continue;
				note_muted[i][v2] = 1;
				// Increase dynamics of staying note
				for (int x = i; x < i + len[i][v]; ++x) {
					dyn[x][v] = min(127, dyn[x][v] * icf[instr[v]].unis_dyn_mul);
				}
				if (warning_unison_mute < MAX_WARN_UNISON_MUTE) {
					++warning_unison_mute;
					CString est;
					est.Format("Muted note at step %d voice %d because it is unison with voice %d",
						i, v, v2);
					WriteLog(0, est);
				}
			}
		}
	}
}

void MFIn::MergeSmallOverlaps(int step1, int step2) {
	// Merge small overlaps
	for (int i = step1; i <= step2; ++i) {
		// Cycle through steps to ensure that moved note is checked later
		for (int v = 0; v < v_cnt; ++v) if (icf[instr[v]].poly > 1) {
			// Look for note start
			if (!coff[i][v] && !pause[i][v]) {
				// Do not include dstime/detime in time calculation, because it can change result
				// Do not use playback speed in time calculation, because all calculations are relative in this algorithm
				float nlen = etime[i + noff[i][v] - 1] - stime[i];
				// Find other voices of same track having notes at same step
				for (int v2 = 0; v2 <= v_cnt; ++v2) if (v != v2 && track_id[v] == track_id[v2] && !pause[i][v2]) {
					float nlen2 = etime[i + noff[i][v2] - 1] - stime[i - coff[i][v2]];
					// Calculate overlap (abs is protection from bugs)
					float ov = abs(etime[i + noff[i][v2] - 1] - stime[i]);
					// Is overlap small?
					if (ov > nlen * MAX_OVERLAP_POLY || ov > nlen2 * MAX_OVERLAP_POLY) continue;
					int free = 0;
					// Move note from v to v2 voice
					for (int z = i; z <= i + noff[i][v] - 1; ++z) {
						// Check if overwritten note finished
						if (pause[z][v2]) free = 1;
						// If overwritten note finished, do not overwrite next note
						if (free && !pause[z][v2]) break;
						// Copy note
						note[z][v2] = note[z][v];
						len[z][v2] = len[z][v];
						coff[z][v2] = coff[z][v];
						pause[z][v2] = pause[z][v];
						dyn[z][v2] = dyn[z][v];
						midi_ch[z][v2] = midi_ch[z][v];
						// Clear old note
						note[z][v] = 0;
						pause[z][v] = 1;
						dyn[z][v] = 0;
					}
					// Log
					if (debug_level > 1) {
						CString st;
						st.Format("Merged note %s at step %d to note %s from voice %d to voice %d (track %d)",
							GetNoteName(note[i][v2]), i, GetNoteName(note[i - 1][v2]), v, v2, track_id[v]);
						WriteLog(0, st);
					}
				}
			}
		}
	}
}

