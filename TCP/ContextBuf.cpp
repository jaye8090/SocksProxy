#include "StdAfx.h"

#include "ContextBuf.h"
#include "../Lock/AutoLock.h"

CContextBuf::CContextBuf(ContextBuf::PFNPutTip pfnPutTip, void * pThis)
{
	m_pfnPutTip = pfnPutTip;
	m_pThis = pThis;
}

CContextBuf::~CContextBuf(void)
{
}

/******************************************************************************
初始化
******************************************************************************/
void CContextBuf::Init(void)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	m_setContext.clear();
}

/******************************************************************************
分配接受
******************************************************************************/
ContextBuf::SAcceptContext * CContextBuf::AllocAccept(
	ContextBuf::PFNAccept pfnAccept, void * pThis)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//分配一个上下文
	ContextBuf::SAcceptContext * pAcceptContext =
		(ContextBuf::SAcceptContext *)
		malloc(sizeof(ContextBuf::SAcceptContext));
	if (pAcceptContext == NULL)
	{
		_PutTip("", ContextBuf::ALLOC_ACCEPT_ALLOC_CONTEXT_FAIL);
		return NULL;
	}

	//填写上下文数据
	memset(&pAcceptContext->overlap, 0, sizeof(pAcceptContext->overlap));
	pAcceptContext->eOperateType = ContextBuf::OPERATE_ACCEPT;
	pAcceptContext->pfnAccept = pfnAccept;
	pAcceptContext->pThis = pThis;

	//分配缓冲区
	pAcceptContext->pBuf = malloc(sizeof(sockaddr_in) + 16);
	if (pAcceptContext->pBuf == NULL)
	{
		_PutTip("", ContextBuf::ALLOC_ACCEPT_ALLOC_BUF_FAIL);
		return NULL;
	}

	//加入上下文表
	pair<set<ContextBuf::SContext *>::iterator, bool> pr
		= m_setContext.insert(pAcceptContext);
	if (!pr.second)
	{
		_PutTip("", ContextBuf::ALLOC_ACCEPT_INSERT_SET_FAIL);
		return NULL;
	}

	return pAcceptContext;
}

/******************************************************************************
分配连接
******************************************************************************/
ContextBuf::SConnContext * CContextBuf::AllocConn(ContextBuf::PFNConn pfnConn
	, void * pThis)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//分配一个上下文
	ContextBuf::SConnContext * pConnContext = (ContextBuf::SConnContext *)
		malloc(sizeof(ContextBuf::SConnContext));
	if (pConnContext == NULL)
	{
		_PutTip("", ContextBuf::ALLOC_CONN_ALLOC_CONTEXT_FAIL);
		return NULL;
	}

	//填写上下文数据
	memset(&pConnContext->overlap, 0, sizeof(pConnContext->overlap));
	pConnContext->eOperateType = ContextBuf::OPERATE_CONN;
	pConnContext->pfnConn = pfnConn;
	pConnContext->pThis = pThis;

	//加入上下文表
	pair<set<ContextBuf::SContext *>::iterator, bool> pr
		= m_setContext.insert(pConnContext);
	if (!pr.second)
	{
		_PutTip("", ContextBuf::ALLOC_CONN_INSERT_SET_FAIL);
		return NULL;
	}

	return pConnContext;
}

/******************************************************************************
分配接收
******************************************************************************/
ContextBuf::SRecvContext * CContextBuf::AllocRecv(ContextBuf::PFNRecv pfnRecv
	, void * pThis)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//分配一个上下文
	ContextBuf::SRecvContext * pRecvContext = (ContextBuf::SRecvContext *)
		malloc(sizeof(ContextBuf::SRecvContext));
	if (pRecvContext == NULL)
	{
		_PutTip("", ContextBuf::ALLOC_RECV_ALLOC_CONTEXT_FAIL);
		return NULL;
	}

	//填写上下文数据
	memset(&pRecvContext->overlap, 0, sizeof(pRecvContext->overlap));
	pRecvContext->eOperateType = ContextBuf::OPERATE_RECV;
	pRecvContext->pfnRecv = pfnRecv;
	pRecvContext->pThis = pThis;

	//分配缓冲区
	pRecvContext->buf.buf = (char *)malloc(ContextBuf::RECV_BUF_SIZE);
	if (pRecvContext->buf.buf == NULL)
	{
		_PutTip("", ContextBuf::ALLOC_RECV_ALLOC_BUF_FAIL);
		return NULL;
	}
	pRecvContext->buf.len = ContextBuf::RECV_BUF_SIZE;

	//加入上下文表
	pair<set<ContextBuf::SContext *>::iterator, bool> pr
		= m_setContext.insert(pRecvContext);
	if (!pr.second)
	{
		_PutTip("", ContextBuf::ALLOC_RECV_INSERT_SET_FAIL);
		return NULL;
	}

	return pRecvContext;
}

