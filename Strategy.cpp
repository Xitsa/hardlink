#include "stdafx.h"
#include "Strategy.h"

class CDiskNameUtils
{
public:
	static int GetNumberOfDigits(int nDisks)
	{
		if (nDisks >= 90000)
			return 6;
		else if (nDisks >= 9000)
			return 5;
		else if (nDisks >= 900)
			return 4;
		else if (nDisks >= 90)
			return 3;
		else if (nDisks >= 9)
			return 2;
		return 1;
	}

	static CString ComposeDiskDirName(size_t nDiskNumber, int nDigits)
	{
		CString cs;
		cs.Format(_T("disk%0*u"), nDigits, nDiskNumber);
		return cs;
	}
};

class CLogicalFileDisposition: public IFileDisposition, private CDiskNameUtils
{
public:
	CLogicalFileDisposition(ULONG64 totalsize, ULONG64 dvdsize, ILogger* logger)
		: m_dvdsize(dvdsize)
		, m_nDiskNumber(0)
		, m_ulUsedDiskSpace(dvdsize + 1)
		, m_Logger(logger)
		, m_bLastNotFitted(false)
		, m_LastDestination(_T(""))
		, m_Destination(_T("."))
	{
		CString cs;
		int ndisks = (int) ((totalsize + m_dvdsize - 1) / m_dvdsize);
		ndisks = max(ndisks, 1);
		cs.Format(_T("Estimated approximate number of disks required is %d"), ndisks);
		m_Logger->Log(cs);
		m_nDigits = GetNumberOfDigits(ndisks);
	}
	virtual ~CLogicalFileDisposition() 
	{
	}
	virtual void ProcessFile(ULONG64 filesize)
	{
		if (filesize > m_dvdsize)
		{
			m_LastDestination = m_Destination;
			m_Destination = _T("Oversized");
			m_bLastNotFitted = true;
		}
		else
		{
			bool bNeedComposeDiskName = m_bLastNotFitted;
			if (m_ulUsedDiskSpace + filesize > m_dvdsize)
			{
				// switch to the next disk
				++m_nDiskNumber;
				m_ulUsedDiskSpace = 0;
				bNeedComposeDiskName = true;
				CString cs;
				cs.Format(_T("Creating disk %d ..."), m_nDiskNumber);
				m_Logger->Log(cs);
			}

			if (bNeedComposeDiskName)
			{
				m_LastDestination = m_Destination;
				m_Destination = ComposeDiskDirName(m_nDiskNumber, m_nDigits);
			}

			m_ulUsedDiskSpace += filesize;
			m_bLastNotFitted = false;
		}
	}
	virtual CString GetDestination()
	{
		return m_Destination;
	}
	virtual bool IsNeedCreateOrCheck()
	{
		return m_Destination.Compare(m_LastDestination) != 0;
	}
	virtual bool IsDontFit()
	{
		return m_bLastNotFitted;
	}
	virtual CString GetFinalStatistics()
	{
		double fpct = (double) m_ulUsedDiskSpace / (double) m_dvdsize;
		int pct = (int) (fpct * 100);
		pct = min(max(1, pct), 100);
		CString cs;
		cs.Format(_T("Completed splitting into %d disks, last disk is %d%% full."), m_nDiskNumber, pct);
		return cs;
	}
private:

	ILogger* m_Logger;
	int m_nDiskNumber;
	int m_nDigits;
	ULONG64 m_ulUsedDiskSpace;
	ULONG64 m_dvdsize;
	bool m_bLastNotFitted;
	CString m_LastDestination;
	CString m_Destination;
};

class CMaxPackFileDisposition: public IFileDisposition, private CDiskNameUtils
{
public:
	CMaxPackFileDisposition(ULONG64 totalsize, ULONG64 dvdsize, ILogger* logger)
		: m_dvdsize(dvdsize)
		, m_nDiskNumber(0)
		, m_Logger(logger)
		, m_bLastNotFitted(false)
		, m_LastDestination(_T(""))
		, m_Destination(_T("."))
	{
		CString cs;
		int ndisks = (int) ((totalsize + m_dvdsize - 1) / m_dvdsize);
		ndisks = max(ndisks, 1);
		cs.Format(_T("Estimated approximate number of disks required is %d"), ndisks);
		m_Logger->Log(cs);
		m_nDigits = GetNumberOfDigits(ndisks);
	}
	virtual ~CMaxPackFileDisposition() 
	{
	}
	virtual void ProcessFile(ULONG64 filesize)
	{
		if (filesize > m_dvdsize)
		{
			m_LastDestination = m_Destination;
			m_Destination = _T("Oversized");
			m_bLastNotFitted = true;
		}
		else
		{
			bool bNeedComposeDiskName = m_bLastNotFitted;

			if (0 == m_nDiskNumber)
			{
				//Initial
				AddDisk();
				bNeedComposeDiskName = true;
			}


			if (m_ulUsedDiskSpace[m_nDiskNumber - 1] + filesize > m_dvdsize)
			{
				size_t nNextDisk = SeekForFree(m_nDiskNumber, filesize);
				if (0 == nNextDisk)
				{
					AddDisk();
				}
				else
				{
					m_nDiskNumber = nNextDisk;
				}
				bNeedComposeDiskName = true;
			}

			if (bNeedComposeDiskName)
			{
				m_LastDestination = m_Destination;
				m_Destination = ComposeDiskDirName(m_nDiskNumber, m_nDigits);
			}

			m_ulUsedDiskSpace[m_nDiskNumber - 1] += filesize;
			m_bLastNotFitted = false;
		}
	}

	void AddDisk() 
	{
		m_ulUsedDiskSpace.Add(0);
		m_nDiskNumber = m_ulUsedDiskSpace.GetCount();
		CString cs;
		cs.Format(_T("Creating disk %d ..."), m_ulUsedDiskSpace.GetCount());
		m_Logger->Log(cs);
	}

	virtual CString GetDestination()
	{
		return m_Destination;
	}
	virtual bool IsNeedCreateOrCheck()
	{
		return m_Destination.Compare(m_LastDestination) != 0;
	}
	virtual bool IsDontFit()
	{
		return m_bLastNotFitted;
	}
	virtual CString GetFinalStatistics()
	{
		CString cs;
		cs.Format(_T("Completed splitting."));
		return cs;
	}
private:

	// Find next disk with enough free space
	// 0 -- not found
	size_t SeekForFree(size_t nDiskFrom, ULONG64 filesize)
	{
		size_t nCurDisk = 1 + (nDiskFrom % m_ulUsedDiskSpace.GetCount());
		while (nCurDisk != nDiskFrom)
		{
			if (!(m_ulUsedDiskSpace[nCurDisk - 1] + filesize > m_dvdsize))
				return nCurDisk;
			nCurDisk = 1 + (nCurDisk % m_ulUsedDiskSpace.GetCount());
		}
		return 0;
	}

	ILogger* m_Logger;
	size_t m_nDiskNumber;
	int m_nDigits;
	ULONG64 m_dvdsize;
	bool m_bLastNotFitted;
	CString m_LastDestination;
	CString m_Destination;
	CAtlArray<ULONG64> m_ulUsedDiskSpace;
};

IFileDisposition* GetFileDisposition(EFileDispositionStrategy strategy, ULONG64 totalsize, ULONG64 dvdsize, ILogger* logger)
{
	if (efdsMaxFit == strategy)
		return new CMaxPackFileDisposition(totalsize, dvdsize, logger);
	return new CLogicalFileDisposition(totalsize, dvdsize, logger);
}