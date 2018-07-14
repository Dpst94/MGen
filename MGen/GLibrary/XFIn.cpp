// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "XFIn.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

map<CString, int> note_type_value = {
	{ "1024th", 1 },
	{ "512th", 2 },
	{ "256th", 4 },
	{ "128th", 8 },
	{ "64th", 16 },
	{ "32nd", 32 },
	{ "16th", 64 },
	{ "eighth", 128 },
	{ "quarter", 256 },
	{ "half", 512 },
	{ "whole", 1024 },
	{ "breve", 2048 },
	{ "long", 4096 },
	{ "maxima", 8192 }
};

XFIn::XFIn() {
}

XFIn::~XFIn() {
}

void XFIn::GetTextV(CString xpath, vector<CString> &sa) {
	xpath_node_set ns = d.select_nodes(xpath);
	for (xpath_node node : ns) {
		xml_node nd = node.node();
		sa.push_back(nd.text().as_string());
	}
}

CString XFIn::GetText(CString xpath) {
	xpath_node_set ns = d.select_nodes(xpath);

	if (!ns.size()) return "";
	for (xpath_node node : ns) {
		xml_node nd = node.node();
		return nd.text().as_string();
	}
}

int XFIn::AllocateVoice(CString id, int staff, int v, int chord) {
  // Check if this voice exists
	for (int i = 0; i < voice.size(); ++i) {
		if (voice[i].id == id && voice[i].staff == staff && 
			voice[i].v == v && voice[i].chord == chord) return i;
	}
	// Create new voice
	XMLVoice new_voice;
	new_voice.id = id;
	new_voice.staff = staff;
	new_voice.v = v;
	new_voice.chord = chord;
	new_voice.name = GetText("score-partwise/part-list/score-part[@id = '" + id + "']/part-name");
	new_voice.display = GetText("score-partwise/part-list/score-part[@id = '" + id + "']/part-name-display/display-text");
	voice.push_back(new_voice);
	note.resize(note.size() + 1);
	return voice.size() - 1;
}

int XFIn::GetPitchByName(CString pitch) {
	if (pitch == 'C') return 0;
	if (pitch == 'D') return 2;
	if (pitch == 'E') return 4;
	if (pitch == 'F') return 5;
	if (pitch == 'G') return 7;
	if (pitch == 'A') return 9;
	if (pitch == 'B') return 11;
	error = "Cannot parse pitch " + pitch;
	return 0;
}

