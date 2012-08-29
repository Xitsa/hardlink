// HardLinkDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HardLink.h"
#include "HardLinkDlg.h"
#include "FolderDialog.h"
#include <ctype.h>
#include <direct.h>
#include <string.h>
#include <search.h>
#include "Strategy.h"
#include "SplitOptions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define CHECK(cond)  do { if (! (cond))  goto cleanup; } while (0)
#define countof(x)  (sizeof(x) / sizeof((x)[0]))

static const size_t DVDBLKSIZE = 2048;
static ULONG64 HMASK64;
static ULONG64 LMASK64;

class CVStringArray : public CArray<CString, CString>
{
};

class CVFindDataArray : public CArray<WIN32_FIND_DATA, WIN32_FIND_DATA>
{
};

typedef BOOL (WINAPI * PCreateHardlink)(LPCTSTR lpFileName, LPCTSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);

static PCreateHardlink s_pCreateHardlink = NULL;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

// Dialog Data
    enum { IDD = IDD_ABOUTBOX };

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CHardLinkDlg dialog



CHardLinkDlg::CHardLinkDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CHardLinkDlg::IDD, pParent)
	, m_NotFitBehaviour(enfbToOversized)
	, m_FileDispositionStrategy(efdsMaxFit)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    DWORD* pdw = (DWORD*) &HMASK64;
    pdw[0] = 0;
    pdw[1] = 0xFFFFFFFF;

    pdw = (DWORD*) &LMASK64;
    pdw[0] = 0xFFFFFFFF;
    pdw[1] = 0;

    m_pCursor = NULL;
}

void CHardLinkDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHardLinkDlg, CDialog)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    //}}AFX_MSG_MAP
    ON_EN_CHANGE(IDC_COPY_FROM, OnEnChangeCopyFrom)
    ON_BN_CLICKED(IDC_COPY_TO_BROWSE, OnBnClickedCopyToBrowse)
    ON_EN_CHANGE(IDC_COPY_TO, OnEnChangeCopyTo)
    ON_BN_CLICKED(IDC_COPY_FROM_BROWSE, OnBnClickedCopyFromBrowse)
    ON_BN_CLICKED(IDC_DOCOPY, OnBnClickedDoCopy)
    ON_BN_CLICKED(IDC_DOCLEAN_COPY, OnBnClickedDoCleanCopy)
    ON_EN_CHANGE(IDC_SPLIT_FROM, OnEnChangeSplitFrom)
    ON_BN_CLICKED(IDC_SPLIT_FROM_BROWSE, OnBnClickedSplitFromBrowse)
    ON_BN_CLICKED(IDC_DOSPLIT, OnBnClickedDoSplit)
    ON_BN_CLICKED(IDC_DOCLEAN_SPLIT, OnBnClickedDoCleanSplit)
    ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
    ON_EN_CHANGE(IDC_SPLIT_TO, OnEnChangeSplitTo)
    ON_BN_CLICKED(IDC_SPLIT_TO_BROWSE, OnBnClickedSplitToBrowse)
    ON_CBN_SELCHANGE(IDC_SPLITSIZE, OnCbnSelchangeSplitsize)
	ON_BN_CLICKED(IDC_BTN_OPTIONS, OnBnClickedSplitOptions)
END_MESSAGE_MAP()


// CHardLinkDlg message handlers

BOOL CHardLinkDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        CString strAboutMenu;
        strAboutMenu.LoadString(IDS_ABOUTBOX);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);         // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    // GetDlgItem(IDC_LOG)->SendMessage(EM_SHOWSCROLLBAR, SB_VERT, FALSE);

    ReadConfiguration();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CHardLinkDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialog::OnSysCommand(nID, lParam);
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CHardLinkDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CHardLinkDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


void CHardLinkDlg::OnEnChangeCopyFrom()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialog::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.

    // TODO:  Add your control notification handler code here
}

void CHardLinkDlg::OnBnClickedCopyToBrowse()
{
    BrowseDir(IDC_COPY_TO);
}

void CHardLinkDlg::OnEnChangeCopyTo()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialog::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.

    // TODO:  Add your control notification handler code here
}

void CHardLinkDlg::OnBnClickedCopyFromBrowse()
{
    BrowseDir(IDC_COPY_FROM);
}

void CHardLinkDlg::OnBnClickedDoCopy()
{
    SaveConfiguration();

    CString csSrc;
    CString csDst;
    CString cs;
    DWORD dw;
    TCHAR fsname[100];
    CBusyCursor cursor(& m_pCursor);
    cursor.busy();
    GetDlgItem(IDC_COPY_FROM)->GetWindowText(csSrc);
    TidyDirName(csSrc);
    GetDlgItem(IDC_COPY_TO)->GetWindowText(csDst);
    TidyDirName(csDst);
    if (! ValidateDirSyntax(csSrc))  return;
    if (! ValidateDirSyntax(csDst))  return;

    if (! IsDirExists(csSrc))
    {
        Error(_T("Source folder does not exist."));
        return;
    }

    if (IsDirUnder(csSrc, csDst))
    {
        Error(_T("Target folder cannot reside under source."));
        return;
    }

    if (IsDirUnder(csDst, csSrc))
    {
        Error(_T("Source folder cannot reside under target."));
        return;
    }

    if (! CheckOrMakeDir(csDst))
    {
        return;
    }

    if (0 == csSrc.CompareNoCase(csDst))
    {
        Error(_T("Source folder cannot be the same as the target."));
        return;
    }


    // check src and dst are on the same drive
    if (0 != csSrc.Left(2).CompareNoCase(csDst.Left(2)) || csSrc.Mid(1, 1) != _T(":"))
    {
        Error(_T("Source and target folders should be on the same volume."));
        return;
    }

    fsname[0] = _T('\0');
    if (! GetVolumeInformation(csSrc.Left(3), NULL, 0, NULL, & dw, & dw, fsname, countof(fsname)))
    {
        SysError(_T("Cannot check file system type for the specified folder."));
        return;
    }

    if (0 != _tcsicmp(fsname, _T("NTFS")))
    {
        Error(_T("Folders should be located on NTFS volume."));
        return;
    }

    cs.Format(_T("Copying folder %s to %s ..."), csSrc, csDst);
    LogImpl(_T(""));
    LogImpl(cs);

    if (DoCopyDir(csSrc, csDst))
    {
        LogImpl(_T("Completed copying."));
    }
    else
    {
        LogImpl(_T("Failed to copy."));
    }
}

