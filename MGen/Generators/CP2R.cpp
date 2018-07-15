// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "CP2R.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

CP2R::CP2R() {
	// Data ready
	data_ready.resize(MAX_DATA_READY);
	data_ready_persist.resize(MAX_DATA_READY_PERSIST);
	warn_data_ready.resize(MAX_DATA_READY);
	warn_data_ready_persist.resize(MAX_DATA_READY_PERSIST);
}

CP2R::~CP2R() {
}

void CP2R::CreateLinks() {
	SET_READY(DR_fli);
	// Set first steps in case there is pause
	for (int v = 0; v < av_cnt; ++v) {
		int prev_note = -1;
		int lpos = 0;
		int l = 0;
		minl[v] = 10000;
		maxl[v] = 0;
		for (int i = 0; i < ep2; ++i) {
			if (prev_note != cc[v][i]) {
				// Save linked note length
				if (prev_note != -1) {
					llen[v][lpos - 1] = l;
					rlen[v][lpos - 1] = l;
					if (minl[v] > l) minl[v] = l;
					if (maxl[v] < l) maxl[v] = l;
					l = 0;
				}
				prev_note = cc[v][i];
				fli[v][lpos] = i;
				++lpos;
			}
			fli2[v][lpos - 1] = i;
			bli[v][i] = lpos - 1;
			l++;
		}
		fli_size[v] = lpos;
		llen[v][lpos - 1] = l;
		rlen[v][lpos - 1] = l;
	}
}

void CP2R::GetVca() {
	SET_READY(DR_vca);
	// Set first steps in case there is pause
	for (int s = 0; s < ep2; ++s) {
		vca[s] = 0;
		hva[s] = 0;
		lva[s] = av_cnt - 1;
		for (int v = 0; v < av_cnt; ++v) {
			if (cc[v][s]) {
				++vca[s];
				if (v > hva[s]) hva[s] = v;
				if (v < lva[s]) lva[s] = v;
			}
		}
	}
}

void CP2R::SendCP() {
	CreateLinks();
	int real_len = cc[0].size();
	int full_len = floor((real_len + 1) / 8 + 1) * 8;
	ResizeVectors(step0 + full_len);
	for (int v = 0; v < av_cnt; ++v) {
		int vi = vid[v];
		for (int ls = 0; ls < fli_size[v]; ++ls) {
			for (int s = fli[v][ls]; s <= fli2[v][ls]; ++s) {
				if (cc[v][s]) {
					note[step0 + s][vi] = cc[v][s];
					pause[step0 + s][vi] = 0;
				}
				else {
					note[step0 + s][vi] = 0;
					pause[step0 + s][vi] = 1;
				}
				len[step0 + s][vi] = llen[v][ls];
				coff[step0 + s][vi] = s - fli[v][ls];
				tempo[step0 + s] = cp_tempo;
			}
		}
	}
	for (int s = step0 + real_len; s < step0 + full_len; ++s) tempo[s] = tempo[s - 1];
	CountOff(step0, step0 + full_len - 1);
	CountTime(step0, step0 + full_len - 1);
	UpdateNoteMinMax(step0, step0 + full_len - 1);
	UpdateTempoMinMax(step0, step0 + full_len - 1);
	t_generated = step0 + full_len - 1;
	Adapt(0, t_generated);
	t_sent = t_generated;
}

inline void CP2R::ClearReady() {
	fill(data_ready.begin(), data_ready.end(), 0);
}

inline void CP2R::SetReady(int id) {
	data_ready[id] = 1;
}

inline void CP2R::SetReady(int id, int id2) {
	data_ready[id] = 1;
	data_ready[id2] = 1;
}

inline void CP2R::SetReady(int id, int id2, int id3) {
	data_ready[id] = 1;
	data_ready[id2] = 1;
	data_ready[id3] = 1;
}

inline void CP2R::ClearReadyPersist(int id) {
	data_ready_persist[id] = 0;
}

inline void CP2R::ClearReadyPersist(int id, int id2) {
	data_ready_persist[id] = 0;
	data_ready_persist[id2] = 0;
}

inline void CP2R::ClearReadyPersist(int id, int id2, int id3) {
	data_ready_persist[id] = 0;
	data_ready_persist[id2] = 0;
	data_ready_persist[id3] = 0;
}

inline void CP2R::SetReadyPersist(int id) {
	data_ready_persist[id] = 1;
}

inline void CP2R::SetReadyPersist(int id, int id2) {
	data_ready_persist[id] = 1;
	data_ready_persist[id2] = 1;
}

