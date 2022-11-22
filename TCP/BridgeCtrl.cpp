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
初始化
******************************************************************************/
void CBridgeCtrl::Init(void)
{
	m_mapBridge.clear();
	m_mapLimit.clear();
	m_mapRecvItem.clear();
}

/******************************************************************************
添加桥接
******************************************************************************/
void CBridgeCtrl::AddBridge(SOCKET socket1, SOCKET socket2)
{
	//正向加入桥接表
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

	//反向加入桥接表
	pr = m_mapBridge.insert(make_pair(socket2, socket1));
	if (!pr.second)
	{
		_PutTip("", BridgeCtrl::ADD_BRIDGE_INSERT_BRIDGE_FAIL2);
		return;
	}
}

/******************************************************************************
删除桥接
******************************************************************************/
void CBridgeCtrl::DelBridge(SOCKET socket)
{
	//正向查找
	if (socket == INVALID_SOCKET)
	{
		_PutTip("", BridgeCtrl::DEL_BRIDGE_SOCKET_INVALID1);
		return;
	}
	map<SOCKET, SOCKET>::iterator it1 = m_mapBridge.find(socket);

	//没有桥接
	if (it1 == m_mapBridge.end())
	{
		return;
	}

	//反向查找
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

	//从表中删除
	try
	{
		//从限制表中删除
		m_mapLimit.erase(socket);
		m_mapLimit.erase(it1->second);

		//从桥接表中删除
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
限制接收
******************************************************************************/
void CBridgeCtrl::LimitRecv(SOCKET sSend
	, ContextBuf::SSendContext * pSendContext)
{
	//没有桥接
	if (!_IsBridged(sSend))
	{
		return;
	}

	//加入限制表
	if (sSend == INVALID_SOCKET
		|| pSendContext == NULL)
	{
		_PutTip("", BridgeCtrl::LIMIT_RECV_INSERT_LIMIT_PARAM);
		return;
	}
	pair<map<SOCKET, ContextBuf::SSendContext *>::iterator, bool> pr
		= m_mapLimit.insert(make_pair(sSend, pSendContext));

	//无需更新
	if (pr.second)
	{
		return;
	}

	//更新接收条目键
	_UpdateRecvItemKey(pr.first->second, pSendContext);
	//更新发送上下文
	pr.first->second = pSendContext;
}

/******************************************************************************
暂停接收
******************************************************************************/
bool CBridgeCtrl::PauseRecv(
	SOCKET sRecv, ContextBuf::SRecvContext * pRecvContext)
{
	//获取发送上下文
	ContextBuf::SSendContext * pSendContext = _GetSendContext(sRecv);

	//没有桥接，或者没有限制。
	if (pSendContext == NULL)
	{
		return false;
	}

	//分配一个接收条目
	BridgeCtrl::SRecvItem * pRecvItem = (BridgeCtrl::SRecvItem *)
		malloc(sizeof(BridgeCtrl::SRecvItem));
	if (pRecvItem == NULL)
	{
		_PutTip("", BridgeCtrl::PAUSE_RECV_ALLOC_RECV_ITEM_FAIL);
		return false;
	}

	//填写接收条目
	pRecvItem->sRecv = sRecv;
	pRecvItem->pRecvContext = pRecvContext;

	//加入接收条目表
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
恢复接收
******************************************************************************/
bool CBridgeCtrl::ResumeRecv(
	SOCKET & sRecv, ContextBuf::SRecvContext * & pRecvContext
	, SOCKET sSend, ContextBuf::SSendContext * pSendContext)
{
	//取消限制
	_CancelLimit(sSend, pSendContext);

	//查找接收条目表
	if (pSendContext == NULL)
	{
		_PutTip("", BridgeCtrl::RESUME_RECV_SEND_CONTEXT_NULL);
		return false;
	}
	map<ContextBuf::SSendContext *, BridgeCtrl::SRecvItem *>::iterator it
		= m_mapRecvItem.find(pSendContext);

	//没有暂停接收，或者发送上下文不是最后一个。
	if (it == m_mapRecvItem.end())
	{
		return false;
	}

	//获取接收套接字和接收上下文
	BridgeCtrl::SRecvItem * pRecvItem = it->second;
	if (pRecvItem == NULL)
	{
		_PutTip("", BridgeCtrl::RESUME_RECV_RECV_ITEM_NULL);
		return false;
	}
	sRecv = pRecvItem->sRecv;
	pRecvContext = pRecvItem->pRecvContext;

	//从接收条目表中删除
	try
	{
		m_mapRecvItem.erase(it);
	}
	catch (...)
	{
		_PutTip("", BridgeCtrl::RESUME_RECV_ERASE_EXCEPTION);
		return false;
	}

	//释放接收条目
	free(pRecvItem);

	return true;
}

/******************************************************************************
更新接收条目键
******************************************************************************/
void CBridgeCtrl::_UpdateRecvItemKey(ContextBuf::SSendContext * pSendContextOld
	, ContextBuf::SSendContext * pSendContextNew)
{
	//查找接收条目表
	if (pSendContextOld == NULL)
	{
		_PutTip("", BridgeCtrl::UPDATE_RECV_ITEM_KEY_FIND_MAP_PARAM);
		return;
	}
	map<ContextBuf::SSendContext *, BridgeCtrl::SRecvItem *>::iterator it
		= m_mapRecvItem.find(pSendContextOld);

	//没有暂停接收
	if (it == m_mapRecvItem.end())
	{
		return;
	}

	//接收条目
	BridgeCtrl::SRecvItem * pRecvItem = it->second;

	//从接收条目表中删除老键值对
	try
	{
		m_mapRecvItem.erase(it);
	}
	catch (...)
	{
		_PutTip("", BridgeCtrl::UPDATE_RECV_ITEM_KEY_ERASE_EXCEPTION);
		return;
	}

	//把新键值对加入接收条目表
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
是否已经桥接
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
获取发送上下文
******************************************************************************/
ContextBuf::SSendContext * CBridgeCtrl::_GetSendContext(SOCKET sRecv)
{
	//查找桥接表
	if (sRecv == INVALID_SOCKET)
	{
		_PutTip("", BridgeCtrl::GET_SEND_CONTEXT_RECV_SOCKET_INVALID);
		return NULL;
	}
	map<SOCKET, SOCKET>::iterator itBridge = m_mapBridge.find(sRecv);

	//没有桥接
	if (itBridge == m_mapBridge.end())
	{
		return NULL;
	}

	//查找限制表
	if (itBridge->second == INVALID_SOCKET)
	{
		_PutTip("", BridgeCtrl::GET_SEND_CONTEXT_SEND_SOCKET_INVALID);
		return NULL;
	}
	map<SOCKET, ContextBuf::SSendContext *>::iterator itLimit
		= m_mapLimit.find(itBridge->second);

	//没有限制
	if (itLimit == m_mapLimit.end())
	{
		return NULL;
	}

	//发送上下文
	ContextBuf::SSendContext * pSendContext = itLimit->second;
	if (pSendContext == NULL)
	{
		_PutTip("", BridgeCtrl::GET_SEND_CONTEXT_SEND_CONTEXT_NULL);
		return NULL;
	}

	return pSendContext;
}

/******************************************************************************
取消限制
******************************************************************************/
void CBridgeCtrl::_CancelLimit(
	SOCKET sSend, ContextBuf::SSendContext * pSendContext)
{
	//查找限制表
	if (sSend == INVALID_SOCKET)
	{
		_PutTip("", BridgeCtrl::CALCEL_LIMIT_SOCKET_INVALID);
		return;
	}
	map<SOCKET, ContextBuf::SSendContext *>::iterator it
		= m_mapLimit.find(sSend);

	//没有限制，或者套接字已经关闭。
	if (it == m_mapLimit.end())
	{
		return;
	}

	//发送上下文不是最后一个，或者套接字已经重用。
	if (it->second != pSendContext)
	{
		return;
	}

	//从限制表中删除
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
输出提示
******************************************************************************/
void CBridgeCtrl::_PutTip(string strTip, int iErr, string strErr)
{
	//输出提示回调
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "BridgeCtrl", iErr, strErr, m_pThis);
}