BOOL CHardLinkDlg::IsDirExists(LPCTSTR dir)
{
    DWORD dw = GetFileAttributes(dir);
    if (dw == INVALID_FILE_ATTRIBUTES) return FALSE;
    return 0 != (dw & FILE_ATTRIBUTE_DIRECTORY);
}

BOOL CHardLinkDlg::CheckOrMakeDir(LPCTSTR dir, BOOL force)
{
    DWORD dw = GetFileAttributes(dir);
    if (dw == INVALID_FILE_ATTRIBUTES)
    {
        if (! force)
        {
            CString cs;
            cs.Format(_T("Folder %s does not exist.\r\nDo you want to create it?"), dir);
            HCURSOR hCursor = NULL;
            if (m_pCursor) hCursor = m_pCursor->suspend();
            int res = AfxMessageBox(cs, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON1 | MB_TASKMODAL | MB_SETFOREGROUND);
            if (m_pCursor) m_pCursor->resume(hCursor);
            if (res != IDYES)
                return FALSE;
        }

        if (TRUE == xmkdir(dir))
        {
            return TRUE;
        }
        else
        {
            // CString cs;
            // cs.Format("Unable to create folder %s.", dir);
            // Error(cs);
            return FALSE;
        }
    }
    else if (0 != (dw & FILE_ATTRIBUTE_DIRECTORY))
    {
        return TRUE;
    }
    else
    {
        CString cs;
        cs.Format(_T("File %s exists and is not a folder."), dir);
        Error(cs);
        return FALSE;
    }
}

// multilevel mkdir
BOOL CHardLinkDlg::xmkdir(LPCTSTR dir)
{
    CString csDir;
    CString cs = dir;
    LPCTSTR token;

    token = _tcstok(cs.LockBuffer(), _T("\\"));

    while (token)
    {
        if (! csDir.IsEmpty())
            csDir += _T("\\");
        csDir += token;
        if (! IsDirExists(csDir))
        {
            if (0 != _tmkdir(csDir))
            {
                CString csError;
                csError.Format(_T("Unable to create folder %s."), csDir);
                Error(csError);
                cs.UnlockBuffer();
                return FALSE;
            }
        }

        token = _tcstok(NULL, _T("\\"));
    }

    cs.UnlockBuffer();

    return TRUE;
}

BOOL CHardLinkDlg::DoCopyDir(LPCTSTR srcdir, LPCTSTR dstdir)
{
    if (! CheckDirEmpty(dstdir))
        return FALSE;

    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA ff;
    CString csDir;
    CString csError;
    CString csSrcPath;
    CString csDstPath;
    BOOL done = FALSE;
    CVStringArray aFiles;
    CVStringArray aDirs;
    int k;

    memset(&ff, 0, sizeof ff);
    csDir = srcdir;
    if (lastchar(csDir) != _T('\\'))  csDir += _T('\\');
    csDir += _T("*.*");
    hFind = FindFirstFile(csDir, & ff);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        DWORD dwError = GetLastError();
        if (dwError == ERROR_NO_MORE_FILES)  goto completed;
        csError.Format(_T("Unable to enumerate folder %s"), srcdir);
        SysError(csError, dwError);
        CHECK(FALSE);
    }

    do
    {
        if (ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (! (0 == _tcsicmp(ff.cFileName, _T(".")) || 0 == _tcsicmp(ff.cFileName, _T(".."))))
                aDirs.Add(ff.cFileName);
        }
        else
        {
            aFiles.Add(ff.cFileName);
        }
    }
    while (FindNextFile(hFind, & ff));

    if (GetLastError() != ERROR_NO_MORE_FILES)
    {
        DWORD dwError = GetLastError();
        csError.Format(_T("Unable to enumerate folder %s"), srcdir);
        SysError(csError, dwError);
        CHECK(FALSE);
    }

    FindClose(hFind);
    hFind = INVALID_HANDLE_VALUE;

    // copy files
    for (k = 0;  k < aFiles.GetCount();  k++)
    {
        csSrcPath = srcdir;
        if (lastchar(csSrcPath) != _T('\\'))  csSrcPath += _T('\\');
        csSrcPath += aFiles.GetAt(k);

        csDstPath = dstdir;
        if (lastchar(csDstPath) != _T('\\'))  csDstPath += _T('\\');
        csDstPath += aFiles.GetAt(k);

        if (! DoCreateHardlink(csDstPath, csSrcPath, NULL))
        {
            DWORD dwError = GetLastError();
            csError.Format(_T("Unable to create hard link %s for source file %s"), csDstPath, csSrcPath);
            SysError(csError, dwError);
            CHECK(FALSE);
        }
    }

    for (k = 0;  k < aDirs.GetCount();  k++)
    {
        csSrcPath = srcdir;
        if (lastchar(csSrcPath) != _T('\\'))  csSrcPath += _T('\\');
        csSrcPath += aDirs.GetAt(k);

        csDstPath = dstdir;
        if (lastchar(csDstPath) != _T('\\'))  csDstPath += _T('\\');
        csDstPath += aDirs.GetAt(k);

        CHECK(xmkdir(csDstPath));
        CHECK(DoCopyDir(csSrcPath, csDstPath));
    }

