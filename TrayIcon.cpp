#include "StdAfx.h"

#include "TrayIcon.h"

//任务栏创建消息
UINT CTrayIcon::m_wmTaskbarCreated =
	RegisterWindowMessage(_T("TaskbarCreated"));

CTrayIcon::CTrayIcon(TrayIcon::PFNPutTip pfnPutTip, void * pThis)
{
	m_pfnPutTip = pfnPutTip;
	m_pThis = pThis;
}

CTrayIcon::~CTrayIcon(void)
{
}

/******************************************************************************
初始化
******************************************************************************/
void CTrayIcon::Init(CWnd * pMainWnd, CString strTip, UINT uiIconID)
{
	m_pMainWnd = pMainWnd;

	m_strTip = strTip;

	m_uiIconID = uiIconID;

	memset(&m_IconData, 0, sizeof(m_IconData));
}

/******************************************************************************
添加
******************************************************************************/
void CTrayIcon::Add(void)
{
	//填写图标数据
	m_IconData.cbSize = sizeof(m_IconData);
	if (m_pMainWnd == NULL)
	{
		_PutTip("", TrayIcon::ADD_MAIN_WND_NULL);
		return;
	}
	m_IconData.hWnd = m_pMainWnd->m_hWnd;
	m_IconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_IconData.uCallbackMessage = WM_TRAY_ICON;
	CWinApp * pApp = AfxGetApp();
	if (pApp == NULL
		|| m_uiIconID == 0)
	{
		_PutTip("", TrayIcon::ADD_LOAD_ICON_PARAM);
		return;
	}
	m_IconData.hIcon = pApp->LoadIcon(m_uiIconID);
	if (m_IconData.hIcon == NULL)
	{
		_PutTip("", TrayIcon::ADD_LOAD_ICON_FAIL);
		return;
	}
	if (m_strTip == _T(""))
	{
		_PutTip("", TrayIcon::ADD_TIP_EMPTY);
		return;
	}
	wcscpy_s(m_IconData.szTip, m_strTip);

	//添加
	if (!Shell_NotifyIcon(NIM_ADD, &m_IconData))
	{
		_PutTip("", TrayIcon::ADD_ADD_FAIL);
		return;
	}
}

/******************************************************************************
删除
******************************************************************************/
void CTrayIcon::Del(void)
{
	if (!Shell_NotifyIcon(NIM_DELETE, &m_IconData))
	{
		_PutTip("", TrayIcon::DEL_DEL_FAIL);
		return;
	}
}

/******************************************************************************
输出提示信息
******************************************************************************/
void CTrayIcon::_PutTip(string strTip, int iErr, string strErr)
{
	//输出提示回调
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "TrayIcon", iErr, strErr, m_pThis);
}