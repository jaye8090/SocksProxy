#include "StdAfx.h"

#include "BridgeCtrl.h"

CBridgeCtrl::CBridgeCtrl(BridgeCtrl::PFNPutTip pfnPutTip, void * pThis)
{
	m_pfnPutTip = pfnPutTip;
	m_pThis = pThis;
}

CBridgeCtrl::~CBridgeCtrl(void)
{
}

/******************************************************************************
��ʼ��
******************************************************************************/
void CBridgeCtrl::Init(void)
{
	m_mapBridge.clear();
	m_mapLimit.clear();
	m_mapRecvItem.clear();
}

/******************************************************************************
����Ž�
******************************************************************************/
void CBridgeCtrl::AddBridge(SOCKET socket1, SOCKET socket2)
{
	//��������Žӱ�
	if (socket1 == INVALID_SOCKET
		|| socket2 == INVALID_SOCKET)
	{
		_PutTip("", BridgeCtrl::ADD_BRIDGE_SOCKET_INVALID);
		return;
	}
	pair<map<SOCKET, SOCKET>::iterator, bool> pr
		= m_mapBridge.insert(make_pair(socket1, socket2));
	if (!pr.second)
	{
		_PutTip("", BridgeCtrl::ADD_BRIDGE_INSERT_BRIDGE_FAIL1);
		return;
	}

	//��������Žӱ�
	pr = m_mapBridge.insert(make_pair(socket2, socket1));
	if (!pr.second)
	{
		_PutTip("", BridgeCtrl::ADD_BRIDGE_INSERT_BRIDGE_FAIL2);
		return;
	}
}

/******************************************************************************
ɾ���Ž�
******************************************************************************/
void CBridgeCtrl::DelBridge(SOCKET socket)
{
	//�������
	if (socket == INVALID_SOCKET)
	{
		_PutTip("", BridgeCtrl::DEL_BRIDGE_SOCKET_INVALID1);
		return;
	}
	map<SOCKET, SOCKET>::iterator it1 = m_mapBridge.find(socket);

	//û���Ž�
	if (it1 == m_mapBridge.end())
	{
		return;
	}

	//�������
	if (it1->second == INVALID_SOCKET)
	{
		_PutTip("", BridgeCtrl::DEL_BRIDGE_SOCKET_INVALID2);
		return;
	}
	map<SOCKET, SOCKET>::iterator it2 = m_mapBridge.find(it1->second);
	if (it2 == m_mapBridge.end())
	{
		_PutTip("", BridgeCtrl::DEL_BRIDGE_NOT_EXIST);
		return;
	}

	//�ӱ���ɾ��
	try
	{
		//�����Ʊ���ɾ��
		m_mapLimit.erase(socket);
		m_mapLimit.erase(it1->second);

		//���Žӱ���ɾ��
		m_mapBridge.erase(it1);
		m_mapBridge.erase(it2);
	}
	catch (...)
	{
		_PutTip("", BridgeCtrl::DEL_BRIDGE_ERASE_EXCEPTION);
		return;
	}
}

/******************************************************************************
���ƽ���
******************************************************************************/
void CBridgeCtrl::LimitRecv(SOCKET sSend
	, ContextBuf::SSendContext * pSendContext)
{
	//û���Ž�
	if (!_IsBridged(sSend))
	{
		return;
	}

	//�������Ʊ�
	if (sSend == INVALID_SOCKET
		|| pSendContext == NULL)
	{
		_PutTip("", BridgeCtrl::LIMIT_RECV_INSERT_LIMIT_PARAM);
		return;
	}
	pair<map<SOCKET, ContextBuf::SSendContext *>::iterator, bool> pr
		= m_mapLimit.insert(make_pair(sSend, pSendContext));

	//�������
	if (pr.second)
	{
		return;
	}

	//���½�����Ŀ��
	_UpdateRecvItemKey(pr.first->second, pSendContext);
	//���·���������
	pr.first->second = pSendContext;
}