completed:

    done = TRUE;

cleanup:

    if (hFind != INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
    }

    return done;
}

BOOL CHardLinkDlg::CheckDirEmpty(LPCTSTR dirname)
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA ff;
    CString csDir;
    CString csError;
    BOOL done = FALSE;

    memset(&ff, 0, sizeof ff);
    csDir = dirname;
    if (lastchar(csDir) != _T('\\'))  csDir += _T('\\');
    csDir += _T("*.*");
    hFind = FindFirstFile(csDir, & ff);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        DWORD dwError = GetLastError();
        csError.Format(_T("Unable to enumerate folder %s"), dirname);
        SysError(csError, dwError);
        CHECK(FALSE);
    }

    do
    {
        if (! (0 == _tcsicmp(ff.cFileName, _T(".")) || 0 == _tcsicmp(ff.cFileName, _T(".."))))
        {
            csError.Format(_T("Folder %s is not empty."), dirname);
            Error(csError);
            CHECK(FALSE);
        }
    }
    while (FindNextFile(hFind, & ff));

    if (GetLastError() != ERROR_NO_MORE_FILES)
    {
        DWORD dwError = GetLastError();
        csError.Format(_T("Unable to enumerate folder %s"), dirname);
        SysError(csError, dwError);
        CHECK(FALSE);
    }

    done = TRUE;

cleanup:

    if (hFind != INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
    }

    return done;
}

void CHardLinkDlg::OnBnClickedDoCleanCopy()
{
    CBusyCursor cursor(& m_pCursor);
    cursor.busy();
    SaveConfiguration();
    CleanDir(IDC_COPY_TO);
}

void CHardLinkDlg::OnEnChangeSplitFrom()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialog::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.

    // TODO:  Add your control notification handler code here
}

void CHardLinkDlg::OnBnClickedSplitFromBrowse()
{
    BrowseDir(IDC_SPLIT_FROM);
}


void CHardLinkDlg::OnBnClickedDoSplit()
{
    SaveConfiguration();

    CString csSrc;
    CString csDst;
    CString cs;
    DWORD dw;
    TCHAR fsname[100];
    ULONG64 totalsize = 0;
    size_t dvdsize_mb;
    CBusyCursor cursor(& m_pCursor);
    cursor.busy();
    GetDlgItem(IDC_SPLIT_FROM)->GetWindowText(csSrc);
    TidyDirName(csSrc);
    GetDlgItem(IDC_SPLIT_TO)->GetWindowText(csDst);
    TidyDirName(csDst);
    if (! ValidateDirSyntax(csSrc))  return;
    if (! ValidateDirSyntax(csDst))  return;

    if (! IsDirExists(csSrc))
    {
        Error(_T("Source folder does not exist."));
        return;
    }

    if (IsDirUnder(csSrc, csDst))
    {
        Error(_T("Target folder cannot reside under source."));
        return;
    }

    if (IsDirUnder(csDst, csSrc))
    {
        Error(_T("Source folder cannot reside under target."));
        return;
    }

    if (! CheckOrMakeDir(csDst))
    {
        return;
    }

    if (0 == csSrc.CompareNoCase(csDst))
    {
        Error(_T("Source folder cannot be the same as the target."));
        return;
    }
    // check src and dst are on the same drive
    if (0 != csSrc.Left(2).CompareNoCase(csDst.Left(2)) || csSrc.Mid(1, 1) != _T(":"))
    {
        Error(_T("Source and target folders should be on the same volume."));
        return;
    }

    fsname[0] = _T('\0');
    if (! GetVolumeInformation(csSrc.Left(3), NULL, 0, NULL, & dw, & dw, fsname, countof(fsname)))
    {
        SysError(_T("Cannot check file system type for the specified folder."));
        return;
    }

    if (0 != _tcsicmp(fsname, _T("NTFS")))
    {
        Error(_T("Folders should be located on NTFS volume."));
        return;
    }

    if (! ReadSplitSizeControl(& dvdsize_mb))
        return;
    ULONG64 dvdsize = dvdsize_mb;
    dvdsize *= 1024;
    dvdsize *= 1024;

    // compute total size
    cs.Format(_T("Scanning source folder %s ..."), csSrc);
    LogImpl(_T(""));
    LogImpl(cs);
    if (! GetFolderSize(csSrc, & totalsize, DVDBLKSIZE))
        return;

	CAutoPtr<IFileDisposition> FileDispostion(GetFileDisposition(m_FileDispositionStrategy, totalsize, dvdsize, this));


    cs.Format(_T("Splitting folder %s to %s ..."), csSrc, csDst);
    LogImpl(cs);

    m_csSrcRootDir = csSrc;
    m_csDstRootDir = csDst;

    if (DoSplitDir(_T(""), FileDispostion))
    {
        LogImpl(FileDispostion->GetFinalStatistics());
    }
    else
    {
        LogImpl(_T("FAILED TO SPLIT."));
    }
}

