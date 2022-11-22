#include "StdAfx.h"

#include "Channel.h"
#include "../Lock/AutoLock.h"
#include "../SocksProxyDlg.h"
#include "UserConn.h"
#include "DstConn.h"

CChannel::CChannel(Channel::PFNPutTip pfnPutTip, void * pThis)
	: m_eventStop(FALSE, TRUE)
	, m_eventExit(TRUE, TRUE)
{
	m_pfnPutTip = pfnPutTip;
	m_pThis = pThis;

	m_pMainWnd = NULL;

	m_socket = INVALID_SOCKET;

	m_setBridge.clear();
}

CChannel::~CChannel(void)
{
}

/******************************************************************************
初始化
******************************************************************************/
bool CChannel::Init(CSocksProxyDlg * pMainWnd, unsigned short usPort)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	m_pMainWnd = pMainWnd;

	m_socket = INVALID_SOCKET;

	m_setBridge.clear();

	//绑定端口
	if (m_pMainWnd == NULL
		|| usPort <= 0
		|| usPort >= 65535)
	{
		_PutTip("", Channel::INIT_BIND_PARAM);
		return false;
	}
	if (!m_pMainWnd->m_TCP.Bind(m_socket, usPort))
	{
		return false;
	}

	//监听套接字
	if (m_socket == INVALID_SOCKET)
	{
		_PutTip("", Channel::INIT_SOCKET_INVALID);
		return false;
	}
	m_pMainWnd->m_TCP.Listen(m_socket, _callbackAccept, this);

	//取消退出事件
	if (!m_eventExit.ResetEvent())
	{
		_PutTip("", Channel::INIT_RESET_EVENT_FAIL);
		return false;
	}

	//启动检查活跃线程
	if (AfxBeginThread(_threadCheckAlive, this) == NULL)
	{
		_PutTip("", Channel::INIT_BEGIN_THREAD_FAIL);
		return false;
	}

	return true;
}

/******************************************************************************
删除用户连接
******************************************************************************/
void CChannel::DelUserConn(Channel::SBridge * pBridge)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//目标连接存在
	if (pBridge == NULL
		|| pBridge->pUserConn == NULL)
	{
		_PutTip("", Channel::DEL_USER_CONN_BRIDGE_PARAM);
		return;
	}
	if (pBridge->pDstConn != NULL)
	{
		//关闭目标连接
		pBridge->pDstConn->Close();

		//删除用户连接
		delete pBridge->pUserConn;
		pBridge->pUserConn = NULL;
	}
	//目标连接为空
	else
	{
		//删除用户连接
		delete pBridge->pUserConn;
		pBridge->pUserConn = NULL;

		//删除桥接
		_DelBridge(pBridge);
	}
}

/******************************************************************************
删除目标连接
******************************************************************************/
void CChannel::DelDstConn(Channel::SBridge * pBridge)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//用户连接存在
	if (pBridge == NULL
		|| pBridge->pDstConn == NULL)
	{
		_PutTip("", Channel::DEL_DST_CONN_BRIDGE_PARAM);
		return;
	}
	if (pBridge->pUserConn != NULL)
	{
		//关闭用户连接
		pBridge->pUserConn->Close();

		//删除目标连接
		delete pBridge->pDstConn;
		pBridge->pDstConn = NULL;
	}
	//用户连接为空
	else
	{
		//删除目标连接
		delete pBridge->pDstConn;
		pBridge->pDstConn = NULL;

		//删除桥接
		_DelBridge(pBridge);
	}
}

/******************************************************************************
获取连接数
******************************************************************************/
unsigned int CChannel::GetConnCnt(void)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	return m_setBridge.size();
}

/******************************************************************************
关闭
******************************************************************************/
void CChannel::Close(void)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//已经关闭
	if (m_socket == INVALID_SOCKET)
	{
		return;
	}

	//关闭套接字，停止监听端口。
	if (m_pMainWnd == NULL)
	{
		_PutTip("", Channel::CLOSE_CLOSE_PARAM);
		return;
	}
	m_pMainWnd->m_TCP.Close(m_socket);
	m_socket = INVALID_SOCKET;

	//关闭所有桥接
	_CloseAllBridge();

	//触发停止事件
	if (!m_eventStop.SetEvent())
	{
		_PutTip("", Channel::CLOSE_SET_EVENT_FAIL);
		return;
	}
}

/******************************************************************************
是否已经关闭
******************************************************************************/
bool CChannel::IsClose(void)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	if (m_setBridge.empty()
		&& m_eventExit.Lock(0))
	{
		return true;
	}

	return false;
}

/******************************************************************************
检查活跃线程
******************************************************************************/
UINT CChannel::_threadCheckAlive(LPVOID pParam)
{
	if (pParam == NULL)
	{
		return 0;
	}
	CChannel * pThis = (CChannel *)pParam;

	//检查活跃
	pThis->_CheckAlive();

	//触发退出事件
	if (!pThis->m_eventExit.SetEvent())
	{
		pThis->_PutTip("", Channel::THREAD_CHECK_ALIVE_SET_EVENT_FAIL);
		return 0;
	}

	return 0;
}