inline void CP2R::SetReadyPersist(int id, int id2, int id3) {
	data_ready_persist[id] = 1;
	data_ready_persist[id2] = 1;
	data_ready_persist[id3] = 1;
}

inline void CP2R::CheckReady(int id) {
	if (!data_ready[id] && !warn_data_ready[id]) {
		++warn_data_ready[id];
		CString est;
		est.Format("Attemp to use data element '%d' while it is not ready yet", id);
		WriteLog(5, est);
		ASSERT(0);
	}
}

inline void CP2R::CheckReady(int id, int id2) {
	CheckReady(id);
	CheckReady(id2);
}

inline void CP2R::CheckReady(int id, int id2, int id3) {
	CheckReady(id);
	CheckReady(id2);
	CheckReady(id3);
}

inline void CP2R::CheckReadyPersist(int id) {
	if (!data_ready_persist[id] && !warn_data_ready_persist[id]) {
		++warn_data_ready_persist[id];
		CString est;
		est.Format("Attemp to use persistent data element '%d' while it is not ready yet", id);
		WriteLog(5, est);
		ASSERT(0);
	}
}

inline void CP2R::CheckReadyPersist(int id, int id2) {
	CheckReadyPersist(id);
	CheckReadyPersist(id2);
}

inline void CP2R::CheckReadyPersist(int id, int id2, int id3) {
	CheckReadyPersist(id);
	CheckReadyPersist(id2);
	CheckReadyPersist(id3);
}

void CP2R::AnalyseCP() {
	CLEAR_READY();
	ClearFlags(0, c_len);
	GetDiatonic(0, c_len);
	GetPitchClass(0, c_len);
	CreateLinks();
	GetLClimax();
	GetLeapSmooth();
}

void CP2R::ClearFlags(int step1, int step2) {
	for (int v = 0; v < av_cnt; ++v) {
		for (int s = step1; s < step2; ++s) {
			flag[v][s].clear();
			fsl[v][s].clear();
			fvl[v][s].clear();
		}
	}
	fpenalty = 0;
}

void CP2R::GetPitchClass(int step1, int step2) {
	CHECK_READY(DR_c);
	SET_READY(DR_pc);
	for (int v = 0; v < av_cnt; ++v) {
		for (int s = step1; s < step2; ++s) {
			pc[v][s] = c[v][s] % 7;
			pcc[v][s] = (cc[v][s] + 12 - tonic_cur) % 12;
		}
	}
}

void CP2R::GetDiatonic(int step1, int step2) {
	SET_READY(DR_c);
	if (minor_cur) {
		for (int v = 0; v < av_cnt; ++v) {
			for (int s = step1; s < step2; ++s) {
				c[v][s] = m_CC_C(cc[v][s], tonic_cur);
			}
		}
	}
	else {
		for (int v = 0; v < av_cnt; ++v) {
			for (int s = step1; s < step2; ++s) {
				c[v][s] = maj_CC_C(cc[v][s], tonic_cur);
			}
		}
	}
}

void CP2R::GetLeapSmooth() {
	CHECK_READY(DR_c);
	SET_READY(DR_leap, DR_slur);
	for (int v = 0; v < av_cnt; ++v) {
		for (int i = 0; i < ep2 - 1; ++i) {
			// Find all leaps
			leap[v][i] = 0;
			smooth[v][i] = 0;
			slur[v][i + 1] = 0;
			if (cc[v][i] == cc[v][i + 1]) slur[v][i + 1] = 1;
			if (c[v][i + 1] - c[v][i] > 1) leap[v][i] = 1;
			else if (c[v][i + 1] - c[v][i] < -1) leap[v][i] = -1;
			// Find all smooth
			else if (c[v][i + 1] - c[v][i] == 1) smooth[v][i] = 1;
			else if (c[v][i + 1] - c[v][i] == -1) smooth[v][i] = -1;
		}
		leap[v][ep2 - 1] = 0;
		smooth[v][ep2 - 1] = 0;
		slur[v][0] = 0;
	}
}

void CP2R::GetLClimax() {
	SET_READY(DR_lclimax);
	for (int v = 0; v < av_cnt; ++v) {
		GetMovingMax(cc[v], max(lclimax_notes, lclimax_mea * npm), lclimax[v]);
		GetMovingMax(cc[v], lclimax_mea5[vsp[v]][av_cnt][0] * npm, lclimax2[v]);
	}
}

