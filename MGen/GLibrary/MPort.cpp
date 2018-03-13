// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "MPort.h"
#include "SmRnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

MPort::MPort() {
	mo = 0;
	last_cc.resize(128);
}

MPort::~MPort() {
	StopMIDI();
}

void MPort::StartMIDI(int midi_device_i, int from)
{
	if (midi_device_i == -1) return;
	start_time();
	// Clear old sent messages
	midi_buf_next.clear();
	midi_sent_msg = 0;
	midi_sent_msg2 = 0;
	midi_sent_t2 = 0;
	midi_sent_t3 = 0;
	// Clear flags
	for (int i = 0; i < v_cnt; i++) warning_note_short[i] = 0;
	midi_last_run = 1;
	buf_underrun = 0;
	midi_play_step = 0;
	midi_start_time = 0;
	if (from > 0) {
		midi_sent = from;
		midi_sent_t = 0; // CGLib::time() + MIDI_BUF_PROTECT
	}
	else {
		midi_sent_t = 0;
		midi_sent = 0;
	}
	if (debug_level > 1) {
		CString est;
		est.Format("Trying to open midi device %d...", midi_device_i);
		WriteLog(4, est);
	}
	mo = new CMidiOut;
	if (mo->StartMidi(midi_device_i)) {
		CString est;
		est.Format("Cannot open midi device %d: %s", midi_device_i, mo->m_error);
		WriteLog(5, est);
	}
	CString est;
	est.Format("Open MIDI: device %d", midi_device_i);
	WriteLog(4, est);
}

void MPort::LogInstruments() {
	// Show instruments
	CString est;
	CString st, st2;
	/*
	int v_cnt2;
	// Get maximum voice mapped
	for (int i = MAX_VOICE - 1; i >= 0; --i) {
	if (instr[i] < InstGName.size() - 1) {
	v_cnt2 = i + 1;
	break;
	}
	}
	*/
	st2 = "Voice to instrument mapping: ";
	for (int i = 0; i < v_cnt; i++) {
		st.Format("%d ", instr[i]);
		st2 += st;
	}
	st2 += ". Instrument channels: ";
	for (int i = 0; i < icf.size(); i++) {
		st.Format("%d ", icf[i].channel);
		st2 += st;
	}
	est.Format("%s", st2);
	WriteLog(4, est);
}

void MPort::AddMidiEvent(long long timestamp, int mm_type, int data1, int data2)
{
	long long real_timestamp = timestamp + midi_start_time + midi_prepause;
	PmEvent event;
	event.timestamp = real_timestamp;
	event.message = Pm_Message(mm_type, data1, data2);
	if (amidi_export) {
		// Do not include real start time
		event.timestamp = timestamp + midi_prepause;
		midifile_buf[midi_stage][midi_track].push_back(event);
		return;
	}
	// Check if event is in future
	if (real_timestamp >= midi_sent_t) {
		// If it is not the last SendMIDI, postpone future events
		if ((!midi_last_run) && (real_timestamp > midi_buf_lim)) {
			if (!v_stage[midi_voice] && icf[instr[midi_voice]].port)
				midi_buf_next.push_back(event);
			// Save maximum message and its time
			if (real_timestamp > midi_sent_t3) {
				midi_sent_t3 = real_timestamp;
				midi_sent_msg3 = event.message;
			}
			if (debug_level > 1) {
				CString est;
				est.Format("Postponed AddMidiEvent to %lld step %d, type %02X, data %d/%d (after %lld step %d, type %02X, data %d/%d) [start = %lld, lim = %lld]",
					timestamp, midi_current_step, mm_type, data1, data2, midi_sent_t - midi_start_time, midi_sent,
					Pm_MessageStatus(midi_sent_msg), Pm_MessageData1(midi_sent_msg), Pm_MessageData2(midi_sent_msg), midi_start_time, midi_buf_lim - midi_start_time);
				WriteLog(4, est);
			}
		}
		else {
			if (midi_sending_buf_next || (!v_stage[midi_voice] && icf[instr[midi_voice]].port)) {
				midi_buf.push_back(event);
			}
			// Save maximum message and its time
			if (real_timestamp > midi_sent_t2) {
				midi_sent_t2 = real_timestamp;
				midi_sent_msg2 = event.message;
			}
		}
	}
	else {
		CString est;
		est.Format("Blocked AddMidiEvent to past %lld ms, step %d, type %02X, data %d/%d (before %lld ms, step %d, type %02X, data %d/%d) [start = %lld]",
			timestamp, midi_current_step, mm_type, data1, data2, midi_sent_t - midi_start_time, midi_sent,
			Pm_MessageStatus(midi_sent_msg), Pm_MessageData1(midi_sent_msg), Pm_MessageData2(midi_sent_msg), midi_start_time); // , midi_buf_lim - midi_start_time
		WriteLog(5, est);
	}
	// Debug log
	//CString st;
	//st.Format("%d: At %d type %d, data %d/%d blocked %d\n", CGLib::time(), timestamp, mm_type, data1, data2, midi_sent_t-midi_start_time);
	//AppendLineToFile("midi.log", st);
}

