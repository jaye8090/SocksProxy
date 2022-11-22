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
��ʼ��
******************************************************************************/
void CTCP::Init(void)
{
	//��ʼ���׽��ֻ�����
	m_SocketBuf.Init();
	//��ʼ�������Ļ�����
	m_ContextBuf.Init();

	//��ʼ���Žӿ���
	m_BridgeCtrl.Init();

	m_hIOCP = NULL;

	m_uiThreadCnt = 0;
	m_pThreadSem = NULL;

	m_pfnAcceptEx = NULL;
	m_pfnConnectEx = NULL;
	m_pfnGetAcceptExSockaddrs = NULL;

	//����������IOCP
	_StartupCreateIOCP();
	//����IOCP�߳�
	_StartupIOCPThread();
	//������չ����
	_LoadExFunc();
}

/******************************************************************************
��
******************************************************************************/
bool CTCP::Bind(SOCKET & socket, unsigned short usLocalPort)
{
	//��
	return Bind(socket, INADDR_ANY, usLocalPort);
}

/******************************************************************************
��
******************************************************************************/
bool CTCP::Bind(SOCKET & socket, unsigned long ulLocalIP
	, unsigned short usLocalPort)
{
	//�����׽���
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
����
******************************************************************************/
void CTCP::Listen(SOCKET socket, ContextBuf::PFNAccept pfnAccept, void * pThis)
{
	//�����׽���
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

	//�����׽���
	if (listen(socket, SOMAXCONN) != 0)
	{
		_PutTip("", TCP::LISTEN_LISTEN_FAIL);
		return;
	}

	//�������������
	if (pfnAccept == NULL
		|| pThis == NULL)
	{
		_PutTip("", TCP::LISTEN_ALLOC_PARAM);
		return;
	}
	ContextBuf::SAcceptContext * pAcceptContext =
		m_ContextBuf.AllocAccept(pfnAccept, pThis);

	//�����׽�����Ч
	_SetSocketValid(socket, pAcceptContext);
	//Ͷ�ݽ�������
	_PostAccept(socket, pAcceptContext);
}

/******************************************************************************
����
******************************************************************************/
void CTCP::Recv(SOCKET socket, ContextBuf::PFNRecv pfnRecv, void * pThis)
{
	//�������������
	if (pfnRecv == NULL
		|| pThis == NULL)
	{
		_PutTip("", TCP::RECV_ALLOC_PARAM);
		return;
	}
	ContextBuf::SRecvContext * pRecvContext =
		m_ContextBuf.AllocRecv(pfnRecv, pThis);

	//�����׽�����Ч
	_SetSocketValid(socket, pRecvContext);
	//Ͷ�ݽ�������
	_PostRecv(socket, pRecvContext);
}

/******************************************************************************
����
******************************************************************************/
void CTCP::Conn(SOCKET & socket
	, unsigned long ulRemoteIP, unsigned short usRemotePort
	, ContextBuf::PFNConn pfnConn, void * pThis)
{
	//����
	Conn(socket, INADDR_ANY, ulRemoteIP, usRemotePort, pfnConn, pThis);
}

/******************************************************************************
����
******************************************************************************/
void CTCP::Conn(SOCKET & socket, unsigned long ulLocalIP
	, unsigned long ulRemoteIP, unsigned short usRemotePort
	, ContextBuf::PFNConn pfnConn, void * pThis)
{
	//�����׽���
	socket = m_SocketBuf.Create(ulLocalIP, 0);

	//�����׽���
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

	//��дԶ�̵�ַ��Ϣ
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

	//��������������
	if (pfnConn == NULL
		|| pThis == NULL)
	{
		_PutTip("", TCP::CONN_ALLOC_PARAM);
		return;
	}
	ContextBuf::SConnContext * pConnContext =
		m_ContextBuf.AllocConn(pfnConn, pThis);

	//Ͷ����������
	_PostConn(socket, addrRemote, pConnContext);
}

/******************************************************************************
����
******************************************************************************/
void CTCP::Send(SOCKET socket, void * pData, unsigned int uiDataSize)
{
	//���䷢��������
	if (pData == NULL
		|| uiDataSize <= 0)
	{
		_PutTip("", TCP::SEND_ALLOC_PARAM);
		return;
	}
	ContextBuf::SSendContext * pSendContext =
		m_ContextBuf.AllocSend(pData, uiDataSize);

	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//Ͷ�ݷ�������
	if (!_PostSend(socket, pSendContext))
	{
		return;
	}

	//���ƽ���
	if (socket == INVALID_SOCKET
		|| pSendContext == NULL)
	{
		_PutTip("", TCP::SEND_LIMIT_RECV_PARAM);
		return;
	}
	m_BridgeCtrl.LimitRecv(socket, pSendContext);
}

/******************************************************************************
�Ž�
******************************************************************************/
void CTCP::Bridge(SOCKET socket1, SOCKET socket2)
{
	//������
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
�ر�
******************************************************************************/
void CTCP::Close(SOCKET socket)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//���׽�����Ч����ɾ��
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

	//ɾ���Ž�
	m_BridgeCtrl.DelBridge(socket);
	//�ر��׽���
	m_SocketBuf.Close(socket);
}

/******************************************************************************
ֹͣ
******************************************************************************/
void CTCP::Stop(void)
{
	//�˳�IOCP�߳�
	_ExitIOCPThread();
	//����׽��ֻ�����
	m_SocketBuf.Clear();
	//��������Ļ�����
	m_ContextBuf.Clear();
	//��������
	_CleanupNet();
}

/******************************************************************************
����������IOCP
******************************************************************************/
void CTCP::_StartupCreateIOCP(void)
{
	//�����׽���
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		_PutTip("", TCP::STARTUP_CREATE_IOCP_STARTUP_FAIL);
		return;
	}

	//�ж��׽��ְ汾
	if (LOBYTE(wsaData.wVersion) != 2
		|| HIBYTE(wsaData.wVersion) != 2)
	{
		_PutTip("", TCP::STARTUP_CREATE_IOCP_VER_ERR);
		return;
	}

	//����IOCP
	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_hIOCP == NULL)
	{
		_PutTip("", TCP::STARTUP_CREATE_IOCP_CREATE_FAIL);
		return;
	}
}

