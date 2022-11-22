#include "StdAfx.h"

#include "TCP.h"
#include "../Lock/AutoLock.h"

CTCP::CTCP(TCP::PFNPutTip pfnPutTip, void * pThis)
	: m_SocketBuf(pfnPutTip, pThis)
	, m_ContextBuf(pfnPutTip, pThis)
	, m_BridgeCtrl(pfnPutTip, pThis)
{
	m_pfnPutTip = pfnPutTip;
	m_pThis = pThis;
}

CTCP::~CTCP(void)
{
}

/******************************************************************************
初始化
******************************************************************************/
void CTCP::Init(void)
{
	//初始化套接字缓冲区
	m_SocketBuf.Init();
	//初始化上下文缓冲区
	m_ContextBuf.Init();

	//初始化桥接控制
	m_BridgeCtrl.Init();

	m_hIOCP = NULL;

	m_uiThreadCnt = 0;
	m_pThreadSem = NULL;

	m_pfnAcceptEx = NULL;
	m_pfnConnectEx = NULL;
	m_pfnGetAcceptExSockaddrs = NULL;

	//启动并创建IOCP
	_StartupCreateIOCP();
	//启动IOCP线程
	_StartupIOCPThread();
	//加载扩展函数
	_LoadExFunc();
}

/******************************************************************************
绑定
******************************************************************************/
bool CTCP::Bind(SOCKET & socket, unsigned short usLocalPort)
{
	//绑定
	return Bind(socket, INADDR_ANY, usLocalPort);
}

/******************************************************************************
绑定
******************************************************************************/
bool CTCP::Bind(SOCKET & socket, unsigned long ulLocalIP
	, unsigned short usLocalPort)
{
	//创建套接字
	if (usLocalPort <= 0
		|| usLocalPort >= 65535)
	{
		_PutTip("", TCP::BIND_LOCAL_PORT_INVALID);
		return false;
	}
	socket = m_SocketBuf.Create(ulLocalIP, usLocalPort);
	if (socket == INVALID_SOCKET)
	{
		return false;
	}

	return true;
}

/******************************************************************************
监听
******************************************************************************/
void CTCP::Listen(SOCKET socket, ContextBuf::PFNAccept pfnAccept, void * pThis)
{
	//关联套接字
	if (m_hIOCP == NULL
		|| socket == INVALID_SOCKET)
	{
		_PutTip("", TCP::LISTEN_ASSOCIATE_SOCKET_PARAM);
		return;
	}
	if (CreateIoCompletionPort((HANDLE)socket, m_hIOCP, socket, 0) != m_hIOCP)
	{
		_PutTip("", TCP::LISTEN_ASSOCIATE_SOCKET_FAIL);
		return;
	}

	//监听套接字
	if (listen(socket, SOMAXCONN) != 0)
	{
		_PutTip("", TCP::LISTEN_LISTEN_FAIL);
		return;
	}

	//分配接受上下文
	if (pfnAccept == NULL
		|| pThis == NULL)
	{
		_PutTip("", TCP::LISTEN_ALLOC_PARAM);
		return;
	}
	ContextBuf::SAcceptContext * pAcceptContext =
		m_ContextBuf.AllocAccept(pfnAccept, pThis);

	//设置套接字有效
	_SetSocketValid(socket, pAcceptContext);
	//投递接受请求
	_PostAccept(socket, pAcceptContext);
}

/******************************************************************************
接收
******************************************************************************/
void CTCP::Recv(SOCKET socket, ContextBuf::PFNRecv pfnRecv, void * pThis)
{
	//分配接收上下文
	if (pfnRecv == NULL
		|| pThis == NULL)
	{
		_PutTip("", TCP::RECV_ALLOC_PARAM);
		return;
	}
	ContextBuf::SRecvContext * pRecvContext =
		m_ContextBuf.AllocRecv(pfnRecv, pThis);

	//设置套接字有效
	_SetSocketValid(socket, pRecvContext);
	//投递接收请求
	_PostRecv(socket, pRecvContext);
}

/******************************************************************************
连接
******************************************************************************/
void CTCP::Conn(SOCKET & socket
	, unsigned long ulRemoteIP, unsigned short usRemotePort
	, ContextBuf::PFNConn pfnConn, void * pThis)
{
	//连接
	Conn(socket, INADDR_ANY, ulRemoteIP, usRemotePort, pfnConn, pThis);
}

