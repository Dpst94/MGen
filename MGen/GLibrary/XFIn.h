// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#pragma once

#include "../pugixml/pugixml.hpp"

using namespace pugi;

struct XMLNote {
	float pos = 0; // position inside measure
	char pitch; // with alteration applied
	int dur; // duration
	int dur_div; // duration divisions
	char alter;
	bool rest = true;
	bool tie_start;
	bool tie_stop;
	CString lyric;
	CString words;
	char fifths = 100;
	CString mode;
};

struct XMLMeasure {
	char beats = 0;
	char beat_type = 0;
	CString barline;
};

struct XMLVoice {
	CString id;
	CString name;
	CString display;
	int staff;
	int v;
	int chord;
};

class XFIn
{
public:
	XFIn();
	~XFIn();
	void LoadXML(CString pth);
	void ValidateXML();

	// Input
	CString path;

	// Error
	CString error;

	// Result
	CString encoder;
	CString software;
	CString encoding_date;
	CString encoding_description;
	vector<XMLVoice> voice; // [v]
	vector<XMLMeasure> mea; // [m]
	vector<vector<vector<XMLNote>>> note; // [v][m][]

private:
	void GetTextV(CString xpath, vector<CString>& sa);
	CString GetText(CString xpath);
	int AllocateVoice(CString id, int staff, int v, int chord);
	int GetPitchByName(CString pitch);

	xml_document d;

};

