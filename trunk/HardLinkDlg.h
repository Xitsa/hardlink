// HardLinkDlg.h : header file
//

#pragma once

class CVFindDataArray;
class CVStringArray;

class CBusyCursor
{
private:
    HCURSOR m_hCursor;
    CBusyCursor** m_pRef;

public:
    CBusyCursor()
    {
        m_hCursor = NULL;
        m_pRef = NULL;
    }

    CBusyCursor(CBusyCursor** pRef)
    {
        m_hCursor = NULL;
        m_pRef = NULL;

        if (pRef && *pRef == NULL)
        {
            m_pRef = pRef;
            *pRef = this;
        }
    }

    ~CBusyCursor()
    {
        if (m_pRef) *m_pRef = NULL;
        endbusy();
    }

    void busy()
    {
        if (m_hCursor == NULL)
            m_hCursor = ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT)));
    }

    void endbusy()
    {
        if (m_hCursor != NULL)
        {
            ::SetCursor(m_hCursor);
            m_hCursor = NULL;
        }
    }

    HCURSOR suspend()
    {
        HCURSOR hc = m_hCursor;
        endbusy();
        return hc;
    }

    void resume(HCURSOR hc)
    {
        if (m_hCursor == NULL)
        {
            busy();
            m_hCursor = hc;
        }
    }
};


// CHardLinkDlg dialog
class CHardLinkDlg : public CDialog
{
// Construction
public:
    CHardLinkDlg(CWnd* pParent = NULL); // standard constructor

// Dialog Data
    enum { IDD = IDD_HARDLINK_DIALOG };

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support


private:
    CString m_csSrcRootDir;
    CString m_csDstRootDir;
    int m_nDiskNumber;
    int m_nDigits;
    CString m_csDiskDirName;
    ULONG64 m_ulUsedDiskSpace;
    ULONG64 m_dvdsize;
    CBusyCursor* m_pCursor;

private:
    void BrowseDir(int idc);
    void CleanDir(int idc);
    void TidyDirName(CString& cs);
    BOOL ValidateDirSyntax(LPCTSTR cp);
    void Error(LPCTSTR cp);
    TCHAR lastchar(LPCTSTR cp);
    BOOL DoCleanDir(LPCTSTR dirname);
    void Log(LPCTSTR cp);
    BOOL DoCopyDir(LPCTSTR srcdir, LPCTSTR dstdir);
    BOOL IsDirExists(LPCTSTR dir);
    BOOL CheckOrMakeDir(LPCTSTR dir, BOOL force = FALSE);
    BOOL xmkdir(LPCTSTR dir);
    BOOL CheckDirEmpty(LPCTSTR dirname);
    BOOL DoCreateHardlink(LPCTSTR lpFileName, LPCTSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
    BOOL ReadConfiguration();
    BOOL SaveConfiguration();
    BOOL GetFolderSize(LPCTSTR dir, ULONG64* totalsize, size_t blksize);
    BOOL DoSplitDir(LPCTSTR subdir, BOOL& rbPrintStartMsg);
    BOOL ReadSplitSizeControl(size_t* psize);
    CString ComposeDiskDirName(int nDiskNumber, int nDigits);
    ULONG64 GetFileSize(const WIN32_FIND_DATA& ff, size_t blksize);
    void SortFiles(CVFindDataArray& aFiles);
    void SortFolders(CVStringArray& aDirs);
    static void ParseBaseAndNumber(LPCTSTR cp, CString& csBase, int* pnumber);
    static int __cdecl qcompareFindDataPointersAlnum(const void *e1, const void *e2);
    static int __cdecl qcompareFolderNames(const void *e1, const void *e2);
    BOOL SaveDir(int idc, LPCTSTR valname);
    BOOL SaveValue(LPCTSTR valname, LPCTSTR value);
    BOOL ReadDir(int idc, LPCTSTR valname);
    BOOL ReadValue(LPCTSTR valname, CString& cs);
    BOOL IsDirUnder(LPCTSTR rootdir, LPCTSTR dir);
    void CHardLinkDlg::SysError(LPCTSTR cp);
    void CHardLinkDlg::SysError(LPCTSTR errmsg, DWORD dwError);

// Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnEnChangeCopyFrom();
    afx_msg void OnBnClickedCopyToBrowse();
    afx_msg void OnEnChangeCopyTo();
    afx_msg void OnBnClickedCopyFromBrowse();
    afx_msg void OnBnClickedDoCopy();
    afx_msg void OnBnClickedDoCleanCopy();
    afx_msg void OnEnChangeSplitFrom();
    afx_msg void OnBnClickedSplitFromBrowse();
    afx_msg void OnBnClickedDoSplit();
    afx_msg void OnCbnSelchangeCombo1();
    afx_msg void OnBnClickedDoCleanSplit();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnEnChangeSplitTo();
    afx_msg void OnBnClickedSplitToBrowse();
    afx_msg void OnCbnSelchangeSplitsize();
};