/******************************************************************************
����IOCP�߳�
******************************************************************************/
void CTCP::_StartupIOCPThread(void)
{
	//��ȡCPU����
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	//CPU�������߳���
	m_uiThreadCnt = si.dwNumberOfProcessors;

	//�½��߳��ź���
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

	//�����߳�
	for (int i = 0; i < (int)m_uiThreadCnt; i++)
	{
		//�߳��ź�������
		if (!m_pThreadSem->Lock())
		{
			_PutTip("", TCP::STARTUP_IOCP_THREAD_SEM_LOCK_FAIL);
			return;
		}
		//����IOCP�����߳�
		CWinThread * pThread = AfxBeginThread(_threadIOCPWork, this);
		if (pThread == NULL)
		{
			_PutTip("", TCP::STARTUP_IOCP_THREAD_BEGIN_FAIL);
			return;
		}
	}
}

/******************************************************************************
������չ����
******************************************************************************/
void CTCP::_LoadExFunc(void)
{
	//��ʱ�׽���
	SOCKET sTemp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sTemp == INVALID_SOCKET)
	{
		_PutTip("", TCP::LOAD_EX_FUNC_SOCKET_INVALID);
		return;
	}

	//���ؽ��ܺ���
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

	//�������Ӻ���
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

	//���ػ�ȡ��ַ����
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

	//�ر��׽���
	if (closesocket(sTemp) != 0)
	{
		_PutTip("", TCP::LOAD_EX_FUNC_CLOSE_SOCKET_FAIL);
		return;
	}
}