void MPort::AddTransitionKs(int i, long long stimestamp, int ks)
{
	int v = midi_voice;
	int pi = i - poff[i][v];
	int ei = i + len[i][v] - 1;
	AddKsOn(stimestamp - min(MAX_TRANS_DELAY,
		((sstime[i][v] - sstime[pi][v]) * 100 / m_pspeed + dstime[i][v] - dstime[pi][v]) / 10), ks, 10);
	AddKsOff(stimestamp + min(MAX_TRANS_DELAY,
		((setime[ei][v] - sstime[i][v]) * 100 / m_pspeed + detime[ei][v] - dstime[i][v]) / 10), ks, 0);
}

void MPort::AddKs(long long stimestamp, int ks) {
	AddKsOn(stimestamp, ks, 101);
	AddKsOff(stimestamp + 5, ks, 0);
}

void MPort::AddTransitionCC(int i, long long stimestamp, int CC, int value1, int value2) {
	int v = midi_voice;
	int pi = i - poff[i][v];
	int ei = i + len[i][v] - 1;
	AddCC(stimestamp - min(MAX_TRANS_DELAY,
		((sstime[i][v] - sstime[pi][v]) * 100 / m_pspeed + dstime[i][v] - dstime[pi][v]) / 10), CC, value1);
	AddCC(stimestamp + min(MAX_TRANS_DELAY,
		((setime[ei][v] - sstime[i][v]) * 100 / m_pspeed + detime[ei][v] - dstime[i][v]) / 10), CC, value2);
}

// Check that dstime is not too low
void MPort::CheckDstime(int i, int v)
{
	if (dstime[i][v] - MAX_TRANS_DELAY < -MAX_AHEAD && warning_ahead > MAX_WARN_MIDI_AHEAD) {
		CString st;
		st.Format("Warning: step %d, voice %d has dstime %.0f, while MAX_AHEAD=%d, MAX_TRANS_DELAY=%d. Risk of event blocking (can be not seen in logs)! Probably too long legato_ahead or random_start. Or you have to increase MAX_AHEAD.",
			i, v, dstime[i][v], MAX_AHEAD, MAX_TRANS_DELAY);
		WriteLog(5, st);
		++warning_ahead;
	}
}

void MPort::GetMidiPrePause() {
	midi_prepause = 0;
	long long stimestamp;
	// Check first steps of each voice
	for (int v = 0; v < v_cnt; ++v) {
		for (int i = 0; i < t_sent; ++i) {
			if (!pause[i][v]) {
				stimestamp = sstime[i][v] * 100 / m_pspeed + dstime[i][v];
				midi_prepause = max(midi_prepause, -stimestamp);
				break;
			}
		}
		midi_prepause = max(midi_prepause, icf[instr[v]].mute_predelay);
	}
	midi_prepause += INIT_PRESTEPS * INIT_PRESTEP;
}