/******************************************************************************
检查活跃
******************************************************************************/
void CChannel::_CheckAlive(void)
{
	//循环检查
	while (true)
	{
		//间隔时间，停止事件触发则退出
		if (m_eventStop.Lock(Channel::CHECK_ALIVE_SPACE_TIME))
		{
			return;
		}

		//互斥锁
		CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

		//已经关闭
		if (m_socket == INVALID_SOCKET)
		{
			return;
		}

		//遍历桥接表
		set<Channel::SBridge *>::iterator it = m_setBridge.begin();
		while (it != m_setBridge.end())
		{
			//获取桥接
			Channel::SBridge * pBridge = *it;
			if (pBridge == NULL)
			{
				_PutTip("", Channel::CHECK_ALIVE_BRIDGE_NULL);
				return;
			}

			//检查活跃
			if (pBridge->pUserConn != NULL)
			{
				pBridge->pUserConn->CheckAlive();
			}

			//下一个桥接
			it++;
		}
	}
}

/******************************************************************************
接受回调函数
******************************************************************************/
void CChannel::_callbackAccept(SOCKET socket
	, unsigned long ulRemoteIP, void * pThis)
{
	if (pThis == NULL)
	{
		return;
	}
	CChannel * pThis2 = (CChannel *)pThis;

	//处理接受
	pThis2->_ProcessAccept(socket);
}

/******************************************************************************
关闭所有桥接
******************************************************************************/
void CChannel::_CloseAllBridge(void)
{
	//遍历桥接表
	set<Channel::SBridge *>::iterator it = m_setBridge.begin();
	while (it != m_setBridge.end())
	{
		//获取桥接
		Channel::SBridge * pBridge = *it;
		if (pBridge == NULL)
		{
			_PutTip("", Channel::CLOSE_ALL_BRIDGE_BRIDGE_NULL);
			return;
		}

		//关闭用户连接
		if (pBridge->pUserConn != NULL)
		{
			pBridge->pUserConn->Close();
		}

		//下一个桥接
		it++;
	}
}

/******************************************************************************
处理接受
******************************************************************************/
void CChannel::_ProcessAccept(SOCKET socket)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//监听停止
	if (socket == INVALID_SOCKET)
	{
		return;
	}

	//已经关闭
	if (m_socket == INVALID_SOCKET)
	{
		return;
	}

	//新建桥接
	_NewBridge(socket);
}

/******************************************************************************
删除桥接
******************************************************************************/
void CChannel::_DelBridge(Channel::SBridge * pBridge)
{
	//删除桥接
	if (pBridge == NULL)
	{
		_PutTip("", Channel::DEL_BRIDGE_BRIDGE_NULL);
		return;
	}
	delete pBridge;

	//从桥接表中删除
	try
	{
		if (m_setBridge.erase(pBridge) != 1)
		{
			_PutTip("", Channel::DEL_BRIDGE_NOT_EXIST);
			return;
		}
	}
	catch (...)
	{
		_PutTip("", Channel::DEL_BRIDGE_ERASE_EXCEPTION);
		return;
	}
}

/******************************************************************************
新建桥接
******************************************************************************/
void CChannel::_NewBridge(SOCKET socket)
{
	//新建桥接
	Channel::SBridge * pBridge = new(nothrow) Channel::SBridge;
	if (pBridge == NULL)
	{
		_PutTip("", Channel::NEW_BRIDGE_NEW_FAIL);
		return;
	}
	pBridge->pDstConn = NULL;

	//新建用户连接
	pBridge->pUserConn = new(nothrow) CUserConn(m_pfnPutTip, m_pThis);
	if (pBridge->pUserConn == NULL)
	{
		_PutTip("", Channel::NEW_BRIDGE_NEW_USER_CONN_FAIL);
		return;
	}

	//初始化用户连接
	if (m_pMainWnd == NULL
		|| socket == INVALID_SOCKET)
	{
		_PutTip("", Channel::NEW_BRIDGE_INIT_USER_CONN_PARAM);
		return;
	}
	pBridge->pUserConn->Init(m_pMainWnd, this, pBridge, socket);

	//添加到桥接表
	pair<set<Channel::SBridge *>::iterator, bool> pr
		= m_setBridge.insert(pBridge);
	if (!pr.second)
	{
		_PutTip("", Channel::NEW_BRIDGE_INSERT_SET_FAIL);
		return;
	}
}

/******************************************************************************
输出提示
******************************************************************************/
void CChannel::_PutTip(string strTip, int iErr, string strErr)
{
	//输出提示回调
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "Channel", iErr, strErr, m_pThis);
}