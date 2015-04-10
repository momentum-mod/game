#include "cbase.h"
#include "fasttimer.h"
#include "filesystem.h"
#include "utlbuffer.h"

extern IFileSystem *filesystem;

#ifdef WIN32
#define NEWLINE "\r\n"
#else
#define NEWLINE "\n"
#endif

class Timer
{
public:
	Timer() : m_bIsRunning(false), m_flSecondsRecord(0.0f)
	{
		inLevelLoad = false;
		curTime = 0.0f;
		totalTicks = 0.0f;
		startTick = 0;
		offset = 0.0f;
		offsetBefore = 0.0f;
		startTime = 0.0f;
		m_vStart.Init();
		m_vGoal.Init();
	}
	~Timer() {}

	static Timer *timer()
	{
		static Timer *timer = new Timer();
		return timer;
	}

	void Init(float offsetAfterLoad)
	{
		//offset = offsetAfterLoad - offsetBefore;
		startTick = offsetAfterLoad;
		SetLevelLoad(false);
		//m_bIsRunning = false;
		/*
		// Make sure the maps directory is there (you never know...).
		filesystem->CreateDirHierarchy("maps", "MOD");
		CUtlBuffer buffer;
		char *pszPath = GetMapFilePath();
		DevMsg("Trying to get record: %s\n", pszPath);
		if (filesystem->ReadFile(pszPath, "MOD", buffer))
		{
		char *pszRecord = new char[buffer.Size() + 1];
		buffer.GetString(pszRecord);
		pszRecord[buffer.Size()] = '\0';
		if (sscanf(pszRecord, "%f %f %f" NEWLINE "%f %f %f" NEWLINE "%f",
		&m_vStart.x, &m_vStart.y, &m_vStart.z,
		&m_vGoal.x, &m_vGoal.y, &m_vGoal.z,
		&m_flSecondsRecord) != 7)
		Warning("Record file %s is malformed\n", pszPath);
		delete[] pszRecord;
		}
		delete[] pszPath;
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
		if (!pPlayer)
		return;*/
		//pPlayer->SetStartPosition(m_vStart);
		//pPlayer->SetGoalPosition(m_vGoal);
	}

	void SetOffsetBefore(float newOff) {
		offsetBefore = newOff;
	}

	void Start()
	{
		//offsetBefore = 0.0f;
		//offset = 0.0f;
		//startTime = gpGlobals->realtime;
		totalTicks = 0;
		startTick = gpGlobals->tickcount;
		//Msg("Starttime: %f\n", startTime);
		SetRunning(true);
		//DispatchStateChangeMessage();
	}

	void SetLevelLoad(bool newVal) {
		inLevelLoad = newVal;
	}

	bool InLevelLoad() {
		return inLevelLoad;
	}

	void Stop()
	{
		//m_ftTimer->End();
		m_bIsRunning = false;
		SetOffsetBefore(0.0f);
		totalTicks = 0;
		//DispatchStateChangeMessage();
		/*float flSecondsTime = GetCurrentTime();
		if (flSecondsTime < m_flSecondsRecord || m_flSecondsRecord == 0.0f)
		{
		m_flSecondsRecord = flSecondsTime;
		DevMsg("New map record: %.4f seconds\n", m_flSecondsRecord);
		WriteMapFile();
		}*/
	}

	bool IsRunning()
	{
		return m_bIsRunning;
	}

	void SetRunning(bool newb) {
		m_bIsRunning = newb;
	}

	void CalcTime() {
		curTime = (float)gpGlobals->tickcount;
		totalTicks += curTime - startTick;
		startTick = curTime;
	}

	float GetCurrentTime()
	{
		if (m_bIsRunning) CalcTime();
		return totalTicks;
	}

	void DispatchTimeToBeatMessage()
	{
		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();
		UserMessageBegin(user, "Timer_TimeToBeat");
		WRITE_FLOAT(m_flSecondsRecord);
		MessageEnd();
	}

	void DispatchTimeMessage()
	{
		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();
		UserMessageBegin(user, "Timer_Time");
		WRITE_FLOAT(GetCurrentTime());
		MessageEnd();
	}

	void SetStartPosition(Vector start)
	{
		//UTIL_GetLocalPlayer()->SetStartPosition(start);
		m_vStart = start;
		WriteMapFile();
	}

	void SetGoalPosition(Vector goal)
	{
		// UTIL_GetLocalPlayer()->SetGoalPosition(goal);
		m_vGoal = goal;
		WriteMapFile();
	}

	float GetOffsetBefore() {
		return offsetBefore;
	}

private:
	Vector m_vStart;
	Vector m_vGoal;
	CFastTimer *m_ftTimer;
	CCycleCount m_ccCycles;
	bool inLevelLoad;
	bool m_bIsRunning;
	bool m_IsPaused;
	float m_flSecondsRecord;
	float offset;
	float offsetBefore;
	float curTime;
	float startTime;

	float totalTicks;//to be sent to the hud and converted into HH:MM:SS
	float startTick;//the tick in which the level/reload started


	// Inform the HUD about status changes of the timer so it can fire up some
	// fancy animation effects.
	void DispatchStateChangeMessage()
	{
		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();

		UserMessageBegin(user, "Timer_StateChange");
		WRITE_BOOL(m_bIsRunning);
		MessageEnd();
	}

	void WriteMapFile()
	{
		char *pszPath = GetMapFilePath();
		FileHandle_t fh = filesystem->Open(pszPath, "w", "MOD");
		if (fh)
		{
			filesystem->FPrintf(fh, "%f %f %f%s",
				m_vStart.x, m_vStart.y, m_vStart.z, NEWLINE);
			filesystem->FPrintf(fh, "%f %f %f%s",
				m_vGoal.x, m_vGoal.y, m_vGoal.z, NEWLINE);
			filesystem->FPrintf(fh, "%f", m_flSecondsRecord);
			filesystem->Close(fh);
		}
		delete[] pszPath;
	}

	// Caller is responsible for delete[]'ing the array.
	char *GetMapFilePath()
	{
		const char *pszMapname = (gpGlobals->mapname).ToCStr();
		size_t sz = Q_strlen("maps/") + strlen(pszMapname) + Q_strlen(".bla") + 1;
		char *pszPath = new char[sz];
		Q_strncpy(pszPath, "maps/", sz);
		Q_strncat(pszPath, pszMapname, sz);
		Q_strncat(pszPath, ".bla", sz);
		Q_FixSlashes(pszPath);
		return pszPath;
	}
};

#undef NEWLINE