void MPort::SendMIDI(int step1, int step2)
{
	if (step2 == step1) return;
	long long time_start = CGLib::time();
	long long timestamp_current = CGLib::time();
	// Note start timestamp
	long long stimestamp;
	// Note end timestamp
	long long etimestamp;
	// Check if this is first run
	if ((step1 == 0) || (!midi_sent_t) || (!midi_start_time)) {
		midi_first_run = 1;
		GetMidiPrePause();
	}
	else midi_first_run = 0;
	if (midi_first_run) LogInstruments();
	// Set real time when playback started (approximate, because SStime can vary)
	if (!midi_start_time) midi_start_time = timestamp_current + MIDI_BUF_PROTECT -
		(long long)(stime[step1] / m_pspeed * 100);
	// Set real time when playback started
	if (!midi_sent_t) {
		if (amidi_export) {
			midi_sent_t = (long long)(stime[step1] / m_pspeed * 100) + midi_start_time;
		}
		else {
			midi_sent_t = (long long)(stime[step1] / m_pspeed * 100) + midi_start_time - 100;
		}
	}
	// Check if we have buf underrun
	if (midi_sent_t < timestamp_current) {
		CString st;
		st.Format("SendMIDI got buf underrun in %lld ms (steps %d - %d)",
			timestamp_current - midi_sent_t, step1, step2);
		WriteLog(5, st);
		buf_underrun = 1;
		return;
	}
	// Check if buf is full
	if (midi_sent_t - timestamp_current > MIN_MIDI_BUF_MSEC) {
		if (debug_level > 1) {
			CString st;
			st.Format("SendMIDI: no need to send (full buf = %lld ms) (steps %d - %d) playback is at %lld",
				midi_sent_t - timestamp_current, step1, step2, timestamp_current - midi_start_time);
			WriteLog(4, st);
		}
		return;
	}
	if (debug_level > 1) {
		CString est;
		est.Format("SendMIDI: need to send (full buf = %lld ms) (steps %d - %d) playback is at %lld",
			midi_sent_t - timestamp_current, step1, step2, timestamp_current - midi_start_time);
		WriteLog(4, est);
	}
	int i;
	if (!amidi_export) {
		if (!mutex_output.try_lock_for(chrono::milliseconds(3000))) {
			WriteLog(0, "SendMIDI mutex timed out: will try later");
		}
	}
	int step21 = 0; // Voice-dependent first step
	int step22 = 0; // Voice-independent last step
	float time = 0;
	// Find last step not too far (approximate, because SStime can vary)
	for (i = step1; i <= step2; i++) {
		step22 = i;
		if (i == 0) time = stime[i] * 100 / m_pspeed;
		else time = etime[i - 1] * 100 / m_pspeed;
		if (!amidi_export)
			if ((long long)time + midi_start_time - timestamp_current > MAX_MIDI_BUF_MSEC) break;
	}
	// If we cut notes, this is not last run
	if (step22 < step2) midi_last_run = 0;
	// Send previous buffer if exists
	midi_buf.clear();
	// Calculate midi right limit (approximate, because SStime can vary)
	midi_buf_lim = midi_start_time + (long long)(stime[step22] * 100.0 / m_pspeed);
	// Decrease right limit to allow for legato ahead, random start and ks/cc transitions
	if (!midi_last_run) midi_buf_lim -= MAX_AHEAD;
	// Sort by timestamp before sending
	if (midi_buf_next.size() > 0) {
		qsort(midi_buf_next.data(), midi_buf_next.size(), sizeof(PmEvent), PmEvent_comparator);
		midi_sending_buf_next = 1;
		vector <PmEvent> mbn = midi_buf_next;
		midi_buf_next.clear();
		// Set step to zero, because we do not know real steps of postponed notes
		midi_current_step = 0;
		for (int i = 0; i < mbn.size(); ++i) {
			// Subtract prepause, because it is already added in buffer
			AddMidiEvent(mbn[i].timestamp - midi_start_time - midi_prepause,
				Pm_MessageStatus(mbn[i].message),
				Pm_MessageData1(mbn[i].message), Pm_MessageData2(mbn[i].message));
		}
		midi_sending_buf_next = 0;
		if (debug_level > 1) {
			CString est;
			est.Format("Postponed events sent (%d) - now postponed %d",
				mbn.size(), midi_buf_next.size());
			WriteLog(4, est);
		}
	}
	for (int v = 0; v < v_cnt; v++) {
		// Clear last cc
		last_cc.clear();
		last_cc.resize(128, -1);
		// Initialize voice
		int ei;
		int ncount = 0;
		int ii = instr[v];
		midi_channel = icf[ii].channel - 1;
		midi_channel_saved = midi_channel;
		midi_track = icf[ii].track;
		midi_stage = v_stage[v];
		midi_voice = v;
		midi_current_step = 0;
		// Send initialization commands
		if (midi_first_run) {
			for (auto const& it : icf[ii].InitCommands) {
				AddMidiEvent(midi_sent_t - midi_start_time - midi_prepause,
					Pm_MessageStatus(it) + midi_channel,
					Pm_MessageData1(it), Pm_MessageData2(it));
				if (Pm_MessageStatus(it) == MIDI_NOTEON) {
					AddKsOff(midi_sent_t - midi_start_time - midi_prepause + 3,
						Pm_MessageData1(it), 0);
				}
			}
			// Send pan
			AddCC(midi_sent_t - midi_start_time - midi_prepause, 10,
				(icf[ii].pan * 127) / 100);
			// Send vol
			AddCC(midi_sent_t - midi_start_time - midi_prepause, 7,
				(icf[ii].vol * icf[ii].vol_default * master_vol) / 10000);
			if (icf[ii].trem_chan > -1) {
				// These CC can seem to be already sent, so clear them
				last_cc.clear();
				last_cc.resize(128, -1);
				midi_channel = icf[ii].trem_chan - 1;
				// Send pan
				AddCC(midi_sent_t - midi_start_time - midi_prepause, 10,
					(icf[ii].pan * 127) / 100);
				// Send vol
				AddCC(midi_sent_t - midi_start_time - midi_prepause, 7,
					(icf[ii].vol * icf[ii].vol_default * master_vol) / 10000);
				midi_channel = midi_channel_saved;
				// These CC can seem to be already sent, so clear them
				last_cc.clear();
				last_cc.resize(128, -1);
			}
		}
		// Move to note start
		if (coff[step1][v] > 0) {
			if (midi_first_run) step21 = step1 + noff[step1][v];
			else step21 = step1 - coff[step1][v];
		}
		else step21 = step1;
		// Count notes
		for (i = step21; i < step22; i++) {
			ncount++;
			if (i + len[i][v] > step22) break;
			// Set new buffer limit to beginning of last note
			if (noff[i][v] == 0) break;
			i += noff[i][v] - 1;
		}
		// Send notes
		i = step21;
		for (int x = 0; x < ncount; x++) {
			midi_current_step = i;
			ei = max(0, i + len[i][v] - 1);
			if (!pause[i][v]) {
				int my_note;
				// Replace note
				if (icf[ii].replace_pitch > -1) my_note = icf[ii].replace_pitch;
				else if (icf[ii].map_pitch.find(note[i][v]) != icf[ii].map_pitch.end()) {
					my_note = icf[ii].map_pitch[note[i][v]];
				}
				else my_note = note[i][v] + play_transpose[v];
				if (artic[i][v] == aTREM) {
					if (icf[ii].trem_replace > -1) {
						my_note = icf[ii].trem_replace;
					}
					if (icf[ii].trem_transpose) {
						my_note += icf[ii].trem_transpose;
					}
					if (icf[ii].map_tremolo.find(note[i][v]) != icf[ii].map_tremolo.end()) {
						my_note = icf[ii].map_tremolo[note[i][v]];
					}
				}
				// Note ON if it is not blocked and was not yet sent
				if (artic[i][v] == aTREM && icf[ii].trem_chan > -1) midi_channel = icf[ii].trem_chan - 1;
				stimestamp = sstime[i][v] * 100 / m_pspeed + dstime[i][v];
				CheckDstime(i, v);
				if ((stimestamp + midi_start_time + midi_prepause >= midi_sent_t) && (i >= midi_sent)) {
					if (!note_muted[i][v]) {
						AddNoteOn(stimestamp, my_note, vel[i][v]);
					}
					if (icf[ii].type == itEIS) {
						// Send bow
						if (GetBit(filter[i][v], fTASTO)) {
							if (icf[ii].NameToKsw.find("Sul tasto") != icf[ii].NameToKsw.end())
								AddKs(stimestamp - 3, icf[ii].NameToKsw["Sul tasto"]);
							if (icf[ii].NameToKsw.find("Harmonics") != icf[ii].NameToKsw.end())
								AddKs(stimestamp - 3, icf[ii].NameToKsw["Harmonics"]);
						}
						else if (GetBit(filter[i][v], fPONT)) {
							AddKs(stimestamp - 3, icf[ii].NameToKsw["Sul ponticello"]);
						}
						else {
							AddKs(stimestamp - 3, icf[ii].NameToKsw["Normal"]);
						}
						// Send slur
						if (artic[i][v] == aSLUR) {
							AddTransitionKs(i, stimestamp, icf[ii].NameToKsw["Slur while held"]);
						}
						// Send staccato
						if (artic[i][v] == aSTAC) {
							AddKs(stimestamp - 3, icf[ii].NameToKsw["Staccato"]);
						}
						else if (artic[i][v] == aPIZZ) {
							AddKs(stimestamp - 3, icf[ii].NameToKsw["Pizzicato"]);
						}
						else if (artic[i][v] == aTREM) {
							AddKs(stimestamp - 3, icf[ii].NameToKsw["Tremolo"]);
						}
						else {
							AddKs(stimestamp - 3, icf[ii].NameToKsw["Sustain"]);
						}
						// Send rebow retrigger
						if (artic[i][v] == aREBOW) {
							AddTransitionCC(i, stimestamp, icf[ii].CC_retrigger, 100, 0);
						}
					}
					// Send transition ks
					if (icf[ii].type == itSMB || icf[ii].type == itSMW) {
						// Frullato
						if (artic[i][v] == aTREM && icf[ii].trem_activate > -1) {
							for (auto const& it : icf[ii].tech[icf[ii].trem_activate]) {
								AddMidiEvent(stimestamp - 3,
									Pm_MessageStatus(it) + midi_channel,
									Pm_MessageData1(it), Pm_MessageData2(it));
								if (Pm_MessageStatus(it) == MIDI_NOTEON) {
									AddKsOff(stimestamp + 3,
										Pm_MessageData1(it), 0);
								}
							}
						}
						// Open
						if (artic[i][v] != aTREM && icf[ii].trem_deactivate > -1) {
							for (auto const& it : icf[ii].tech[icf[ii].trem_deactivate]) {
								AddMidiEvent(stimestamp - 3,
									Pm_MessageStatus(it) + midi_channel,
									Pm_MessageData1(it), Pm_MessageData2(it));
								if (Pm_MessageStatus(it) == MIDI_NOTEON) {
									AddKsOff(stimestamp + 3,
										Pm_MessageData1(it), 0);
								}
							}
						}
					}
					if (icf[ii].type == itSMB) {
						// Mute
						if (GetBit(filter[i][v], fMUTE) && icf[ii].mute_activate > -1) {
							for (auto const& it : icf[ii].tech[icf[ii].mute_activate]) {
								AddMidiEvent(stimestamp - icf[ii].mute_predelay,
									Pm_MessageStatus(it) + midi_channel,
									Pm_MessageData1(it), Pm_MessageData2(it));
								if (Pm_MessageStatus(it) == MIDI_NOTEON) {
									AddKsOff(stimestamp - icf[ii].mute_predelay + 1,
										Pm_MessageData1(it), 0);
								}
							}
						}
						// Open
						if (!GetBit(filter[i][v], fMUTE) && icf[ii].mute_deactivate > -1) {
							for (auto const& it : icf[ii].tech[icf[ii].mute_deactivate]) {
								AddMidiEvent(stimestamp - icf[ii].mute_predelay,
									Pm_MessageStatus(it) + midi_channel,
									Pm_MessageData1(it), Pm_MessageData2(it));
								if (Pm_MessageStatus(it) == MIDI_NOTEON) {
									AddKsOff(stimestamp - icf[ii].mute_predelay + 1,
										Pm_MessageData1(it), 0);
								}
							}
						}
						if (artic[i][v] == aSPLITPO_CHROM) {
							AddTransitionKs(i, stimestamp, icf[ii].ks1 + 12);
							AddTransitionKs(i, stimestamp, icf[ii].ks1 + 0);
						}
						if (artic[i][v] == aSPLITPO_MIX) {
							AddTransitionKs(i, stimestamp, icf[ii].ks1 + 12);
							AddTransitionKs(i, stimestamp, icf[ii].ks1 + 1);
						}
						if (artic[i][v] == aSPLITPO_ARAB) {
							AddTransitionKs(i, stimestamp, icf[ii].ks1 + 12);
							AddTransitionKs(i, stimestamp, icf[ii].ks1 + 3);
						}
						if (artic[i][v] == aSPLITPO_PENT) {
							AddTransitionKs(i, stimestamp, icf[ii].ks1 + 12);
							AddTransitionKs(i, stimestamp, icf[ii].ks1 + 4);
						}
						if (artic[i][v] == aGLISS) {
							AddTransitionKs(i, stimestamp, icf[ii].ks1 + 12);
							AddTransitionKs(i, stimestamp, icf[ii].ks1 + 2);
						}
						if (artic[i][v] == aGLISS2) {
							AddTransitionKs(i, stimestamp, icf[ii].ks1 + 12);
							AddTransitionKs(i, stimestamp, icf[ii].ks1 + 5);
						}
					}
				}
				// Note OFF if it is in window
				if (ei <= step22) {
					// Note OFF
					// ndur = (etime[ei] - stime[i]) * 100 / m_pspeed + detime[ei][v] - dstime[i][v];
					etimestamp = setime[ei][v] * 100 / m_pspeed + detime[ei][v];
					AddNoteOff(etimestamp, my_note, 0);
					// Send note ending ks
					if (icf[ii].type == itSMB) {
						if (artic[ei][v] == aEND_SFL) {
							AddKs(etimestamp - icf[ii].end_sfl_dur, icf[ii].ks1 + 11);
						}
						if (artic[ei][v] == aEND_PBD) {
							AddKs(etimestamp - icf[ii].end_pbd_dur, icf[ii].ks1 + 4);
						}
						if (artic[ei][v] == aEND_VIB2) {
							AddKs(etimestamp - icf[ii].end_vib2_dur, icf[ii].ks1 + 6);
						}
						if (artic[ei][v] == aEND_VIB) {
							AddKs(etimestamp - icf[ii].end_vib_dur, icf[ii].ks1 + 5);
						}
					}
				}
				midi_channel = midi_channel_saved;
			}
			// Go to next note
			if (noff[i][v] == 0) break;
			i += noff[i][v];
		}
		// Send CC
		if (icf[ii].trem_chan > -1) midi_channel = icf[ii].trem_chan - 1;
		InterpolateCC(icf[ii].CC_dyn, icf[ii].rnd_dyn, step1, step22, dyn, ii, v);
		InterpolateCC(icf[ii].CC_vib, icf[ii].rnd_vib, step1, step22, vib, ii, v);
		InterpolateCC(icf[ii].CC_vibf, icf[ii].rnd_vibf, step1, step22, vibf, ii, v);
		SendPedalCC(step1, step22, ii, v);
		midi_channel = midi_channel_saved;
	}
	// Sort by timestamp before sending
	qsort(midi_buf.data(), midi_buf.size(), sizeof(PmEvent), PmEvent_comparator);
	// Send
	for (int i = 0; i < midi_buf.size(); i++) {
		mo->QueueEvent(midi_buf[i]);
	}
	// Count time
	long long time_stop = CGLib::time();
	CString st;
	st.Format("MIDI write %d (%d postponed) events: steps %d/%d - %d/%d (%lld to %lld ms) [to future %lld to %lld ms] (in %lld ms) playback is at %lld ms. Limit %lld. Last postponed %lld. Step22 stopped increasing at %.0f ms. Start time: %lld, current time: %lld. Prepause %.0f ms",
		midi_buf.size(), midi_buf_next.size(), step21, step1, step22, step2,
		midi_sent_t - midi_start_time, midi_sent_t2 - midi_start_time,
		midi_sent_t - timestamp_current, midi_sent_t2 - timestamp_current,
		time_stop - time_start, timestamp_current - midi_start_time, midi_buf_lim - midi_start_time,
		midi_sent_t3 - midi_start_time, time, midi_start_time, timestamp_current, midi_prepause);
	WriteLog(4, st);
	// Save last sent position
	midi_sent = step22;
	midi_sent_t = midi_sent_t2;
	midi_sent_msg = midi_sent_msg2;
	if (!amidi_export) {
		mutex_output.unlock();
	}
}