/******************************************************************************
�����׽�����Ч
******************************************************************************/
void CTCP::_SetSocketValid(SOCKET socket, ContextBuf::SContext * pContext)
{
	//������
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
Ͷ�ݽ�������
******************************************************************************/
void CTCP::_PostAccept(SOCKET socket
	, ContextBuf::SAcceptContext * pAcceptContext)
{
	//�����׽���
	if (pAcceptContext == NULL)
	{
		_PutTip("", TCP::POST_ACCEPT_CONTEXT_NULL);
		return;
	}
	pAcceptContext->socket = m_SocketBuf.Create();

	//�����׽���
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

	//Ͷ�ݽ�������
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
		//��ȡ������
		int iErr = WSAGetLastError();
		if (iErr != ERROR_IO_PENDING)
		{
			string strErr = "";
			//�Ѵ�����������
			char acBuf[12] = {0};
			_itoa_s(iErr, acBuf, sizeof(acBuf), 10);
			strErr += acBuf;
			strErr += ".";
			//�����ʾ
			_PutTip("", TCP::POST_ACCEPT_ACCEPT_FAIL, strErr);
			return;
		}
	}
}

/******************************************************************************
Ͷ�ݽ�������
******************************************************************************/
void CTCP::_PostRecv(SOCKET socket, ContextBuf::SRecvContext * pRecvContext)
{
	//Ͷ�ݽ�������
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
		//��ȡ������
		int iErr = WSAGetLastError();
		//�Ѿ��Ͽ�
		if (iErr == WSAECONNRESET
			|| iErr == WSAECONNABORTED)
		{
			//Ͷ�ݶϿ�֪ͨ
			_PostDiscon(pRecvContext);
			return;
		}
		if (iErr != WSA_IO_PENDING)
		{
			string strErr = "";
			//�Ѵ�����������
			char acBuf[12] = {0};
			_itoa_s(iErr, acBuf, sizeof(acBuf), 10);
			strErr += acBuf;
			strErr += ".";
			//�����ʾ
			_PutTip("", TCP::POST_RECV_RECV_FAIL, strErr);
			return;
		}
	}
}

/******************************************************************************
Ͷ����������
******************************************************************************/
void CTCP::_PostConn(SOCKET socket, sockaddr_in & addrRemote
	, ContextBuf::SConnContext * pConnContext)
{
	//Ͷ����������
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
		//��ȡ������
		int iErr = WSAGetLastError();
		if (iErr != ERROR_IO_PENDING)
		{
			string strErr = "";
			//�Ѵ�����������
			char acBuf[12] = {0};
			_itoa_s(iErr, acBuf, sizeof(acBuf), 10);
			strErr += acBuf;
			strErr += ".";
			//�����ʾ
			_PutTip("", TCP::POST_CONN_CONN_FAIL, strErr);
			return;
		}
	}
}

/******************************************************************************
Ͷ�ݷ�������
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
		//��ȡ������
		int iErr = WSAGetLastError();
		//�Ѿ��Ͽ�
		if (iErr == WSAECONNRESET
			|| iErr == WSAECONNABORTED)
		{
			//�ͷ�������
			m_ContextBuf.Free(pSendContext);
			return false;
		}
		if (iErr != WSA_IO_PENDING)
		{
			string strErr = "";
			//�Ѵ�����������
			char acBuf[12] = {0};
			_itoa_s(iErr, acBuf, sizeof(acBuf), 10);
			strErr += acBuf;
			strErr += ".";
			//�����ʾ
			_PutTip("", TCP::POST_SEND_SEND_FAIL, strErr);
			return false;
		}
	}

	return true;
}

/******************************************************************************
�˳�IOCP�߳�
******************************************************************************/
void CTCP::_ExitIOCPThread(void)
{
	//�����˳�֪ͨ
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

	//�߳��ź�������������ʾ�̶߳������ˡ�
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

	//ɾ���ź���
	delete m_pThreadSem;
	m_pThreadSem = NULL;
}