/******************************************************************************
连接
******************************************************************************/
void CTCP::Conn(SOCKET & socket, unsigned long ulLocalIP
	, unsigned long ulRemoteIP, unsigned short usRemotePort
	, ContextBuf::PFNConn pfnConn, void * pThis)
{
	//创建套接字
	socket = m_SocketBuf.Create(ulLocalIP, 0);

	//关联套接字
	if (socket == INVALID_SOCKET
		|| m_hIOCP == NULL)
	{
		_PutTip("", TCP::CONN_ASSOCIATE_SOCKET_PARAM);
		return;
	}
	if (CreateIoCompletionPort((HANDLE)socket, m_hIOCP, socket, 0) != m_hIOCP)
	{
		_PutTip("", TCP::CONN_ASSOCIATE_SOCKET_FAIL);
		return;
	}

	//填写远程地址信息
	if (ulRemoteIP == 0
		|| usRemotePort <= 0
		|| usRemotePort >= 65535)
	{
		_PutTip("", TCP::CONN_REMOTE_ADDR_INVALID);
		return;
	}
	sockaddr_in addrRemote = {0};
	addrRemote.sin_family = AF_INET;
	addrRemote.sin_addr.s_addr = ulRemoteIP;
	addrRemote.sin_port = htons(usRemotePort);

	//分配连接上下文
	if (pfnConn == NULL
		|| pThis == NULL)
	{
		_PutTip("", TCP::CONN_ALLOC_PARAM);
		return;
	}
	ContextBuf::SConnContext * pConnContext =
		m_ContextBuf.AllocConn(pfnConn, pThis);

	//投递连接请求
	_PostConn(socket, addrRemote, pConnContext);
}

/******************************************************************************
发送
******************************************************************************/
void CTCP::Send(SOCKET socket, void * pData, unsigned int uiDataSize)
{
	//分配发送上下文
	if (pData == NULL
		|| uiDataSize <= 0)
	{
		_PutTip("", TCP::SEND_ALLOC_PARAM);
		return;
	}
	ContextBuf::SSendContext * pSendContext =
		m_ContextBuf.AllocSend(pData, uiDataSize);

	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//投递发送请求
	if (!_PostSend(socket, pSendContext))
	{
		return;
	}

	//限制接收
	if (socket == INVALID_SOCKET
		|| pSendContext == NULL)
	{
		_PutTip("", TCP::SEND_LIMIT_RECV_PARAM);
		return;
	}
	m_BridgeCtrl.LimitRecv(socket, pSendContext);
}

/******************************************************************************
桥接
******************************************************************************/
void CTCP::Bridge(SOCKET socket1, SOCKET socket2)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	if (socket1 == INVALID_SOCKET
		|| socket2 == INVALID_SOCKET)
	{
		_PutTip("", TCP::BRIDGE_SOCKET_INVALID);
		return;
	}
	m_BridgeCtrl.AddBridge(socket1, socket2);
}

/******************************************************************************
关闭
******************************************************************************/
void CTCP::Close(SOCKET socket)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//从套接字有效表中删除
	if (socket == INVALID_SOCKET)
	{
		_PutTip("", TCP::CLOSE_SOCKET_INVALID);
		return;
	}
	try
	{
		m_mapSocketValid.erase(socket);
	}
	catch (...)
	{
		_PutTip("", TCP::CLOSE_ERASE_EXCEPTION);
		return;
	}

	//删除桥接
	m_BridgeCtrl.DelBridge(socket);
	//关闭套接字
	m_SocketBuf.Close(socket);
}

/******************************************************************************
停止
******************************************************************************/
void CTCP::Stop(void)
{
	//退出IOCP线程
	_ExitIOCPThread();
	//清空套接字缓冲区
	m_SocketBuf.Clear();
	//清空上下文缓冲区
	m_ContextBuf.Clear();
	//清理网络
	_CleanupNet();
}

/******************************************************************************
启动并创建IOCP
******************************************************************************/
void CTCP::_StartupCreateIOCP(void)
{
	//启动套接字
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		_PutTip("", TCP::STARTUP_CREATE_IOCP_STARTUP_FAIL);
		return;
	}

	//判断套接字版本
	if (LOBYTE(wsaData.wVersion) != 2
		|| HIBYTE(wsaData.wVersion) != 2)
	{
		_PutTip("", TCP::STARTUP_CREATE_IOCP_VER_ERR);
		return;
	}

	//创建IOCP
	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_hIOCP == NULL)
	{
		_PutTip("", TCP::STARTUP_CREATE_IOCP_CREATE_FAIL);
		return;
	}
}

/******************************************************************************
启动IOCP线程
******************************************************************************/
void CTCP::_StartupIOCPThread(void)
{
	//获取CPU数量
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	//CPU数量的线程数
	m_uiThreadCnt = si.dwNumberOfProcessors;

	//新建线程信号量
	if (m_uiThreadCnt <= 0)
	{
		_PutTip("", TCP::STARTUP_IOCP_THREAD_THREAD_CNT_INVALID);
		return;
	}
	try
	{
		m_pThreadSem = new CSemaphore(m_uiThreadCnt, m_uiThreadCnt);
	}
	catch (...)
	{
		_PutTip("", TCP::STARTUP_IOCP_THREAD_NEW_SEM_EXCEPTION);
		return;
	}
	if (m_pThreadSem == NULL)
	{
		_PutTip("", TCP::STARTUP_IOCP_THREAD_NEW_SEM_NULL);
		return;
	}

	//启动线程
	for (int i = 0; i < (int)m_uiThreadCnt; i++)
	{
		//线程信号量加锁
		if (!m_pThreadSem->Lock())
		{
			_PutTip("", TCP::STARTUP_IOCP_THREAD_SEM_LOCK_FAIL);
			return;
		}
		//启动IOCP工作线程
		CWinThread * pThread = AfxBeginThread(_threadIOCPWork, this);
		if (pThread == NULL)
		{
			_PutTip("", TCP::STARTUP_IOCP_THREAD_BEGIN_FAIL);
			return;
		}
	}
}

