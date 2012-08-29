// SplitOptions.cpp : implementation file
//

#include "stdafx.h"
#include "HardLink.h"
#include "SplitOptions.h"


// CSplitOptions dialog

IMPLEMENT_DYNAMIC(CSplitOptions, CDialog)
CSplitOptions::CSplitOptions(EFileDispositionStrategy strategy, ENotFitBehaviour notFitBehaviour, CWnd* pParent /*=NULL*/)
	: CDialog(CSplitOptions::IDD, pParent)
	, m_Strategy(strategy)
	, m_NotFitBehaviour(notFitBehaviour)
{
}

CSplitOptions::~CSplitOptions()
{
}

BOOL CSplitOptions::OnInitDialog()
{
	BOOL Result = CDialog::OnInitDialog();
	switch (m_Strategy)
	{
	case efdsLogical:
		m_LogicalStrategy.SetCheck(TRUE);
		break;
	case efdsMaxFit:
		m_MaxPackStrategy.SetCheck(TRUE);
		break;
	}
	switch (m_NotFitBehaviour)
	{
	case enfbStop:
		m_OS_Stop.SetCheck(TRUE);
		break;
	case enfbWarn:
		m_OS_Warn.SetCheck(TRUE);
		break;
	case enfbToOversized:
		m_OS_Copy.SetCheck(TRUE);
		break;
	}
	return Result;
}

void CSplitOptions::OnOK()
{
	m_Strategy = GetStrategyI();
	m_NotFitBehaviour = GetNotFitBehaviourI();

	CDialog::OnOK();
}

EFileDispositionStrategy CSplitOptions::GetStrategyI()
{
	if (m_LogicalStrategy.GetCheck())
		return efdsLogical;
	return efdsMaxFit;
}

ENotFitBehaviour CSplitOptions::GetNotFitBehaviourI()
{
	if (m_OS_Stop.GetCheck())
		return enfbStop;
	else if (m_OS_Warn.GetCheck())
		return enfbWarn;
	return enfbToOversized;
}


void CSplitOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOGICAL_STRUCTURE, m_LogicalStrategy);
	DDX_Control(pDX, IDC_MAXIMUM_PACKING, m_MaxPackStrategy);
	DDX_Control(pDX, IDC_OS_STOP, m_OS_Stop);
	DDX_Control(pDX, IDC_OS_WARN, m_OS_Warn);
	DDX_Control(pDX, IDC_OS_OVERSIZED, m_OS_Copy);
}


BEGIN_MESSAGE_MAP(CSplitOptions, CDialog)
END_MESSAGE_MAP()


// CSplitOptions message handlers
