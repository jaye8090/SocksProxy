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
��ʼ��
******************************************************************************/
void CContextBuf::Init(void)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	m_setContext.clear();
}

/******************************************************************************
�������
******************************************************************************/
ContextBuf::SAcceptContext * CContextBuf::AllocAccept(
	ContextBuf::PFNAccept pfnAccept, void * pThis)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//����һ��������
	ContextBuf::SAcceptContext * pAcceptContext =
		(ContextBuf::SAcceptContext *)
		malloc(sizeof(ContextBuf::SAcceptContext));
	if (pAcceptContext == NULL)
	{
		_PutTip("", ContextBuf::ALLOC_ACCEPT_ALLOC_CONTEXT_FAIL);
		return NULL;
	}

	//��д����������
	memset(&pAcceptContext->overlap, 0, sizeof(pAcceptContext->overlap));
	pAcceptContext->eOperateType = ContextBuf::OPERATE_ACCEPT;
	pAcceptContext->pfnAccept = pfnAccept;
	pAcceptContext->pThis = pThis;

	//���仺����
	pAcceptContext->pBuf = malloc(sizeof(sockaddr_in) + 16);
	if (pAcceptContext->pBuf == NULL)
	{
		_PutTip("", ContextBuf::ALLOC_ACCEPT_ALLOC_BUF_FAIL);
		return NULL;
	}

	//���������ı�
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
��������
******************************************************************************/
ContextBuf::SConnContext * CContextBuf::AllocConn(ContextBuf::PFNConn pfnConn
	, void * pThis)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//����һ��������
	ContextBuf::SConnContext * pConnContext = (ContextBuf::SConnContext *)
		malloc(sizeof(ContextBuf::SConnContext));
	if (pConnContext == NULL)
	{
		_PutTip("", ContextBuf::ALLOC_CONN_ALLOC_CONTEXT_FAIL);
		return NULL;
	}

	//��д����������
	memset(&pConnContext->overlap, 0, sizeof(pConnContext->overlap));
	pConnContext->eOperateType = ContextBuf::OPERATE_CONN;
	pConnContext->pfnConn = pfnConn;
	pConnContext->pThis = pThis;

	//���������ı�
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
�������
******************************************************************************/
ContextBuf::SRecvContext * CContextBuf::AllocRecv(ContextBuf::PFNRecv pfnRecv
	, void * pThis)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//����һ��������
	ContextBuf::SRecvContext * pRecvContext = (ContextBuf::SRecvContext *)
		malloc(sizeof(ContextBuf::SRecvContext));
	if (pRecvContext == NULL)
	{
		_PutTip("", ContextBuf::ALLOC_RECV_ALLOC_CONTEXT_FAIL);
		return NULL;
	}

	//��д����������
	memset(&pRecvContext->overlap, 0, sizeof(pRecvContext->overlap));
	pRecvContext->eOperateType = ContextBuf::OPERATE_RECV;
	pRecvContext->pfnRecv = pfnRecv;
	pRecvContext->pThis = pThis;

	//���仺����
	pRecvContext->buf.buf = (char *)malloc(ContextBuf::RECV_BUF_SIZE);
	if (pRecvContext->buf.buf == NULL)
	{
		_PutTip("", ContextBuf::ALLOC_RECV_ALLOC_BUF_FAIL);
		return NULL;
	}
	pRecvContext->buf.len = ContextBuf::RECV_BUF_SIZE;

	//���������ı�
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
���䷢��
******************************************************************************/
ContextBuf::SSendContext * CContextBuf::AllocSend(void * pData
	, unsigned int uiDataSize)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//����һ��������
	ContextBuf::SSendContext * pSendContext = (ContextBuf::SSendContext *)
		malloc(sizeof(ContextBuf::SSendContext));
	if (pSendContext == NULL)
	{
		_PutTip("", ContextBuf::ALLOC_SEND_ALLOC_CONTEXT_FAIL);
		return NULL;
	}

	//��д����������
	memset(&pSendContext->overlap, 0, sizeof(pSendContext->overlap));
	pSendContext->eOperateType = ContextBuf::OPERATE_SEND;

	//���仺����
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

	//��������
	if (pData == NULL)
	{
		_PutTip("", ContextBuf::ALLOC_SEND_DATA_NULL);
		return NULL;
	}
	memcpy(pSendContext->buf.buf, pData, uiDataSize);

	//���������ı�
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
�ͷ�
******************************************************************************/
void CContextBuf::Free(ContextBuf::SContext * pContext)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//�������ı���ɾ��
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

	//�ͷŻ�����
	_FreeBuf(pContext);
	//�ͷ�������
	free(pContext);
}

/******************************************************************************
���
******************************************************************************/
void CContextBuf::Clear(void)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	//���������ı�
	set<ContextBuf::SContext *>::iterator it = m_setContext.begin();
	while (it != m_setContext.end())
	{
		//��ȡ������
		ContextBuf::SContext * pContext = *it;
		if (pContext == NULL)
		{
			_PutTip("", ContextBuf::CLEAR_CONTEXT_NULL);
			return;
		}

		//�������ı���ɾ��
		try
		{
			it = m_setContext.erase(it);
		}
		catch (...)
		{
			_PutTip("", ContextBuf::CLEAR_ERASE_EXCEPTION);
			return;
		}

		//�ͷŻ�����
		_FreeBuf(pContext);
		//�ͷ�������
		free(pContext);
	}
}

/******************************************************************************
�ͷŻ�����
******************************************************************************/
void CContextBuf::_FreeBuf(ContextBuf::SContext * pContext)
{
	//��ȡ������
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

	//�ͷŻ�����
	if (pBuf != NULL)
	{
		free(pBuf);
	}
}

/******************************************************************************
�����ʾ
******************************************************************************/
void CContextBuf::_PutTip(string strTip, int iErr, string strErr)
{
	//�����ʾ�ص�
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "ContextBuf", iErr, strErr, m_pThis);
}