BOOL CHardLinkDlg::DoSplitDir(LPCTSTR subdir, IFileDisposition* fileDisposition)
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA ff;
    CString csDir;
    CString csDirMsg;
    CString csError;
    BOOL done = FALSE;
    CVStringArray aDirs;
    CVFindDataArray aFiles;
    int k;
    bool createDstFolder = true;

    memset(&ff, 0, sizeof ff);
    csDir = m_csSrcRootDir;
    if (lastchar(csDir) != _T('\\'))  csDir += _T('\\');
    if (_tcslen(subdir) > 0)
    {
        csDir += subdir;
        if (lastchar(csDir) != _T('\\'))  csDir += _T('\\');
    }
    csDirMsg = csDir;
    csDir += _T("*.*");
    hFind = FindFirstFile(csDir, & ff);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        DWORD dwError = GetLastError();
        if (dwError == ERROR_NO_MORE_FILES)  goto completed;
        csError.Format(_T("Unable to enumerate folder %s"), csDir);
        SysError(csError, dwError);
        CHECK(FALSE);
    }

    do
    {
        if (ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (! (0 == _tcsicmp(ff.cFileName, _T(".")) || 0 == _tcsicmp(ff.cFileName, _T(".."))))
                aDirs.Add(ff.cFileName);
        }
        else
        {
            aFiles.Add(ff);
        }
    }
    while (FindNextFile(hFind, & ff));

    if (GetLastError() != ERROR_NO_MORE_FILES)
    {
        DWORD dwError = GetLastError();
        csError.Format(_T("Unable to enumerate folder %s"), csDirMsg);
        SysError(csError, dwError);
        CHECK(FALSE);
    }

    FindClose(hFind);
    hFind = INVALID_HANDLE_VALUE;

    SortFiles(aFiles);
    SortFolders(aDirs);

    // process files
    for (k = 0;  k < aFiles.GetCount();  k++)
    {

        ff = aFiles.GetAt(k);
        ULONG64 filesize = GetFileSize(ff, DVDBLKSIZE);
		fileDisposition->ProcessFile(filesize);
		createDstFolder = createDstFolder || fileDisposition->IsNeedCreateOrCheck();

        CString csSrcPath;
        csSrcPath = m_csSrcRootDir;
        if (lastchar(csSrcPath) != _T('\\'))  csSrcPath += _T('\\');
        if (_tcslen(subdir) > 0)
        {
            csSrcPath += subdir;
            if (lastchar(csSrcPath) != _T('\\'))  csSrcPath += _T('\\');
        }
        csSrcPath += ff.cFileName;

        if (fileDisposition->IsDontFit())
        {
			switch (m_NotFitBehaviour)
			{
			case enfbStop:
				csError.Format(_T("File %s exceeds specified disk size and cannot fit on a disk."), csSrcPath);
				Error(csError);
				CHECK(FALSE);
				break;
			case enfbWarn:
				csError.Format(_T("File %s exceeds specified disk size and cannot fit on a disk."), csSrcPath);
				LogImpl(csError);
				continue;
				break;
			}
        }

        CString csDstPath;
        csDstPath = m_csDstRootDir;
        if (lastchar(csDstPath) != _T('\\'))  csDstPath += _T('\\');
        csDstPath += fileDisposition->GetDestination();
        csDstPath += _T('\\');
        if (_tcslen(subdir) > 0)
        {
            csDstPath += subdir;
        }
        if (createDstFolder)
        {
            CheckOrMakeDir(csDstPath, TRUE);
            createDstFolder = false;
        }

        if (_tcslen(subdir) > 0)
        {
            if (lastchar(csDstPath) != _T('\\'))  csDstPath += _T('\\');
        }
        csDstPath += ff.cFileName;

        if (! DoCreateHardlink(csDstPath, csSrcPath, NULL))
        {
            DWORD dwError = GetLastError();
            csError.Format(_T("Unable to create hard link %s for source file %s"), csDstPath, csSrcPath);
            SysError(csError, dwError);
            CHECK(FALSE);
        }
    }

    // process subfolders
    for (k = 0;  k < aDirs.GetCount();  k++)
    {
        csDir = subdir;
        if (lastchar(csDir) != _T('\\'))  csDir += _T('\\');
        csDir += aDirs.GetAt(k);
        CHECK(DoSplitDir(csDir, fileDisposition));
    }

completed:

    done = TRUE;

cleanup:

    if (hFind != INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
    }

    return done;
}

// sort files in alphanumeric order
void CHardLinkDlg::SortFiles(CVFindDataArray& aFiles)
{
    int count = (int) aFiles.GetSize();
    if (count <= 1)  return;

    WIN32_FIND_DATA* pFindData = new WIN32_FIND_DATA[count + 1];
    WIN32_FIND_DATA** ppFindData = new PWIN32_FIND_DATA[count + 1];
    int k;

    for (k = 0;  k < count;  k++)
    {
        pFindData[k] = aFiles.GetAt(k);
        ppFindData[k] = & pFindData[k];
    }

    qsort(ppFindData, count, sizeof(ppFindData[0]), CHardLinkDlg::qcompareFindDataPointersAlnum);

    for (k = 0;  k < count;  k++)
    {
        aFiles.SetAt(k, * ppFindData[k]);
    }

    delete pFindData;
    delete ppFindData;
}

int __cdecl CHardLinkDlg::qcompareFindDataPointersAlnum(const void *e1, const void *e2)
{
    WIN32_FIND_DATA** pp1 = (WIN32_FIND_DATA**) e1;
    WIN32_FIND_DATA* p1 = *pp1;

    WIN32_FIND_DATA** pp2 = (WIN32_FIND_DATA**) e2;
    WIN32_FIND_DATA* p2 = *pp2;

    int cmr = _tcscmp(p1->cFileName, p2->cFileName);
    if (cmr == 0) return 0;
    if (cmr < 0) return -1;
    return 1;
}

