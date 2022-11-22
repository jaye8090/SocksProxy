#include "StdAfx.h"

#include "NetMonitor.h"
#include "Lock/AutoLock.h"
#include "SocksProxyDlg.h"

CNetMonitor::CNetMonitor(NetMonitor::PFNPutTip pfnPutTip, void * pThis)
	: m_eventStop(FALSE, TRUE)
	, m_eventExit(TRUE, TRUE)
{
	m_pfnPutTip = pfnPutTip;
	m_pThis = pThis;
}

CNetMonitor::~CNetMonitor(void)
{
}

/******************************************************************************
��ʼ��
******************************************************************************/
void CNetMonitor::Init(CSocksProxyDlg * pMainWnd)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	m_pMainWnd = pMainWnd;

	m_uiNetSpeed = 0;
	m_uiTraffic = 0;

	//ȡ���˳��¼�
	if (!m_eventExit.ResetEvent())
	{
		_PutTip("", NetMonitor::INIT_RESET_EVENT_FAIL);
		return;
	}

	//���������߳�
	if (AfxBeginThread(_threadMonitor, this) == NULL)
	{
		_PutTip("", NetMonitor::INIT_BEGIN_THREAD_FAIL);
		return;
	}
}

/******************************************************************************
�������
******************************************************************************/
void CNetMonitor::AddData(unsigned int uiDataSize)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//����
	if (uiDataSize <= 0)
	{
		_PutTip("", NetMonitor::ADD_DATA_DATA_SIZE_INVALID);
		return;
	}
	m_uiNetSpeed += uiDataSize;

	//����
	m_uiTraffic += uiDataSize;
}

/******************************************************************************
ֹͣ
******************************************************************************/
void CNetMonitor::Stop(void)
{
	//����ֹͣ�¼�
	if (!m_eventStop.SetEvent())
	{
		_PutTip("", NetMonitor::STOP_SET_EVENT_FAIL);
		return;
	}

	//�ȴ��˳��¼�
	if (!m_eventExit.Lock())
	{
		_PutTip("", NetMonitor::STOP_LOCK_EVENT_FAIL);
		return;
	}
}

/******************************************************************************
�����߳�
******************************************************************************/
UINT CNetMonitor::_threadMonitor(LPVOID pParam)
{
	if (pParam == NULL)
	{
		return 0;
	}
	CNetMonitor * pThis = (CNetMonitor *)pParam;

	//����
	pThis->_Monitor();

	//�����˳��¼�
	if (!pThis->m_eventExit.SetEvent())
	{
		pThis->_PutTip("", NetMonitor::THREAD_MONITOR_SET_EVENT_FAIL);
		return 0;
	}

	return 0;
}

/******************************************************************************
����
******************************************************************************/
void CNetMonitor::_Monitor(void)
{
	//ѭ������
	while (true)
	{
		//���1�룬ֹͣ�¼��������˳�
		if (m_eventStop.Lock(1000))
		{
			return;
		}

		//��ʾ������
		if (m_pMainWnd == NULL)
		{
			_PutTip("", NetMonitor::MONITOR_MAIN_WND_NULL);
			return;
		}
		CString strConnCnt = _T("");
		strConnCnt.Format(_T("%d")
			, m_pMainWnd->m_Channel.GetConnCnt());
		m_pMainWnd->m_editConnCnt.SetWindowText(strConnCnt);

		//������
		CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

		//��ʾ���ٺ�����
		m_pMainWnd->m_editNetSpeed.SetWindowText(
			_GetNetSpeedStr(m_uiNetSpeed));
		m_pMainWnd->m_editTraffic.SetWindowText(_GetTrafficStr(m_uiTraffic));

		//��������
		m_uiNetSpeed = 0;
	}
}

/******************************************************************************
��ȡ�����ַ���
******************************************************************************/
CString CNetMonitor::_GetNetSpeedStr(unsigned int uiNetSpeed)
{
	CString strNetSpeed = _T("");
	if (uiNetSpeed >= 1024 * 1024 * 1024)
	{
		strNetSpeed.Format(_T("%.2f GB/S")
			, (float)uiNetSpeed / 1024 / 1024 / 1024);
	}
	else if (uiNetSpeed >= 1024 * 1024)
	{
		strNetSpeed.Format(_T("%.2f MB/S"), (float)uiNetSpeed / 1024 / 1024);
	}
	else if (uiNetSpeed >= 1024)
	{
		strNetSpeed.Format(_T("%.2f KB/S"), (float)uiNetSpeed / 1024);
	}
	else
	{
		strNetSpeed.Format(_T("%d B/S"), uiNetSpeed);
	}

	return strNetSpeed;
}

/******************************************************************************
��ȡ�����ַ���
******************************************************************************/
CString CNetMonitor::_GetTrafficStr(unsigned __int64 uiTraffic)
{
	//����PB
	unsigned int uiTrafficPB = (unsigned int)
		(uiTraffic / 1024 / 1024 / 1024 / 1024 / 1024);

	//����TB
	unsigned int uiTrafficTB = uiTraffic / 1024 / 1024 / 1024 / 1024 % 1024;
	//����GB
	unsigned int uiTrafficGB = uiTraffic / 1024 / 1024 / 1024 % 1024;
	//����MB
	unsigned int uiTrafficMB = uiTraffic / 1024 / 1024 % 1024;
	//����KB
	unsigned int uiTrafficKB = uiTraffic / 1024 % 1024;
	//����B
	unsigned int uiTrafficB = uiTraffic % 1024;

	//�����ַ���
	CString strTraffic = _T("");
	strTraffic.Format(_T("%dPB %dTB %dGB %dMB %dKB %dB")
		, uiTrafficPB, uiTrafficTB, uiTrafficGB
		, uiTrafficMB, uiTrafficKB, uiTrafficB);

	return strTraffic;
}

/******************************************************************************
�����ʾ
******************************************************************************/
void CNetMonitor::_PutTip(string strTip, int iErr, string strErr)
{
	//�����ʾ�ص�
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "NetMonitor", iErr, strErr, m_pThis);
}