int CP2R::FailMsh() {
	SET_READY(DR_msh);
	// Detect basic msh (based on downbeats and leaps)
	GetBasicMsh();
	if (FailSyncVIntervals()) return 1;
	if (FailVIntervals()) return 1;
	return 0;
}

void CP2R::GetBasicMsh() {
	CHECK_READY(DR_c, DR_fli, DR_leap);
	int first = 1;
	// Main calculation
	for (ls = 0; ls < fli_size[v]; ++ls) {
		s = fli[v][ls];
		if (!cc[v][s]) continue;
		if (first) {
			first = 0;
			// First note is always downbeat
			msh[v][ls] = pFirst;
			continue;
		}
		s2 = fli2[v][ls];
		// Sus start is always harmonic
		if (sus[v][ls]) msh[v][ls] = pSusStart;
		// Downbeat
		if (s % npm == 0) {
			// Long on downbeat
			if (llen[v][ls] > 4 || leap[v][s2])	msh[v][ls] = pDownbeat;
			// Downbeat note not surrounded by descending stepwise movement
			// TODO: Optimize for generation
			else if (smooth[v][s - 1] != -1 || (s2 < ep2 - 1 && smooth[v][s2 + 1] != -1)) {
				msh[v][ls] = pDownbeat;
			}
		}
		else if (abs(leap[v][s - 1]) > 2) msh[v][ls] = pLeapTo;
		else if (s2 < ep2 - 1 && abs(leap[v][s2]) > 2) msh[v][ls] = pLeapFrom;
		else {
			msh[v][ls] = pAux;
		}
	}
}

void CP2R::GetMeasureMsh() {
	// Get last measure step
	int mea_end = mli[ms] + npm - 1;
	// Prevent going out of window
	if (mea_end >= ep2) return;
	for (ls = bli[v][s0]; ls <= bli[v][mea_end]; ++ls) {
		s = fli[v][ls];
		if (!cc[v][s]) continue;
		s2 = fli2[v][ls];
		// First note is always downbeat
		if (s == fin[v]) msh[v][ls] = pFirst;
		// Sus start is always harmonic
		else if (sus[v][ls]) msh[v][ls] = pSusStart;
		// Downbeat
		else if (s % npm == 0) msh[v][ls] = pDownbeat;
		else if (s > 0 && leap[v][s - 1]) msh[v][ls] = pLeapTo;
		else if (s2 < ep2 - 1 && leap[v][s2]) msh[v][ls] = pLeapFrom;
		else msh[v][ls] = pAux;
	}
}

void CP2R::EvaluateMsh() {
	hpenalty = 0;
	// Get last measure step
	int mea_end = mli[ms] + npm - 1;
	// Prevent going out of window
	if (mea_end >= ep2) return;
	for (ls = bli[v][s0]; ls <= bli[v][mea_end]; ++ls) {
		s = fli[v][ls];
		if (!cc[v][s]) continue;
		// For first suspension in measure, evaluate last step. In other cases - first step
		if (s <= s0 && sus[v][ls]) {
			// For first suspended dissonance resolved note do not check msh
			if (susres[v][ls]) continue;
		}
		else {
			// For all other notes, check msh and iHarm4
			if (msh[v][ls] <= 0) continue;
		}
		if (!cchnv[pcc[v][s]]) {
			++hpenalty;
			// Do not flag discord if suspension, because it will be flagged in sus algorithm
			if (sus[v][ls]) {}
			else if (msh[v][ls] == pFirst) FLAGA(551, s, s, v);
			else if (msh[v][ls] == pDownbeat) FLAGA(83, s, s, v);
			else if (msh[v][ls] == pLeapTo) FLAGA(36, s, s, v);
			else if (msh[v][ls] == pLeapFrom) FLAGA(187, s, s, v);
			// pLastLT cannot be dissonance, because it is set only if it is not dissonance
			else if (msh[v][ls] == pSusStart) FLAGA(458, s, s, v);
			// pSusRes does not have separate flag, because it is marked as not resolved
			// This is protection against wrong melodic shape value
			else if (msh[v][ls] > 0) FLAGA(83, s, s, v);
		}
	}
}

