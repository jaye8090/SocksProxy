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
初始化
******************************************************************************/
void CNetMonitor::Init(CSocksProxyDlg * pMainWnd)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	m_pMainWnd = pMainWnd;

	m_uiNetSpeed = 0;
	m_uiTraffic = 0;

	//取消退出事件
	if (!m_eventExit.ResetEvent())
	{
		_PutTip("", NetMonitor::INIT_RESET_EVENT_FAIL);
		return;
	}

	//启动监视线程
	if (AfxBeginThread(_threadMonitor, this) == NULL)
	{
		_PutTip("", NetMonitor::INIT_BEGIN_THREAD_FAIL);
		return;
	}
}

/******************************************************************************
添加数据
******************************************************************************/
void CNetMonitor::AddData(unsigned int uiDataSize)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//网速
	if (uiDataSize <= 0)
	{
		_PutTip("", NetMonitor::ADD_DATA_DATA_SIZE_INVALID);
		return;
	}
	m_uiNetSpeed += uiDataSize;

	//流量
	m_uiTraffic += uiDataSize;
}

/******************************************************************************
停止
******************************************************************************/
void CNetMonitor::Stop(void)
{
	//触发停止事件
	if (!m_eventStop.SetEvent())
	{
		_PutTip("", NetMonitor::STOP_SET_EVENT_FAIL);
		return;
	}

	//等待退出事件
	if (!m_eventExit.Lock())
	{
		_PutTip("", NetMonitor::STOP_LOCK_EVENT_FAIL);
		return;
	}
}

/******************************************************************************
监视线程
******************************************************************************/
UINT CNetMonitor::_threadMonitor(LPVOID pParam)
{
	if (pParam == NULL)
	{
		return 0;
	}
	CNetMonitor * pThis = (CNetMonitor *)pParam;

	//监视
	pThis->_Monitor();

	//触发退出事件
	if (!pThis->m_eventExit.SetEvent())
	{
		pThis->_PutTip("", NetMonitor::THREAD_MONITOR_SET_EVENT_FAIL);
		return 0;
	}

	return 0;
}

/******************************************************************************
监视
******************************************************************************/
void CNetMonitor::_Monitor(void)
{
	//循环监视
	while (true)
	{
		//间隔1秒，停止事件触发则退出
		if (m_eventStop.Lock(1000))
		{
			return;
		}

		//显示连接数
		if (m_pMainWnd == NULL)
		{
			_PutTip("", NetMonitor::MONITOR_MAIN_WND_NULL);
			return;
		}
		CString strConnCnt = _T("");
		strConnCnt.Format(_T("%d")
			, m_pMainWnd->m_Channel.GetConnCnt());
		m_pMainWnd->m_editConnCnt.SetWindowText(strConnCnt);

		//互斥锁
		CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

		//显示网速和流量
		m_pMainWnd->m_editNetSpeed.SetWindowText(
			_GetNetSpeedStr(m_uiNetSpeed));
		m_pMainWnd->m_editTraffic.SetWindowText(_GetTrafficStr(m_uiTraffic));

		//网速清零
		m_uiNetSpeed = 0;
	}
}

/******************************************************************************
获取网速字符串
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
获取流量字符串
******************************************************************************/
CString CNetMonitor::_GetTrafficStr(unsigned __int64 uiTraffic)
{
	//流量PB
	unsigned int uiTrafficPB = (unsigned int)
		(uiTraffic / 1024 / 1024 / 1024 / 1024 / 1024);

	//流量TB
	unsigned int uiTrafficTB = uiTraffic / 1024 / 1024 / 1024 / 1024 % 1024;
	//流量GB
	unsigned int uiTrafficGB = uiTraffic / 1024 / 1024 / 1024 % 1024;
	//流量MB
	unsigned int uiTrafficMB = uiTraffic / 1024 / 1024 % 1024;
	//流量KB
	unsigned int uiTrafficKB = uiTraffic / 1024 % 1024;
	//流量B
	unsigned int uiTrafficB = uiTraffic % 1024;

	//流量字符串
	CString strTraffic = _T("");
	strTraffic.Format(_T("%dPB %dTB %dGB %dMB %dKB %dB")
		, uiTrafficPB, uiTrafficTB, uiTrafficGB
		, uiTrafficMB, uiTrafficKB, uiTrafficB);

	return strTraffic;
}

/******************************************************************************
输出提示
******************************************************************************/
void CNetMonitor::_PutTip(string strTip, int iErr, string strErr)
{
	//输出提示回调
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "NetMonitor", iErr, strErr, m_pThis);
}