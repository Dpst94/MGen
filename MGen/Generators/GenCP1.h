#pragma once
#include "GenCA1.h"

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

class CGenCP1 :
	public CGenCA1
{
public:
	CGenCP1();
	~CGenCP1();

	void Generate() override;

protected:
	void LoadConfigLine(CString * sN, CString * sV, int idata, float fdata);
	int InitCP();
	void MakeNewCP();
	void SingleCPInit();
	void MultiCPInit();
	inline void ReseedCP();
	inline int FailAlteredInt2(int i, int c1, int c2, int flag);
	inline void GetAlteredInt(int i, int c1, int c2, int flag);
	inline int FailAlteredInt();
	inline int FailCrossInt2(int i, int i_1, int c1, int c2, int flag);
	inline int FailCrossInt();
	inline void GetVIntervals();
	inline void Get4th();
	inline int FailVMotion();
	inline int FailSus1();
	inline int GetAntici(int antici_load);
	inline int FailSus2();
	inline int FailSusCount();
	inline int FailSus();
	inline int FailDisSus();
	inline int FailUnison();
	inline int FailDis();
	inline int FailPcoSus();
	inline int FailPco();
	inline int SkipSus(int notes);
	inline int DetectPDD();
	inline int DetectDNT();
	inline int DetectCambiata();
	inline void SavePattern(int pattern);
	inline int DetectPatterns();
	inline void GetBasicMsh();
	inline int FailAdjacentTritone2(int ta, int t1, int t2, int tb);
	inline int FailAdjacentTritones();
	inline int FailTritones2();
	inline void SetMsh(int ls, vector<int> &l_msh, int val);
	inline void ApplyPDD(int ls, vector<int> &l_msh, int state);
	inline void ApplyDNT(int ls, vector<int> &l_msh, int state);
	inline void ApplyCam(int ls, vector<int> &l_msh, int state);
	inline void ApplyCam2(int ls, vector<int> &l_msh, int state);
	inline void ApplyFixedPat();
	inline int FailRhythm();
	inline int FailRhythm2();
	inline int FailRhythm4();
	inline int FailRhythm3();
	inline int FailRhythm5();
	inline int FailPcoApart();
	inline int FailPcoApartStep2(int iv, int & pco_last, int & mli_last, int & pco_last2);
	inline int FailPcoApartStep();
	inline int FailVIntervals();
	inline int FailVirtual4th();
	inline void SaveCP();
	inline void ShowBestRejectedCP();
	inline void SaveCPIfRp();
	inline int FailSlurs();
	void ScanCPInit();
	inline void SendMsh(int pos, int i, int v, int av, int x);
	void CalcPmap2();
	void GetPmap2();
	CString GetPmapLogHeader2();
	CString GetPmapLogSt2();
	void LogPmap2();
	inline void SendHarmColorCP(int pos, int v, int chm_id);
	inline int SendCP();
	inline int FailMissSlurs();
	inline int FailCPInterval();
	inline int FailOverlap();
	void RandomSWACP();
	void ShowLiningCP(vector<int>& cc);
	void SWACP(int i, int dp);
	inline int FailFirstIntervals();
	inline int FailLastIntervals();
	inline void GetNoteTypes();
	inline void CreateULinks();
	inline void GetMeasures();
	inline int FailStartPause();
	void EmulateSASCP();
	inline void GetCfli();
	inline int FailGisTrail2();
	inline int FailHarmStep(int i, const int * hv, int & count, int & wcount, int repeat_letters, int miss_letters, int flagr, int flagm);
	inline int EvalHarm();
	inline int FailTonicCP();
	inline void RemoveHarmDuplicate();
	inline int FailHarm();
	inline void GetBhli();
	inline void GetHarmBass();
	inline void OptimizeLastMeasure();
	inline int FailMaxNoteLen();
	void ScanCP(int t, int v);

	void LoadCantusHigh();
	void LoadSpecies();
	void ShrinkCantus();
	void TransposeCantus();

	// Load midi
	void LoadCP(CString path);
	void ProcessInter(int pos, int pos_old, std::vector<std::vector<std::pair<int, int>>> &inter, int hid, std::vector<int> &min_len, std::vector<int> &max_len);

	// Variables
	int cp_culm = 0; // Position of counterpoint culmination
	int warn_wrong_fn = 0; // If warning of wrong fn has fired

	// Cantus
	int cf_nmin = 0; // Minimum note in cantus (chromatic)
	int cf_nmax = 0; // Maximum note in cantus (chromatic)

	// Analysis
	vector <vector<int>> scpoint; // [v][s] Source cpoint for processing

	// PcoApart
	int pco5_last = 0; // Last note start
	int pco8_last = 0;
	int pco5_last2 = 0; // Last note end
	int pco8_last2 = 0;
	int mli5_last = 0; // Last note measure
	int mli8_last = 0;

	// CP1
	vector<int> ivl; // [s] Diatonic interval between voices
	vector<int> civl; // [s] Chromatic interval between voices
	vector<int> ivlc; // [s] Diatonic interval between voices (class)
	vector<int> civlc; // [s] Chromatic interval between voices (class)
	vector<int> civlc2; // [s] Shows lilypond optimized civlc (0 for unisone, 12 for octave)
	vector<int> tivl; // [s] Type of interval between voices
	vector<int> motion; // [s] Melody motion type
	vector<int> sus; // [ls] Note suspension flag (when above zero, links to first cantus-changing step)
	vector<int> susres; // [ls] =1 if sus is resolved correctly
	vector<int> isus; // [ls] Points to sus position or note start if there is no sus
	vector<int> cfli; // [cfs] Forward links to each cf note
	vector<int> hli; // [hs] Forward links to first notes of each harmony
	vector<int> ha64; // [hs] Audible 6/4 chord, while hbc will show root position or sixth chord
	vector<int> hli2; // [hs] Forward links to last notes of each harmony
	vector<int> hbcc; // [hs] Bass note of each harmony (chromatic)
	vector<int> hbc; // [hs] Bass note of each harmony (diatonic)
	vector<int> bhli; // [s] Back links to first notes of each harmony
	vector<int> mshb; // [ls] Melody shape types for fli (basic without patterns)
	vector<int> mshf; // [ls] Melody shape types for fli (with fixed patterns)
	vector<int> pat; // [ls] Pattern (cambiata, dnt...) for fli
	vector<int> pat_state; // [ls] Pattern (cambiata, dnt...) for fli state: 0 - not applied, 1 - fixed, 2,3 - variants

};