/******************************************************************************
��������
******************************************************************************/
void CTCP::_CleanupNet(void)
{
	//�����׽���
	if (WSACleanup() != 0)
	{
		_PutTip("", TCP::CLEANUP_NET_CLEANUP_FAIL);
		return;
	}

	//�ر�IOCP
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
IOCP�����߳�
******************************************************************************/
UINT CTCP::_threadIOCPWork(LPVOID pParam)
{
	if (pParam == NULL)
	{
		return 0;
	}
	CTCP * pThis = (CTCP *)pParam;

	//IOCP����
	pThis->_IOCPWork();
	//�˳�IOCP����
	pThis->_ExitIOCPWork();

	return 0;
}

/******************************************************************************
IOCP����
******************************************************************************/
void CTCP::_IOCPWork(void)
{
	while (true)
	{
		//��ȡIO֪ͨ
		DWORD dwErr = 0;
		BOOL bIOSucc = FALSE;
		DWORD dwBytes = 0;
		SOCKET socket = INVALID_SOCKET;
		ContextBuf::SContext * pContext = NULL;
		_GetIONotice(dwErr, bIOSucc, dwBytes, socket, pContext);

		//�Ƿ��˳�
		if (_IsExit(bIOSucc, dwBytes, socket, pContext))
		{
			return;
		}

		//�Ƿ����ȡ��
		if (_IsAcceptCancel(dwErr, bIOSucc, dwBytes, socket, pContext))
		{
			//�������ȡ��
			_ProcessAcceptCancel(bIOSucc
				, (ContextBuf::SAcceptContext *)pContext);
			continue;
		}

		//�Ƿ�Ͽ�
		if (_IsDiscon(dwErr, bIOSucc, dwBytes, socket, pContext))
		{
			//����Ͽ�
			_ProcessDiscon((ContextBuf::SRecvContext *)pContext);
			continue;
		}

		//����IO֪ͨ
		_ProcessIONotice(dwErr, bIOSucc, dwBytes, socket, pContext);
	}
}

/******************************************************************************
�˳�IOCP����
******************************************************************************/
void CTCP::_ExitIOCPWork(void)
{
	//�߳��ź�������
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
��ȡIO֪ͨ
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
�Ƿ��˳�
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
�Ƿ����ȡ��
******************************************************************************/
bool CTCP::_IsAcceptCancel(DWORD dwErr, BOOL bIOSucc, DWORD dwBytes
	, SOCKET socket, ContextBuf::SContext * pContext)
{
	//����������ֹ
	if (dwErr == ERROR_OPERATION_ABORTED
		&& !bIOSucc
		&& dwBytes == 0
		&& socket != INVALID_SOCKET
		&& pContext != NULL
		&& pContext->eOperateType == ContextBuf::OPERATE_ACCEPT)
	{
		return true;
	}

	//����ȡ��֪ͨ
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
�������ȡ��
******************************************************************************/
void CTCP::_ProcessAcceptCancel(BOOL bIOSucc
	, ContextBuf::SAcceptContext * pAcceptContext)
{
	//���������ص�
	if (pAcceptContext == NULL
		|| pAcceptContext->pfnAccept == NULL
		|| pAcceptContext->pThis == NULL)
	{
		_PutTip("", TCP::PROCESS_ACCEPT_CANCEL_CALLBACK_PARAM);
		return;
	}
	pAcceptContext->pfnAccept(INVALID_SOCKET, 0, pAcceptContext->pThis);

	//����������ֹ
	if (!bIOSucc)
	{
		//�رս���������������׽���
		Close(pAcceptContext->socket);
	}

	//�ͷ�������
	m_ContextBuf.Free(pAcceptContext);
}

/******************************************************************************
�Ƿ�Ͽ�
******************************************************************************/
bool CTCP::_IsDiscon(DWORD dwErr, BOOL bIOSucc, DWORD dwBytes, SOCKET socket
	, ContextBuf::SContext * pContext)
{
	//�����Ͽ�
	if (bIOSucc
		&& dwBytes == 0
		&& socket != INVALID_SOCKET
		&& pContext != NULL
		&& pContext->eOperateType == ContextBuf::OPERATE_RECV)
	{
		return true;
	}

	//�쳣�Ͽ�
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

	//�Ͽ�֪ͨ
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
����Ͽ�
******************************************************************************/
void CTCP::_ProcessDiscon(ContextBuf::SRecvContext * pRecvContext)
{
	//���ӶϿ��ص�
	if (pRecvContext == NULL
		|| pRecvContext->pfnRecv == NULL
		|| pRecvContext->pThis == NULL)
	{
		_PutTip("", TCP::PROCESS_DISCON_CALLBACK_PARAM);
		return;
	}
	pRecvContext->pfnRecv(NULL, 0, pRecvContext->pThis);

	//�ͷ�������
	m_ContextBuf.Free(pRecvContext);
}

/******************************************************************************
����IO֪ͨ
******************************************************************************/
void CTCP::_ProcessIONotice(DWORD dwErr, BOOL bIOSucc, DWORD dwBytes
	, SOCKET socket, ContextBuf::SContext * pContext)
{
	//�жϲ�������
	if (pContext == NULL)
	{
		_PutTip("", TCP::PROCESS_IO_NOTICE_CONTEXT_NULL);
		return;
	}
	switch (pContext->eOperateType)
	{
	//����֪ͨ
	case ContextBuf::OPERATE_ACCEPT:
		{
			//�������֪ͨ
			_ProcessAcceptNotice(dwErr, bIOSucc, socket
				, (ContextBuf::SAcceptContext *)pContext);
		}
		break;

	//����֪ͨ
	case ContextBuf::OPERATE_CONN:
		{
			//��������֪ͨ
			_ProcessConnNotice(dwErr, bIOSucc
				, (ContextBuf::SConnContext *)pContext);
		}
		break;

	//����֪ͨ
	case ContextBuf::OPERATE_SEND:
		{
			//������֪ͨ
			_ProcessSendNotice(dwErr, bIOSucc, dwBytes, socket
				, (ContextBuf::SSendContext *)pContext);
		}
		break;

	//����֪ͨ
	case ContextBuf::OPERATE_RECV:
		{
			//�������֪ͨ
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
�������֪ͨ
******************************************************************************/
void CTCP::_ProcessAcceptNotice(DWORD dwErr, BOOL bIOSucc, SOCKET socket
	, ContextBuf::SAcceptContext * pAcceptContext)
{
	//����ʧ��
	if (!bIOSucc)
	{
		string strErr = "";
		//�Ѵ�����������
		char acBuf[12] = {0};
		_itoa_s(dwErr, acBuf, sizeof(acBuf), 10);
		strErr += acBuf;
		strErr += ".";
		//�����ʾ
		_PutTip("", TCP::PROCESS_ACCEPT_NOTICE_ACCEPT_FAIL, strErr);
		return;
	}

	//��ȡ��ַ
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

	//���ܻص�
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

	//����Ͷ�ݽ�������
	_PostAcceptContinue(socket, pAcceptContext);
}

/******************************************************************************
��������֪ͨ
******************************************************************************/
void CTCP::_ProcessConnNotice(DWORD dwErr, BOOL bIOSucc
	, ContextBuf::SConnContext * pConnContext)
{
	//���ӻص�
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
		//�Ѵ�����������
		char acBuf[12] = {0};
		_itoa_s(dwErr, acBuf, sizeof(acBuf), 10);
		strErr += acBuf;
		strErr += ".";
	}
	pConnContext->pfnConn(bSucc, strErr, pConnContext->pThis);

	//�ͷ�������
	m_ContextBuf.Free(pConnContext);
}

/******************************************************************************
������֪ͨ
******************************************************************************/
void CTCP::_ProcessSendNotice(DWORD dwErr, BOOL bIOSucc, DWORD dwBytes
	, SOCKET socket, ContextBuf::SSendContext * pSendContext)
{
	//����ʧ��
	if (!bIOSucc
		&& dwErr != ERROR_NETNAME_DELETED
		&& dwErr != ERROR_OPERATION_ABORTED)
	{
		string strErr = "";
		//�Ѵ�����������
		char acBuf[12] = {0};
		_itoa_s(dwErr, acBuf, sizeof(acBuf), 10);
		strErr += acBuf;
		strErr += ".";
		//�����ʾ
		_PutTip("", TCP::PROCESS_SEND_NOTICE_SEND_FAIL, strErr);
		return;
	}

	//���ͳɹ�
	if (pSendContext == NULL)
	{
		_PutTip("", TCP::PROCESS_SEND_NOTICE_CONTEXT_NULL);
		return;
	}
	if (bIOSucc)
	{
		//���ͳ����Ƿ���ȷ
		if (dwBytes != pSendContext->buf.len)
		{
			_PutTip("", TCP::PROCESS_SEND_NOTICE_SEND_LEN_INVALID);
			return;
		}
	}

	//�ָ�����
	_ResumeRecv(socket, pSendContext);
	//�ͷ�������
	m_ContextBuf.Free(pSendContext);
}

/******************************************************************************
�������֪ͨ
******************************************************************************/
void CTCP::_ProcessRecvNotice(DWORD dwErr, BOOL bIOSucc, DWORD dwBytes
	, SOCKET socket, ContextBuf::SRecvContext * pRecvContext)
{
	//����ʧ��
	if (!bIOSucc)
	{
		string strErr = "";
		//�Ѵ�����������
		char acBuf[12] = {0};
		_itoa_s(dwErr, acBuf, sizeof(acBuf), 10);
		strErr += acBuf;
		strErr += ".";
		//�����ʾ
		_PutTip("", TCP::PROCESS_RECV_NOTICE_RECV_FAIL, strErr);
		return;
	}

	//���ջص�
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

	//����Ͷ�ݽ�������
	_PostRecvContinue(socket, pRecvContext);
}

/******************************************************************************
����Ͷ�ݽ�������
******************************************************************************/
void CTCP::_PostAcceptContinue(SOCKET socket
	, ContextBuf::SAcceptContext * pAcceptContext)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//�׽����Ƿ���Ч
	if (_IsSocketValid(socket, pAcceptContext))
	{
		//Ͷ�ݽ�������
		_PostAccept(socket, pAcceptContext);
	}
	else
	{
		//Ͷ�ݽ���ȡ��֪ͨ
		_PostAcceptCancel(pAcceptContext);
	}
}

/******************************************************************************
�ָ�����
******************************************************************************/
void CTCP::_ResumeRecv(SOCKET sSend, ContextBuf::SSendContext * pSendContext)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//�ָ�����
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

	//����Ͷ�ݽ�������
	_PostRecvContinue(sRecv, pRecvContext);
}

/******************************************************************************
����Ͷ�ݽ�������
******************************************************************************/
void CTCP::_PostRecvContinue(SOCKET socket
	, ContextBuf::SRecvContext * pRecvContext)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//�׽����Ƿ���Ч
	if (_IsSocketValid(socket, pRecvContext))
	{
		//��ͣ����
		if (socket == INVALID_SOCKET
			|| pRecvContext == NULL)
		{
			_PutTip("", TCP::POST_RECV_CONTINUE_PAUSE_PARAM);
			return;
		}
		if (!m_BridgeCtrl.PauseRecv(socket, pRecvContext))
		{
			//Ͷ�ݽ�������
			_PostRecv(socket, pRecvContext);
		}
	}
	else
	{
		//Ͷ�ݶϿ�֪ͨ
		_PostDiscon(pRecvContext);
	}
}

/******************************************************************************
�׽����Ƿ���Ч
******************************************************************************/
bool CTCP::_IsSocketValid(SOCKET socket, ContextBuf::SContext * pContext)
{
	//�׽����Ѿ��ر�
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

	//�׽����Ѿ�������
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
Ͷ�ݽ���ȡ��֪ͨ
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
Ͷ�ݶϿ�֪ͨ
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
�����ʾ
******************************************************************************/
void CTCP::_PutTip(string strTip, int iErr, string strErr)
{
	//�����ʾ�ص�
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "TCP", iErr, strErr, m_pThis);
}