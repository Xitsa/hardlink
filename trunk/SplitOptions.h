#pragma once

#include "Strategy.h"

// CSplitOptions dialog

class CSplitOptions : public CDialog
{
	DECLARE_DYNAMIC(CSplitOptions)
	CButton m_LogicalStrategy;
	CButton m_MaxPackStrategy;
	CButton m_OS_Stop;
	CButton m_OS_Warn;
	CButton m_OS_Copy;
	EFileDispositionStrategy m_Strategy; 
	ENotFitBehaviour m_NotFitBehaviour;

	EFileDispositionStrategy GetStrategyI(); 
	ENotFitBehaviour GetNotFitBehaviourI();

public:
	CSplitOptions(EFileDispositionStrategy strategy, ENotFitBehaviour notFitBehaviour, CWnd* pParent = NULL);   // standard constructor
	virtual ~CSplitOptions();
	virtual BOOL OnInitDialog();
	EFileDispositionStrategy GetStrategy() { return m_Strategy; }
	ENotFitBehaviour GetNotFitBehaviour() { return m_NotFitBehaviour; }

// Dialog Data
	enum { IDD = IDD_SPLIT_OPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
};
