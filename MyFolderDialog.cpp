// MyFolderDialog.cpp: implementation of the CMyFolderDialog class.
//	$Copyright �  1998 Kenneth M. Reed, ALL RIGHTS RESERVED. $
//	$Header: MyFolderDialog.cpp  Revision:1.1  Fri Aug 21 19:46:14 1998  KenReed $
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
// #include "TestFolder.h"

#include "MyFolderDialog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyFolderDialog::CMyFolderDialog(	LPCTSTR lpszFolderName, 
									DWORD dwFlags, 
									CWnd* pParentWnd,
									LPCTSTR pszFileFilter)
	: CFolderDialog(lpszFolderName, dwFlags, pParentWnd)
{
	if (pszFileFilter == NULL)
		m_strFileFilter = _T("*.*");
	else
		m_strFileFilter = pszFileFilter;
}

CMyFolderDialog::~CMyFolderDialog()
{

}

void CMyFolderDialog::OnSelChanged(ITEMIDLIST* pIdl)
{
	CFolderDialog::OnSelChanged(pIdl);

	CFileFind file;
	CString strFullFilter;
	strFullFilter = CString(m_szPath) + _T("\\") + m_strFileFilter;

	if (file.FindFile(strFullFilter))
	{
		EnableOK(TRUE);
		file.Close();
	}
	else
	{
		EnableOK(FALSE);
	}
}