// sort folders mostly in alphanumeric order, but "day12" comes after "day2"
void CHardLinkDlg::SortFolders(CVStringArray& aDirs)
{
    int count = (int) aDirs.GetSize();
    if (count <= 1)  return;

    CVStringArray aTemp;
    int k;
    for (k = 0;  k < count;  k++)
        aTemp.SetAtGrow(k, aDirs.GetAt(k));

    LPCTSTR* pcp = new LPCTSTR[count + 1];

    for (k = 0;  k < count;  k++)
        pcp[k] = aTemp.GetAt(k);

    qsort(pcp, count, sizeof(pcp[0]), CHardLinkDlg::qcompareFolderNames);

    for (k = 0;  k < count;  k++)
    {
        CString cs = pcp[k];
        aDirs.SetAt(k, cs);
    }

    delete pcp;
}

int __cdecl CHardLinkDlg::qcompareFolderNames(const void *e1, const void *e2)
{
    LPCTSTR* pcp1 = (LPCTSTR*) e1;
    LPCTSTR cp1 = *pcp1;

    LPCTSTR* pcp2 = (LPCTSTR*) e2;
    LPCTSTR cp2 = *pcp2;

    CString csBase1;
    int number1;
    CString csBase2;
    int number2;

    ParseBaseAndNumber(cp1, csBase1, & number1);
    ParseBaseAndNumber(cp2, csBase2, & number2);

    if (csBase1 == csBase2 && number1 >= 0 && number2 >= 0)
    {
        if (number1 == number2)  return 0;
        if (number1 < number2)  return -1;
        return 1;
    }
    else
    {
        int cmr = _tcscmp(cp1, cp2);
        if (cmr == 0) return 0;
        if (cmr < 0) return -1;
        return 1;
    }
}

void CHardLinkDlg::ParseBaseAndNumber(LPCTSTR cp, CString& csBase, int* pnumber)
{
    *pnumber = -1;
    csBase = cp;

    int len = (int) _tcslen(cp);

    if (len == 0)  return;

    LPCTSTR xp = cp + len - 1;
    for (;;)
    {
        if (xp < cp)  break;
        if (! _istdigit(*xp)) break;
        xp--;
    }

    if (xp[1] == _T('\0'))
        return;

    LPCTSTR np;
    int resnum = 0;
    for (np = xp + 1; *np;  np++)
    {
        if (resnum > 214748363) return;
        resnum = resnum * 10 + (*np - _T('0'));
    }
    *pnumber = resnum;

    csBase = _T("");

    for (np = cp;  np <= xp;  np++)
        csBase += *np;
}

CString CHardLinkDlg::ComposeDiskDirName(int nDiskNumber, int nDigits)
{
    CString csNum;
    CString csZero = _T("0");
    csNum.Format(_T("%d"), nDiskNumber);
    while ((int) _tcslen(csNum) < nDigits)
        csNum = csZero + csNum;
    CString cs = _T("disk");
    cs += csNum;
    return cs;
}

BOOL CHardLinkDlg::GetFolderSize(LPCTSTR dir, ULONG64* totalsize, size_t blksize)
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA ff;
    CString csDir;
    CString csError;
    BOOL done = FALSE;
    CVStringArray aDirs;
    int k;

    memset(&ff, 0, sizeof ff);
    csDir = dir;
    if (lastchar(csDir) != _T('\\'))  csDir += _T('\\');
    csDir += _T("*.*");
    hFind = FindFirstFile(csDir, & ff);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        DWORD dwError = GetLastError();
        if (dwError == ERROR_NO_MORE_FILES)  goto completed;
        csError.Format(_T("Unable to enumerate folder %s"), dir);
        SysError(csError, dwError);
        CHECK(FALSE);
    }

    do
    {
        if (ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (! (0 == _tcsicmp(ff.cFileName, _T(".")) || 0 == _tcsicmp(ff.cFileName, _T(".."))))
                aDirs.Add(ff.cFileName);
        }
        else
        {
            *totalsize += GetFileSize(ff, blksize);
        }
    }
    while (FindNextFile(hFind, & ff));

    if (GetLastError() != ERROR_NO_MORE_FILES)
    {
        DWORD dwError = GetLastError();
        csError.Format(_T("Unable to enumerate folder %s"), dir);
        SysError(csError, dwError);
        CHECK(FALSE);
    }

    for (k = 0;  k < aDirs.GetCount();  k++)
    {
        csDir = dir;
        if (lastchar(csDir) != _T('\\'))  csDir += _T('\\');
        csDir += aDirs.GetAt(k);
        CHECK(GetFolderSize(csDir, totalsize, blksize));
    }

completed:

    done = TRUE;

cleanup:

    if (hFind != INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
    }

    return done;
}

ULONG64 CHardLinkDlg::GetFileSize(const WIN32_FIND_DATA& ff, size_t blksize)
{
    ULONG64 hsize = ff.nFileSizeHigh;
    ULONG64 lsize = ff.nFileSizeLow;
    hsize &= LMASK64;
    lsize &= LMASK64;
    ULONG64 size = (hsize << 32) + lsize;
    size = ((size + blksize - 1) / blksize) * blksize;
    return size;
}

BOOL CHardLinkDlg::ReadSplitSizeControl(size_t* psize)
{
    CString cs;
    CString cs2;
    LPCTSTR cp;
    int k;
    size_t size = 0;

    CComboBox* pcombo = (CComboBox*) GetDlgItem(IDC_SPLITSIZE);
    pcombo->GetWindowText(cs);

    // strip comment
    k = cs.Find(_T(" ("));
    if (k != -1)
        cs = cs.Left(k);

    cp = cs;
    while (*cp && (*cp == _T(' ') || *cp == _T('\t')))
        cp++;
    cs2 = cp;
    while (lastchar(cs2) == _T(' ') || lastchar(cs2) == _T('\t'))
        cs2 = cs2.Left(cs2.GetLength() - 1);

    cp = cs2;
    while (*cp)
    {
        if (! isdigit(*cp))
        {
            Error(_T("Invalid character in size selector: not a number."));
            return FALSE;
        }

        if (size > 1000000)
        {
            Error(_T("Invalid selected split size: too large."));
            return FALSE;
        }

        size = size * 10 + (*cp++ - _T('0'));
    }

    if (size < 10)
    {
        Error(_T("Invalid selected split size: too small."));
        return FALSE;
    }
    else if (size > 500000)
    {
        Error(_T("Invalid selected split size: too large."));
        return FALSE;
    }
    else
    {
        *psize = size;
        return TRUE;
    }
}