/******************************************************************************
分配发送
******************************************************************************/
ContextBuf::SSendContext * CContextBuf::AllocSend(void * pData
	, unsigned int uiDataSize)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//分配一个上下文
	ContextBuf::SSendContext * pSendContext = (ContextBuf::SSendContext *)
		malloc(sizeof(ContextBuf::SSendContext));
	if (pSendContext == NULL)
	{
		_PutTip("", ContextBuf::ALLOC_SEND_ALLOC_CONTEXT_FAIL);
		return NULL;
	}

	//填写上下文数据
	memset(&pSendContext->overlap, 0, sizeof(pSendContext->overlap));
	pSendContext->eOperateType = ContextBuf::OPERATE_SEND;

	//分配缓冲区
	if (uiDataSize <= 0)
	{
		_PutTip("", ContextBuf::ALLOC_SEND_DATA_SIZE_INVALID);
		return NULL;
	}
	pSendContext->buf.buf = (char *)malloc(uiDataSize);
	if (pSendContext->buf.buf == NULL)
	{
		_PutTip("", ContextBuf::ALLOC_SEND_ALLOC_BUF_FAIL);
		return NULL;
	}
	pSendContext->buf.len = uiDataSize;

	//拷贝数据
	if (pData == NULL)
	{
		_PutTip("", ContextBuf::ALLOC_SEND_DATA_NULL);
		return NULL;
	}
	memcpy(pSendContext->buf.buf, pData, uiDataSize);

	//加入上下文表
	pair<set<ContextBuf::SContext *>::iterator, bool> pr
		= m_setContext.insert(pSendContext);
	if (!pr.second)
	{
		_PutTip("", ContextBuf::ALLOC_SEND_INSERT_SET_FAIL);
		return NULL;
	}

	return pSendContext;
}

/******************************************************************************
释放
******************************************************************************/
void CContextBuf::Free(ContextBuf::SContext * pContext)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//从上下文表中删除
	if (pContext == NULL)
	{
		_PutTip("", ContextBuf::FREE_CONTEXT_NULL);
		return;
	}
	try
	{
		if (m_setContext.erase(pContext) != 1)
		{
			_PutTip("", ContextBuf::FREE_CONTEXT_NOT_EXIST);
			return;
		}
	}
	catch (...)
	{
		_PutTip("", ContextBuf::FREE_ERASE_EXCEPTION);
		return;
	}

	//释放缓冲区
	_FreeBuf(pContext);
	//释放上下文
	free(pContext);
}

/******************************************************************************
清空
******************************************************************************/
void CContextBuf::Clear(void)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//遍历上下文表
	set<ContextBuf::SContext *>::iterator it = m_setContext.begin();
	while (it != m_setContext.end())
	{
		//获取上下文
		ContextBuf::SContext * pContext = *it;
		if (pContext == NULL)
		{
			_PutTip("", ContextBuf::CLEAR_CONTEXT_NULL);
			return;
		}

		//从上下文表中删除
		try
		{
			it = m_setContext.erase(it);
		}
		catch (...)
		{
			_PutTip("", ContextBuf::CLEAR_ERASE_EXCEPTION);
			return;
		}

		//释放缓冲区
		_FreeBuf(pContext);
		//释放上下文
		free(pContext);
	}
}

/******************************************************************************
释放缓冲区
******************************************************************************/
void CContextBuf::_FreeBuf(ContextBuf::SContext * pContext)
{
	//获取缓冲区
	void * pBuf = NULL;
	if (pContext == NULL)
	{
		_PutTip("", ContextBuf::FREE_BUF_CONTEXT_NULL);
		return;
	}
	if (pContext->eOperateType == ContextBuf::OPERATE_ACCEPT)
	{
		pBuf = ((ContextBuf::SAcceptContext *)pContext)->pBuf;
	}
	else if (pContext->eOperateType == ContextBuf::OPERATE_SEND)
	{
		pBuf = ((ContextBuf::SSendContext *)pContext)->buf.buf;
	}
	else if (pContext->eOperateType == ContextBuf::OPERATE_RECV)
	{
		pBuf = ((ContextBuf::SRecvContext *)pContext)->buf.buf;
	}

	//释放缓冲区
	if (pBuf != NULL)
	{
		free(pBuf);
	}
}

/******************************************************************************
输出提示
******************************************************************************/
void CContextBuf::_PutTip(string strTip, int iErr, string strErr)
{
	//输出提示回调
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "ContextBuf", iErr, strErr, m_pThis);
}