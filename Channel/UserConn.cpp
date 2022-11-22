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
初始化
******************************************************************************/
void CUserConn::Init(CSocksProxyDlg * pMainWnd, CChannel * pChannel
	, Channel::SBridge * pBridge, SOCKET socket)
{
	//互斥锁
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

	//接收数据
	if (m_pMainWnd == NULL
		|| m_socket == INVALID_SOCKET)
	{
		_PutTip("", UserConn::INIT_RECV_PARAM);
		return;
	}
	m_pMainWnd->m_TCP.Recv(m_socket, _callbackRecv, this);
}

/******************************************************************************
连接响应
******************************************************************************/
void CUserConn::ConnResponse(void)
{
	//互斥锁
	if (m_pBridge == NULL)
	{
		_PutTip("", UserConn::CONN_RESPONSE_BRIDGE_NULL);
		return;
	}
	CAutoLock lock(&m_pBridge->csLock, m_pfnPutTip, m_pThis);

	//已经关闭
	if (m_socket == INVALID_SOCKET)
	{
		return;
	}

	//打包连接响应包
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

	//发送数据
	_SendData(pack);
	//连接请求完成
	m_bConnRequestDone = true;
}

/******************************************************************************
通信接收
******************************************************************************/
void CUserConn::CommRecv(CDataPack & data)
{
	//互斥锁
	if (m_pBridge == NULL)
	{
		_PutTip("", UserConn::COMM_RECV_BRIDGE_NULL);
		return;
	}
	CAutoLock lock(&m_pBridge->csLock, m_pfnPutTip, m_pThis);

	//已经关闭
	if (m_socket == INVALID_SOCKET)
	{
		return;
	}

	//发送数据
	_SendData(data);
}

/******************************************************************************
检查活跃
******************************************************************************/
void CUserConn::CheckAlive(void)
{
	//互斥锁
	if (m_pBridge == NULL)
	{
		_PutTip("", UserConn::CHECK_ALIVE_BRIDGE_NULL);
		return;
	}
	CAutoLock lock(&m_pBridge->csLock, m_pfnPutTip, m_pThis);

	//是否已经掉线
	if (m_AliveTimeCnt.End() < UserConn::OFFLINE_TIME)
	{
		return;
	}

	//关闭
	Close();
}