void CHardLinkDlg::OnCbnSelchangeCombo1()
{
    // TODO: Add your control notification handler code here
}

void CHardLinkDlg::OnBnClickedSplitOptions()
{
	CSplitOptions SO(m_FileDispositionStrategy, m_NotFitBehaviour, this);
	if (SO.DoModal() == IDOK)
	{
		m_FileDispositionStrategy = SO.GetStrategy();
		m_NotFitBehaviour = SO.GetNotFitBehaviour();
	}
}

void CHardLinkDlg::OnBnClickedDoCleanSplit()
{
    CBusyCursor cursor(& m_pCursor);
    cursor.busy();
    SaveConfiguration();
    CleanDir(IDC_SPLIT_TO);
}

void CHardLinkDlg::OnBnClickedCancel()
{
    SaveConfiguration();
    OnCancel();
}

void CHardLinkDlg::OnEnChangeSplitTo()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialog::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.

    // TODO:  Add your control notification handler code here
}

void CHardLinkDlg::OnBnClickedSplitToBrowse()
{
    BrowseDir(IDC_SPLIT_TO);
}

void CHardLinkDlg::OnCbnSelchangeSplitsize()
{
    // TODO: Add your control notification handler code here
}

void CHardLinkDlg::BrowseDir(int idc)
{
    CString cs;
    GetDlgItem(idc)->GetWindowText(cs);
    CFolderDialog dlg(cs, BIF_EDITBOX | BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT, this);
    if (IDOK == dlg.DoModal())
    {
        GetDlgItem(idc)->SetWindowText(dlg.GetPathName());
    }
}

void CHardLinkDlg::CleanDir(int idc)
{
    CString cs;
    GetDlgItem(idc)->GetWindowText(cs);
    TidyDirName(cs);
    if (! ValidateDirSyntax(cs))  return;
    CString cs2;
    cs2.Format(_T("Do you want to delete contents of folder %s ?"), cs);
    HCURSOR hCursor = NULL;
    if (m_pCursor) hCursor = m_pCursor->suspend();
    if (IDOK == AfxMessageBox(cs2, MB_OKCANCEL | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_TASKMODAL | MB_SETFOREGROUND))
    {
        if (m_pCursor) m_pCursor->resume(hCursor);
        cs2.Format(_T("Cleaning data from folder %s ..."), cs);
        LogImpl(_T(""));
        LogImpl(cs2);
        if (DoCleanDir(cs))
        {
            cs2.Format(_T("Cleaned data from folder %s."), cs);
            LogImpl(cs2);
        }
        else
        {
            cs2.Format(_T("Failed to clean data from folder %s."), cs);
            LogImpl(cs2);
        }
    }
    else
    {
        if (m_pCursor) m_pCursor->resume(hCursor);
    }
}

void CHardLinkDlg::TidyDirName(CString& cs)
{
    CString cs2;
    LPCTSTR cp = cs;

    while (*cp && (*cp == _T(' ') || *cp == _T('\t')))
        cp++;
    cs2 = cp;
    while (lastchar(cs2) == _T(' ') || lastchar(cs2) == _T('\t'))
        cs2 = cs2.Left(cs2.GetLength() - 1);
    cs2.Replace(_T('/'), _T('\\'));
    if (lastchar(cs2) != _T('\\') && lastchar(cs2) != _T(':'))  cs2 += _T("\\");
    while (cs2.Replace(_T("\\\\"), _T("\\"))) /* keep replacing */ ;
    cp = cs = cs2;
    if (isalpha(*cp) && cp[1] == _T(':') && cp[2] == _T('\\') && cp[3] == _T('\0'))
    {
        // no-op
    }
    else
    {
        cs = cs.Left(cs.GetLength() - 1);
    }
}

TCHAR CHardLinkDlg::lastchar(LPCTSTR cp)
{
    size_t len = _tcslen(cp);
    if (len == 0)  return _T('\0');
    else return cp[len - 1];
}

BOOL CHardLinkDlg::ValidateDirSyntax(LPCTSTR cp)
{
    if (! (isalpha(*cp) && cp[1] == _T(':') && cp[2] == _T('\\')))
    {
        CString cs;
        cs.Format(_T("Invalid folder name: %s"), cp);
        Error(cs);
        return FALSE;
    }
    return TRUE;
}

void CHardLinkDlg::Error(LPCTSTR cp)
{
    HCURSOR hCursor = NULL;
    if (m_pCursor) hCursor = m_pCursor->suspend();
    AfxMessageBox(cp, MB_OK | MB_ICONHAND | MB_TASKMODAL | MB_SETFOREGROUND);
    if (m_pCursor) m_pCursor->resume(hCursor);
}

void CHardLinkDlg::SysError(LPCTSTR cp)
{
    SysError(cp, GetLastError());
}

