// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "XFIn.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

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

int XFIn::AllocateVoice(CString id, int staff, int chord) {
  // Check if this voice exists
	for (int i = 0; i < voice.size(); ++i) {
		if (voice[i].id == id && voice[i].staff == staff && voice[i].chord == chord) return i;
	}
	// Create new voice
	XMLVoice new_voice;
	new_voice.id = id;
	new_voice.staff = staff;
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
		}
		if (!strcmp(nd.name(), "note")) {
			CString part_id = nd.parent().parent().attribute("id").as_string();
			int cur_div = nd.parent().child("attributes").child("divisions").text().as_int();
			if (cur_div) divisions = cur_div;
			int v = nd.child("voice").text().as_int();
			int staff = nd.child("staff").text().as_int();
			if (staff >= words.size()) words.resize(staff + 1);
			int m = nd.parent().attribute("number").as_int();
			if (nd.child("chord").name()[0] != '\0') {
				++chord;
			}
			else chord = 0;
			vi = AllocateVoice(part_id, staff, chord);
			// Load measure if it is first note in measure
			if (mea.size() <= m) {
				mea.resize(m + 1);
				mea[m].barline = nd.parent().child("barline").child("bar-style").text().as_string();
				if (nd.parent().child("attributes").child("key").child("fifths").name()[0] != '\0')
					mea[m].fifths = nd.parent().child("attributes").child("key").child("fifths").text().as_int();
				mea[m].mode = nd.parent().child("attributes").child("key").child("mode").text().as_string();
				mea[m].beats = nd.parent().child("attributes").child("time").child("beats").text().as_int();
				mea[m].beat_type = nd.parent().child("attributes").child("time").child("beat-type").text().as_int();
			}
			note[vi].resize(m + 1);
			int ni = note[vi][m].size();
			note[vi][m].resize(ni + 1);
			if (nd.find_child_by_attribute("tie", "type", "stop")) {
				note[vi][m][ni].tie_stop = 1;
			}
			if (nd.find_child_by_attribute("tie", "type", "start")) {
				note[vi][m][ni].tie_start = 1;
			}
			if (nd.child("rest").name()[0] != '\0') {
				note[vi][m][ni].rest = 1;
			}
			else {
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
		}
	}
}

void XFIn::TestXML(CString pth) {
	path = pth;
	error = "";
	xml_parse_result result = d.load_file(path);
	if (!result) {
		error = "Cannot open file " + path;
		return;
	}

	for (xml_node tool : d.child("Profile").child("Tools").children("Tool"))
	{
		int timeout = tool.attribute("Timeout").as_int();

		if (timeout > 0)
			std::cout << "Tool " << tool.attribute("Filename").value() << " has timeout " << timeout << "\n";
	}
}

void XFIn::TestXPath(CString pth) {
	path = pth;
	error = "";
	xml_parse_result result = d.load_file(path);
	if (!result) {
		error = "Cannot open file " + path;
		return;
	}

	xpath_node_set tools_with_timeout = d.select_nodes("/Profile/Tools/Tool[@Timeout > 0]");

	for (xpath_node node : tools_with_timeout)
	{
		xml_node tool = node.node();
		std::cout << "Tool " << tool.attribute("Filename").value() <<
			" has timeout " << tool.attribute("Timeout").as_int() << "\n";
	}
}