/******************************************************************************
加载扩展函数
******************************************************************************/
void CTCP::_LoadExFunc(void)
{
	//临时套接字
	SOCKET sTemp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sTemp == INVALID_SOCKET)
	{
		_PutTip("", TCP::LOAD_EX_FUNC_SOCKET_INVALID);
		return;
	}

	//加载接受函数
	GUID guidAccept = WSAID_ACCEPTEX;
	m_pfnAcceptEx = NULL;
	DWORD dw = 0;
	if (WSAIoctl(sTemp, SIO_GET_EXTENSION_FUNCTION_POINTER
		, &guidAccept, sizeof(guidAccept)
		, &m_pfnAcceptEx, sizeof(m_pfnAcceptEx), &dw, NULL, NULL) != 0)
	{
		_PutTip("", TCP::LOAD_EX_FUNC_GET_ACCEPT_FAIL);
		return;
	}

	//加载连接函数
	GUID guidConn = WSAID_CONNECTEX;
	m_pfnConnectEx = NULL;
	dw = 0;
	if (WSAIoctl(sTemp, SIO_GET_EXTENSION_FUNCTION_POINTER
		, &guidConn, sizeof(guidConn)
		, &m_pfnConnectEx, sizeof(m_pfnConnectEx), &dw, NULL, NULL) != 0)
	{
		_PutTip("", TCP::LOAD_EX_FUNC_GET_CONNECT_FAIL);
		return;
	}

	//加载获取地址函数
	GUID guidGetAddr = WSAID_GETACCEPTEXSOCKADDRS;
	m_pfnGetAcceptExSockaddrs = NULL;
	dw = 0;
	if (WSAIoctl(sTemp, SIO_GET_EXTENSION_FUNCTION_POINTER
		, &guidGetAddr, sizeof(guidGetAddr), &m_pfnGetAcceptExSockaddrs
		, sizeof(m_pfnGetAcceptExSockaddrs), &dw, NULL, NULL) != 0)
	{
		_PutTip("", TCP::LOAD_EX_FUNC_GET_GET_ADDR_FAIL);
		return;
	}

	//关闭套接字
	if (closesocket(sTemp) != 0)
	{
		_PutTip("", TCP::LOAD_EX_FUNC_CLOSE_SOCKET_FAIL);
		return;
	}
}

/******************************************************************************
设置套接字有效
******************************************************************************/
void CTCP::_SetSocketValid(SOCKET socket, ContextBuf::SContext * pContext)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	if (socket == INVALID_SOCKET
		|| pContext == NULL)
	{
		_PutTip("", TCP::SET_SOCKET_VALID_INSERT_PARAM);
		return;
	}
	pair<map<SOCKET, ContextBuf::SContext *>::iterator, bool> pr
		= m_mapSocketValid.insert(make_pair(socket, pContext));
	if (!pr.second)
	{
		_PutTip("", TCP::SET_SOCKET_VALID_INSERT_FAIL);
		return;
	}
}

/******************************************************************************
投递接受请求
******************************************************************************/
void CTCP::_PostAccept(SOCKET socket
	, ContextBuf::SAcceptContext * pAcceptContext)
{
	//创建套接字
	if (pAcceptContext == NULL)
	{
		_PutTip("", TCP::POST_ACCEPT_CONTEXT_NULL);
		return;
	}
	pAcceptContext->socket = m_SocketBuf.Create();

	//关联套接字
	if (pAcceptContext->socket == INVALID_SOCKET
		|| m_hIOCP == NULL)
	{
		_PutTip("", TCP::POST_ACCEPT_ASSOCIATE_SOCKET_PARAM);
		return;
	}
	if (CreateIoCompletionPort((HANDLE)pAcceptContext->socket, m_hIOCP
		, pAcceptContext->socket, 0) != m_hIOCP)
	{
		_PutTip("", TCP::POST_ACCEPT_ASSOCIATE_SOCKET_FAIL);
		return;
	}

	//投递接受请求
	if (m_pfnAcceptEx == NULL
		|| socket == INVALID_SOCKET
		|| pAcceptContext == NULL
		|| pAcceptContext->socket == INVALID_SOCKET
		|| pAcceptContext->pBuf == NULL)
	{
		_PutTip("", TCP::POST_ACCEPT_ACCEPT_PARAM);
		return;
	}
	if (!m_pfnAcceptEx(socket, pAcceptContext->socket, pAcceptContext->pBuf, 0
		, 0, sizeof(sockaddr_in) + 16, NULL, &pAcceptContext->overlap))
	{
		//获取错误码
		int iErr = WSAGetLastError();
		if (iErr != ERROR_IO_PENDING)
		{
			string strErr = "";
			//把错误码接入错误串
			char acBuf[12] = {0};
			_itoa_s(iErr, acBuf, sizeof(acBuf), 10);
			strErr += acBuf;
			strErr += ".";
			//输出提示
			_PutTip("", TCP::POST_ACCEPT_ACCEPT_FAIL, strErr);
			return;
		}
	}
}

