// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "GenCA3.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

CGenCA3::CGenCA3() {
}

CGenCA3::~CGenCA3() {
}

void CGenCA3::InitAnalysis() {
	minl.resize(av_cnt);
	maxl.resize(av_cnt);
	fli_size.resize(av_cnt);
	fli.resize(av_cnt);
	fli2.resize(av_cnt);
	bli.resize(av_cnt);
	cc.resize(av_cnt);
	retr.resize(av_cnt);
	llen.resize(av_cnt);
	rlen.resize(av_cnt);
	for (int v = 0; v < av_cnt; ++v) {
		fli[v].resize(c_len);
		fli2[v].resize(c_len);
		bli[v].resize(c_len);
		cc[v].resize(c_len);
		retr[v].resize(c_len);
		llen[v].resize(c_len);
		rlen[v].resize(c_len);
	}
}

void CGenCA3::LoadConfigLine(CString* sN, CString* sV, int idata, float fdata) {
	LoadVar(sN, sV, "musicxml_file", &musicxml_file);

	CGenCP2::LoadConfigLine(sN, sV, idata, fdata);
}

int CGenCA3::XML_to_CP() {
	// Intermediate measures vector
	vector<int> im;

	av_cnt = xfi.voice.size();
	InitAnalysis();
	cp_tempo = 0;
	vname.resize(av_cnt);
	for (int vi = 0; vi < xfi.voice.size(); ++vi) {
		int v = av_cnt - vi - 1;
		int pos = 0;
		vname[v] = xfi.voice[vi].name;
		for (int m = 1; m < xfi.mea.size(); ++m) {
			im.resize(pos + 1);
			im[pos] = 1;
			for (int ni = 0; ni < xfi.note[vi][m].size(); ++ni) {
				int len = xfi.note[vi][m][ni].dur * 2.0 / xfi.note[vi][m][ni].dur_div;
				cc[v].resize(pos + len);
				retr[v].resize(pos + len);
				if (xfi.note[vi][m][ni].tempo && !cp_tempo)
					cp_tempo = xfi.note[vi][m][ni].tempo;
				if (!xfi.note[vi][m][ni].rest && !xfi.note[vi][m][ni].tie_stop) retr[v][pos] = 1;
				for (int s = 0; s < len; ++s) {
					cc[v][pos + s] = xfi.note[vi][m][ni].pitch;
				}
				pos += len;
				c_len = max(c_len, pos);
			}
		}
	}
	if (!cp_tempo) cp_tempo = 100;
	im.resize(c_len);
	ep2 = c_len;
	ResizeVectors(t_allocated, av_cnt);
	InitAnalysis();
	// Explode music into separate exercises
	// State: 0 - find note, 1 - find pause
	int state = 0;
	int s1 = 0;
	cp_id = 0;
	for (int s = 0; s < c_len; ++s) {
		int is_pause = 1;
		for (int v = 0; v < av_cnt; ++v) {
			if (cc[v][s]) {
				is_pause = 0;
				break;
			}
		}
		if (!state) {
			if (!is_pause) {
				s1 = s;
				state = 1;
			}
		}
		else {
			if (is_pause || s == c_len - 1) {
				int s2;
				if (!is_pause) s2 = s;
				else s2 = s - 1;
				// Move left to measure
				while (!im[s1] && s1 > 0) --s1;
				// Copy cp
				cp.resize(cp_id + 1);
				cp_retr.resize(cp_id + 1);
				cp_mea.resize(cp_id + 1);
				cp_vid.resize(cp_id + 1);
				cp[cp_id].resize(av_cnt);
				cp_retr[cp_id].resize(av_cnt);
				cp_mea[cp_id].resize(av_cnt);
				cp_vid[cp_id].resize(av_cnt);
				for (int v = 0; v < av_cnt; ++v) {
					cp[cp_id][v].resize(s2 - s1 + 1);
					cp_retr[cp_id][v].resize(s2 - s1 + 1);
					cp_mea[cp_id][v].resize(s2 - s1 + 1);
					cp_vid[cp_id][v] = v;
					for (int s3 = s1; s3 <= s2; ++s3) {
						cp[cp_id][v][s3 - s1] = cc[v][s3];
						cp_retr[cp_id][v][s3 - s1] = retr[v][s3];
						cp_mea[cp_id][v][s3 - s1] = im[s3];
					}
				}
				cp_id++;
				state = 0;
			}
		}
	}
	// Remove empty voices
	for (cp_id = 0; cp_id < cp.size(); ++cp_id) {
		vector<int> empty;
		empty.resize(av_cnt, 1);
		for (int v = 0; v < av_cnt; ++v) {
			for (int s = 0; s < cp[cp_id][v].size(); ++s) {
				if (cp[cp_id][v][s]) {
					empty[v] = 0;
					break;
				}
			}
		}
		for (int v = av_cnt-1; v >= 0; --v) {
			if (empty[v]) {
				verase(cp[cp_id], v);
				verase(cp_retr[cp_id], v);
				verase(cp_mea[cp_id], v);
				verase(cp_vid[cp_id], v);
			}
		}
	}
	return 0;
}

int CGenCA3::CheckXML() {
	for (int m = 1; m < xfi.mea.size(); ++m) {
		if (m > 1) {
			if (xfi.mea[m].beats != xfi.mea[m - 1].beats || xfi.mea[m].beat_type != xfi.mea[m - 1].beat_type) {
				CString est;
				est.Format("Measure %d size is changed",
					m);
				WriteLog(5, est);
				error = 10;
				return 1;
			}
		}
		for (int vi = 0; vi < xfi.voice.size(); ++vi) {
			for (int ni = 0; ni < xfi.note[vi][m].size(); ++ni) {
				float len = xfi.note[vi][m][ni].dur * 0.25 / xfi.note[vi][m][ni].dur_div;
				if (len < 0.125) {
					CString est;
					est.Format("Note length %.3f is shorter than 1/8 (0.125). Measure %d, vi %d, part id %s, part name %s, staff %d, voice %d, chord %d, beat %d/%d, note %d of %d",
						len, m, vi, xfi.voice[vi].id, xfi.voice[vi].name, 
						xfi.voice[vi].staff, xfi.voice[vi].v, xfi.voice[vi].chord,
						xfi.mea[m].beats, xfi.mea[m].beat_type, ni + 1, xfi.note[vi][m].size());
					WriteLog(5, est);
					error = 9;
					return 1;
				}
			}
		}
	}
	return 0;
}

void CGenCA3::Generate() {
	if (musicxml_file == "") {
		WriteLog(5, "MusicXML file not specified in configuration file");
		error = 7;
		return;
	}
	xfi.LoadXML(musicxml_file);
	if (!xfi.error.IsEmpty()) {
		WriteLog(5, xfi.error);
		error = 8;
		return;
	}
	xfi.ValidateXML();
	if (!xfi.error.IsEmpty()) {
		WriteLog(5, xfi.error);
		error = 9;
		return;
	}
	if (CheckXML()) return;
	if (XML_to_CP()) return;
	SendCP();
	//WriteLog(1, "Loaded MusicXML successfully");
}

void CGenCA3::SaveLy(CString dir, CString fname) {
}
