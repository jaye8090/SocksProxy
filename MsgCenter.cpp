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
初始化
******************************************************************************/
void CMsgCenter::Init(CSocksProxyDlg * pMainWnd)
{
	m_pMainWnd = pMainWnd;

	m_bClose = false;

	//运行消息中心
	_Run();
}

/******************************************************************************
运行消息中心
******************************************************************************/
void CMsgCenter::_Run(void)
{
	//启动消息处理线程
	CWinThread * pThread = AfxBeginThread(_threadProcessMsg, this);
	if (pThread == NULL)
	{
		_PutTip("", MsgCenter::RUN_BEGIN_THREAD_FAIL);
		return;
	}
}

/******************************************************************************
发送消息
******************************************************************************/
void CMsgCenter::PostMsg(unsigned int uiMsg)
{
	PostMsg(uiMsg, NULL);
}

/******************************************************************************
发送消息
******************************************************************************/
bool CMsgCenter::PostMsg(unsigned int uiMsg, void * pParam)
{
	//互斥锁作用范围
	{
		//消息队列互斥锁
		CAutoLock lock(&m_csMsg, m_pfnPutTip, m_pThis);

		//已经关闭，而且有参数。
		if (m_bClose
			&& pParam != NULL)
		{
			return false;
		}

		//关闭
		if (uiMsg == MsgCenter::MSG_CLOSE)
		{
			m_bClose = true;
		}

		//往消息队列中插入一个消息
		MsgCenter::SMsg sMsg;
		sMsg.uiMsg = uiMsg;
		sMsg.pParam = pParam;
		m_queueMsg.push(sMsg);
	}

	//通知消息到达
	if (!m_eventMsg.SetEvent())
	{
		_PutTip("", MsgCenter::POST_MSG_SET_EVENT_FAIL);
		return false;
	}

	return true;
}

/******************************************************************************
消息处理线程
******************************************************************************/
UINT CMsgCenter::_threadProcessMsg(LPVOID pParam)
{
	if (pParam == NULL)
	{
		return 0;
	}
	CMsgCenter * pThis = (CMsgCenter *)pParam;
	MsgCenter::SMsg sMsg = {0};

	//消息循环
	while (true)
	{
		//获取消息，没获取到则重新获取
		if (!pThis->_GetMsg(sMsg))
		{
			continue;
		}

		//分发消息
		if (pThis->_DispatchMsg(sMsg))
		{
			return 0;
		}
	}
}

/******************************************************************************
获取消息
******************************************************************************/
bool CMsgCenter::_GetMsg(MsgCenter::SMsg & sMsg)
{
	//消息队列是否为空
	bool bEmpty = true;

	//互斥锁作用范围
	{
		//消息队列互斥锁
		CAutoLock lock(&m_csMsg, m_pfnPutTip, m_pThis);

		//从消息队列中提取一个消息
		bEmpty = m_queueMsg.empty();
		if (!bEmpty)
		{
			sMsg = m_queueMsg.front();
			m_queueMsg.pop();
		}
	}

	//如果消息队列为空，则等待
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
分发消息
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

	//初始化托盘图标
	m_pMainWnd->m_TrayIcon.Init(m_pMainWnd, _T("Socks4+5代理服务器")
		, IDR_MAINFRAME);
	//添加托盘图标
	m_pMainWnd->m_TrayIcon.Add();
	//初始化TCP
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
	if (m_pMainWnd->MessageBox(_T("确定要关闭软件吗？"), _T("关闭软件")
		, MB_OKCANCEL | MB_ICONQUESTION | MB_DEFBUTTON2) != IDOK)
	{
		return;
	}

	PostMsg(MsgCenter::MSG_CLOSE);
}

void CMsgCenter::_OnClose()
{
	_PutTip("正在关闭软件...");

	//关闭通道
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

	//停止TCP
	m_pMainWnd->m_TCP.Stop();
	//停止网络监视
	m_pMainWnd->m_NetMonitor.Stop();
	//删除托盘图标
	m_pMainWnd->m_TrayIcon.Del();

	_PutTip("正在关闭软件...关闭完成。");

	//保存提示
	m_pMainWnd->m_TipsBox.SaveTips();
	//关闭完成消息
	m_pMainWnd->PostMessage(WM_CLOSE_DONE);
}

void CMsgCenter::_OnProcessTrayIcon(unsigned int uiMsg)
{
	if (m_pMainWnd == NULL)
	{
		_PutTip("", MsgCenter::ON_PROCESS_TRAY_ICON_MAIN_WND_NULL);
		return;
	}
	//鼠标左键
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
		_PutTip("代理已经开始！");
		return;
	}

	//获取端口
	if (m_pMainWnd == NULL)
	{
		_PutTip("", MsgCenter::ON_BUTTON_START_PROXY_MAIN_WND_NULL);
		return;
	}
	unsigned short usPort = _wtoi(m_pMainWnd->m_strPort);
	if (usPort <= 0
		|| usPort >= 65535)
	{
		_PutTip("请输入正确的端口号！");
		return;
	}

	//初始化通道
	if (!m_pMainWnd->m_Channel.Init(m_pMainWnd, usPort))
	{
		_PutTip("此端口号已经被占用！");
		return;
	}

	//初始化网络监视
	m_pMainWnd->m_NetMonitor.Init(m_pMainWnd);

	bStartProxy = true;
	_PutTip("开始代理。");
}

void CMsgCenter::_OnSaveConfig()
{
	//写入设置信息
	if (m_pMainWnd == NULL)
	{
		_PutTip("", MsgCenter::ON_SAVE_CONFIG_MAIN_WND_NULL);
		return;
	}
	m_pMainWnd->WriteConfig();

	//保存设置
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
输出提示
******************************************************************************/
void CMsgCenter::_PutTip(string strTip, int iErr, string strErr)
{
	//输出提示回调
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "MsgCenter", iErr, strErr, m_pThis);
}