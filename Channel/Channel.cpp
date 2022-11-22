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
��ʼ��
******************************************************************************/
bool CChannel::Init(CSocksProxyDlg * pMainWnd, unsigned short usPort)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	m_pMainWnd = pMainWnd;

	m_socket = INVALID_SOCKET;

	m_setBridge.clear();

	//�󶨶˿�
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

	//�����׽���
	if (m_socket == INVALID_SOCKET)
	{
		_PutTip("", Channel::INIT_SOCKET_INVALID);
		return false;
	}
	m_pMainWnd->m_TCP.Listen(m_socket, _callbackAccept, this);

	//ȡ���˳��¼�
	if (!m_eventExit.ResetEvent())
	{
		_PutTip("", Channel::INIT_RESET_EVENT_FAIL);
		return false;
	}

	//��������Ծ�߳�
	if (AfxBeginThread(_threadCheckAlive, this) == NULL)
	{
		_PutTip("", Channel::INIT_BEGIN_THREAD_FAIL);
		return false;
	}

	return true;
}

/******************************************************************************
ɾ���û�����
******************************************************************************/
void CChannel::DelUserConn(Channel::SBridge * pBridge)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//Ŀ�����Ӵ���
	if (pBridge == NULL
		|| pBridge->pUserConn == NULL)
	{
		_PutTip("", Channel::DEL_USER_CONN_BRIDGE_PARAM);
		return;
	}
	if (pBridge->pDstConn != NULL)
	{
		//�ر�Ŀ������
		pBridge->pDstConn->Close();

		//ɾ���û�����
		delete pBridge->pUserConn;
		pBridge->pUserConn = NULL;
	}
	//Ŀ������Ϊ��
	else
	{
		//ɾ���û�����
		delete pBridge->pUserConn;
		pBridge->pUserConn = NULL;

		//ɾ���Ž�
		_DelBridge(pBridge);
	}
}

/******************************************************************************
ɾ��Ŀ������
******************************************************************************/
void CChannel::DelDstConn(Channel::SBridge * pBridge)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//�û����Ӵ���
	if (pBridge == NULL
		|| pBridge->pDstConn == NULL)
	{
		_PutTip("", Channel::DEL_DST_CONN_BRIDGE_PARAM);
		return;
	}
	if (pBridge->pUserConn != NULL)
	{
		//�ر��û�����
		pBridge->pUserConn->Close();

		//ɾ��Ŀ������
		delete pBridge->pDstConn;
		pBridge->pDstConn = NULL;
	}
	//�û�����Ϊ��
	else
	{
		//ɾ��Ŀ������
		delete pBridge->pDstConn;
		pBridge->pDstConn = NULL;

		//ɾ���Ž�
		_DelBridge(pBridge);
	}
}

/******************************************************************************
��ȡ������
******************************************************************************/
unsigned int CChannel::GetConnCnt(void)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	return m_setBridge.size();
}

/******************************************************************************
�ر�
******************************************************************************/
void CChannel::Close(void)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//�Ѿ��ر�
	if (m_socket == INVALID_SOCKET)
	{
		return;
	}

	//�ر��׽��֣�ֹͣ�����˿ڡ�
	if (m_pMainWnd == NULL)
	{
		_PutTip("", Channel::CLOSE_CLOSE_PARAM);
		return;
	}
	m_pMainWnd->m_TCP.Close(m_socket);
	m_socket = INVALID_SOCKET;

	//�ر������Ž�
	_CloseAllBridge();

	//����ֹͣ�¼�
	if (!m_eventStop.SetEvent())
	{
		_PutTip("", Channel::CLOSE_SET_EVENT_FAIL);
		return;
	}
}

/******************************************************************************
�Ƿ��Ѿ��ر�
******************************************************************************/
bool CChannel::IsClose(void)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	if (m_setBridge.empty()
		&& m_eventExit.Lock(0))
	{
		return true;
	}

	return false;
}

/******************************************************************************
����Ծ�߳�
******************************************************************************/
UINT CChannel::_threadCheckAlive(LPVOID pParam)
{
	if (pParam == NULL)
	{
		return 0;
	}
	CChannel * pThis = (CChannel *)pParam;

	//����Ծ
	pThis->_CheckAlive();

	//�����˳��¼�
	if (!pThis->m_eventExit.SetEvent())
	{
		pThis->_PutTip("", Channel::THREAD_CHECK_ALIVE_SET_EVENT_FAIL);
		return 0;
	}

	return 0;
}