void MPort::SendPedalCC(int step1, int step2, int ii, int v) {
	for (int i = step1; i < step2; i++) {
		if (GetBit(filter[i][v], fPEDAL)) {
			long long stimestamp = sstime[i][v] * 100 / m_pspeed + dstime[i][v];
			AddCC(stimestamp, 64, 127);
		}
		if (GetBit(filter[i][v], fUNPEDAL)) {
			long long stimestamp = sstime[i][v] * 100 / m_pspeed + dstime[i][v];
			AddCC(stimestamp, 64, 0);
		}
	}
}

// First cc sent by this function is with i = step1 - 2, time = stime[i + 1] = stime[step1-1]
// Last cc sent by this function is with i = step2 - 2, time = etime[i] = etime[step2-2] = stime[step2-1]
void MPort::InterpolateCC(int CC, float rnd, int step1, int step2, vector< vector <unsigned char> > & dv, int ii, int v)
{
	//CString st;
	//st.Format("Send CC%d from %d to %d", CC, step1, step2);
	//WriteLog(4, st);
	if (!CC) return;
	CSmoothRandom sr;
	int steps;
	float fsteps;
	// Linear interpolation
	vector <float> cc_lin;
	// Moving average
	vector <float> cc_ma;
	// Time of cc step
	vector <float> cc_time;
	// Time of last cc sent here
	float last_time = stime[step2 - 1];
	int first_cc = 0;
	int last_cc = 0;
	// Find step that will give enough information for ma junction
	int pre_cc = 0;
	int first_step = step1 - 2;
	for (int i = step1 - 3; i >= 0; --i) {
		// Get CC steps count
		fsteps = (float)(icf[ii].CC_steps) / 1000.0 * (etime[i] - stime[i]);
		steps = max(1, fsteps);
		pre_cc += steps;
		if (pre_cc > icf[ii].CC_ma) {
			first_step = i;
			break;
		}
	}
	for (int i = first_step; i < step2; i++) {
		if (i < 0) continue;
		midi_current_step = i;
		// Get CC steps count
		fsteps = (float)(icf[ii].CC_steps) / 1000.0 * (etime[i] - stime[i]);
		// Check if need to skip note steps
		//skip = 1.0 / max(0.0000001, fsteps);
		//if (skip > 1 && i % skip && coff[i][v] && noff[i][v] != 1 && i != step1 - 2 && i != step2 - 2) continue;
		steps = max(1, fsteps);
		if (steps % 2 == 0) steps++;
		// Calculate first and last ma positions to send
		if (i == step1 - 1) first_cc = cc_lin.size();
		if (i == step2 - 1) last_cc = cc_lin.size() - 1;
		// Linear interpolation
		for (int c = 0; c < steps; c++) {
			cc_time.push_back(stime[i] * 100 / m_pspeed + (etime[i] - stime[i]) * 100 / m_pspeed*(float)c / (float)steps);
			// Left cc steps
			if (c < steps / 2) {
				if (i == 0) cc_lin.push_back(dv[i][v]);
				else cc_lin.push_back((floor(steps * 0.5 - c) * dv[i - 1][v] + floor(c + 1 + steps / 2) * dv[i][v]) / steps);
			}
			// Right cc steps
			else {
				cc_lin.push_back((floor(steps * 1.5 - c) * dv[i][v] + floor(c - steps / 2) * dv[i + 1][v]) / steps);
			}
		} // for c
	} // for i
		// Detect abrupt changes and additionally smoothen them
	for (int c = 2; c < cc_lin.size(); ++c) {
		// Wait until change is abrupt
		if (abs(cc_lin[c] - cc_lin[c - 1]) < 20) continue;
		int left = c;
		int left0 = max(0, c - 10);
		// Find leftmost unchanged value
		for (int i = c - 2; i >= left0; --i) {
			if (cc_lin[c - 1] != cc_lin[i]) break;
			left = i;
		}
		// Exit if value is unstable
		if (left > c - 1) continue;
		// Interpolate
		for (int i = left; i < c; ++i) {
			cc_lin[i] = cc_lin[left] + (cc_lin[c] - cc_lin[left]) * (float)(i - left) / (c - left);
		}
	}
	cc_ma.resize(cc_lin.size());
	int CC_ma2 = icf[ii].CC_ma / 2;
	// Move cc sending ma window to the left
	first_cc = max(0, first_cc - CC_ma2 - 1);
	last_cc = max(0, last_cc - CC_ma2 - 1);
	// Set border ma
	cc_ma[0] = cc_lin[0];
	cc_ma[cc_lin.size() - 1] = cc_lin[cc_lin.size() - 1];
	// First moving averages
	for (int c = 1; c <= CC_ma2; c++) {
		int lsteps = c * 2 + 1;
		cc_ma[c] = 0;
		for (int i = 0; i < lsteps; ++i) {
			cc_ma[c] += cc_lin[i] / (float)lsteps;
		}
	}
	// Extend moving average
	for (int c = CC_ma2 + 1; c < cc_lin.size() - CC_ma2 - 1; ++c) {
		cc_ma[c] = cc_ma[c - 1] + (cc_lin[c + CC_ma2] - cc_lin[c - CC_ma2 - 1]) / (float)(icf[ii].CC_ma);
	}
	// Last moving averages
	cc_ma[0] = cc_lin[0];
	for (int c = cc_lin.size() - CC_ma2 - 1; c < cc_lin.size() - 1; c++) {
		int lsteps = (cc_lin.size() - 1 - c) * 2 + 1;
		cc_ma[c] = 0;
		for (int i = cc_lin.size() - lsteps; i < cc_lin.size(); ++i) {
			cc_ma[c] += cc_lin[i] / (float)lsteps;
		}
	}
	// Randomize from first_cc
	for (int c = first_cc; c < cc_lin.size(); ++c) {
		float t = cc_time[c];
		// Calculate fadeout
		float fadeout = 1;
		if (last_time - CC_FADEOUT_RESERVE - t < CC_FADEOUT) fadeout = max(0, last_time - CC_FADEOUT_RESERVE - t) / CC_FADEOUT;
		// Add random
		sr.MakeNext();
		cc_ma[c] += sr.sig / sr.s_range * (float)rnd * (float)cc_ma[c] / 200.0 * fadeout;
		// Check limits
		if (cc_ma[c] < 1) cc_ma[c] = 1;
		if (cc_ma[c] > 127) cc_ma[c] = 127;
	}
	// Send starting CC
	int is_first_cc = 1;
	// Send ma CC
	for (int c = first_cc; c <= last_cc; c++) {
		float t = cc_time[c];
		if (is_first_cc) {
			if (midi_first_run) AddCC(midi_sent_t - midi_start_time - midi_prepause,
				CC, cc_ma[c]);
			is_first_cc = 0;
		}
		AddCC(t, CC, cc_ma[c]);
	}
}

