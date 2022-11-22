#include "StdAfx.h"

#include "DstConn.h"
#include "../SocksProxyDlg.h"
#include "../Lock/AutoLock.h"
#include "../DataProcess/DataPack.h"
#include "Channel.h"
#include "UserConn.h"

CDstConn::CDstConn(DstConn::PFNPutTip pfnPutTip, void * pThis)
{
	m_pfnPutTip = pfnPutTip;
	m_pThis = pThis;
}

CDstConn::~CDstConn(void)
{
}

/******************************************************************************
��ʼ��
******************************************************************************/
void CDstConn::Init(CSocksProxyDlg * pMainWnd, CChannel * pChannel
	, Channel::SBridge * pBridge, SOCKET sUserConn
	, unsigned long ulDstIP, unsigned short usDstPort)
{
	//������
	if (pBridge == NULL)
	{
		_PutTip("", DstConn::INIT_BRIDGE_NULL);
		return;
	}
	CAutoLock lock(&pBridge->csLock, m_pfnPutTip, m_pThis);

	m_pMainWnd = pMainWnd;

	m_pChannel = pChannel;
	m_pBridge = pBridge;

	m_socket = INVALID_SOCKET;

	//��������
	if (m_pMainWnd == NULL
		|| ulDstIP == 0
		|| usDstPort <= 0
		|| usDstPort >= 65535)
	{
		_PutTip("", DstConn::INIT_CONN_PARAM);
		return;
	}
	m_pMainWnd->m_TCP.Conn(m_socket, ulDstIP, usDstPort, _callbackConn, this);

	//�Ž�
	if (sUserConn == INVALID_SOCKET
		|| m_socket == INVALID_SOCKET)
	{
		_PutTip("", DstConn::INIT_BRIDGE_PARAM);
		return;
	}
	m_pMainWnd->m_TCP.Bridge(sUserConn, m_socket);
}

/******************************************************************************
ͨ�ŷ���
******************************************************************************/
void CDstConn::CommSend(void * pData, unsigned int uiDataSize)
{
	//������
	if (m_pBridge == NULL)
	{
		_PutTip("", DstConn::COMM_SEND_BRIDGE_NULL);
		return;
	}
	CAutoLock lock(&m_pBridge->csLock, m_pfnPutTip, m_pThis);

	//�Ѿ��ر�
	if (m_socket == INVALID_SOCKET)
	{
		return;
	}

	//��������
	_SendData(pData, uiDataSize);
}

/******************************************************************************
�ر�
******************************************************************************/
void CDstConn::Close(void)
{
	//������
	if (m_pBridge == NULL)
	{
		_PutTip("", DstConn::CLOSE_BRIDGE_NULL);
		return;
	}
	CAutoLock lock(&m_pBridge->csLock, m_pfnPutTip, m_pThis);

	if (m_pMainWnd == NULL)
	{
		_PutTip("", DstConn::CLOSE_MAIN_WND_NULL);
		return;
	}
	if (m_socket != INVALID_SOCKET)
	{
		m_pMainWnd->m_TCP.Close(m_socket);
		m_socket = INVALID_SOCKET;
	}
}

/******************************************************************************
���ӻص�����
******************************************************************************/
void CDstConn::_callbackConn(bool bSucc, string strErr, void * pThis)
{
	if (pThis == NULL)
	{
		return;
	}
	CDstConn * pThis2 = (CDstConn *)pThis;

	//��������
	bool bDelConn = false;
	pThis2->_ProcessConn(bDelConn, bSucc, strErr);

	//����ɾ������
	pThis2->_ProcessDelConn(bDelConn);
}

/******************************************************************************
���ջص�����
******************************************************************************/
void CDstConn::_callbackRecv(void * pData, unsigned int uiDataSize
	, void * pThis)
{
	if (pThis == NULL)
	{
		return;
	}
	CDstConn * pThis2 = (CDstConn *)pThis;

	//�������
	bool bDelConn = false;
	pThis2->_ProcessRecv(bDelConn, pData, uiDataSize);

	//����ɾ������
	pThis2->_ProcessDelConn(bDelConn);
}

