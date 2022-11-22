#include "StdAfx.h"

#include "UserConn.h"
#include "../SocksProxyDlg.h"
#include "../Lock/AutoLock.h"
#include "../DataProcess/DataPack.h"
#include "Channel.h"
#include "DstConn.h"

CUserConn::CUserConn(UserConn::PFNPutTip pfnPutTip, void * pThis)
	: m_UserPtl(pfnPutTip, pThis)
{
	m_pfnPutTip = pfnPutTip;
	m_pThis = pThis;
}

CUserConn::~CUserConn(void)
{
}

/******************************************************************************
��ʼ��
******************************************************************************/
void CUserConn::Init(CSocksProxyDlg * pMainWnd, CChannel * pChannel
	, Channel::SBridge * pBridge, SOCKET socket)
{
	//������
	if (pBridge == NULL)
	{
		_PutTip("", UserConn::INIT_BRIDGE_NULL);
		return;
	}
	CAutoLock lock(&pBridge->csLock, m_pfnPutTip, m_pThis);

	m_pMainWnd = pMainWnd;

	m_pChannel = pChannel;
	m_pBridge = pBridge;

	m_socket = socket;

	m_bAuthRequestDone = false;
	m_bConnRequestDone = false;

	m_ucSocksVer = 0;

	m_AliveTimeCnt.Begin();

	//��������
	if (m_pMainWnd == NULL
		|| m_socket == INVALID_SOCKET)
	{
		_PutTip("", UserConn::INIT_RECV_PARAM);
		return;
	}
	m_pMainWnd->m_TCP.Recv(m_socket, _callbackRecv, this);
}

/******************************************************************************
������Ӧ
******************************************************************************/
void CUserConn::ConnResponse(void)
{
	//������
	if (m_pBridge == NULL)
	{
		_PutTip("", UserConn::CONN_RESPONSE_BRIDGE_NULL);
		return;
	}
	CAutoLock lock(&m_pBridge->csLock, m_pfnPutTip, m_pThis);

	//�Ѿ��ر�
	if (m_socket == INVALID_SOCKET)
	{
		return;
	}

	//���������Ӧ��
	CDataPack pack(m_pfnPutTip, m_pThis);
	//SOCKS5
	if (m_ucSocksVer == 0x05)
	{
		m_UserPtl.PackSocks5ConnResponse(pack);
	}
	//SOCKS4
	else
	{
		m_UserPtl.PackSocks4ConnResponse(pack);
	}

	//��������
	_SendData(pack);
	//�����������
	m_bConnRequestDone = true;
}

/******************************************************************************
ͨ�Ž���
******************************************************************************/
void CUserConn::CommRecv(CDataPack & data)
{
	//������
	if (m_pBridge == NULL)
	{
		_PutTip("", UserConn::COMM_RECV_BRIDGE_NULL);
		return;
	}
	CAutoLock lock(&m_pBridge->csLock, m_pfnPutTip, m_pThis);

	//�Ѿ��ر�
	if (m_socket == INVALID_SOCKET)
	{
		return;
	}

	//��������
	_SendData(data);
}

/******************************************************************************
����Ծ
******************************************************************************/
void CUserConn::CheckAlive(void)
{
	//������
	if (m_pBridge == NULL)
	{
		_PutTip("", UserConn::CHECK_ALIVE_BRIDGE_NULL);
		return;
	}
	CAutoLock lock(&m_pBridge->csLock, m_pfnPutTip, m_pThis);

	//�Ƿ��Ѿ�����
	if (m_AliveTimeCnt.End() < UserConn::OFFLINE_TIME)
	{
		return;
	}

	//�ر�
	Close();
}

/******************************************************************************
�ر�
******************************************************************************/
void CUserConn::Close(void)
{
	//������
	if (m_pBridge == NULL)
	{
		_PutTip("", UserConn::CLOSE_BRIDGE_NULL);
		return;
	}
	CAutoLock lock(&m_pBridge->csLock, m_pfnPutTip, m_pThis);

	if (m_pMainWnd == NULL)
	{
		_PutTip("", UserConn::CLOSE_MAIN_WND_NULL);
		return;
	}
	if (m_socket != INVALID_SOCKET)
	{
		m_pMainWnd->m_TCP.Close(m_socket);
		m_socket = INVALID_SOCKET;
	}
}