void MPort::StopMIDI()
{
	WriteLog(4, "Stop MIDI");
	if (mo) delete mo;
	mo = 0;
}

int MPort::GetPlayStep() {
	if (buf_underrun == 1) {
		midi_play_step = 0;
	}
	else {
		// Don't need lock, because this function is called from OnDraw, which already has lock
		int step1 = midi_play_step;
		int step2 = midi_sent;
		int cur_step = 0, currentElement;
		long long searchElement = CGLib::time() - midi_start_time - midi_prepause;
		while (step1 <= step2) {
			cur_step = (step1 + step2) / 2;
			currentElement = stime[cur_step] * 100 / m_pspeed;
			if (currentElement < searchElement) {
				step1 = cur_step + 1;
			}
			else if (currentElement > searchElement) {
				step2 = cur_step - 1;
			}
			else {
				break;
			}
		}
		midi_play_step = cur_step;
		//mutex_output.unlock();
	}
	return midi_play_step;
}

void MPort::AddNoteOn(long long timestamp, int data1, int data2)
{
	// Check if range valid
	if ((data1 < icf[instr[midi_voice]].nmin) || (data1 > icf[instr[midi_voice]].nmax)) {
		if (warning_note_wrong[midi_voice] < 4) {
			CString st;
			st.Format("Blocked note %d/%d step %d time %lld in voice %d instrument %s/%s out of range %d-%d",
				data1, data2, midi_current_step, timestamp, midi_voice, icf[instr[midi_voice]].group,
				icf[instr[midi_voice]].name,
				icf[instr[midi_voice]].nmin, icf[instr[midi_voice]].nmax);
			WriteLog(1, st);
			warning_note_wrong[midi_voice] ++;
		}
		return;
	}
	AddMidiEvent(timestamp, MIDI_NOTEON + midi_channel, data1, data2);
}

