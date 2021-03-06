// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#pragma once
#include "MPort.h"

#define MAX_WARN_MIDI_OVERLAP 5
#define MAX_WARN_MIDI_SHORT 5
#define MAX_WARN_MIDI_LONG 5
#define MAX_WARN_UNISON_MUTE 5
#define MAX_OVERLAP_MONO 0.2
// Maximum allowed overlap for melody in polyphonic instrument (1 = 100% note length)
// Maximum allowed overlap for monophonic instrument (1 = 100% note length)
#define MAX_OVERLAP_POLY 0.2
// Trailing pauses to add at the end of track. Should be 2 or greater
#define TAIL_STEPS 4
// Minimum cantus size in steps to allow loading
#define MIN_CANTUS_SIZE 4
// Minimum counterpoint size in steps to allow loading
#define MIN_CP_SIZE 4

class MFIn :
	public MPort
{
public:
	MFIn();
	~MFIn();

	void LoadMidi(CString path);
	void UnisonMute(int step1, int step2);
	void MergeSmallOverlaps(int step1, int step2);

	void FillPause(int start, int length, int v);

	int midifile_loaded = 0; // If MIDI was loaded from file
	int xmlfile_loaded = 0; // If XML was loaded from file
	int midifile_type = 1; // Type of MIDI file loaded

protected:
  // Warnings
	int warning_loadmidi_align = 0;
	int warning_loadmidi_short = 0;
	int warning_loadmidi_long = 0;
	int warning_loadmidi_overlap = 0;
	int warning_unison_mute = 0;

};

