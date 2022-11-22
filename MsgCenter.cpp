#include "StdAfx.h"

#include "MsgCenter.h"
#include "SocksProxyDlg.h"
#include "Lock/AutoLock.h"

CMsgCenter::CMsgCenter(MsgCenter::PFNPutTip pfnPutTip, void * pThis)
{
	m_pfnPutTip = pfnPutTip;
	m_pThis = pThis;
}

CMsgCenter::~CMsgCenter(void)
{
}

/******************************************************************************
��ʼ��
******************************************************************************/
void CMsgCenter::Init(CSocksProxyDlg * pMainWnd)
{
	m_pMainWnd = pMainWnd;

	m_bClose = false;

	//������Ϣ����
	_Run();
}

/******************************************************************************
������Ϣ����
******************************************************************************/
void CMsgCenter::_Run(void)
{
	//������Ϣ�����߳�
	CWinThread * pThread = AfxBeginThread(_threadProcessMsg, this);
	if (pThread == NULL)
	{
		_PutTip("", MsgCenter::RUN_BEGIN_THREAD_FAIL);
		return;
	}
}

/******************************************************************************
������Ϣ
******************************************************************************/
void CMsgCenter::PostMsg(unsigned int uiMsg)
{
	PostMsg(uiMsg, NULL);
}

/******************************************************************************
������Ϣ
******************************************************************************/
bool CMsgCenter::PostMsg(unsigned int uiMsg, void * pParam)
{
	//���������÷�Χ
	{
		//��Ϣ���л�����
		CAutoLock lock(&m_csMsg, m_pfnPutTip, m_pThis);

		//�Ѿ��رգ������в�����
		if (m_bClose
			&& pParam != NULL)
		{
			return false;
		}

		//�ر�
		if (uiMsg == MsgCenter::MSG_CLOSE)
		{
			m_bClose = true;
		}

		//����Ϣ�����в���һ����Ϣ
		MsgCenter::SMsg sMsg;
		sMsg.uiMsg = uiMsg;
		sMsg.pParam = pParam;
		m_queueMsg.push(sMsg);
	}

	//֪ͨ��Ϣ����
	if (!m_eventMsg.SetEvent())
	{
		_PutTip("", MsgCenter::POST_MSG_SET_EVENT_FAIL);
		return false;
	}

	return true;
}

/******************************************************************************
��Ϣ�����߳�
******************************************************************************/
UINT CMsgCenter::_threadProcessMsg(LPVOID pParam)
{
	if (pParam == NULL)
	{
		return 0;
	}
	CMsgCenter * pThis = (CMsgCenter *)pParam;
	MsgCenter::SMsg sMsg = {0};

	//��Ϣѭ��
	while (true)
	{
		//��ȡ��Ϣ��û��ȡ�������»�ȡ
		if (!pThis->_GetMsg(sMsg))
		{
			continue;
		}

		//�ַ���Ϣ
		if (pThis->_DispatchMsg(sMsg))
		{
			return 0;
		}
	}
}

/******************************************************************************
��ȡ��Ϣ
******************************************************************************/
bool CMsgCenter::_GetMsg(MsgCenter::SMsg & sMsg)
{
	//��Ϣ�����Ƿ�Ϊ��
	bool bEmpty = true;

	//���������÷�Χ
	{
		//��Ϣ���л�����
		CAutoLock lock(&m_csMsg, m_pfnPutTip, m_pThis);

		//����Ϣ��������ȡһ����Ϣ
		bEmpty = m_queueMsg.empty();
		if (!bEmpty)
		{
			sMsg = m_queueMsg.front();
			m_queueMsg.pop();
		}
	}

	//�����Ϣ����Ϊ�գ���ȴ�
	if (bEmpty)
	{
		if (!m_eventMsg.Lock())
		{
			_PutTip("", MsgCenter::GET_MSG_LOCK_EVENT_FAIL);
			return false;
		}
		return false;
	}

	return true;
}

/******************************************************************************
�ַ���Ϣ
******************************************************************************/
bool CMsgCenter::_DispatchMsg(const MsgCenter::SMsg & sMsg)
{
	switch (sMsg.uiMsg)
	{
	case MsgCenter::MSG_INIT_SOFTWARE:
		{
			_OnInitSoftware();
		}
		break;

	case MsgCenter::MSG_ADD_TRAY_ICON:
		{
			_OnAddTrayIcon();
		}
		break;

	case MsgCenter::MSG_CLOSE_BOX:
		{
			_OnCloseBox();
		}
		break;

	case MsgCenter::MSG_CLOSE:
		{
			_OnClose();
			return true;
		}
		break;

	case MsgCenter::MSG_PROCESS_TRAY_ICON:
		{
			_OnProcessTrayIcon((unsigned int)sMsg.pParam);
		}
		break;

	case MsgCenter::MSG_BUTTON_START_PROXY:
		{
			_OnButtonStartProxy();
		}
		break;

	case MsgCenter::MSG_SAVE_CONFIG:
		{
			_OnSaveConfig();
		}
		break;

	case MsgCenter::MSG_PROCESS_STARTUP_RUN:
		{
			_OnProcessStartupRun();
		}
		break;

	default:
		{
			_PutTip("", MsgCenter::DISPATCH_MSG_UNKNOW_MSG);
			return false;
		}
		break;
	}

	return false;
}

