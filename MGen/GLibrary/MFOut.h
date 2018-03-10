// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#pragma once
#include "MFIn.h"

// Tail for export adapted midi in ms
#define EXPORT_MIDI_TAIL 1000

// Bass instruments
const int bass_program[] = { 45, 33, 70, 58, 34, 35, 36, 37, 38 };

class MFOut :
	public MFIn
{
public:
	MFOut();
	~MFOut();

	void ExportAdaptedMidi(CString dir, CString fname);
	void SaveMidi(CString dir, CString fname);
};