/******************************************************************************
投递接收请求
******************************************************************************/
void CTCP::_PostRecv(SOCKET socket, ContextBuf::SRecvContext * pRecvContext)
{
	//投递接收请求
	if (socket == INVALID_SOCKET
		|| pRecvContext == NULL
		|| pRecvContext->buf.buf == NULL
		|| pRecvContext->buf.len <= 0)
	{
		_PutTip("", TCP::POST_RECV_RECV_PARAM);
		return;
	}
	DWORD dwFlags = 0;
	if (WSARecv(socket, &pRecvContext->buf, 1
		, NULL, &dwFlags, &pRecvContext->overlap, NULL) != 0)
	{
		//获取错误码
		int iErr = WSAGetLastError();
		//已经断开
		if (iErr == WSAECONNRESET
			|| iErr == WSAECONNABORTED)
		{
			//投递断开通知
			_PostDiscon(pRecvContext);
			return;
		}
		if (iErr != WSA_IO_PENDING)
		{
			string strErr = "";
			//把错误码接入错误串
			char acBuf[12] = {0};
			_itoa_s(iErr, acBuf, sizeof(acBuf), 10);
			strErr += acBuf;
			strErr += ".";
			//输出提示
			_PutTip("", TCP::POST_RECV_RECV_FAIL, strErr);
			return;
		}
	}
}

/******************************************************************************
投递连接请求
******************************************************************************/
void CTCP::_PostConn(SOCKET socket, sockaddr_in & addrRemote
	, ContextBuf::SConnContext * pConnContext)
{
	//投递连接请求
	if (m_pfnConnectEx == NULL
		|| socket == INVALID_SOCKET
		|| addrRemote.sin_addr.s_addr == 0
		|| addrRemote.sin_port == 0
		|| pConnContext == NULL)
	{
		_PutTip("", TCP::POST_CONN_CONN_PARAM);
		return;
	}
	if (!m_pfnConnectEx(socket, (sockaddr *)&addrRemote, sizeof(addrRemote)
		, NULL, 0, NULL, &pConnContext->overlap))
	{
		//获取错误码
		int iErr = WSAGetLastError();
		if (iErr != ERROR_IO_PENDING)
		{
			string strErr = "";
			//把错误码接入错误串
			char acBuf[12] = {0};
			_itoa_s(iErr, acBuf, sizeof(acBuf), 10);
			strErr += acBuf;
			strErr += ".";
			//输出提示
			_PutTip("", TCP::POST_CONN_CONN_FAIL, strErr);
			return;
		}
	}
}

/******************************************************************************
投递发送请求
******************************************************************************/
bool CTCP::_PostSend(SOCKET socket, ContextBuf::SSendContext * pSendContext)
{
	if (socket == INVALID_SOCKET
		|| pSendContext == NULL
		|| pSendContext->buf.buf == NULL
		|| pSendContext->buf.len <= 0)
	{
		_PutTip("", TCP::POST_SEND_SEND_PARAM);
		return false;
	}
	if (WSASend(socket, &pSendContext->buf, 1
		, NULL, 0, &pSendContext->overlap, NULL) != 0)
	{
		//获取错误码
		int iErr = WSAGetLastError();
		//已经断开
		if (iErr == WSAECONNRESET
			|| iErr == WSAECONNABORTED)
		{
			//释放上下文
			m_ContextBuf.Free(pSendContext);
			return false;
		}
		if (iErr != WSA_IO_PENDING)
		{
			string strErr = "";
			//把错误码接入错误串
			char acBuf[12] = {0};
			_itoa_s(iErr, acBuf, sizeof(acBuf), 10);
			strErr += acBuf;
			strErr += ".";
			//输出提示
			_PutTip("", TCP::POST_SEND_SEND_FAIL, strErr);
			return false;
		}
	}

	return true;
}

