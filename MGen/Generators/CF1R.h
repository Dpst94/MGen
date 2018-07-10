#pragma once
#include "CF1D.h"

// THESE MACROS CAN BE DISABLED TO IMPROVE PERFORMANCE

// Check data ready
#define MAX_DATA_READY 30
#define MAX_DATA_READY_PERSIST 20

// Persistent
#define DP_Config				1
#define DP_ConfigTest		2
#define DP_Rules				3
#define DP_RuleParam		5
#define DP_RuleSetParam 6
#define DP_cc_old				7
#define DP_cc_incr			8
#define DP_hv						9
#define DP_hsp					10
#define DP_fstat				11
#define DP_fcor					12
#define DP_fblock				13
#define DP_mli					14
#define DP_cfli					15

// Non-persistent
#define DR_c						1
#define DR_fli					2
#define DR_macc					3
#define DR_pc						4
#define DR_nmin					5
#define DR_cc_order			7
#define DR_leap					8
#define DR_slur					9
#define DR_culm_ls			10
#define DR_ivl					11
#define DR_sus					12
#define DR_motion				13
#define DR_beat					14
#define DR_rpenalty_cur 15
#define DR_msh					17
#define DR_uli					18
#define DR_pat					19
#define DR_mshb				  20
#define DR_mshf				  21
#define DR_retrigger    22
#define DR_hbc			    23
#define DR_hli			    24
#define DR_lclimax			25

#ifdef CF_DEBUG

#define CLEAR_READY() ClearReady()
#define CLEAR_READY_PERSIST(...) ClearReadyPersist(##__VA_ARGS__)
#define SET_READY(...) SetReady(##__VA_ARGS__)
#define SET_READY_PERSIST(...) SetReadyPersist(##__VA_ARGS__)
#define CHECK_READY(...) CheckReady(##__VA_ARGS__)
#define CHECK_READY_PERSIST(...) CheckReadyPersist(##__VA_ARGS__)

// Check rule usage
#define ASSERT_RULE(id) { if (SubRuleName[cspecies][id].IsEmpty() && warn_rule_undefined < 5) { ++warn_rule_undefined; CString est; est.Format("Detected undefined rule usage: %d", id); WriteLog(5, est); ASSERT(0); } }

#else

#define CLEAR_READY() 
#define SET_READY(...) 
#define CLEAR_READY_PERSIST(...) 
#define SET_READY_PERSIST(...) 
#define CHECK_READY(...) 
#define CHECK_READY_PERSIST(...) 

#define ASSERT_RULE(id) 

#endif

// Checks if we have leap or melody direction change here: needs to be not last note
#define MELODY_SEPARATION(s, s1) (!s || (leap[s - 1]) || ((c[s] - c[s - 1])*(c[s1] - c[s]) < 0))

// This variant does not use SWA rpenalty optimization
//#define SWA_OPTIMIZER(i)
// This variant skips flags inside SWA window when evaluating not full cp (performance increase)
//#define SWA_OPTIMIZER(i) || (task == tCor && method == mSWA && i >= swa1 && i < swa2 && ep2 < c_len)
// This variant always skips flags inside SWA window (more performance increase
#define SWA_OPTIMIZER(i) || (task == tCor && method == mSWA && (i) >= swa1 && (i) < swa2)

// Report violation from main function
#define FLAG(id, i) do {  \
	ASSERT_RULE(id);  \
	if ((skip_flags SWA_OPTIMIZER(i)) && (accept[id] == 0)) goto skip;  \
	flags[0] = 0;  \
	++flags[id];  \
	anflags[cpv][i].push_back(id);  \
	anfl[cpv][i].push_back(i);  \
} while (0)

// Report violation from main function and save link
#define FLAGL(id, i, i2) do {  \
	ASSERT_RULE(id);  \
	if ((skip_flags SWA_OPTIMIZER(i)) && (accept[id] == 0)) goto skip;  \
	flags[0] = 0;  \
	++flags[id];  \
	anflags[cpv][i].push_back(id);  \
	anfl[cpv][i].push_back(i2);  \
} while (0)