void CHardLinkDlg::SysError(LPCTSTR errmsg, DWORD dwError)
{
    CString csError;
    LPTSTR s = NULL;
    if (0 == ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError, 0, (LPTSTR) &s, 0, NULL))
    {
        csError.Format(_T("Unknown error code 0x%08X (%d)"), dwError, LOWORD(dwError));
    }
    else
    {
        LPTSTR p = _tcschr(s, _T('\r'));
        if (p)
        {
            /* lose CRLF */
            *p = _T('\0');
        }
        csError = s;
        ::LocalFree(s);
    }

    CString cs = errmsg;
    cs += _T("\r\n");
    cs += _T("\r\n");
    cs += csError;
    Error(cs);
}

void CHardLinkDlg::LogImpl(LPCTSTR cp)
{
    CString cs;
    GetDlgItem(IDC_LOG)->GetWindowText(cs);
    cs += _T("\r\n");
    cs += cp;
    CEdit* pLog = (CEdit*) GetDlgItem(IDC_LOG);
    ::LockWindowUpdate(*pLog);
    pLog->SetWindowText(cs);
    pLog->SendMessage(WM_VSCROLL, SB_BOTTOM, NULL);
    pLog->SendMessage(WM_HSCROLL, SB_LEFT, NULL);
    ::LockWindowUpdate(NULL);
    pLog->UpdateWindow();

    // LRESULT lr = pLog->SendMessage(EM_CHARFROMPOS, 0, 0);
    // if (HIWORD(lr) != 0)
    //     pLog->SendMessage(EM_SHOWSCROLLBAR, SB_VERT, TRUE);
}

BOOL CHardLinkDlg::DoCreateHardlink(LPCTSTR lpFileName, LPCTSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    if (s_pCreateHardlink == NULL)
    {
        HMODULE hKERNEL32 = GetModuleHandle(_T("KERNEL32"));
        CString procname = (sizeof(TCHAR) == 2) ? _T("CreateHardLinkW") : _T("CreateHardLinkA");
        s_pCreateHardlink = (PCreateHardlink) GetProcAddress(hKERNEL32, (sizeof(TCHAR) == 2) ? "CreateHardLinkW" : "CreateHardLinkA");

        if (s_pCreateHardlink == NULL)
        {
            Error(_T("This version of Windows does not expose CreateHardlink function in KERNEL32."));
            return FALSE;
        }
    }

    return (*s_pCreateHardlink)(lpFileName, lpExistingFileName, lpSecurityAttributes);
}

BOOL CHardLinkDlg::DoCleanDir(LPCTSTR dirname)
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA ff;
    CString csDir;
    CString csError;
    CString csPath;
    BOOL done = FALSE;
    CVStringArray aFiles;
    CVStringArray aDirs;
    int k;

    memset(&ff, 0, sizeof ff);
    csDir = dirname;
    if (lastchar(csDir) != _T('\\'))  csDir += _T('\\');
    csDir += _T("*.*");
    hFind = FindFirstFile(csDir, & ff);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        DWORD dwError = GetLastError();
        if (dwError == ERROR_NO_MORE_FILES)  goto completed;
        csError.Format(_T("Unable to enumerate folder %s"), dirname);
        SysError(csError, dwError);
        CHECK(FALSE);
    }

    do
    {
        csPath = dirname;
        if (lastchar(csPath) != _T('\\'))  csPath += _T('\\');
        csPath += ff.cFileName;
        if (ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (! (0 == _tcsicmp(ff.cFileName, _T(".")) || 0 == _tcsicmp(ff.cFileName, _T(".."))))
                aDirs.Add(csPath);
        }
        else
        {
            aFiles.Add(csPath);
        }
    }
    while (FindNextFile(hFind, & ff));

    if (GetLastError() != ERROR_NO_MORE_FILES)
    {
        DWORD dwError = GetLastError();
        csError.Format(_T("Unable to enumerate folder %s"), dirname);
        SysError(csError, dwError);
        CHECK(FALSE);
    }

    FindClose(hFind);
    hFind = INVALID_HANDLE_VALUE;

    // remove files
    for (k = 0;  k < aFiles.GetCount();  k++)
    {
        csPath = aFiles.GetAt(k);
        if (! DeleteFile(csPath))
        {
            DWORD dwError = GetLastError();
            csError.Format(_T("Unable to remove file %s"), csPath);
            SysError(csError, dwError);
            CHECK(FALSE);
        }
    }

    for (k = 0;  k < aDirs.GetCount();  k++)
    {
        csPath = aDirs.GetAt(k);
        CHECK(DoCleanDir(csPath));
        if (! RemoveDirectory(csPath))
        {
            DWORD dwError = GetLastError();
            csError.Format(_T("Unable to remove folder %s"), csPath);
            SysError(csError, dwError);
            CHECK(FALSE);
        }
    }

completed:

    done = TRUE;

cleanup:

    if (hFind != INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
    }

    return done;
}

