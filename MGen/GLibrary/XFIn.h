// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#pragma once

#include "../pugixml/pugixml.hpp"

using namespace pugi;

struct XMLNote {
	float pos = 0; // position inside measure
	char pitch = 0; // with alteration applied
	int dur = 0; // duration
	int dur_div = 1; // duration divisions
	char alter = 0;
	bool rest = true;
	bool tie_start = false;
	bool tie_stop = false;
	bool grace = false;
	float tempo = 0;
	CString lyric;
	CString words;
	char fifths = 100;
	CString mode;
};

struct XMLMeasure {
	char beats = 0;
	char beat_type = 0;
	float len = 0; // Measure length in whole notes
	CString barline;
};

struct XMLVoice {
	CString id;
	CString name;
	CString display;
	int staff;
	int v;
	int chord;
	float average_pitch;
};

class XFIn
{
public:
	XFIn();
	~XFIn();
	void LoadXML(CString pth);
	int ReorderVoices(float pdif);
	void ValidateXML();

	// Input
	CString path;

	// Errors
	CString error;
	CString warning;

	// Result
	CString encoder;
	CString software;
	CString encoding_date;
	CString encoding_description;
	vector<XMLVoice> voice; // [v]
	vector<XMLMeasure> mea; // [m]
	vector<vector<vector<XMLNote>>> note; // [v][m][]

private:
	void ReorderChords();
	int ReorderTwoVoices(float pdif);
	void GetTextV(CString xpath, vector<CString>& sa);
	CString GetText(CString xpath);
	int AllocateVoice(CString part_id, int staff, int v, int chord);
	int GetPitchByName(CString pitch);

	xml_document d;

};