// Report violation from child function
#define FLAG2(id, i) do { \
  ASSERT_RULE(id);  \
  if ((skip_flags SWA_OPTIMIZER(i)) && (accept[id] == 0)) return 1;  \
	flags[0] = 0;  \
	++flags[id];  \
	anflags[cpv][i].push_back(id);  \
	anfl[cpv][i].push_back(i);  \
} while (0)

// Flag and go to next cycle
#define FLAG2C(id) do { \
	if (!s3 && !s4 && !s5) { \
		FLAG2(id, sus[ls]); \
		goto skipsus; \
	} \
} while (0)


// Report violation and save link
#define FLAG2L(id, i, i2) do { \
  ASSERT_RULE(id);  \
  if ((skip_flags SWA_OPTIMIZER(i)) && (accept[id] == 0)) return 1;  \
	flags[0] = 0;  \
	++flags[id];  \
	anflags[cpv][i].push_back(id);  \
	anfl[cpv][i].push_back(i2);  \
} while (0)

// For harmony
#define FLAG3(id, i) do {  \
	ASSERT_RULE(id);  \
	if (!accept[id]) {  \
		last_flag=id;  \
		return 1;  \
	}  \
} while (0)

// Simply flag
#define FLAG4(id, i) do {  \
	ASSERT_RULE(id);  \
	flags[0] = 0;  \
	++flags[id];  \
	anflags[cpv][i].push_back(id);  \
} while (0)

// Letters in harmonies
const int hvt[] = { 1, 0, 1, 0, 0, 1, 0 };
const int hvd[] = { 0, 0, 1, 0, 1, 0, 1 };
const int hvs[] = { 0, 1, 0, 1, 0, 1, 0 };

#define SCAN_VISUAL_CODE_BASE 3
const char scan_visual_code[] = { '!', '.', ',', ':', ';', '`', '|', '(', ')', '[', ']', '{', '}', ' / ', '\\' };

const CString TaskNames[] = {
	"evaluate", // 0
	"correct", // 1
	"generate" // 2
};

// Task
#define tGen 0
#define tEval 1
#define tCor 2

// Harmonic functions
#define hUndefined -1
#define hTon 0
#define hSub 1
#define hDom 2

// Interval types
#define iDis   -2 // Dissonance
#define iHarm4 -1 // Harmonic 4th
#define iPco   2 // Perfect consonance
#define iIco   3 // Imperfect consonance