void CP2R::GetMsh() {
	flaga.clear();
	for (ms = 0; ms < mli.size(); ++ms) {
		fill(chn.begin(), chn.end(), 0);
		fill(cchn.begin(), cchn.end(), 0);
		s0 = mli[ms];
		vc = vca[s0];
		int s9;
		// Get last measure step
		int mea_end = mli[ms] + npm - 1;
		// Prevent going out of window
		if (mea_end >= ep2) break;
		// Build harmony using already detected msh notes
		for (v = 0; v < av_cnt; ++v) {
			for (ls = bli[v][s0]; ls <= bli[v][mea_end]; ++ls) {
				s = fli[v][ls];
				// Skip pauses
				if (!cc[v][s]) continue;
				sp = vsp[v];
				s2 = fli2[v][ls];
				int lmsh = 0;
				// Skip first suspension, mark last suspension
				if (sus[v][ls]) {
					if (s < s0) continue;
					else lmsh = pSusStart;
				}
				// First note
				else if (s == fin[v]) lmsh = pFirst;
				// Downbeat
				else if (s % npm == 0) {
					// Long on downbeat
					if (llen[v][ls] > 4 || leap[v][s2])	lmsh = pDownbeat;
					// Downbeat note not surrounded by descending stepwise movement
					// TODO: Optimize for generation
					else if (smooth[v][s - 1] != -1 || (s2 < ep2 - 1 && smooth[v][s2] != -1)) {
						lmsh = pDownbeat;
					}
				}
				else if (abs(leap[v][s - 1]) > 2) lmsh = pLeapTo;
				else if (s2 < ep2 - 1 && abs(leap[v][s2]) > 2) lmsh = pLeapFrom;
				else {
					lmsh = pAux;
				}
				// Skip non-harmonic and ambiguous notes
				if (lmsh <= 0) continue;
				// Record note
				++chn[pc[v][s]];
				++cchn[pcc[v][s]];
			}
		}
		// Main chord
		int lchm;
		int lchm_alter;
		GetHarm(0, 0, lchm, lchm_alter);
		// Possible chords
		vector <int> cpos;
		cpos.resize(7);
		// Detect all possible chords
		for (hv = 0; hv < 7; ++hv) {
			// At least one note exists
			if (!chn[hv] && !chn[(hv + 2) % 7] && !chn[(hv + 4) % 7]) continue;
			// No other notes should exist
			if (chn[(hv + 1) % 7] || chn[(hv + 3) % 7] || chn[(hv + 5) % 7]) continue;
			// Do not check for 7th:  || chn[(hv + 6) % 7]
			cpos[hv] = 1;
			/*
			CString st, est;
			est.Format("Possible chord %s in measure %d:%d",
				degree_name[hv], cp_id + 1, ms + 1);
			WriteLog(1, est);
			*/
		}
		// Scan all possible chords
		for (int hv2 = lchm + 7; hv2 > lchm; --hv2) {
			hv = hv2 % 7;
			if (!cpos[hv]) continue;
			for (hv_alt = 0; hv_alt <= 1; ++hv_alt) {
				// Only for melodic minor
				if (hv_alt && !mminor) continue;
				fill(cchnv.begin(), cchnv.end(), 0);
				cchnv[(c_cc[hv + 14] + 24 - bn) % 12] = 1;
				cchnv[(c_cc[hv + 16] + 24 - bn) % 12] = 1;
				cchnv[(c_cc[hv + 18] + 24 - bn) % 12] = 1;
				if (mminor) {
					if (hv_alt) {
						if (cchnv[8]) {
							// Skip if this variant conflicts with detected notes
							if (cchn[8]) continue;
							// Convert to altered
							cchnv[8] = 0;
							cchnv[9] = 1;
						}
						else if (cchnv[10]) {
							// Skip if this variant conflicts with detected notes
							if (cchn[10]) continue;
							// Convert to altered
							cchnv[10] = 0;
							cchnv[11] = 1;
						}
						else continue;
					}
					else {
						if (cchnv[8]) {
							// Skip if this variant conflicts with detected notes
							if (cchn[9]) continue;
						}
						if (cchnv[10]) {
							// Skip if this variant conflicts with detected notes
							if (cchn[11]) continue;
						}
					}
				}
				CString st, est;
				est.Format("Checking chord %s%s in measure %d:%d:", 
					degree_name[hv], hv_alt?"*":"", cp_id+1, ms+1);
				for (int i = 0; i < 12; ++i) {
					st.Format(" %d", cchnv[i]);
					est += st;
					if (i == 5) est += " -";
				}
				//WriteLog(1, est);
				flaga.clear();
				for (v = 0; v < av_cnt; ++v) {
					sp = vsp[v];
					GetMeasureMsh();
					ls = bli[v][s0];
					s = fli[v][ls];
					s2 = fli2[v][ls];
					DetectSus();
					DetectPDD();
					DetectDNT();
					DetectCambiata();
					EvaluateMsh();
				}
				// Stop evaluating variants if all is ok
				if (!hpenalty) break;
			}
			// Stop evaluating variants if all is ok
			if (!hpenalty) break;
		}
		// Save flags
		for (int fl = 0; fl < flaga.size(); ++fl) {
			v = flaga[fl].voice;
			FLAGL(flaga[fl].id, flaga[fl].s, flaga[fl].fsl, flaga[fl].fvl);
		}
	}
}

