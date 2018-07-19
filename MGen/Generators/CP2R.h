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
#define FLAGV(id, s, s2) do { \
  ASSERT_RULE(id);  \
  if (skip_flags && !(*vaccept)[id]) return 1;  \
	flag[v][s].push_back(id);  \
	fsl[v][s].push_back(s2);  \
	fvl[v][s].push_back(-1);  \
} while (0)



class CP2R :
	public CP2D
{
public:
	CP2R();
	~CP2R();

protected:
	inline void CreateLinks();
	inline void GetVca();

	void SendComment(int pos, int v, int x, int i);

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
	void AnalyseCP();
	inline int EvaluateCP();
	inline void ClearFlags(int step1, int step2);
	inline void GetPitchClass(int step1, int step2);
	inline void GetDiatonic(int step1, int step2);
	inline void GetLeapSmooth();
	inline void GetLClimax();
	inline void GetNoteTypes();
	inline int FailGisTrail();
	inline int FailFisTrail();
	inline int FailMinor();
	inline int FailMinorStepwise();
	void MergeNotes(int step1, int step2, int v);
	inline void GetBasicMsh();
	inline void ApplyFixedPat();
};