/******************************************************************************
��������
******************************************************************************/
void CDstConn::_SendData(void * pData, unsigned int uiDataSize)
{
	//��������
	if (m_pMainWnd == NULL
		|| m_socket == INVALID_SOCKET
		|| pData == NULL
		|| uiDataSize <= 0)
	{
		_PutTip("", DstConn::SEND_DATA_SEND_PARAM);
		return;
	}
	m_pMainWnd->m_TCP.Send(m_socket, pData, uiDataSize);

	//����������������
	m_pMainWnd->m_NetMonitor.AddData(uiDataSize);
}

/******************************************************************************
��������
******************************************************************************/
void CDstConn::_ProcessConn(bool & bDelConn, bool bSucc, string strErr)
{
	//������
	if (m_pBridge == NULL)
	{
		_PutTip("", DstConn::PROCESS_CONN_BRIDGE_NULL);
		return;
	}
	CAutoLock lock(&m_pBridge->csLock, m_pfnPutTip, m_pThis);

	//���ӳɹ�
	if (bSucc)
	{
		//�Ѿ��ر�
		if (m_socket == INVALID_SOCKET)
		{
			//ɾ������
			bDelConn = true;
			return;
		}

		//��������
		if (m_pMainWnd == NULL)
		{
			_PutTip("", DstConn::PROCESS_CONN_MAIN_WND_NULL);
			return;
		}
		m_pMainWnd->m_TCP.Recv(m_socket, _callbackRecv, this);

		//������Ӧ
		if (m_pBridge->pUserConn == NULL)
		{
			_PutTip("", DstConn::PROCESS_CONN_USER_CONN_NULL);
			return;
		}
		m_pBridge->pUserConn->ConnResponse();
	}
	//����ʧ��
	else
	{
		//ɾ������
		bDelConn = true;
		//�ر�
		Close();
	}
}

/******************************************************************************
����ɾ������
******************************************************************************/
void CDstConn::_ProcessDelConn(bool bDelConn)
{
	//�Ƿ�ɾ������
	if (!bDelConn)
	{
		return;
	}

	//ɾ��Ŀ������
	if (m_pChannel == NULL
		|| m_pBridge == NULL)
	{
		_PutTip("", DstConn::PROCESS_DEL_CONN_DEL_PARAM);
		return;
	}
	m_pChannel->DelDstConn(m_pBridge);
}

/******************************************************************************
�������
******************************************************************************/
void CDstConn::_ProcessRecv(bool & bDelConn, void * pData
	, unsigned int uiDataSize)
{
	//������
	if (m_pBridge == NULL)
	{
		_PutTip("", DstConn::PROCESS_RECV_BRIDGE_NULL);
		return;
	}
	CAutoLock lock(&m_pBridge->csLock, m_pfnPutTip, m_pThis);

	//���ӶϿ�
	if (pData == NULL)
	{
		//ɾ������
		bDelConn = true;
		//�ر�
		Close();
		return;
	}

	//�Ѿ��ر�
	if (m_socket == INVALID_SOCKET)
	{
		return;
	}

	//ͨ�Ž���
	if (m_pBridge->pUserConn == NULL
		|| uiDataSize <= 0)
	{
		_PutTip("", DstConn::PROCESS_RECV_COMM_RECV_PARAM);
		return;
	}
	CDataPack pack(m_pfnPutTip, m_pThis);
	pack.Push(pData, uiDataSize);
	m_pBridge->pUserConn->CommRecv(pack);

	//����������������
	if (m_pMainWnd == NULL)
	{
		_PutTip("", DstConn::PROCESS_RECV_MAIN_WND_NULL);
		return;
	}
	m_pMainWnd->m_NetMonitor.AddData(uiDataSize);
}

/******************************************************************************
�����ʾ
******************************************************************************/
void CDstConn::_PutTip(string strTip, int iErr, string strErr)
{
	//�����ʾ�ص�
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "DstConn", iErr, strErr, m_pThis);
}