/******************************************************************************
退出IOCP线程
******************************************************************************/
void CTCP::_ExitIOCPThread(void)
{
	//发送退出通知
	if (m_hIOCP == NULL)
	{
		_PutTip("", TCP::EXIT_IOCP_THREAD_IOCP_NULL);
		return;
	}
	for (int i = 0; i < (int)m_uiThreadCnt; i++)
	{
		if (!PostQueuedCompletionStatus(m_hIOCP, 0, INVALID_SOCKET, NULL))
		{
			_PutTip("", TCP::EXIT_IOCP_THREAD_POST_EXIT_MSG_FAIL);
			return;
		}
	}

	//线程信号量能锁尽，表示线程都结束了。
	if (m_pThreadSem == NULL)
	{
		_PutTip("", TCP::EXIT_IOCP_THREAD_THREAD_SEM_NULL);
		return;
	}
	for (int i = 0; i < (int)m_uiThreadCnt; i++)
	{
		if (!m_pThreadSem->Lock())
		{
			_PutTip("", TCP::EXIT_IOCP_THREAD_LOCK_FAIL);
			return;
		}
	}

	//删除信号量
	delete m_pThreadSem;
	m_pThreadSem = NULL;
}

/******************************************************************************
清理网络
******************************************************************************/
void CTCP::_CleanupNet(void)
{
	//清理套接字
	if (WSACleanup() != 0)
	{
		_PutTip("", TCP::CLEANUP_NET_CLEANUP_FAIL);
		return;
	}

	//关闭IOCP
	if (m_hIOCP == NULL)
	{
		_PutTip("", TCP::CLEANUP_NET_IOCP_NULL);
		return;
	}
	if (!CloseHandle(m_hIOCP))
	{
		_PutTip("", TCP::CLEANUP_NET_CLOSE_IOCP_FAIL);
		return;
	}
	m_hIOCP = NULL;
}

/******************************************************************************
IOCP工作线程
******************************************************************************/
UINT CTCP::_threadIOCPWork(LPVOID pParam)
{
	if (pParam == NULL)
	{
		return 0;
	}
	CTCP * pThis = (CTCP *)pParam;

	//IOCP工作
	pThis->_IOCPWork();
	//退出IOCP工作
	pThis->_ExitIOCPWork();

	return 0;
}

/******************************************************************************
IOCP工作
******************************************************************************/
void CTCP::_IOCPWork(void)
{
	while (true)
	{
		//获取IO通知
		DWORD dwErr = 0;
		BOOL bIOSucc = FALSE;
		DWORD dwBytes = 0;
		SOCKET socket = INVALID_SOCKET;
		ContextBuf::SContext * pContext = NULL;
		_GetIONotice(dwErr, bIOSucc, dwBytes, socket, pContext);

		//是否退出
		if (_IsExit(bIOSucc, dwBytes, socket, pContext))
		{
			return;
		}

		//是否接受取消
		if (_IsAcceptCancel(dwErr, bIOSucc, dwBytes, socket, pContext))
		{
			//处理接受取消
			_ProcessAcceptCancel(bIOSucc
				, (ContextBuf::SAcceptContext *)pContext);
			continue;
		}

		//是否断开
		if (_IsDiscon(dwErr, bIOSucc, dwBytes, socket, pContext))
		{
			//处理断开
			_ProcessDiscon((ContextBuf::SRecvContext *)pContext);
			continue;
		}

		//处理IO通知
		_ProcessIONotice(dwErr, bIOSucc, dwBytes, socket, pContext);
	}
}

/******************************************************************************
退出IOCP工作
******************************************************************************/
void CTCP::_ExitIOCPWork(void)
{
	//线程信号量解锁
	if (m_pThreadSem == NULL)
	{
		_PutTip("", TCP::EXIT_IOCP_WORK_THREAD_SEM_NULL);
		return;
	}
	if (!m_pThreadSem->Unlock())
	{
		_PutTip("", TCP::EXIT_IOCP_WORK_SEM_UNLOCK_FAIL);
		return;
	}
}

/******************************************************************************
获取IO通知
******************************************************************************/
void CTCP::_GetIONotice(DWORD & dwErr, BOOL & bIOSucc, DWORD & dwBytes
	, SOCKET & socket, ContextBuf::SContext * & pContext)
{
	if (m_hIOCP == NULL)
	{
		_PutTip("", TCP::GET_IO_NOTICE_IOCP_NULL);
		return;
	}
	bIOSucc = GetQueuedCompletionStatus(m_hIOCP, &dwBytes
		, (PULONG_PTR)&socket, (LPOVERLAPPED *)&pContext, INFINITE);
	if (!bIOSucc)
	{
		dwErr = GetLastError();
	}
}

/******************************************************************************
是否退出
******************************************************************************/
bool CTCP::_IsExit(BOOL bIOSucc, DWORD dwBytes, SOCKET socket
	, ContextBuf::SContext * pContext)
{
	if (bIOSucc
		&& dwBytes == 0
		&& socket == INVALID_SOCKET
		&& pContext == NULL)
	{
		return true;
	}

	return false;
}