void MPort::AddKsOn(long long timestamp, int data1, int data2)
{
	// Check if range valid
	if ((data1 >= icf[instr[midi_voice]].nmin) && (data1 <= icf[instr[midi_voice]].nmax)) {
		if (warning_note_wrong[midi_voice] < 4) {
			CString st;
			st.Format("Blocked keyswitch %d/%d time %lld in voice %d instrument %d %s in note range %d-%d",
				data1, data2, timestamp, midi_voice, instr[midi_voice], icf[instr[midi_voice]].group,
				icf[instr[midi_voice]].nmin, icf[instr[midi_voice]].nmax);
			WriteLog(5, st);
			warning_note_wrong[midi_voice] ++;
		}
		return;
	}
	AddMidiEvent(timestamp, MIDI_NOTEON + midi_channel, data1, data2);
}

void MPort::AddNoteOff(long long timestamp, int data1, int data2)
{
	// Check if range valid
	if ((data1 < icf[instr[midi_voice]].nmin) || (data1 > icf[instr[midi_voice]].nmax)) {
		if (warning_note_wrong[midi_voice] < 4) {
			CString st;
			st.Format("Blocked note %d/%d time %lld in voice %d instrument %s/%s out of range %d-%d",
				data1, data2, timestamp, midi_voice, icf[instr[midi_voice]].group,
				icf[instr[midi_voice]].name,
				icf[instr[midi_voice]].nmin, icf[instr[midi_voice]].nmax);
			WriteLog(1, st);
			warning_note_wrong[midi_voice] ++;
		}
		return;
	}
	AddMidiEvent(timestamp, MIDI_NOTEOFF + midi_channel, data1, data2);
}