void CP2R::DetectDNT() {
	if (!accept[sp][vc][0][258]) return;
	// Suspension will conflict with DNT
	int ls0 = bli[v][mli[ms]];
	if (sus[v][ls]) return;
	int ls_max = bli[v][mli[ms] + npm - 1] - 3;
	if (ls_max > fli_size[v] - 4) ls_max = fli_size[v] - 4;
	// Not enough notes for DNT
	if (ls_max < ls0) return;
	for (ls = ls0; ls <= ls_max; ++ls) {
		s = fli[v][ls];
		s2 = fli2[v][ls];
		// No pauses
		if (!cc[v][s] || !cc[v][s2 + 1] || !cc[v][fli[v][ls + 2]] || !cc[v][fli[v][ls + 3]]) return;
		// First note must be chord tone
		if (!cchnv[pcc[v][s2]]) return;
		// Note 1 is short
		if (llen[v][ls] < 2) continue;
		// Note 2 is long
		if (llen[v][ls + 1] > 2) continue;
		// Movement is stepwize
		if (!smooth[v][s2]) continue;
		if (ls < fli_size[v] - 2) {
			// Note 2 is longer than 1
			if (llen[v][ls + 1] > llen[v][ls]) continue;
			// Note 3 is long
			if (llen[v][ls + 2] > 2) continue;
			// Wrong
			if (abs(leap[v][fli2[v][ls + 1]]) != 2) continue;
			// Leap has same direction
			if (leap[v][fli2[v][ls + 1]] * smooth[v][s2] > 0) continue;
			// Leap in (before DNT)
			if (ls > 0 && leap[v][fli2[v][ls - 1]]) {
				if (!accept[sp][vc][0][3]) continue;
			}
			if (ls < fli_size[v] - 3) {
				// Note 4 is short
				if (llen[v][ls + 3] < 2) continue;
				// Note 1 and 4 are different
				if (cc[v][s] != cc[v][fli[v][ls + 3]]) continue;
				// Note 3 is longer than 4
				if (llen[v][ls + 2] > llen[v][ls + 3] && (ep2 == c_len || ls < fli_size[v] - 4)) continue;
				// Movements are stepwize
				if (!smooth[v][fli2[v][ls + 2]]) continue;
				// Both movements have same direction
				if (smooth[v][s2] != smooth[v][fli2[v][ls + 2]]) continue;
				if (ls < fli_size[v] - 4) {
					// Leap from note 4
					if (leap[v][fli2[v][ls + 3]]) {
						if (!accept[sp][vc][0][97]) continue;
					}
					// Apply pattern
					msh[v][ls] = pHarmonicDNT1;
					msh[v][ls + 1] = pAuxDNT1;
					msh[v][ls + 2] = pAuxDNT2;
					msh[v][ls + 3] = pHarmonicDNT2;
				}
			}
		}
	}
}

