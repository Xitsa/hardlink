#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class ILogger
{
public:
	virtual ~ILogger() {}
	virtual void Log(CString message) = 0;
};

class IFileDisposition
{
public:
	virtual ~IFileDisposition() {}
	virtual void ProcessFile(ULONG64 filesize) = 0;
	virtual CString GetDestination() = 0;
	virtual bool IsNeedCreateOrCheck() = 0;
	virtual bool IsDontFit() = 0;
	virtual CString GetFinalStatistics() = 0;
};

typedef enum
{
	// Original algorithm
	  efdsLogical
	// Maximum fitting
	, efdsMaxFit
} EFileDispositionStrategy;

typedef enum
{
	  enfbStop
	, enfbWarn
	, enfbToOversized
} ENotFitBehaviour;

IFileDisposition* GetFileDisposition(EFileDispositionStrategy strategy, ULONG64 totalsize, ULONG64 dvdsize, ILogger* logger);