void CMsgCenter::_OnInitSoftware()
{
	if (m_pMainWnd == NULL)
	{
		_PutTip("", MsgCenter::ON_INIT_SOFTWARE_MAIN_WND_NULL);
		return;
	}

	//��ʼ������ͼ��
	m_pMainWnd->m_TrayIcon.Init(m_pMainWnd, _T("Socks4+5���������")
		, IDR_MAINFRAME);
	//�������ͼ��
	m_pMainWnd->m_TrayIcon.Add();
	//��ʼ��TCP
	m_pMainWnd->m_TCP.Init();
}

void CMsgCenter::_OnAddTrayIcon()
{
	if (m_pMainWnd == NULL)
	{
		_PutTip("", MsgCenter::ON_ADD_TRAY_ICON_MAIN_WND_NULL);
		return;
	}
	m_pMainWnd->m_TrayIcon.Add();
}

void CMsgCenter::_OnCloseBox()
{
	if (m_pMainWnd == NULL)
	{
		_PutTip("", MsgCenter::ON_CLOSE_BOX_MAIN_WND_NULL);
		return;
	}
	if (m_pMainWnd->MessageBox(_T("ȷ��Ҫ�ر������"), _T("�ر����")
		, MB_OKCANCEL | MB_ICONQUESTION | MB_DEFBUTTON2) != IDOK)
	{
		return;
	}

	PostMsg(MsgCenter::MSG_CLOSE);
}

void CMsgCenter::_OnClose()
{
	_PutTip("���ڹر����...");

	//�ر�ͨ��
	if (m_pMainWnd == NULL)
	{
		_PutTip("", MsgCenter::ON_CLOSE_MAIN_WND_NULL);
		return;
	}
	m_pMainWnd->m_Channel.Close();
	while (!m_pMainWnd->m_Channel.IsClose())
	{
		Sleep(100);
	}

	//ֹͣTCP
	m_pMainWnd->m_TCP.Stop();
	//ֹͣ�������
	m_pMainWnd->m_NetMonitor.Stop();
	//ɾ������ͼ��
	m_pMainWnd->m_TrayIcon.Del();

	_PutTip("���ڹر����...�ر���ɡ�");

	//������ʾ
	m_pMainWnd->m_TipsBox.SaveTips();
	//�ر������Ϣ
	m_pMainWnd->PostMessage(WM_CLOSE_DONE);
}

void CMsgCenter::_OnProcessTrayIcon(unsigned int uiMsg)
{
	if (m_pMainWnd == NULL)
	{
		_PutTip("", MsgCenter::ON_PROCESS_TRAY_ICON_MAIN_WND_NULL);
		return;
	}
	//������
	if (uiMsg == WM_LBUTTONDOWN)
	{
		if (m_pMainWnd->IsWindowVisible())
		{
			m_pMainWnd->ShowWindow(SW_HIDE);
		}
		else
		{
			m_pMainWnd->ShowWindow(SW_SHOW);
			SwitchToThisWindow(m_pMainWnd->m_hWnd, TRUE);
		}
	}
}

void CMsgCenter::_OnButtonStartProxy()
{
	static bool bStartProxy = false;
	if (bStartProxy)
	{
		_PutTip("�����Ѿ���ʼ��");
		return;
	}

	//��ȡ�˿�
	if (m_pMainWnd == NULL)
	{
		_PutTip("", MsgCenter::ON_BUTTON_START_PROXY_MAIN_WND_NULL);
		return;
	}
	unsigned short usPort = _wtoi(m_pMainWnd->m_strPort);
	if (usPort <= 0
		|| usPort >= 65535)
	{
		_PutTip("��������ȷ�Ķ˿ںţ�");
		return;
	}

	//��ʼ��ͨ��
	if (!m_pMainWnd->m_Channel.Init(m_pMainWnd, usPort))
	{
		_PutTip("�˶˿ں��Ѿ���ռ�ã�");
		return;
	}

	//��ʼ���������
	m_pMainWnd->m_NetMonitor.Init(m_pMainWnd);

	bStartProxy = true;
	_PutTip("��ʼ����");
}

void CMsgCenter::_OnSaveConfig()
{
	//д��������Ϣ
	if (m_pMainWnd == NULL)
	{
		_PutTip("", MsgCenter::ON_SAVE_CONFIG_MAIN_WND_NULL);
		return;
	}
	m_pMainWnd->WriteConfig();

	//��������
	m_pMainWnd->m_ConfigCenter.SaveConfig();
}

void CMsgCenter::_OnProcessStartupRun()
{
	if (m_pMainWnd == NULL)
	{
		_PutTip("", MsgCenter::ON_PROCESS_STARTUP_RUN_MAIN_WND_NULL);
		return;
	}
	m_pMainWnd->m_ConfigCenter.Lock();
	BOOL bStartupRun = m_pMainWnd->m_ConfigCenter.bStartupRun;
	m_pMainWnd->m_ConfigCenter.Unlock();

	if (bStartupRun)
	{
		_OnButtonStartProxy();
	}
}

/******************************************************************************
�����ʾ
******************************************************************************/
void CMsgCenter::_PutTip(string strTip, int iErr, string strErr)
{
	//�����ʾ�ص�
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "MsgCenter", iErr, strErr, m_pThis);
}