void XFIn::LoadXML(CString pth) {
	xpath_node_set ns;
	int divisions = 2;
	int vi = 0;
	int chord = 0;
	int v = 1;
	int max_mea = 1;
	CString mode;
	char fifths = 0;
	char beats = 4;
	char beat_type = 4;
	float m_pos = 0;
	float m_pos_prev = 0;
	float tempo = 0;
	// Init
	path = pth;
	error = "";
	mea.clear();
	voice.clear();
	// Load file
	xml_parse_result result = d.load_file(path);
	if (!result) {
		error = "Cannot open file " + path;
		return;
	}
	// General information
	encoding_date = GetText("score-partwise/identification/encoding/encoding-date");
	encoder = GetText("score-partwise/identification/encoding/encoder");
	software = GetText("score-partwise/identification/encoding/software");
	encoding_description = GetText("score-partwise/identification/encoding/encoding-description");

	try {
		ns = d.select_nodes("score-partwise/part/measure/*");
	}
	catch (const pugi::xpath_exception& e)
	{
		error.Format("XPath select failed: %s", e.what());
		return;
	}
	vector<CString> words;
	for (xpath_node node : ns) {
		xml_node nd = node.node();
		if (!strcmp(nd.name(), "direction")) {
			int staff = nd.child("staff").text().as_int();
			if (staff >= words.size()) words.resize(staff + 1);
			words[staff] = nd.child("direction-type").child("words").text().as_string();
			if (nd.child("direction-type").child("metronome").child("per-minute").name()[0] != '\0') {
				tempo = nd.child("direction-type").child("metronome").child("per-minute").text().as_float() *
					note_type_value[nd.child("direction-type").child("metronome").child("beat-unit").text().as_string()] / 256.0;
				//if (!tempo) tempo = 100;
			}
		}
		if (!strcmp(nd.name(), "note")) {
			CString part_id = nd.parent().parent().attribute("id").as_string();
			int cur_div = nd.parent().child("attributes").child("divisions").text().as_int();
			if (cur_div) divisions = cur_div;
			int cur_v = nd.child("voice").text().as_int();
			if (cur_v) v = cur_v;
			int staff = nd.child("staff").text().as_int();
			if (staff >= words.size()) words.resize(staff + 1);
			int m = nd.parent().attribute("number").as_int();
			if (m > max_mea) max_mea = m;
			if (nd.child("chord").name()[0] != '\0') {
				++chord;
			}
			else chord = 0;
			vi = AllocateVoice(part_id, staff, v, chord);
			// Load measure if it is first note in measure
			if (mea.size() <= m) {
				mea.resize(m + 1);
				mea[m].barline = nd.parent().child("barline").child("bar-style").text().as_string();
			}
			// Load measure if this is first note in measure in this voice
			if (note[vi].size() <= m) {
				m_pos = 0;
				if (nd.parent().child("attributes").child("key").child("fifths").name()[0] != '\0')
					fifths = nd.parent().child("attributes").child("key").child("fifths").text().as_int();
				mode = nd.parent().child("attributes").child("key").child("mode").text().as_string();
				if (nd.parent().child("attributes").child("time").child("beats").text().as_int())
					beats = nd.parent().child("attributes").child("time").child("beats").text().as_int();
				mea[m].beats = beats;
				if (nd.parent().child("attributes").child("time").child("beat-type").text().as_int())
					beat_type = nd.parent().child("attributes").child("time").child("beat-type").text().as_int();
				mea[m].beat_type = beat_type;
			}
			if (chord)
				m_pos = m_pos_prev;
			note[vi].resize(m + 1);
			int ni = note[vi][m].size();
			note[vi][m].resize(ni + 1);
			note[vi][m][ni].pos = m_pos;
			note[vi][m][ni].fifths = fifths;
			note[vi][m][ni].mode = mode;
			if (tempo) {
				note[vi][m][ni].tempo = tempo;
				tempo = 0;
			}
			if (nd.find_child_by_attribute("tie", "type", "stop")) {
				note[vi][m][ni].tie_stop = 1;
			}
			if (nd.find_child_by_attribute("tie", "type", "start")) {
				note[vi][m][ni].tie_start = 1;
			}
			if (nd.child("grace").name()[0] != '\0') {
				note[vi][m][ni].grace = true;
			}
			if (nd.child("rest").name()[0] != '\0') {
				note[vi][m][ni].rest = true;
			}
			else {
				note[vi][m][ni].rest = false;
				int alter = nd.child("pitch").child("alter").text().as_int();
				note[vi][m][ni].pitch =
					12 * nd.child("pitch").child("octave").text().as_int() +
					GetPitchByName(nd.child("pitch").child("step").text().as_string()) +
					alter;
				note[vi][m][ni].alter = alter;
			}
			note[vi][m][ni].dur = nd.child("duration").text().as_int();
			note[vi][m][ni].dur_div = divisions;
			// Load text
			note[vi][m][ni].lyric = nd.child("lyric").child("text").text().as_string();
			if (!words[staff].IsEmpty()) {
				note[vi][m][ni].words = words[staff];
				words[staff].Empty();
			}
			m_pos_prev = m_pos;
			m_pos += note[vi][m][ni].dur * 0.25 / divisions;
		}
	}
	// Set same measure number for all voices
	for (int vi = 0; vi < voice.size(); ++vi) {
		note[vi].resize(max_mea + 1);
	}
}

