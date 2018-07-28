#pragma once
#include "CP2D.h"

// THESE MACROS CAN BE DISABLED TO IMPROVE PERFORMANCE

// Check data ready
#define MAX_DATA_READY 30
#define MAX_DATA_READY_PERSIST 20

// Persistent
#define DP_Config				1

// Non-persistent
#define DR_fli					1
#define DR_vca					2
#define DR_c						3
#define DR_pc						4
#define DR_leap					5
#define DR_slur					6
#define DR_lclimax			7
#define DR_beat					8
#define DR_sus					9
#define DR_msh					10
#define DR_mshb					11
#define DR_nmin					12
#define DR_dtp 					13
#define DR_macc					14
#define DR_hli					15
#define DR_hbc					16

#ifdef CF_DEBUG

#define CLEAR_READY() ClearReady()
#define CLEAR_READY_PERSIST(...) ClearReadyPersist(##__VA_ARGS__)
#define SET_READY(...) SetReady(##__VA_ARGS__)
#define SET_READY_PERSIST(...) SetReadyPersist(##__VA_ARGS__)
#define CHECK_READY(...) CheckReady(##__VA_ARGS__)
#define CHECK_READY_PERSIST(...) CheckReadyPersist(##__VA_ARGS__)

// Check rule usage
#define ASSERT_RULE(id) { if (ruleinfo[id].SubRuleName.IsEmpty() && warn_rule_undefined < 5) { ++warn_rule_undefined; CString est; est.Format("Detected undefined rule usage: %d", id); WriteLog(5, est); ASSERT(0); } }

#else

#define CLEAR_READY() 
#define SET_READY(...) 
#define CLEAR_READY_PERSIST(...) 
#define SET_READY_PERSIST(...) 
#define CHECK_READY(...) 
#define CHECK_READY_PERSIST(...) 

#define ASSERT_RULE(id) 

#endif

// Movement types
#define mStay 0
#define mParallel 1
#define mDirect 2
#define mOblique 3
#define mContrary 4

// Melody shape types
#define pDownbeat 1
#define pLeapTo 2
#define pLeapFrom 3
#define pSusStart 4
#define pSusRes 5
#define pLastLT 6
#define pLong 7
#define pAux -1
#define pPass -2

// Patterns
#define pCam 1 // Cambiata
#define pDNT 2 // Double-neighbour tone
#define pPDD 3 // Passing downbeat dissonance

// Report violation and save link inside voice
#define FLAGH(id, s) do { \
  ASSERT_RULE(id);  \
  if (skip_flags && !(*vaccept)[id]) return 1;  \
	flag[0][s].push_back(id);  \
	fsl[0][s].push_back(s);  \
	fvl[0][s].push_back(0);  \
} while (0)

// Report violation and save link inside voice
#define FLAGHL(id, s, s2) do { \
  ASSERT_RULE(id);  \
  if (skip_flags && !(*vaccept)[id]) return 1;  \
	flag[0][s].push_back(id);  \
	fsl[0][s].push_back(s2);  \
	fvl[0][s].push_back(0);  \
} while (0)

// Report violation and save link inside voice
#define FLAGV(id, s) do { \
  ASSERT_RULE(id);  \
  if (skip_flags && !(*vaccept)[id]) return 1;  \
	flag[v][s].push_back(id);  \
	fsl[v][s].push_back(s);  \
	fvl[v][s].push_back(v);  \
} while (0)

// Report violation and save link inside voice
#define FLAGVL(id, s, s2) do { \
  ASSERT_RULE(id);  \
  if (skip_flags && !(*vaccept)[id]) return 1;  \
	flag[v][s].push_back(id);  \
	fsl[v][s].push_back(s2);  \
	fvl[v][s].push_back(v);  \
} while (0)

