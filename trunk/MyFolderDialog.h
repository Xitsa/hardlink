// MyFolderDialog.h: interface for the CMyFolderDialog class.
//	$Copyright �  1998 Kenneth M. Reed, ALL RIGHTS RESERVED. $
//	$Header: MyFolderDialog.h  Revision:1.2  Fri Aug 21 19:43:18 1998  KenReed $
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYFOLDERDIALOG_H__F530A123_0ABC_11D2_9F64_EEF2A65C1C66__INCLUDED_)
#define AFX_MYFOLDERDIALOG_H__F530A123_0ABC_11D2_9F64_EEF2A65C1C66__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "FolderDialog.h"

class CMyFolderDialog : public CFolderDialog  
{
public:
	CMyFolderDialog(LPCTSTR lpszFolderName = NULL, 
					DWORD dwFlags = NULL, 
					CWnd* pParentWnd = NULL,
					LPCTSTR pszFileFilter = NULL);
	virtual ~CMyFolderDialog();
protected:
	virtual void OnSelChanged(ITEMIDLIST* pIdl);

protected:
	CString m_strFileFilter;
};

#endif // !defined(AFX_MYFOLDERDIALOG_H__F530A123_0ABC_11D2_9F64_EEF2A65C1C66__INCLUDED_)