BOOL CHardLinkDlg::ReadConfiguration()
{
    ReadDir(IDC_COPY_FROM, _T("CopyFrom"));
    ReadDir(IDC_COPY_TO, _T("CopyTo"));
    ReadDir(IDC_SPLIT_FROM, _T("SplitFrom"));
    ReadDir(IDC_SPLIT_TO, _T("SplitTo"));

    CString csList;
    ReadValue(_T("SplitSizeList"), csList);

    if (csList == _T(""))
    {
        csList += _T("740 (CD)");
        csList += _T("|");
        csList += _T("4400 (DVD5)");
        csList += _T("|");
        csList += _T("8800 (DVD9)");
    }

    CString csSelected;
    ReadValue(_T("SelectedSplitSize"), csSelected);
    if (csSelected == _T(""))
        csSelected = _T("4400 (DVD5)");

    CComboBox* pcombo = (CComboBox*) GetDlgItem(IDC_SPLITSIZE);
    LPCTSTR token = _tcstok(csList.LockBuffer(), _T("|"));
    int iSelected = -1;

    while (token)
    {
        int index = pcombo->AddString(token);

        if (0 == _tcscmp(token, csSelected))
            iSelected = index;

        token = _tcstok(NULL, _T("|"));
    }

    csList.UnlockBuffer();

    if (iSelected >= 0)
    {
        pcombo->SetCurSel(iSelected);
    }
    else
    {
        pcombo->SetCurSel(pcombo->AddString(csSelected));
    }

	CString FileDisposition;
	if (ReadValue(_T("FileDisposition"), FileDisposition))
	{
		int num = _ttoi(FileDisposition);
		if (1 == (num / 1000))
			m_FileDispositionStrategy = efdsLogical;
		else
			m_FileDispositionStrategy = efdsMaxFit;
		if (1 == (num % 1000))
			m_NotFitBehaviour = enfbStop;
		else if ( 2 == (num % 1000))
			m_NotFitBehaviour = enfbWarn;
		else 
			m_NotFitBehaviour = enfbToOversized;
	}
	else
	{
		m_FileDispositionStrategy = efdsMaxFit;
		m_NotFitBehaviour = enfbStop;
	}
    return TRUE;
}

BOOL CHardLinkDlg::ReadDir(int idc, LPCTSTR valname)
{
    CString cs;
    ReadValue(valname, cs);
    GetDlgItem(idc)->SetWindowText(cs);
    return TRUE;
}

BOOL CHardLinkDlg::ReadValue(LPCTSTR valname, CString& cs)
{
    cs = _T("");
    HKEY hKey = NULL;
    RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\oboguev\\HardLink\\UserSettings"), 0, KEY_READ, & hKey);
    if (hKey != NULL)
    {
        TCHAR buf[2050];
        DWORD dwSize = sizeof(buf);
        DWORD dwType = REG_NONE;
        if (ERROR_SUCCESS == RegQueryValueEx(hKey, valname, NULL, & dwType, (BYTE*) buf, & dwSize) && dwType == REG_SZ)
        {
            buf[min(countof(buf) - 1, dwSize / sizeof(TCHAR) - 1)] = _T('\0');
            cs = buf;
        }
        RegCloseKey(hKey);
    }
    return TRUE;
}

BOOL CHardLinkDlg::SaveConfiguration()
{
    SaveDir(IDC_COPY_FROM, _T("CopyFrom"));
    SaveDir(IDC_COPY_TO, _T("CopyTo"));
    SaveDir(IDC_SPLIT_FROM, _T("SplitFrom"));
    SaveDir(IDC_SPLIT_TO, _T("SplitTo"));

    CStringArray csa;
    CString cs;
    CComboBox* pcombo = (CComboBox*) GetDlgItem(IDC_SPLITSIZE);
    int count = pcombo->GetCount();
    int k;
    for (k = 0;  k < count;  k++)
    {
        pcombo->GetLBText(k, cs);
        if (cs != _T(""))  csa.Add(cs);
    }

    CString csSelected;
    pcombo->GetWindowText(csSelected);
    SaveValue(_T("SelectedSplitSize"), csSelected);

    BOOL bSelFound = FALSE;
    cs = _T("");
    for (k = 0;  k < csa.GetCount();  k++)
    {
        if (cs != _T(""))  cs += _T("|");
        cs += csa.GetAt(k);
        if (csa.GetAt(k) == csSelected)
            bSelFound = TRUE;
    }
    if (! bSelFound)
    {
        if (cs != _T(""))  cs += _T("|");
        cs += csSelected;
    }
    SaveValue(_T("SplitSizeList"), cs);

	CString FileDisposition;
	{
		int NotFitBehaviour = 3;
		switch (m_NotFitBehaviour)
		{
		case enfbStop:
			NotFitBehaviour = 1;
			break;
		case enfbWarn:
			NotFitBehaviour = 2;
			break;
		case enfbToOversized:
			NotFitBehaviour = 3;
			break;
		}
		int Strategy = m_FileDispositionStrategy == efdsLogical? 1 : 2;
		FileDisposition.Format(_T("%d"), Strategy*1000 + NotFitBehaviour);
	}
	SaveValue(_T("FileDisposition"), FileDisposition);
    return TRUE;
}

BOOL CHardLinkDlg::SaveDir(int idc, LPCTSTR valname)
{
    CString cs;
    GetDlgItem(idc)->GetWindowText(cs);
    TidyDirName(cs);
    return SaveValue(valname, cs);
}

BOOL CHardLinkDlg::SaveValue(LPCTSTR valname, LPCTSTR value)
{
    HKEY hKey = NULL;
    BOOL done = FALSE;
    CHECK(ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\oboguev\\HardLink\\UserSettings"), 0, NULL, 0, KEY_READ | KEY_WRITE, NULL, & hKey, NULL));
    CHECK(ERROR_SUCCESS == RegSetValueEx(hKey, valname, 0, REG_SZ, (const BYTE*) value, (DWORD) (_tcslen(value) + 1) * sizeof(TCHAR)));
    done = TRUE;

cleanup:

    if (hKey)  RegCloseKey(hKey);
    return done;
}

BOOL CHardLinkDlg::IsDirUnder(LPCTSTR rootdir, LPCTSTR dir)
{
    CString csRoot = rootdir;
    if (lastchar(csRoot) != _T('\\'))  csRoot += _T("\\");
    CString csDir = dir;
    if (lastchar(csDir) != _T('\\'))  csDir += _T("\\");
    if (_tcslen(csDir) < _tcslen(csRoot))
        return FALSE;
    if (0 == csDir.Left((int) _tcslen(csRoot)).CompareNoCase(csRoot))
        return TRUE;
    else
        return FALSE;
}