/******************************************************************************
是否接受取消
******************************************************************************/
bool CTCP::_IsAcceptCancel(DWORD dwErr, BOOL bIOSucc, DWORD dwBytes
	, SOCKET socket, ContextBuf::SContext * pContext)
{
	//接受请求被终止
	if (dwErr == ERROR_OPERATION_ABORTED
		&& !bIOSucc
		&& dwBytes == 0
		&& socket != INVALID_SOCKET
		&& pContext != NULL
		&& pContext->eOperateType == ContextBuf::OPERATE_ACCEPT)
	{
		return true;
	}

	//接受取消通知
	if (bIOSucc
		&& dwBytes == 0
		&& socket == INVALID_SOCKET
		&& pContext != NULL
		&& pContext->eOperateType == ContextBuf::OPERATE_ACCEPT)
	{
		return true;
	}

	return false;
}

/******************************************************************************
处理接受取消
******************************************************************************/
void CTCP::_ProcessAcceptCancel(BOOL bIOSucc
	, ContextBuf::SAcceptContext * pAcceptContext)
{
	//监听结束回调
	if (pAcceptContext == NULL
		|| pAcceptContext->pfnAccept == NULL
		|| pAcceptContext->pThis == NULL)
	{
		_PutTip("", TCP::PROCESS_ACCEPT_CANCEL_CALLBACK_PARAM);
		return;
	}
	pAcceptContext->pfnAccept(INVALID_SOCKET, 0, pAcceptContext->pThis);

	//接受请求被终止
	if (!bIOSucc)
	{
		//关闭接受上下文里面的套接字
		Close(pAcceptContext->socket);
	}

	//释放上下文
	m_ContextBuf.Free(pAcceptContext);
}

/******************************************************************************
是否断开
******************************************************************************/
bool CTCP::_IsDiscon(DWORD dwErr, BOOL bIOSucc, DWORD dwBytes, SOCKET socket
	, ContextBuf::SContext * pContext)
{
	//正常断开
	if (bIOSucc
		&& dwBytes == 0
		&& socket != INVALID_SOCKET
		&& pContext != NULL
		&& pContext->eOperateType == ContextBuf::OPERATE_RECV)
	{
		return true;
	}

	//异常断开
	if (!bIOSucc
		&& dwBytes == 0
		&& socket != INVALID_SOCKET
		&& pContext != NULL
		&& pContext->eOperateType == ContextBuf::OPERATE_RECV)
	{
		if (dwErr == ERROR_CONNECTION_ABORTED
			|| dwErr == ERROR_NETNAME_DELETED
			|| dwErr == ERROR_SEM_TIMEOUT)
		{
			return true;
		}
	}

	//断开通知
	if (bIOSucc
		&& dwBytes == 0
		&& socket == INVALID_SOCKET
		&& pContext != NULL
		&& pContext->eOperateType == ContextBuf::OPERATE_RECV)
	{
		return true;
	}

	return false;
}

/******************************************************************************
处理断开
******************************************************************************/
void CTCP::_ProcessDiscon(ContextBuf::SRecvContext * pRecvContext)
{
	//连接断开回调
	if (pRecvContext == NULL
		|| pRecvContext->pfnRecv == NULL
		|| pRecvContext->pThis == NULL)
	{
		_PutTip("", TCP::PROCESS_DISCON_CALLBACK_PARAM);
		return;
	}
	pRecvContext->pfnRecv(NULL, 0, pRecvContext->pThis);

	//释放上下文
	m_ContextBuf.Free(pRecvContext);
}

/******************************************************************************
处理IO通知
******************************************************************************/
void CTCP::_ProcessIONotice(DWORD dwErr, BOOL bIOSucc, DWORD dwBytes
	, SOCKET socket, ContextBuf::SContext * pContext)
{
	//判断操作类型
	if (pContext == NULL)
	{
		_PutTip("", TCP::PROCESS_IO_NOTICE_CONTEXT_NULL);
		return;
	}
	switch (pContext->eOperateType)
	{
	//接受通知
	case ContextBuf::OPERATE_ACCEPT:
		{
			//处理接受通知
			_ProcessAcceptNotice(dwErr, bIOSucc, socket
				, (ContextBuf::SAcceptContext *)pContext);
		}
		break;

	//连接通知
	case ContextBuf::OPERATE_CONN:
		{
			//处理连接通知
			_ProcessConnNotice(dwErr, bIOSucc
				, (ContextBuf::SConnContext *)pContext);
		}
		break;

	//发送通知
	case ContextBuf::OPERATE_SEND:
		{
			//处理发送通知
			_ProcessSendNotice(dwErr, bIOSucc, dwBytes, socket
				, (ContextBuf::SSendContext *)pContext);
		}
		break;

	//接收通知
	case ContextBuf::OPERATE_RECV:
		{
			//处理接收通知
			_ProcessRecvNotice(dwErr, bIOSucc, dwBytes, socket
				, (ContextBuf::SRecvContext *)pContext);
		}
		break;

	default:
		{
			_PutTip("", TCP::PROCESS_IO_NOTICE_UNKNOWN_OPERATE);
			return;
		}
		break;
	}
}