/******************************************************************************
����Ծ
******************************************************************************/
void CChannel::_CheckAlive(void)
{
	//ѭ�����
	while (true)
	{
		//���ʱ�䣬ֹͣ�¼��������˳�
		if (m_eventStop.Lock(Channel::CHECK_ALIVE_SPACE_TIME))
		{
			return;
		}

		//������
		CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

		//�Ѿ��ر�
		if (m_socket == INVALID_SOCKET)
		{
			return;
		}

		//�����Žӱ�
		set<Channel::SBridge *>::iterator it = m_setBridge.begin();
		while (it != m_setBridge.end())
		{
			//��ȡ�Ž�
			Channel::SBridge * pBridge = *it;
			if (pBridge == NULL)
			{
				_PutTip("", Channel::CHECK_ALIVE_BRIDGE_NULL);
				return;
			}

			//����Ծ
			if (pBridge->pUserConn != NULL)
			{
				pBridge->pUserConn->CheckAlive();
			}

			//��һ���Ž�
			it++;
		}
	}
}

/******************************************************************************
���ܻص�����
******************************************************************************/
void CChannel::_callbackAccept(SOCKET socket
	, unsigned long ulRemoteIP, void * pThis)
{
	if (pThis == NULL)
	{
		return;
	}
	CChannel * pThis2 = (CChannel *)pThis;

	//�������
	pThis2->_ProcessAccept(socket);
}

/******************************************************************************
�ر������Ž�
******************************************************************************/
void CChannel::_CloseAllBridge(void)
{
	//�����Žӱ�
	set<Channel::SBridge *>::iterator it = m_setBridge.begin();
	while (it != m_setBridge.end())
	{
		//��ȡ�Ž�
		Channel::SBridge * pBridge = *it;
		if (pBridge == NULL)
		{
			_PutTip("", Channel::CLOSE_ALL_BRIDGE_BRIDGE_NULL);
			return;
		}

		//�ر��û�����
		if (pBridge->pUserConn != NULL)
		{
			pBridge->pUserConn->Close();
		}

		//��һ���Ž�
		it++;
	}
}

/******************************************************************************
�������
******************************************************************************/
void CChannel::_ProcessAccept(SOCKET socket)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//����ֹͣ
	if (socket == INVALID_SOCKET)
	{
		return;
	}

	//�Ѿ��ر�
	if (m_socket == INVALID_SOCKET)
	{
		return;
	}

	//�½��Ž�
	_NewBridge(socket);
}

/******************************************************************************
ɾ���Ž�
******************************************************************************/
void CChannel::_DelBridge(Channel::SBridge * pBridge)
{
	//ɾ���Ž�
	if (pBridge == NULL)
	{
		_PutTip("", Channel::DEL_BRIDGE_BRIDGE_NULL);
		return;
	}
	delete pBridge;

	//���Žӱ���ɾ��
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
�½��Ž�
******************************************************************************/
void CChannel::_NewBridge(SOCKET socket)
{
	//�½��Ž�
	Channel::SBridge * pBridge = new(nothrow) Channel::SBridge;
	if (pBridge == NULL)
	{
		_PutTip("", Channel::NEW_BRIDGE_NEW_FAIL);
		return;
	}
	pBridge->pDstConn = NULL;

	//�½��û�����
	pBridge->pUserConn = new(nothrow) CUserConn(m_pfnPutTip, m_pThis);
	if (pBridge->pUserConn == NULL)
	{
		_PutTip("", Channel::NEW_BRIDGE_NEW_USER_CONN_FAIL);
		return;
	}

	//��ʼ���û�����
	if (m_pMainWnd == NULL
		|| socket == INVALID_SOCKET)
	{
		_PutTip("", Channel::NEW_BRIDGE_INIT_USER_CONN_PARAM);
		return;
	}
	pBridge->pUserConn->Init(m_pMainWnd, this, pBridge, socket);

	//��ӵ��Žӱ�
	pair<set<Channel::SBridge *>::iterator, bool> pr
		= m_setBridge.insert(pBridge);
	if (!pr.second)
	{
		_PutTip("", Channel::NEW_BRIDGE_INSERT_SET_FAIL);
		return;
	}
}

/******************************************************************************
�����ʾ
******************************************************************************/
void CChannel::_PutTip(string strTip, int iErr, string strErr)
{
	//�����ʾ�ص�
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "Channel", iErr, strErr, m_pThis);
}