/******************************************************************************
��ͣ����
******************************************************************************/
bool CBridgeCtrl::PauseRecv(
	SOCKET sRecv, ContextBuf::SRecvContext * pRecvContext)
{
	//��ȡ����������
	ContextBuf::SSendContext * pSendContext = _GetSendContext(sRecv);

	//û���Žӣ�����û�����ơ�
	if (pSendContext == NULL)
	{
		return false;
	}

	//����һ��������Ŀ
	BridgeCtrl::SRecvItem * pRecvItem = (BridgeCtrl::SRecvItem *)
		malloc(sizeof(BridgeCtrl::SRecvItem));
	if (pRecvItem == NULL)
	{
		_PutTip("", BridgeCtrl::PAUSE_RECV_ALLOC_RECV_ITEM_FAIL);
		return false;
	}

	//��д������Ŀ
	pRecvItem->sRecv = sRecv;
	pRecvItem->pRecvContext = pRecvContext;

	//���������Ŀ��
	pair<map<ContextBuf::SSendContext *
		, BridgeCtrl::SRecvItem *>::iterator, bool> pr
		= m_mapRecvItem.insert(make_pair(pSendContext, pRecvItem));
	if (!pr.second)
	{
		_PutTip("", BridgeCtrl::PAUSE_RECV_INSERT_RECV_ITEM_FAIL);
		return false;
	}

	return true;
}

/******************************************************************************
�ָ�����
******************************************************************************/
bool CBridgeCtrl::ResumeRecv(
	SOCKET & sRecv, ContextBuf::SRecvContext * & pRecvContext
	, SOCKET sSend, ContextBuf::SSendContext * pSendContext)
{
	//ȡ������
	_CancelLimit(sSend, pSendContext);

	//���ҽ�����Ŀ��
	if (pSendContext == NULL)
	{
		_PutTip("", BridgeCtrl::RESUME_RECV_SEND_CONTEXT_NULL);
		return false;
	}
	map<ContextBuf::SSendContext *, BridgeCtrl::SRecvItem *>::iterator it
		= m_mapRecvItem.find(pSendContext);

	//û����ͣ���գ����߷��������Ĳ������һ����
	if (it == m_mapRecvItem.end())
	{
		return false;
	}

	//��ȡ�����׽��ֺͽ���������
	BridgeCtrl::SRecvItem * pRecvItem = it->second;
	if (pRecvItem == NULL)
	{
		_PutTip("", BridgeCtrl::RESUME_RECV_RECV_ITEM_NULL);
		return false;
	}
	sRecv = pRecvItem->sRecv;
	pRecvContext = pRecvItem->pRecvContext;

	//�ӽ�����Ŀ����ɾ��
	try
	{
		m_mapRecvItem.erase(it);
	}
	catch (...)
	{
		_PutTip("", BridgeCtrl::RESUME_RECV_ERASE_EXCEPTION);
		return false;
	}

	//�ͷŽ�����Ŀ
	free(pRecvItem);

	return true;
}

/******************************************************************************
���½�����Ŀ��
******************************************************************************/
void CBridgeCtrl::_UpdateRecvItemKey(ContextBuf::SSendContext * pSendContextOld
	, ContextBuf::SSendContext * pSendContextNew)
{
	//���ҽ�����Ŀ��
	if (pSendContextOld == NULL)
	{
		_PutTip("", BridgeCtrl::UPDATE_RECV_ITEM_KEY_FIND_MAP_PARAM);
		return;
	}
	map<ContextBuf::SSendContext *, BridgeCtrl::SRecvItem *>::iterator it
		= m_mapRecvItem.find(pSendContextOld);

	//û����ͣ����
	if (it == m_mapRecvItem.end())
	{
		return;
	}

	//������Ŀ
	BridgeCtrl::SRecvItem * pRecvItem = it->second;

	//�ӽ�����Ŀ����ɾ���ϼ�ֵ��
	try
	{
		m_mapRecvItem.erase(it);
	}
	catch (...)
	{
		_PutTip("", BridgeCtrl::UPDATE_RECV_ITEM_KEY_ERASE_EXCEPTION);
		return;
	}

	//���¼�ֵ�Լ��������Ŀ��
	if (pSendContextNew == NULL
		|| pRecvItem == NULL)
	{
		_PutTip("", BridgeCtrl::UPDATE_RECV_ITEM_KEY_INSERT_MAP_PARAM);
		return;
	}
	pair<map<ContextBuf::SSendContext *
		, BridgeCtrl::SRecvItem *>::iterator, bool> pr
		= m_mapRecvItem.insert(make_pair(pSendContextNew, pRecvItem));
	if (!pr.second)
	{
		_PutTip("", BridgeCtrl::UPDATE_RECV_ITEM_KEY_INSERT_MAP_FAIL);
		return;
	}
}

