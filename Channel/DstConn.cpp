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
初始化
******************************************************************************/
void CDstConn::Init(CSocksProxyDlg * pMainWnd, CChannel * pChannel
	, Channel::SBridge * pBridge, SOCKET sUserConn
	, unsigned long ulDstIP, unsigned short usDstPort)
{
	//互斥锁
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

	//创建连接
	if (m_pMainWnd == NULL
		|| ulDstIP == 0
		|| usDstPort <= 0
		|| usDstPort >= 65535)
	{
		_PutTip("", DstConn::INIT_CONN_PARAM);
		return;
	}
	m_pMainWnd->m_TCP.Conn(m_socket, ulDstIP, usDstPort, _callbackConn, this);

	//桥接
	if (sUserConn == INVALID_SOCKET
		|| m_socket == INVALID_SOCKET)
	{
		_PutTip("", DstConn::INIT_BRIDGE_PARAM);
		return;
	}
	m_pMainWnd->m_TCP.Bridge(sUserConn, m_socket);
}

/******************************************************************************
通信发送
******************************************************************************/
void CDstConn::CommSend(void * pData, unsigned int uiDataSize)
{
	//互斥锁
	if (m_pBridge == NULL)
	{
		_PutTip("", DstConn::COMM_SEND_BRIDGE_NULL);
		return;
	}
	CAutoLock lock(&m_pBridge->csLock, m_pfnPutTip, m_pThis);

	//已经关闭
	if (m_socket == INVALID_SOCKET)
	{
		return;
	}

	//发送数据
	_SendData(pData, uiDataSize);
}

/******************************************************************************
关闭
******************************************************************************/
void CDstConn::Close(void)
{
	//互斥锁
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
连接回调函数
******************************************************************************/
void CDstConn::_callbackConn(bool bSucc, string strErr, void * pThis)
{
	if (pThis == NULL)
	{
		return;
	}
	CDstConn * pThis2 = (CDstConn *)pThis;

	//处理连接
	bool bDelConn = false;
	pThis2->_ProcessConn(bDelConn, bSucc, strErr);

	//处理删除连接
	pThis2->_ProcessDelConn(bDelConn);
}

/******************************************************************************
接收回调函数
******************************************************************************/
void CDstConn::_callbackRecv(void * pData, unsigned int uiDataSize
	, void * pThis)
{
	if (pThis == NULL)
	{
		return;
	}
	CDstConn * pThis2 = (CDstConn *)pThis;

	//处理接收
	bool bDelConn = false;
	pThis2->_ProcessRecv(bDelConn, pData, uiDataSize);

	//处理删除连接
	pThis2->_ProcessDelConn(bDelConn);
}

/******************************************************************************
发送数据
******************************************************************************/
void CDstConn::_SendData(void * pData, unsigned int uiDataSize)
{
	//发送数据
	if (m_pMainWnd == NULL
		|| m_socket == INVALID_SOCKET
		|| pData == NULL
		|| uiDataSize <= 0)
	{
		_PutTip("", DstConn::SEND_DATA_SEND_PARAM);
		return;
	}
	m_pMainWnd->m_TCP.Send(m_socket, pData, uiDataSize);

	//向网络监视添加数据
	m_pMainWnd->m_NetMonitor.AddData(uiDataSize);
}

/******************************************************************************
处理连接
******************************************************************************/
void CDstConn::_ProcessConn(bool & bDelConn, bool bSucc, string strErr)
{
	//互斥锁
	if (m_pBridge == NULL)
	{
		_PutTip("", DstConn::PROCESS_CONN_BRIDGE_NULL);
		return;
	}
	CAutoLock lock(&m_pBridge->csLock, m_pfnPutTip, m_pThis);

	//连接成功
	if (bSucc)
	{
		//已经关闭
		if (m_socket == INVALID_SOCKET)
		{
			//删除连接
			bDelConn = true;
			return;
		}

		//接收数据
		if (m_pMainWnd == NULL)
		{
			_PutTip("", DstConn::PROCESS_CONN_MAIN_WND_NULL);
			return;
		}
		m_pMainWnd->m_TCP.Recv(m_socket, _callbackRecv, this);

		//连接响应
		if (m_pBridge->pUserConn == NULL)
		{
			_PutTip("", DstConn::PROCESS_CONN_USER_CONN_NULL);
			return;
		}
		m_pBridge->pUserConn->ConnResponse();
	}
	//连接失败
	else
	{
		//删除连接
		bDelConn = true;
		//关闭
		Close();
	}
}

/******************************************************************************
处理删除连接
******************************************************************************/
void CDstConn::_ProcessDelConn(bool bDelConn)
{
	//是否删除连接
	if (!bDelConn)
	{
		return;
	}

	//删除目标连接
	if (m_pChannel == NULL
		|| m_pBridge == NULL)
	{
		_PutTip("", DstConn::PROCESS_DEL_CONN_DEL_PARAM);
		return;
	}
	m_pChannel->DelDstConn(m_pBridge);
}

/******************************************************************************
处理接收
******************************************************************************/
void CDstConn::_ProcessRecv(bool & bDelConn, void * pData
	, unsigned int uiDataSize)
{
	//互斥锁
	if (m_pBridge == NULL)
	{
		_PutTip("", DstConn::PROCESS_RECV_BRIDGE_NULL);
		return;
	}
	CAutoLock lock(&m_pBridge->csLock, m_pfnPutTip, m_pThis);

	//连接断开
	if (pData == NULL)
	{
		//删除连接
		bDelConn = true;
		//关闭
		Close();
		return;
	}

	//已经关闭
	if (m_socket == INVALID_SOCKET)
	{
		return;
	}

	//通信接收
	if (m_pBridge->pUserConn == NULL
		|| uiDataSize <= 0)
	{
		_PutTip("", DstConn::PROCESS_RECV_COMM_RECV_PARAM);
		return;
	}
	CDataPack pack(m_pfnPutTip, m_pThis);
	pack.Push(pData, uiDataSize);
	m_pBridge->pUserConn->CommRecv(pack);

	//向网络监视添加数据
	if (m_pMainWnd == NULL)
	{
		_PutTip("", DstConn::PROCESS_RECV_MAIN_WND_NULL);
		return;
	}
	m_pMainWnd->m_NetMonitor.AddData(uiDataSize);
}

/******************************************************************************
输出提示
******************************************************************************/
void CDstConn::_PutTip(string strTip, int iErr, string strErr)
{
	//输出提示回调
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "DstConn", iErr, strErr, m_pThis);
}