void XFIn::ValidateXML() {
	// Check if measure is not filled with notes
	for (int vi = 0; vi < voice.size(); ++vi) {
		for (int m = 1; m < mea.size(); ++m) {
			// Do not check measures without notes
			if (!note[vi][m].size()) continue;
			float stack = note[vi][m][0].pos;
			for (int ni = 0; ni < note[vi][m].size(); ++ni) {
				if (note[vi][m][ni].tie_start) {
					if (ni < note[vi][m].size() - 1) {
						if (note[vi][m][ni + 1].rest) {
							error.Format("Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d, note %d of %d. Note starts tie, but next note in this measure is a rest. Probably, you are using tie in a chord, which is not recommended: better use voices or staffs",
								m, vi, voice[vi].id, voice[vi].name, voice[vi].staff, voice[vi].v, voice[vi].chord,
								mea[m].beats, mea[m].beat_type, ni + 1, note[vi][m].size());
							return;
						}
						if (note[vi][m][ni].pitch != note[vi][m][ni + 1].pitch) {
							error.Format("Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d, note %d of %d. Note starts tie, but next note in this measure has different pitch. Probably, you are using tie in a chord, which is not recommended: better use voices or staffs",
								m, vi, voice[vi].id, voice[vi].name, voice[vi].staff, voice[vi].v, voice[vi].chord,
								mea[m].beats, mea[m].beat_type, ni+1, note[vi][m].size());
							return;
						}
						if (!note[vi][m][ni + 1].tie_stop) {
							error.Format("Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d, note %d of %d. Note starts tie, but next note in this measure does not stop tie. Probably, you are using tie in a chord, which is not recommended: better use voices or staffs",
								m, vi, voice[vi].id, voice[vi].name, voice[vi].staff, voice[vi].v, voice[vi].chord,
								mea[m].beats, mea[m].beat_type, ni+1, note[vi][m].size());
							return;
						}
					}
					else if (m < mea.size() - 1) {
						if (!note[vi][m + 1].size()) {
							error.Format("Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d, note %d of %d. Note starts tie, but next measure does not have note in this voice.",
								m, vi, voice[vi].id, voice[vi].name, voice[vi].staff, voice[vi].v, voice[vi].chord,
								mea[m].beats, mea[m].beat_type, ni + 1, note[vi][m].size());
							return;
						}
						else {
							if (note[vi][m + 1][0].rest) {
								error.Format("Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d, note %d of %d. Note starts tie, but next note is a rest. Probably, you are using tie in a chord, which is not recommended: better use voices or staffs",
									m, vi, voice[vi].id, voice[vi].name, voice[vi].staff, voice[vi].v, voice[vi].chord,
									mea[m].beats, mea[m].beat_type, ni + 1, note[vi][m].size());
								return;
							}
							if (note[vi][m][ni].pitch != note[vi][m + 1][0].pitch) {
								error.Format("Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d, note %d of %d. Note starts tie, but next note has different pitch. Probably, you are using tie in a chord, which is not recommended: better use voices or staffs",
									m, vi, voice[vi].id, voice[vi].name, voice[vi].staff, voice[vi].v, voice[vi].chord,
									mea[m].beats, mea[m].beat_type, ni + 1, note[vi][m].size());
								return;
							}
							if (!note[vi][m + 1][0].tie_stop) {
								error.Format("Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d, note %d of %d. Note starts tie, but next note does not stop tie. Probably, you are using tie in a chord, which is not recommended: better use voices or staffs",
									m, vi, voice[vi].id, voice[vi].name, voice[vi].staff, voice[vi].v, voice[vi].chord,
									mea[m].beats, mea[m].beat_type, ni + 1, note[vi][m].size());
								return;
							}
						}
					}
					else {
						error.Format("Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d, note %d of %d. Note starts tie, but it is last note in this voice.",
							m, vi, voice[vi].id, voice[vi].name, voice[vi].staff, voice[vi].v, voice[vi].chord,
							mea[m].beats, mea[m].beat_type, ni + 1, note[vi][m].size());
						return;
					}
				}
				if (note[vi][m][ni].tie_stop) {
					if (ni) {
						if (note[vi][m][ni - 1].rest) {
							error.Format("Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d, note %d of %d. Note stops tie, but previous note in this measure is a rest. Probably, you are using tie in a chord, which is not recommended: better use voices or staffs",
								m, vi, voice[vi].id, voice[vi].name, voice[vi].staff, voice[vi].v, voice[vi].chord,
								mea[m].beats, mea[m].beat_type, ni + 1, note[vi][m].size());
							return;
						}
						if (note[vi][m][ni].pitch != note[vi][m][ni - 1].pitch) {
							error.Format("Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d, note %d of %d. Note stops tie, but previous note in this measure has different pitch. Probably, you are using tie in a chord, which is not recommended: better use voices or staffs",
								m, vi, voice[vi].id, voice[vi].name, voice[vi].staff, voice[vi].v, voice[vi].chord,
								mea[m].beats, mea[m].beat_type, ni + 1, note[vi][m].size());
							return;
						}
						if (!note[vi][m][ni - 1].tie_start) {
							error.Format("Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d, note %d of %d. Note stops tie, but previous note in this measure does not stop tie. Probably, you are using tie in a chord, which is not recommended: better use voices or staffs",
								m, vi, voice[vi].id, voice[vi].name, voice[vi].staff, voice[vi].v, voice[vi].chord,
								mea[m].beats, mea[m].beat_type, ni + 1, note[vi][m].size());
							return;
						}
					}
					else if (m) {
						if (!note[vi][m - 1].size()) {
							error.Format("Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d, note %d of %d. Note stops tie, but previous measure does not have note in this voice.",
								m, vi, voice[vi].id, voice[vi].name, voice[vi].staff, voice[vi].v, voice[vi].chord,
								mea[m].beats, mea[m].beat_type, ni + 1, note[vi][m].size());
							return;
						}
						else {
							if (note[vi][m - 1][note[vi][m - 1].size() - 1].rest) {
								error.Format("Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d, note %d of %d. Note stops tie, but previous note is a rest. Probably, you are using tie in a chord, which is not recommended: better use voices or staffs",
									m, vi, voice[vi].id, voice[vi].name, voice[vi].staff, voice[vi].v, voice[vi].chord,
									mea[m].beats, mea[m].beat_type, ni + 1, note[vi][m].size());
								return;
							}
							if (note[vi][m][ni].pitch != note[vi][m - 1][note[vi][m - 1].size() - 1].pitch) {
								error.Format("Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d, note %d of %d. Note stops tie, but previous note has different pitch. Probably, you are using tie in a chord, which is not recommended: better use voices or staffs",
									m, vi, voice[vi].id, voice[vi].name, voice[vi].staff, voice[vi].v, voice[vi].chord,
									mea[m].beats, mea[m].beat_type, ni + 1, note[vi][m].size());
								return;
							}
							if (!note[vi][m - 1][note[vi][m - 1].size() - 1].tie_start) {
								error.Format("Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d, note %d of %d. Note stops tie, but previous note does not start tie. Probably, you are using tie in a chord, which is not recommended: better use voices or staffs",
									m, vi, voice[vi].id, voice[vi].name, voice[vi].staff, voice[vi].v, voice[vi].chord,
									mea[m].beats, mea[m].beat_type, ni + 1, note[vi][m].size());
								return;
							}
						}
					}
					else {
						error.Format("Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d, note %d of %d. Note stops tie, but it is the first note in this voice.",
							m, vi, voice[vi].id, voice[vi].name, voice[vi].staff, voice[vi].v, voice[vi].chord,
							mea[m].beats, mea[m].beat_type, ni + 1, note[vi][m].size());
						return;
					}
				}
				if (ni && note[vi][m][ni].pos != stack) {
					error.Format("Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d: note %d of %d. Note starts at position %.3f that is not stack of previous note lengths %.3f",
						m, vi, voice[vi].id, voice[vi].name, voice[vi].staff, voice[vi].v, voice[vi].chord,
						mea[m].beats, mea[m].beat_type, ni + 1, note[vi][m].size(),
						note[vi][m][ni].pos, stack);
					return;
				}
				stack += note[vi][m][ni].dur * 0.25 / note[vi][m][ni].dur_div;
			}
			// Do not check chord voices for note length stack
			if (voice[vi].chord) continue;
			if (stack != mea[m].beats * 1.0 / mea[m].beat_type) {
				error.Format("Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d: %d notes. Need %.3f time but got %.3f",
					m, vi, voice[vi].id, voice[vi].name, voice[vi].staff, voice[vi].v, voice[vi].chord,
					mea[m].beats, mea[m].beat_type, note[vi][m].size(),
					mea[m].beats * 100.0 / mea[m].beat_type, stack);
				return;
			}
		}
	}
}
