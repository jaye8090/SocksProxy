#include "StdAfx.h"

#include "TrayIcon.h"

//������������Ϣ
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
��ʼ��
******************************************************************************/
void CTrayIcon::Init(CWnd * pMainWnd, CString strTip, UINT uiIconID)
{
	m_pMainWnd = pMainWnd;

	m_strTip = strTip;

	m_uiIconID = uiIconID;

	memset(&m_IconData, 0, sizeof(m_IconData));
}

/******************************************************************************
���
******************************************************************************/
void CTrayIcon::Add(void)
{
	//��дͼ������
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

	//���
	if (!Shell_NotifyIcon(NIM_ADD, &m_IconData))
	{
		_PutTip("", TrayIcon::ADD_ADD_FAIL);
		return;
	}
}

/******************************************************************************
ɾ��
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
�����ʾ��Ϣ
******************************************************************************/
void CTrayIcon::_PutTip(string strTip, int iErr, string strErr)
{
	//�����ʾ�ص�
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "TrayIcon", iErr, strErr, m_pThis);
}