class CF1R :
	public CF1D
{
public:
	CF1R();
	~CF1R();

protected:
	inline int FailNoteRepeat(vector<int> &c, int step1, int step2);
	inline int FailNoteSeq(vector<int>& pc);
	inline int FailLocalRange(vector<int>& c, int notes, int mrange, int flag);
	inline int FailLocalPiCount(vector<int>& cc, int notes, int picount, int flag);
	inline int FailLocalMacc(int notes, float mrange, int flag);
	inline void GetMelodyInterval(vector<int>& cc, int step1, int step2, int & nmin, int & nmax);
	inline void GetPitchClass(vector<int>& c, vector<int>& cc, vector<int>& pc, vector<int>& pcc, int step1, int step2);
	inline int FailGisTrail(vector<int>& pcc);
	inline int FailFisTrail(vector<int>& pcc);
	inline int FailMelodyHarmStep(int i, const int * hv, int & count, int & wcount, int & last_flag, int & max_p, int repeat_letters, int miss_letters, int flagr, int flagm);
	inline int EvalMelodyHarm(int p, int & last_flag, int & max_p);
	inline int FailMelodyHarm(vector<int>& pc, vector<int>& pcc);
	inline void GetChromatic(vector<int>& c, vector<int>& cc, int step1, int step2, int minor_cur);
	inline int FailDiatonic(vector<int>& c, vector<int>& cc, int step1, int step2, int minor_cur);
	inline int IsRepeat(int ls1, int ls2, vector<int>& c, vector<int>& cc, vector<int>& leap, int rlen);
	inline int FailAdSymRepeat(vector<int>& c, vector<int>& cc, vector<int>& leap, int rlen);
	inline int FailOutstandingRepeat(vector<int>& c, vector<int>& cc, vector<int>& leap, int scan_len, int rlen, int fid);
	inline int FailLongRepeat(vector<int>& c, vector<int>& cc, vector<int>& leap, int scan_len, int rlen, int fid);
	inline int FailManyLeaps(vector<int>& c, vector<int>& cc, vector<int>& leap, vector<int>& smooth, vector<int>& slur, int mleaps, int mleaped, int mleaps2, int mleaped2, int mleapsteps, int flag1, int flag2, int flag3, int flag4);
	inline void GetLeapSmooth(vector<int>& c, vector<int>& cc, vector<int>& leap, vector<int>& smooth, vector<int>& slur);
	inline int FailLeapSmooth(vector<int>& c, vector<int>& cc, vector<int>& leap, vector<int>& smooth, vector<int>& slur, int l_max_smooth, int l_max_smooth_direct, int csel, int csel2, int flag1, int flag2, int flag3, int flag4, int first_run);
	inline int FailStagnation(vector<int>& cc, vector<int>& nstat, int steps, int notes, int flag);
	inline int FailMultiCulm(vector<int>& cc, vector<int>& slur);
	inline int FailFirstNotes(vector<int>& pc);
	inline int FailLastNotes(vector<int>& pc, vector<int>& pcc);
	inline void CreateLinks(vector<int>& cc, int multivoice);
	inline void CountFillInit(vector<int>& c, int tail_len, int pre, int & t1, int & t2, int & fill_finish);
	inline void CountFill(vector<int>& c, int tail_len, vector<int>& nstat2, vector<int>& nstat3, int & skips, int & fill_to, int pre, int & fill_to_pre, int & fill_from_pre, int & fill_from, int & deviates, int & dev_count, int leap_prev, int & fill_end, int & fill_goal);
	inline void CountFillSkips(int leap_prev, int & skips, int t1, int t2);
	inline void CountFillLimits(vector<int>& c, int pre, int t1, int t2, int & fill_to, int & fill_to_pre, int & fill_from_pre, int & fill_from);
	inline void FailLeapInit(vector<int>& c, int & late_leap, int & presecond, int & leap_next, int & leap_prev, int & arpeg, int & overflow, vector<int>& leap);
	inline int FailLeapMulti(int leap_next, int & arpeg, int & overflow, int & child_leap, vector<int>& c, vector<int>& leap);
	inline int FailLeap(vector<int>& c, vector<int>& cc, vector<int>& leap, vector<int>& smooth, vector<int>& nstat2, vector<int>& nstat3);
	inline int FailLeapFill(vector<int>& c, int late_leap, int leap_prev, int child_leap);
	inline int FailLeapMDC(vector<int>& leap, vector<int>& cc);
	inline float GetTonicWeight(int l_ls, vector<int>& c, vector<int>& cc, vector<int>& pc, int tt);
	inline int FailTonic(vector<int>& c, vector<int>& cc, vector<int>& pc, int tt);
	inline int FailLastNoteRes(vector<int>& pc);
	inline int FailIntervals(vector<int>& c, vector<int>& cc, vector<int>& pc, vector<int>& pcc);
	inline void GetTritoneResolution(int ta, int t1, int t2, int tb, int & res1, int & res2, vector<int>& c, vector<int>& cc, vector<int>& pc, vector<int>& pcc);
	inline int FailTritone(int ta, int t1, int t2, int tb, vector<int>& c, vector<int>& cc, vector<int>& pc, vector<int>& pcc, vector<int>& leap);
	inline int FailTritones(vector<int>& c, vector<int>& cc, vector<int>& pc, vector<int>& pcc, vector<int>& leap);
	inline int FailGlobalFill(vector<int>& c, vector<int>& nstat2);
	inline int FailMinorStepwise(vector<int>& pcc, vector<int>& cc, vector<int>& c);
	inline int FailMinor(vector<int>& pcc, vector<int>& cc);
	inline void GetLClimax();
	inline void maVector(vector<float>& v, vector<float>& v2, int range);
	inline void maVector(vector<int>& v, vector<float>& v2, int range);
	inline void mawVector(vector<int>& v, vector<float>& v2, int range);
	inline void mawVector(vector<float>& v, vector<float>& v2, int range);
	inline void MakeMacc(vector<int>& cc);

	// Scan
	inline void ClearFlags(int step1, int step2);
	void CalcCcIncrement();

	void ScanInit();
	void ScanCantusInit();
	int GetMinSmap();
	int GetMaxSmap();
	void GetRealRange(vector<int>& c, vector<int>& cc);
	void GetSourceRange(vector<int>& cc);
	void ApplySourceRange();
	void SingleCantusInit();
	inline void ResizeToWindow();
	void MakeNewCantus(vector<int>& c, vector<int>& cc);
	void MultiCantusInit(vector<int>& c, vector<int>& cc);
	int FailWindowsLimit();
	inline void CalcFlagStat();
	inline int FailFlagBlock();
	inline int FailAccept();
	inline void NextWindow(vector<int> &cc);
	inline void CalcRpenalty(vector<int>& cc);
	inline void ScanLeft(vector<int> &cc, int &finished);
	inline void ShowBestRejected(vector<int>& cc);
	inline void BackWindow(vector<int>& cc);
	inline int CalcDpenalty(vector<int>& cc1, vector<int>& cc2, int s1, int s2);
	inline void CalcStepDpenalty(vector<int> &cc1, vector<int> &cc2, int i);
	inline int NextSWA(vector<int>& cc, vector<int>& cc_old);
	inline void SaveBestRejected(vector<int>& cc);
	inline void ShowScanSpeed();
	inline char GetScanVisualCode(int i);
	inline void ShowScanStatus();
	void CheckClibSize();
	inline void ReseedCantus();
	inline void TimeBestRejected();
	inline void SaveCantusIfRp();
	void ScanCantus(int t, int v, vector<int>* pcantus);
	inline void ScanRight(vector<int>& cc);
	void WriteFlagCor();
	void ShowFlagStat();
	void ShowStuck();
	CString GetStuck();
	void ShowFlagBlock();
	void SaveCantus();
	void TestRpenalty();
	void TestBestRpenalty();
	inline void TransposeVector(vector<int>& v, int t);
	inline void TransposeVector(vector<float>& v, int t);
	inline void InterpolateNgraph(int v, int step0, int step);
	void SendNgraph(int pos, int i, int v, int x);
	void SendGraph(int pos, int i, int v, int x);
	void SendLyrics(int pos, int v, int av, int x);
	void SendComment(int pos, int v, int av, int x, int i);
	void TransposeCantusBack();
	void SendNotes(int pos, int i, int v, int av, int x, vector<int>& cc);
	void MakeBellDyn(int v, int step1, int step2, int dyn1, int dyn2, int dyn_rand);
	void MakeBellTempo(int step1, int step2, int tempo1, int tempo2);
	int SendPause(int pos, int v);
	inline void MakeLenExport(vector<int>& cc, int av, int retr_on);
	inline void SendHarmColor(int pos, int v);
	void MergeNotes(int step1, int step2, int v);
	CString GetHarmName(int pitch, int alter);
	int SendCantus();
	void RandCantus(vector<int>& c, vector<int>& cc, int step1, int step2);
	void CalculateCcOrder(vector<int>& cc_old, int step1, int step2);

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

	// Pmap
	inline void CalcPmap(vector<int>& pcc, vector<int>& cc, vector<int>& c, vector<int>& smooth, vector<int>& leap);
	inline void GetPmap();
	CString GetPmapLogHeader();
	CString GetPmapLogSt();
	inline void LogPmap();
};