void CP2R::DetectCambiata() {
	if (!accept[sp][vc][0][256]) return;
	// Suspension will conflict with cambiata
	int ls0 = bli[v][mli[ms]];
	if (sus[v][ls0]) return;
	int ls_max = bli[v][mli[ms] + npm - 1] - 3;
	if (ls_max > fli_size[v] - 4) ls_max = fli_size[v] - 4;
	// Not enough notes for cambiata
	if (ls_max < ls0) return;
	for (ls = ls0; ls <= ls_max; ++ls) {
		s = fli[v][ls];
		s2 = fli2[v][ls];
		// No pauses
		if (!cc[v][s] || !cc[v][s2 + 1] || !cc[v][fli[v][ls + 2]] || !cc[v][fli[v][ls + 3]]) return;
		// First note must be chord tone
		if (!cchnv[pcc[v][s2]]) return;
		// Note 1 is short
		if (llen[v][ls] < 2) continue;
		// Note 2 is long
		if (llen[v][ls + 1] > 2) continue;
		// Movement is stepwize
		if (!smooth[v][s2]) continue;
		if (ls < fli_size[v] - 2) {
			// Note 2 is longer than 1
			if (llen[v][ls + 1] > llen[v][ls]) continue;
			// Note 3 is long
			if (llen[v][ls + 2] > 4) continue;
			// Wrong
			if (abs(leap[v][fli2[v][ls + 1]]) != 2) continue;
			// Leap has other direction
			if (leap[v][fli2[v][ls + 1]] * smooth[v][s2] < 0) continue;
			if (ls < fli_size[v] - 3) {
				// Fourth note must be chord tone
				if (!cchnv[pcc[v][fli[v][ls + 3]]]) return;
				// Note 4 is short
				if (llen[v][ls + 3] < 2) continue;
				// Note 3 is longer than 4
				if (llen[v][ls + 2] > llen[v][ls + 3] && (ep2 == c_len || ls < fli_size[v] - 4)) continue;
				// Both movements have different directions
				if (smooth[v][s2] != -smooth[v][fli2[v][ls + 2]]) continue;
				if (ls < fli_size[v] - 4) {
					// Leap from note 4
					if (abs(leap[v][fli2[v][ls + 3]]) > 3) {
						if (!accept[sp][vc][0][97]) continue;
					}
					// Apply pattern
					msh[v][ls] = pHarmonicCam1;
					msh[v][ls + 1] = pAuxCam1;
					msh[v][ls + 2] = pAuxCam2;
					msh[v][ls + 3] = pHarmonicCam2;
				}
			}
		}
	}
}