/******************************************************************************
�Ƿ��Ѿ��Ž�
******************************************************************************/
bool CBridgeCtrl::_IsBridged(SOCKET socket)
{
	if (socket == INVALID_SOCKET)
	{
		_PutTip("", BridgeCtrl::IS_BRIDGED_SOCKET_INVALID);
		return false;
	}
	map<SOCKET, SOCKET>::iterator it = m_mapBridge.find(socket);
	if (it == m_mapBridge.end())
	{
		return false;
	}

	return true;
}

/******************************************************************************
��ȡ����������
******************************************************************************/
ContextBuf::SSendContext * CBridgeCtrl::_GetSendContext(SOCKET sRecv)
{
	//�����Žӱ�
	if (sRecv == INVALID_SOCKET)
	{
		_PutTip("", BridgeCtrl::GET_SEND_CONTEXT_RECV_SOCKET_INVALID);
		return NULL;
	}
	map<SOCKET, SOCKET>::iterator itBridge = m_mapBridge.find(sRecv);

	//û���Ž�
	if (itBridge == m_mapBridge.end())
	{
		return NULL;
	}

	//�������Ʊ�
	if (itBridge->second == INVALID_SOCKET)
	{
		_PutTip("", BridgeCtrl::GET_SEND_CONTEXT_SEND_SOCKET_INVALID);
		return NULL;
	}
	map<SOCKET, ContextBuf::SSendContext *>::iterator itLimit
		= m_mapLimit.find(itBridge->second);

	//û������
	if (itLimit == m_mapLimit.end())
	{
		return NULL;
	}

	//����������
	ContextBuf::SSendContext * pSendContext = itLimit->second;
	if (pSendContext == NULL)
	{
		_PutTip("", BridgeCtrl::GET_SEND_CONTEXT_SEND_CONTEXT_NULL);
		return NULL;
	}

	return pSendContext;
}

/******************************************************************************
ȡ������
******************************************************************************/
void CBridgeCtrl::_CancelLimit(
	SOCKET sSend, ContextBuf::SSendContext * pSendContext)
{
	//�������Ʊ�
	if (sSend == INVALID_SOCKET)
	{
		_PutTip("", BridgeCtrl::CALCEL_LIMIT_SOCKET_INVALID);
		return;
	}
	map<SOCKET, ContextBuf::SSendContext *>::iterator it
		= m_mapLimit.find(sSend);

	//û�����ƣ������׽����Ѿ��رա�
	if (it == m_mapLimit.end())
	{
		return;
	}

	//���������Ĳ������һ���������׽����Ѿ����á�
	if (it->second != pSendContext)
	{
		return;
	}

	//�����Ʊ���ɾ��
	try
	{
		m_mapLimit.erase(it);
	}
	catch (...)
	{
		_PutTip("", BridgeCtrl::CALCEL_LIMIT_ERASE_EXCEPTION);
		return;
	}
}

/******************************************************************************
�����ʾ
******************************************************************************/
void CBridgeCtrl::_PutTip(string strTip, int iErr, string strErr)
{
	//�����ʾ�ص�
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "BridgeCtrl", iErr, strErr, m_pThis);
}