/******************************************************************************
处理接受通知
******************************************************************************/
void CTCP::_ProcessAcceptNotice(DWORD dwErr, BOOL bIOSucc, SOCKET socket
	, ContextBuf::SAcceptContext * pAcceptContext)
{
	//接受失败
	if (!bIOSucc)
	{
		string strErr = "";
		//把错误码接入错误串
		char acBuf[12] = {0};
		_itoa_s(dwErr, acBuf, sizeof(acBuf), 10);
		strErr += acBuf;
		strErr += ".";
		//输出提示
		_PutTip("", TCP::PROCESS_ACCEPT_NOTICE_ACCEPT_FAIL, strErr);
		return;
	}

	//获取地址
	if (m_pfnGetAcceptExSockaddrs == NULL
		|| pAcceptContext == NULL
		|| pAcceptContext->pBuf == NULL)
	{
		_PutTip("", TCP::PROCESS_ACCEPT_NOTICE_GET_ADDR_PARAM);
		return;
	}
	sockaddr_in * pRemoteAddr = NULL;
	int iRemoteAddrLen = 0;
	m_pfnGetAcceptExSockaddrs(pAcceptContext->pBuf, 0, 0
		, sizeof(sockaddr_in) + 16, NULL, NULL
		, (sockaddr **)&pRemoteAddr, &iRemoteAddrLen);

	//接受回调
	if (pAcceptContext->pfnAccept == NULL
		|| pAcceptContext->socket == INVALID_SOCKET
		|| pRemoteAddr == NULL
		|| pRemoteAddr->sin_addr.s_addr == 0
		|| pAcceptContext->pThis == NULL)
	{
		_PutTip("", TCP::PROCESS_ACCEPT_NOTICE_CALLBACK_PARAM);
		return;
	}
	pAcceptContext->pfnAccept(pAcceptContext->socket
		, pRemoteAddr->sin_addr.s_addr, pAcceptContext->pThis);

	//继续投递接受请求
	_PostAcceptContinue(socket, pAcceptContext);
}

/******************************************************************************
处理连接通知
******************************************************************************/
void CTCP::_ProcessConnNotice(DWORD dwErr, BOOL bIOSucc
	, ContextBuf::SConnContext * pConnContext)
{
	//连接回调
	if (pConnContext == NULL
		|| pConnContext->pfnConn == NULL
		|| pConnContext->pThis == NULL)
	{
		_PutTip("", TCP::PROCESS_CONN_NOTICE_CALLBACK_PARAM);
		return;
	}
	bool bSucc = true;
	string strErr = "";
	if (!bIOSucc)
	{
		bSucc = false;
		//把错误码接入错误串
		char acBuf[12] = {0};
		_itoa_s(dwErr, acBuf, sizeof(acBuf), 10);
		strErr += acBuf;
		strErr += ".";
	}
	pConnContext->pfnConn(bSucc, strErr, pConnContext->pThis);

	//释放上下文
	m_ContextBuf.Free(pConnContext);
}

/******************************************************************************
处理发送通知
******************************************************************************/
void CTCP::_ProcessSendNotice(DWORD dwErr, BOOL bIOSucc, DWORD dwBytes
	, SOCKET socket, ContextBuf::SSendContext * pSendContext)
{
	//发送失败
	if (!bIOSucc
		&& dwErr != ERROR_NETNAME_DELETED
		&& dwErr != ERROR_OPERATION_ABORTED)
	{
		string strErr = "";
		//把错误码接入错误串
		char acBuf[12] = {0};
		_itoa_s(dwErr, acBuf, sizeof(acBuf), 10);
		strErr += acBuf;
		strErr += ".";
		//输出提示
		_PutTip("", TCP::PROCESS_SEND_NOTICE_SEND_FAIL, strErr);
		return;
	}

	//发送成功
	if (pSendContext == NULL)
	{
		_PutTip("", TCP::PROCESS_SEND_NOTICE_CONTEXT_NULL);
		return;
	}
	if (bIOSucc)
	{
		//发送长度是否正确
		if (dwBytes != pSendContext->buf.len)
		{
			_PutTip("", TCP::PROCESS_SEND_NOTICE_SEND_LEN_INVALID);
			return;
		}
	}

	//恢复接收
	_ResumeRecv(socket, pSendContext);
	//释放上下文
	m_ContextBuf.Free(pSendContext);
}