/******************************************************************************
���ջص�����
******************************************************************************/
void CUserConn::_callbackRecv(void * pData, unsigned int uiDataSize
	, void * pThis)
{
	if (pThis == NULL)
	{
		return;
	}
	CUserConn * pThis2 = (CUserConn *)pThis;

	//�������
	bool bDelConn = false;
	pThis2->_ProcessRecv(bDelConn, pData, uiDataSize);

	//����ɾ������
	pThis2->_ProcessDelConn(bDelConn);
}

/******************************************************************************
��������
******************************************************************************/
void CUserConn::_SendData(CDataPack & data)
{
	//��ȡ����
	unsigned char * pData = NULL;
	unsigned int uiDataSize = 0;
	data.GetData(pData, uiDataSize);

	//��������
	if (m_pMainWnd == NULL
		|| m_socket == INVALID_SOCKET
		|| pData == NULL
		|| uiDataSize <= 0)
	{
		_PutTip("", UserConn::SEND_DATA_SEND_PARAM);
		return;
	}
	m_pMainWnd->m_TCP.Send(m_socket, pData, uiDataSize);

	//��Ծ��ʱˢ��
	m_AliveTimeCnt.Begin();
}

/******************************************************************************
�������
******************************************************************************/
void CUserConn::_ProcessRecv(bool & bDelConn, void * pData
	, unsigned int uiDataSize)
{
	//������
	if (m_pBridge == NULL)
	{
		_PutTip("", UserConn::PROCESS_RECV_BRIDGE_NULL);
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

	//��Ծ��ʱˢ��
	m_AliveTimeCnt.Begin();
	//����SOCKS����
	_ProcessSocksRequest(pData, uiDataSize);
}

/******************************************************************************
����ɾ������
******************************************************************************/
void CUserConn::_ProcessDelConn(bool bDelConn)
{
	//�Ƿ�ɾ������
	if (!bDelConn)
	{
		return;
	}

	//ɾ���û�����
	if (m_pChannel == NULL
		|| m_pBridge == NULL)
	{
		_PutTip("", UserConn::PROCESS_DEL_CONN_DEL_PARAM);
		return;
	}
	m_pChannel->DelUserConn(m_pBridge);
}

/******************************************************************************
����SOCKS����
******************************************************************************/
void CUserConn::_ProcessSocksRequest(void * pData, unsigned int uiDataSize)
{
	//��������û���
	if (!m_bConnRequestDone)
	{
		//��ȡSOCKS�汾
		if (!_GetSocksVer(pData, uiDataSize))
		{
			return;
		}

		//SOCKS5
		if (m_ucSocksVer == 0x05)
		{
			//��֤����û���
			if (!m_bAuthRequestDone)
			{
				//������֤����
				_ProcessAuthRequest(pData, uiDataSize);
			}
			//��֤�����Ѿ����
			else
			{
				//����SOCKS5��������
				_ProcessSocks5ConnRequest(pData, uiDataSize);
			}
		}
		//SOCKS4
		else
		{
			//����SOCKS4��������
			_ProcessSocks4ConnRequest(pData, uiDataSize);
		}
	}
	//���������Ѿ����
	else
	{
		//����ͨ�ŷ���
		_ProcessCommSend(pData, uiDataSize);
	}
}

/******************************************************************************
��ȡSOCKS�汾
******************************************************************************/
bool CUserConn::_GetSocksVer(void * pData, unsigned int uiDataSize)
{
	if (pData == NULL
		|| uiDataSize <= 0)
	{
		_PutTip("", UserConn::GET_SOCKS_VER_DATA_INVALID);
		return false;
	}
	m_ucSocksVer = m_UserPtl.GetSocksVer(pData, uiDataSize);
	if (m_ucSocksVer == 0)
	{
		//�ر�
		Close();
		return false;
	}

	return true;
}

/******************************************************************************
������֤����
******************************************************************************/
void CUserConn::_ProcessAuthRequest(void * pData, unsigned int uiDataSize)
{
	//������֤�����
	if (pData == NULL
		|| uiDataSize <= 0)
	{
		_PutTip("", UserConn::PROCESS_AUTH_REQUEST_DATA_INVALID);
		return;
	}
	if (!m_UserPtl.ParseAuthRequest(pData, uiDataSize))
	{
		//�ر�
		Close();
		return;
	}

	//�����֤��Ӧ��
	CDataPack pack(m_pfnPutTip, m_pThis);
	m_UserPtl.PackAuthResponse(pack);

	//��������
	_SendData(pack);
	//��֤�������
	m_bAuthRequestDone = true;
}

/******************************************************************************
����SOCKS5��������
******************************************************************************/
void CUserConn::_ProcessSocks5ConnRequest(void * pData
	, unsigned int uiDataSize)
{
	//����SOCKS5���������
	unsigned long ulDstIP = 0;
	unsigned short usDstPort = 0;
	if (pData == NULL
		|| uiDataSize <= 0)
	{
		_PutTip("", UserConn::PROCESS_SOCKS5_CONN_REQUEST_DATA_INVALID);
		return;
	}
	if (!m_UserPtl.ParseSocks5ConnRequest(ulDstIP, usDstPort, pData
		, uiDataSize))
	{
		//�ر�
		Close();
		return;
	}

	//�½���ʼ��Ŀ������
	_NewInitDstConn(ulDstIP, usDstPort);
}

/******************************************************************************
����SOCKS4��������
******************************************************************************/
void CUserConn::_ProcessSocks4ConnRequest(void * pData
	, unsigned int uiDataSize)
{
	//����SOCKS4���������
	unsigned long ulDstIP = 0;
	unsigned short usDstPort = 0;
	if (pData == NULL
		|| uiDataSize <= 0)
	{
		_PutTip("", UserConn::PROCESS_SOCKS4_CONN_REQUEST_DATA_INVALID);
		return;
	}
	if (!m_UserPtl.ParseSocks4ConnRequest(ulDstIP, usDstPort, pData
		, uiDataSize))
	{
		//�ر�
		Close();
		return;
	}

	//�½���ʼ��Ŀ������
	_NewInitDstConn(ulDstIP, usDstPort);
}

/******************************************************************************
����ͨ�ŷ���
******************************************************************************/
void CUserConn::_ProcessCommSend(void * pData, unsigned int uiDataSize)
{
	//ͨ�ŷ���
	if (m_pBridge == NULL
		|| m_pBridge->pDstConn == NULL
		|| pData == NULL
		|| uiDataSize <= 0)
	{
		_PutTip("", UserConn::PROCESS_COMM_SEND_COMM_SEND_PARAM);
		return;
	}
	m_pBridge->pDstConn->CommSend(pData, uiDataSize);
}

/******************************************************************************
�½���ʼ��Ŀ������
******************************************************************************/
void CUserConn::_NewInitDstConn(unsigned long ulDstIP
	, unsigned short usDstPort)
{
	//Ŀ��������Ч
	if (ulDstIP == 0
		|| usDstPort == 0)
	{
		_PutTip("��Ч��SOCKS��������"
			, UserConn::NEW_INIT_DST_CONN_DST_INVALID);
		//�ر�
		Close();
		return;
	}

	//�½�Ŀ������
	if (m_pBridge == NULL
		|| m_pBridge->pDstConn != NULL)
	{
		_PutTip("", UserConn::NEW_INIT_DST_CONN_NEW_PARAM);
		return;
	}
	m_pBridge->pDstConn = new(nothrow) CDstConn(m_pfnPutTip, m_pThis);
	if (m_pBridge->pDstConn == NULL)
	{
		_PutTip("", UserConn::NEW_INIT_DST_CONN_NEW_FAIL);
		return;
	}

	//��ʼ��Ŀ������
	if (m_pMainWnd == NULL
		|| m_pChannel == NULL
		|| m_socket == INVALID_SOCKET)
	{
		_PutTip("", UserConn::NEW_INIT_DST_CONN_INIT_PARAM);
		return;
	}
	m_pBridge->pDstConn->Init(m_pMainWnd, m_pChannel, m_pBridge, m_socket
		, ulDstIP, usDstPort);
}

/******************************************************************************
�����ʾ
******************************************************************************/
void CUserConn::_PutTip(string strTip, int iErr, string strErr)
{
	//�����ʾ�ص�
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "UserConn", iErr, strErr, m_pThis);
}