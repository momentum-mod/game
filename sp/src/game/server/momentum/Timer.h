#include "cbase.h"
#include "fasttimer.h"
#include "filesystem.h"
#include "utlbuffer.h"

extern IFileSystem *filesystem;

#ifdef _WIN32
#define NEWLINE "\r\n"
#else
#define NEWLINE "\n"
#endif

class CTimer
{
	DECLARE_CLASS_NOBASE(CTimer);
public:
	CTimer() {}
	~CTimer() {}

	static CTimer *timer()
	{
		static CTimer *timer = new CTimer();
		return timer;
	}

	void Start(int start)
	{
		m_iStartTick = start;
		SetRunning(true);
		DispatchStateMessage();
	}

	void Stop()
	{
		SetRunning(false);
		DispatchStateMessage();
	}

	void DispatchStateMessage() {
		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();
		UserMessageBegin(user, "Timer_State");
		WRITE_BOOL(m_bIsRunning);
		WRITE_LONG(m_iStartTick);
		MessageEnd();
	}

	bool IsRunning()
	{
		return m_bIsRunning;
	}

	void SetRunning(bool newb) {
		m_bIsRunning = newb;
	}

public:
	int m_iStartTick;
	bool m_bIsRunning;
	

private:
	Vector m_vStart;
	Vector m_vGoal;
	CFastTimer *m_ftTimer;
	CCycleCount m_ccCycles;
	bool m_IsPaused;
	float m_flSecondsRecord;

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