class CP2R :
	public CP2D
{
public:
	CP2R();
	~CP2R();

protected:
	void ScanCP();
	inline void CreateLinks();
	inline void GetVca();
	void SendComment(int pos, int x, int i);
	void SendLining(int pos, int x, int i);
	void SendCP();
	
	// Check data ready
	inline void ClearReady();
	inline void SetReady(int id);
	inline void SetReady(int id, int id2);
	inline void SetReady(int id, int id2, int id3);
	inline void CheckReady(int id);
	inline void CheckReady(int id, int id2);
	inline void CheckReady(int id, int id2, int id3);
	inline void ClearReadyPersist(int id);
	inline void ClearReadyPersist(int id, int id2);
	inline void ClearReadyPersist(int id, int id2, int id3);
	inline void SetReadyPersist(int id);
	inline void SetReadyPersist(int id, int id2);
	inline void SetReadyPersist(int id, int id2, int id3);
	inline void CheckReadyPersist(int id);
	inline void CheckReadyPersist(int id, int id2);
	inline void CheckReadyPersist(int id, int id2, int id3);
	inline int EvaluateCP();
	inline int FailManyLeaps(int mleaps, int mleaped, int mleaps2, int mleaped2, int mleapsteps, int flag1, int flag2, int flag3, int flag4);
	inline void ClearFlags(int step1, int step2);
	inline void GetPitchClass(int step1, int step2);
	inline void GetDiatonic(int step1, int step2);
	inline void GetLeapSmooth();
	inline int FailIntervals();
	inline void GetLClimax();
	inline void GetNoteTypes();
	inline int FailGisTrail();
	inline int FailFisTrail();
	inline int FailMinor();
	inline int FailMinorStepwise();
	void MergeNotes(int step1, int step2);
	inline void GetBasicMsh();
	inline void GetDtp();
	inline void ApplyFixedPat();
	inline void CountFillInit(int tail_len, int pre, int & t1, int & t2, int & fill_end);
	inline void CountFill(int tail_len, int & skips, int & fill_to, int pre, int & fill_to_pre, int & fill_from_pre, int & fill_from, int & deviates, int & dev_count, int leap_prev, int & fill_end, int & fill_goal);
	inline void CountFillSkips(int leap_prev, int & skips, int t1, int t2);
	inline void CountFillLimits(int pre, int t1, int t2, int & fill_to, int & fill_to_pre, int & fill_from_pre, int & fill_from);
	inline void FailLeapInit(int & late_leap, int & presecond, int & leap_next, int & leap_prev, int & arpeg, int & overflow);
	inline int FailLeapMulti(int leap_next, int & arpeg, int & overflow, int & child_leap);
	inline int FailLeap();
	inline int FailLeapFill(int late_leap, int leap_prev, int child_leap);
	inline int FailLeapMDC();
	void GetMelodyInterval(int step1, int step2);
	inline int FailLeapSmooth(int l_max_smooth, int l_max_smooth_direct, int csel, int csel2, int flag1, int flag2, int flag3, int flag4, int first_run);
	inline int FailStagnation(int steps, int notes, int fl);
	inline int FailMultiCulm();
	inline int FailFirstNotes();
	inline int FailLastNotes();
	inline int FailAdSymRepeat(int relen);
	inline int IsRepeat(int ls1, int ls2, int relen);
	inline void maVector(vector<float>& v, vector<float>& v2, int range);
	inline void maVector(vector<int>& v, vector<float>& v2, int range);
	inline void mawVector(vector<float>& v, vector<float>& v2, int range);
	inline void mawVector(vector<int>& v, vector<float>& v2, int range);
	inline void MakeMacc();
	inline int FailLocalMacc(int notes, float mrange, int flag);
	inline int FailLocalRange(int notes, int mrange, int flag);
	inline int FailLocalPiCount(int notes, int picount, int flag);
	inline float GetTonicWeight(int l_ls, int tt);
	inline int FailTonic(int tt);
	inline int FailLastNoteRes();
	inline void GetTritoneResolution(int ta, int t1, int t2, int tb, int & res1, int & res2);
	inline int FailTritone(int ta, int t1, int t2, int tb);
	inline int FailTritones();
	inline int FailGlobalFill();
	inline int FailAdjacentTritone2(int ta, int t1, int t2, int tb);
	inline int FailAdjacentTritones();
	inline int FailTritones2();
	inline int FailRhythm();
	inline int FailRhythm2();
	inline int FailRhythm4();
	inline int FailRhythm3();
	inline int FailRhythm5();
	inline int FailMissSlurs();
	inline int FailSlurs();
	inline int FailStartPause();
	inline int FailMaxNoteLen();
	inline int FailSusCount();
	inline int FailNoteRepeat();
	inline int FailNoteLen();
	inline int FailBeat();
	inline int FailRetrInside();
	inline void GetHarm(vector<int>& chn, vector<int>& cchn);
	inline int FailHarm();
	inline int EvalHarm();
	inline int FailTonicCP();
	inline void RemoveHarmDuplicate();
	inline int FailHarmStep(int i, const int * hv, int & count, int & wcount, int repeat_letters, int miss_letters, int flagr, int flagm);
	inline void GetBhli();
	inline void GetHarmBass();
};