/******************************************************************************
关闭
******************************************************************************/
void CUserConn::Close(void)
{
	//互斥锁
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
接收回调函数
******************************************************************************/
void CUserConn::_callbackRecv(void * pData, unsigned int uiDataSize
	, void * pThis)
{
	if (pThis == NULL)
	{
		return;
	}
	CUserConn * pThis2 = (CUserConn *)pThis;

	//处理接收
	bool bDelConn = false;
	pThis2->_ProcessRecv(bDelConn, pData, uiDataSize);

	//处理删除连接
	pThis2->_ProcessDelConn(bDelConn);
}

/******************************************************************************
发送数据
******************************************************************************/
void CUserConn::_SendData(CDataPack & data)
{
	//获取数据
	unsigned char * pData = NULL;
	unsigned int uiDataSize = 0;
	data.GetData(pData, uiDataSize);

	//发送数据
	if (m_pMainWnd == NULL
		|| m_socket == INVALID_SOCKET
		|| pData == NULL
		|| uiDataSize <= 0)
	{
		_PutTip("", UserConn::SEND_DATA_SEND_PARAM);
		return;
	}
	m_pMainWnd->m_TCP.Send(m_socket, pData, uiDataSize);

	//活跃计时刷新
	m_AliveTimeCnt.Begin();
}

/******************************************************************************
处理接收
******************************************************************************/
void CUserConn::_ProcessRecv(bool & bDelConn, void * pData
	, unsigned int uiDataSize)
{
	//互斥锁
	if (m_pBridge == NULL)
	{
		_PutTip("", UserConn::PROCESS_RECV_BRIDGE_NULL);
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

	//活跃计时刷新
	m_AliveTimeCnt.Begin();
	//处理SOCKS请求
	_ProcessSocksRequest(pData, uiDataSize);
}

/******************************************************************************
处理删除连接
******************************************************************************/
void CUserConn::_ProcessDelConn(bool bDelConn)
{
	//是否删除连接
	if (!bDelConn)
	{
		return;
	}

	//删除用户连接
	if (m_pChannel == NULL
		|| m_pBridge == NULL)
	{
		_PutTip("", UserConn::PROCESS_DEL_CONN_DEL_PARAM);
		return;
	}
	m_pChannel->DelUserConn(m_pBridge);
}

/******************************************************************************
处理SOCKS请求
******************************************************************************/
void CUserConn::_ProcessSocksRequest(void * pData, unsigned int uiDataSize)
{
	//连接请求还没完成
	if (!m_bConnRequestDone)
	{
		//获取SOCKS版本
		if (!_GetSocksVer(pData, uiDataSize))
		{
			return;
		}

		//SOCKS5
		if (m_ucSocksVer == 0x05)
		{
			//认证请求还没完成
			if (!m_bAuthRequestDone)
			{
				//处理认证请求
				_ProcessAuthRequest(pData, uiDataSize);
			}
			//认证请求已经完成
			else
			{
				//处理SOCKS5连接请求
				_ProcessSocks5ConnRequest(pData, uiDataSize);
			}
		}
		//SOCKS4
		else
		{
			//处理SOCKS4连接请求
			_ProcessSocks4ConnRequest(pData, uiDataSize);
		}
	}
	//连接请求已经完成
	else
	{
		//处理通信发送
		_ProcessCommSend(pData, uiDataSize);
	}
}

/******************************************************************************
获取SOCKS版本
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
		//关闭
		Close();
		return false;
	}

	return true;
}

/******************************************************************************
处理认证请求
******************************************************************************/
void CUserConn::_ProcessAuthRequest(void * pData, unsigned int uiDataSize)
{
	//解析认证请求包
	if (pData == NULL
		|| uiDataSize <= 0)
	{
		_PutTip("", UserConn::PROCESS_AUTH_REQUEST_DATA_INVALID);
		return;
	}
	if (!m_UserPtl.ParseAuthRequest(pData, uiDataSize))
	{
		//关闭
		Close();
		return;
	}

	//打包认证响应包
	CDataPack pack(m_pfnPutTip, m_pThis);
	m_UserPtl.PackAuthResponse(pack);

	//发送数据
	_SendData(pack);
	//认证请求完成
	m_bAuthRequestDone = true;
}

/******************************************************************************
处理SOCKS5连接请求
******************************************************************************/
void CUserConn::_ProcessSocks5ConnRequest(void * pData
	, unsigned int uiDataSize)
{
	//解析SOCKS5连接请求包
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
		//关闭
		Close();
		return;
	}

	//新建初始化目标连接
	_NewInitDstConn(ulDstIP, usDstPort);
}

/******************************************************************************
处理SOCKS4连接请求
******************************************************************************/
void CUserConn::_ProcessSocks4ConnRequest(void * pData
	, unsigned int uiDataSize)
{
	//解析SOCKS4连接请求包
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
		//关闭
		Close();
		return;
	}

	//新建初始化目标连接
	_NewInitDstConn(ulDstIP, usDstPort);
}

/******************************************************************************
处理通信发送
******************************************************************************/
void CUserConn::_ProcessCommSend(void * pData, unsigned int uiDataSize)
{
	//通信发送
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
新建初始化目标连接
******************************************************************************/
void CUserConn::_NewInitDstConn(unsigned long ulDstIP
	, unsigned short usDstPort)
{
	//目标主机无效
	if (ulDstIP == 0
		|| usDstPort == 0)
	{
		_PutTip("无效的SOCKS代理请求！"
			, UserConn::NEW_INIT_DST_CONN_DST_INVALID);
		//关闭
		Close();
		return;
	}

	//新建目标连接
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

	//初始化目标连接
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
输出提示
******************************************************************************/
void CUserConn::_PutTip(string strTip, int iErr, string strErr)
{
	//输出提示回调
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "UserConn", iErr, strErr, m_pThis);
}