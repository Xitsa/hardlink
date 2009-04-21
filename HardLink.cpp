// HardLink.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "HardLink.h"
#include "HardLinkDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CHardLinkApp

BEGIN_MESSAGE_MAP(CHardLinkApp, CWinApp)
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CHardLinkApp construction

CHardLinkApp::CHardLinkApp()
{
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
}


// The one and only CHardLinkApp object

CHardLinkApp theApp;


// CHardLinkApp initialization

BOOL CHardLinkApp::InitInstance()
{
    // InitCommonControls() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    InitCommonControls();

    CWinApp::InitInstance();

    AfxEnableControlContainer();

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (hr != S_OK && hr != S_FALSE)
    {
        AfxMessageBox(_T("CoInitialize failed"));
        return FALSE;
    }

    CHardLinkDlg dlg;
    m_pMainWnd = &dlg;
    INT_PTR nResponse = dlg.DoModal();
    if (nResponse == IDOK)
    {
        // TODO: Place code here to handle when the dialog is
        //  dismissed with OK
    }
    else if (nResponse == IDCANCEL)
    {
        // TODO: Place code here to handle when the dialog is
        //  dismissed with Cancel
    }

    if (hr == S_OK)
        CoUninitialize();

    // Since the dialog has been closed, return FALSE so that we exit the
    //  application, rather than start the application's message pump.
    return FALSE;
}