/******************************************************************************
处理接收通知
******************************************************************************/
void CTCP::_ProcessRecvNotice(DWORD dwErr, BOOL bIOSucc, DWORD dwBytes
	, SOCKET socket, ContextBuf::SRecvContext * pRecvContext)
{
	//接收失败
	if (!bIOSucc)
	{
		string strErr = "";
		//把错误码接入错误串
		char acBuf[12] = {0};
		_itoa_s(dwErr, acBuf, sizeof(acBuf), 10);
		strErr += acBuf;
		strErr += ".";
		//输出提示
		_PutTip("", TCP::PROCESS_RECV_NOTICE_RECV_FAIL, strErr);
		return;
	}

	//接收回调
	if (pRecvContext == NULL
		|| pRecvContext->pfnRecv == NULL
		|| pRecvContext->buf.buf == NULL
		|| pRecvContext->buf.len < dwBytes
		|| dwBytes <= 0
		|| pRecvContext->pThis == NULL)
	{
		_PutTip("", TCP::PROCESS_RECV_NOTICE_CALLBACK_PARAM);
		return;
	}
	pRecvContext->pfnRecv(pRecvContext->buf.buf, dwBytes, pRecvContext->pThis);

	//继续投递接收请求
	_PostRecvContinue(socket, pRecvContext);
}

/******************************************************************************
继续投递接受请求
******************************************************************************/
void CTCP::_PostAcceptContinue(SOCKET socket
	, ContextBuf::SAcceptContext * pAcceptContext)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//套接字是否有效
	if (_IsSocketValid(socket, pAcceptContext))
	{
		//投递接受请求
		_PostAccept(socket, pAcceptContext);
	}
	else
	{
		//投递接受取消通知
		_PostAcceptCancel(pAcceptContext);
	}
}

/******************************************************************************
恢复接收
******************************************************************************/
void CTCP::_ResumeRecv(SOCKET sSend, ContextBuf::SSendContext * pSendContext)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//恢复接收
	SOCKET sRecv = INVALID_SOCKET;
	ContextBuf::SRecvContext * pRecvContext = NULL;
	if (sSend == INVALID_SOCKET
		|| pSendContext == NULL)
	{
		_PutTip("", TCP::RESUME_RECV_RESUME_PARAM);
		return;
	}
	if (!m_BridgeCtrl.ResumeRecv(sRecv, pRecvContext, sSend, pSendContext))
	{
		return;
	}

	//继续投递接收请求
	_PostRecvContinue(sRecv, pRecvContext);
}

/******************************************************************************
继续投递接收请求
******************************************************************************/
void CTCP::_PostRecvContinue(SOCKET socket
	, ContextBuf::SRecvContext * pRecvContext)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//套接字是否有效
	if (_IsSocketValid(socket, pRecvContext))
	{
		//暂停接收
		if (socket == INVALID_SOCKET
			|| pRecvContext == NULL)
		{
			_PutTip("", TCP::POST_RECV_CONTINUE_PAUSE_PARAM);
			return;
		}
		if (!m_BridgeCtrl.PauseRecv(socket, pRecvContext))
		{
			//投递接收请求
			_PostRecv(socket, pRecvContext);
		}
	}
	else
	{
		//投递断开通知
		_PostDiscon(pRecvContext);
	}
}

/******************************************************************************
套接字是否有效
******************************************************************************/
bool CTCP::_IsSocketValid(SOCKET socket, ContextBuf::SContext * pContext)
{
	//套接字已经关闭
	if (socket == INVALID_SOCKET)
	{
		_PutTip("", TCP::IS_SOCKET_VALID_SOCKET_INVALID);
		return false;
	}
	map<SOCKET, ContextBuf::SContext *>::iterator it =
		m_mapSocketValid.find(socket);
	if (it == m_mapSocketValid.end())
	{
		return false;
	}

	//套接字已经被重用
	if (pContext == NULL)
	{
		_PutTip("", TCP::IS_SOCKET_VALID_CONTEXT_NULL);
		return false;
	}
	if (it->second != pContext)
	{
		return false;
	}

	return true;
}

/******************************************************************************
投递接受取消通知
******************************************************************************/
void CTCP::_PostAcceptCancel(ContextBuf::SAcceptContext * pAcceptContext)
{
	if (m_hIOCP == NULL
		|| pAcceptContext == NULL)
	{
		_PutTip("", TCP::POST_ACCEPT_CANCEL_POST_PARAM);
		return;
	}
	if (!PostQueuedCompletionStatus(m_hIOCP, 0, INVALID_SOCKET
		, &pAcceptContext->overlap))
	{
		_PutTip("", TCP::POST_ACCEPT_CANCEL_POST_FAIL);
		return;
	}
}

/******************************************************************************
投递断开通知
******************************************************************************/
void CTCP::_PostDiscon(ContextBuf::SRecvContext * pRecvContext)
{
	if (m_hIOCP == NULL
		|| pRecvContext == NULL)
	{
		_PutTip("", TCP::POST_DISCON_POST_PARAM);
		return;
	}
	if (!PostQueuedCompletionStatus(m_hIOCP, 0, INVALID_SOCKET
		, &pRecvContext->overlap))
	{
		_PutTip("", TCP::POST_DISCON_POST_FAIL);
		return;
	}
}

/******************************************************************************
输出提示
******************************************************************************/
void CTCP::_PutTip(string strTip, int iErr, string strErr)
{
	//输出提示回调
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "TCP", iErr, strErr, m_pThis);
}