void MPort::AddKsOff(long long timestamp, int data1, int data2)
{
	// Check if range valid
	if ((data1 >= icf[instr[midi_voice]].nmin) && (data1 <= icf[instr[midi_voice]].nmax)) {
		if (warning_note_wrong[midi_voice] < 4) {
			CString st;
			st.Format("Blocked keyswitch %d/%d time %lld in voice %d instrument %d %s in note range %d-%d",
				data1, data2, timestamp, midi_voice, instr[midi_voice], icf[instr[midi_voice]].group,
				icf[instr[midi_voice]].nmin, icf[instr[midi_voice]].nmax);
			WriteLog(5, st);
			warning_note_wrong[midi_voice] ++;
		}
		return;
	}
	AddMidiEvent(timestamp, MIDI_NOTEOFF + midi_channel, data1, data2);
}

void MPort::AddCC(long long timestamp, int data1, int data2) {
	if (last_cc[data1] == data2) return;
	last_cc[data1] = data2;
	AddMidiEvent(timestamp, MIDI_CC + midi_channel, data1, data2);
}

int MPort::PmEvent_comparator(const void * v1, const void * v2)
{
	const PmEvent *p1 = (PmEvent *)v1;
	const PmEvent *p2 = (PmEvent *)v2;
	//return (p2->timestamp - p1->timestamp);
	if (p1->timestamp < p2->timestamp)
		return -1;
	else if (p1->timestamp > p2->timestamp)
		return +1;
	else
		return 0;
}