void CP2R::DetectSus() {
	susres[v][ls] = 0;
	if (!ms) return;
	if (!cc[v][s0]) return;
	if (!sus[v][ls]) return;
	// Allow if not discord
	if (cchnv[pcc[v][s0]]) {
		// Are there enough notes for resolution ornament?
		if (ls < fli_size[v] - 2) {
			s3 = fli2[v][ls + 1];
			s4 = fli2[v][ls + 2];
			// Is there a dissonance between two consonances, forming stepwise descending movement?
			if (!cchnv[pcc[v][s3]] && cchnv[pcc[v][s4]] && c[v][s2] - c[v][s4] == 1 &&
				llen[v][ls + 2] >= 2 && beat[v][ls + 2] < 10) {
				// Detect stepwise+leap or leap+stepwise
				if ((c[v][s3] - c[v][s2] == 1 && c[v][s3] - c[v][s4] == 2) ||
					(c[v][s2] - c[v][s3] == 2 && c[v][s4] - c[v][s3] == 1)) {
					susres[v][ls] = 1;
					msh[v][ls + 1] = pAux;
				}
			}
		}
		return;
	}
	// Last sus
	if (ls >= fli_size[v] - 1) {
		return;
	}
	// Full measure should be generated
	if (sus[v][ls] + npm >= ep2 && ep2 < c_len) {
		return;
	}
	// Available beats
	s3 = sus[v][ls] + npm / 4;
	s4 = sus[v][ls] + npm / 2;
	s5 = sus[v][ls] + npm * 3 / 4;
	// For species 2 and 4 check only 3rd beat
	if (sp == 2 || sp == 4) {
		s3 = 0;
		s5 = 0;
	}
	// Check which beats are allowed by rules
	if (!accept[sp][vc][0][419]) s3 = 0;
	if (!accept[sp][vc][0][420]) s5 = 0;
	if (s3) ls3 = bli[v][s3];
	if (s4) ls4 = bli[v][s4];
	if (s5) ls5 = bli[v][s5];
	// Notes not on beat? 
	if (!accept[sp][vc][0][286]) {
		if (s3 && cc[v][s3] == cc[v][s3 - 1]) s3 = 0;
		if (s4 && cc[v][s4] == cc[v][s4 - 1]) s4 = 0;
		if (s5 && cc[v][s5] == cc[v][s5 - 1]) s5 = 0;
		FLAGAR(286, s, s, v);
	}
	// Notes too short?
	if (!accept[sp][vc][0][291]) {
		if (s3 && llen[v][ls3] < 2 && ls3 < fli_size[v] - 1) s3 = 0;
		if (s4 && llen[v][ls4] < 2 && ls4 < fli_size[v] - 1) s4 = 0;
		if (s5 && llen[v][ls5] < 2 && ls5 < fli_size[v] - 1) s5 = 0;
		FLAGAR(291, s, s, v);
	}
	// Notes not harmonic?
	if (!accept[sp][vc][0][220]) {
		if (s3 && !cchnv[pcc[v][s3]]) s3 = 0;
		if (s4 && !cchnv[pcc[v][s4]]) s4 = 0;
		if (s5 && !cchnv[pcc[v][s5]]) s5 = 0;
		FLAGAR(220, s, s, v);
	}
	// Resolution by leap
	if (!accept[sp][vc][0][221]) {
		if (s3 && abs(c[v][s3] - c[v][s2]) > 1) s3 = 0;
		if (s4 && abs(c[v][s4] - c[v][s2]) > 1) s4 = 0;
		if (s5 && abs(c[v][s5] - c[v][s2]) > 1) s5 = 0;
		FLAGAR(221, s, s, v);
	}
	// Resolution up not LT
	if (!accept[sp][vc][0][219]) {
		if (s3 && cc[v][s3] > cc[v][s2] && pcc[v][s2] != 11) s3 = 0;
		if (s4 && cc[v][s4] > cc[v][s2] && pcc[v][s2] != 11) s4 = 0;
		if (s5 && cc[v][s5] > cc[v][s2] && pcc[v][s2] != 11) s5 = 0;
		FLAGAR(219, s, s, v);
	}
	// Notes have too many insertions?
	if (!accept[sp][vc][0][292]) {
		if (s3 && ls3 - ls > 3) s3 = 0;
		if (s4 && ls4 - ls > 3) s4 = 0;
		if (s5 && ls5 - ls > 3) s5 = 0;
		FLAGAR(292, s, s, v);
	}
	// First leap is too long?
	if (abs(cc[v][fli[v][ls + 1]] - cc[v][s2]) > sus_insert_max_leap[sp][vc][0]) {
		FLAGA(295, fli[v][ls + 1], sus[v][ls], v);
	}
	// Single insertion, second movement is leap
	if (!accept[sp][vc][0][136]) {
		if (s3 && ls3 == ls + 2 && leap[v][fli2[v][ls + 1]] > 0) s3 = 0;
		if (s4 && ls4 == ls + 2 && leap[v][fli2[v][ls + 1]] > 0) s4 = 0;
		if (s5 && ls5 == ls + 2 && leap[v][fli2[v][ls + 1]] > 0) s5 = 0;
		FLAGAR(136, s, s, v);
	}
	// Single insertion, second movement is leap
	if (!accept[sp][vc][0][296]) {
		if (s3 && ls3 == ls + 2 && leap[v][fli2[v][ls + 1]] < 0) s3 = 0;
		if (s4 && ls4 == ls + 2 && leap[v][fli2[v][ls + 1]] < 0) s4 = 0;
		if (s5 && ls5 == ls + 2 && leap[v][fli2[v][ls + 1]] < 0) s5 = 0;
		FLAGAR(296, s, s, v);
	}
	// Mark insertion as non-harmonic in basic msh if resolution is harmonic, sus ends with dissonance and not both movements are leaps
	if (s3 && ls3 == ls + 2 && cchnv[pcc[v][fli[v][ls + 2]]] &&
		leap[v][fli2[v][ls]] * leap[v][fli2[v][ls + 1]] == 0) msh[v][ls + 1] = pAux;
	if (s4 && ls4 == ls + 2 && cchnv[pcc[v][fli[v][ls + 2]]] &&
		leap[v][fli2[v][ls]] * leap[v][fli2[v][ls + 1]] == 0) msh[v][ls + 1] = pAux;
	if (s5 && ls5 == ls + 2 && cchnv[pcc[v][fli[v][ls + 2]]] &&
		leap[v][fli2[v][ls]] * leap[v][fli2[v][ls + 1]] == 0) msh[v][ls + 1] = pAux;
	// Mark resolution as obligatory harmonic in basic msh and ending as non-harmonic
	// In this case all resolutions are marked, although one of them is enough, but this is a very rare case that several resolutions pass all checks
	if (s3) {
		msh[v][ls3] = pSusRes;
		susres[v][ls] = 1;
	}
	if (s4) {
		msh[v][ls4] = pSusRes;
		susres[v][ls] = 1;
	}
	if (s5) {
		msh[v][ls5] = pSusRes;
		susres[v][ls] = 1;
	}
}

void CP2R::DetectPDD() {
	// First measure
	if (!ms) return;
	if (!cc[v][s0]) return;
	// Last note
	if (s2 == ep2 - 1) return;
	// Second note must be non-chord tone
	if (cchnv[pcc[v][s2]]) return;
	// Suspension will conflict with PDD
	if (sus[v][ls]) return;
	// No pauses
	if (!cc[v][s] || !cc[v][s - 1] || !cc[v][s2 + 1]) return;
	// Stepwize downward movement
	if (c[v][s] - c[v][s - 1] != -1) return;
	// Note 2 is not too long
	if (llen[v][ls] > 4) return;
	if (ls < fli_size[v] - 1) {
		// Stepwize downward movement
		if (c[v][s2 + 1] - c[v][s2] != -1) return;
		// Note 2 is not longer than 3
		if (llen[v][ls] > llen[v][ls + 1] && (ep2 == c_len || ls < fli_size[v] - 2)) return;
		// Third note must be chord note
		if (!cchnv[pcc[v][s2 + 1]]) return;
		// Apply pattern
		msh[v][ls] = pAuxPDD;
		msh[v][ls + 1] = pHarmonicPDD;
	}
}

