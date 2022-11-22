#include "StdAfx.h"

#include "SocketBuf.h"
#include "../Lock/AutoLock.h"

CSocketBuf::CSocketBuf(SocketBuf::PFNPutTip pfnPutTip, void * pThis)
{
	m_pfnPutTip = pfnPutTip;
	m_pThis = pThis;
}

CSocketBuf::~CSocketBuf(void)
{
}

/******************************************************************************
��ʼ��
******************************************************************************/
void CSocketBuf::Init(void)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	m_setSocket.clear();
}

/******************************************************************************
����
******************************************************************************/
SOCKET CSocketBuf::Create(void)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//�����׽���
	SOCKET socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0
		, WSA_FLAG_OVERLAPPED);
	if (socket == INVALID_SOCKET)
	{
		_PutTip("", SocketBuf::CREATE_CREATE_FAIL);
		return INVALID_SOCKET;
	}

	//�����׽��ֱ�
	pair<set<SOCKET>::iterator, bool> pr = m_setSocket.insert(socket);
	if (!pr.second)
	{
		_PutTip("", SocketBuf::CREATE_INSERT_SET_FAIL);
		return INVALID_SOCKET;
	}

	return socket;
}

/******************************************************************************
����
******************************************************************************/
SOCKET CSocketBuf::Create(unsigned long ulLocalIP, unsigned short usLocalPort)
{
	//����
	SOCKET socket = Create();

	//��д���ص�ַ��Ϣ
	if (usLocalPort < 0
		|| usLocalPort >= 65535)
	{
		_PutTip("", SocketBuf::CREATE_ADDR_INVALID);
		return INVALID_SOCKET;
	}
	sockaddr_in addrLocal = {0};
	addrLocal.sin_family = AF_INET;
	addrLocal.sin_addr.s_addr = ulLocalIP;
	addrLocal.sin_port = htons(usLocalPort);

	//���׽���
	if (socket == INVALID_SOCKET)
	{
		_PutTip("", SocketBuf::CREATE_SOCKET_INVALID);
		return INVALID_SOCKET;
	}
	if (bind(socket ,(SOCKADDR *)&addrLocal, sizeof(addrLocal)) != 0)
	{
		if (WSAGetLastError() != WSAEADDRINUSE)
		{
			_PutTip("", SocketBuf::CREATE_BIND_FAIL);
			return INVALID_SOCKET;
		}

		//�ر�
		Close(socket);
		return INVALID_SOCKET;
	}

	return socket;
}

/******************************************************************************
�ر�
******************************************************************************/
void CSocketBuf::Close(SOCKET socket)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//���׽��ֱ���ɾ��
	if (socket == INVALID_SOCKET)
	{
		_PutTip("", SocketBuf::CLOSE_SOCKET_INVALID);
		return;
	}
	try
	{
		if (m_setSocket.erase(socket) != 1)
		{
			_PutTip("", SocketBuf::CLOSE_SOCKET_NOT_EXIST);
			return;
		}
	}
	catch (...)
	{
		_PutTip("", SocketBuf::CLOSE_ERASE_EXCEPTION);
		return;
	}

	//�ر��׽���
	if (closesocket(socket) != 0)
	{
		_PutTip("", SocketBuf::CLOSE_CLOSE_SOCKET_FAIL);
		return;
	}
}

/******************************************************************************
���
******************************************************************************/
void CSocketBuf::Clear(void)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//�����׽��ֱ�
	set<SOCKET>::iterator it = m_setSocket.begin();
	while (it != m_setSocket.end())
	{
		//��ȡ�׽���
		SOCKET socket = *it;
		if (socket == INVALID_SOCKET)
		{
			_PutTip("", SocketBuf::CLEAR_SOCKET_INVALID);
			return;
		}

		//���׽��ֱ���ɾ��
		try
		{
			it = m_setSocket.erase(it);
		}
		catch (...)
		{
			_PutTip("", SocketBuf::CLEAR_ERASE_EXCEPTION);
			return;
		}

		//�ر��׽���
		if (closesocket(socket) != 0)
		{
			_PutTip("", SocketBuf::CLEAR_CLOSE_SOCKET_FAIL);
			return;
		}
	}
}

/******************************************************************************
�����ʾ
******************************************************************************/
void CSocketBuf::_PutTip(string strTip, int iErr, string strErr)
{
	//�����ʾ�ص�
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "SocketBuf", iErr, strErr, m_pThis);
}