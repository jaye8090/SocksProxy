#pragma once

#include <string>
#include <map>
using namespace std;

#include "BridgeCtrlDef.h"

/******************************************************************************
桥接控制类
功能：
	添加、删除桥接，提供桥接控制功能。
作者：
	佳也 2623168833 jaye8090@qq.com
时间：
	2016-8-2 到 2016-8-8
******************************************************************************/
class CBridgeCtrl
{
public:
	CBridgeCtrl(BridgeCtrl::PFNPutTip pfnPutTip = NULL, void * pThis = NULL);

	~CBridgeCtrl(void);

	/**************************************************************************
	初始化
	**************************************************************************/
	void Init(void);

//基本接口
public:
	/**************************************************************************
	添加桥接
	参数：
		socket1		套接字1
		socket2		套接字2
	**************************************************************************/
	void AddBridge(SOCKET socket1, SOCKET socket2);

	/**************************************************************************
	删除桥接
	参数：
		socket		套接字
	**************************************************************************/
	void DelBridge(SOCKET socket);

	/**************************************************************************
	限制接收
	参数：
		sSend				发送套接字
		pSendContext		发送上下文
	**************************************************************************/
	void LimitRecv(SOCKET sSend, ContextBuf::SSendContext * pSendContext);

	/**************************************************************************
	暂停接收
	参数：
		sRecv				接收套接字
		pRecvContext		接收上下文
	返回：
		暂停生效返回true，否则返回false。
	**************************************************************************/
	bool PauseRecv(SOCKET sRecv, ContextBuf::SRecvContext * pRecvContext);

	/**************************************************************************
	恢复接收
	参数：
		sRecv				输出值。接收套接字。
		pRecvContext		输出值。接收上下文。
		sSend				发送套接字
		pSendContext		发送上下文
	返回：
		恢复生效返回true，否则返回false。
	**************************************************************************/
	bool ResumeRecv(SOCKET & sRecv, ContextBuf::SRecvContext * & pRecvContext
		, SOCKET sSend, ContextBuf::SSendContext * pSendContext);

//内部操作
private:
	/**************************************************************************
	更新接收条目键
	参数：
		pSendContextOld		老发送上下文
		pSendContextNew		新发送上下文
	**************************************************************************/
	void _UpdateRecvItemKey(ContextBuf::SSendContext * pSendContextOld
		, ContextBuf::SSendContext * pSendContextNew);

	/**************************************************************************
	是否已经桥接
	参数：
		socket		套接字
	返回：
		已经桥接返回true，否则返回false。
	**************************************************************************/
	bool _IsBridged(SOCKET socket);

	/**************************************************************************
	获取发送上下文
	参数：
		sRecv		接收套接字
	返回：
		已经桥接并限制，则返回发送上下文，否则返回NULL。
	**************************************************************************/
	ContextBuf::SSendContext * _GetSendContext(SOCKET sRecv);

	/**************************************************************************
	取消限制
	参数：
		sSend				发送套接字
		pSendContext		发送上下文
	**************************************************************************/
	void _CancelLimit(SOCKET sSend, ContextBuf::SSendContext * pSendContext);

	/**************************************************************************
	输出提示
	参数：
		strTip		提示
		iErr		错误码
		strErr		错误串
	**************************************************************************/
	void _PutTip(string strTip, int iErr = 0, string strErr = "");

//内部数据
private:
	BridgeCtrl::PFNPutTip		m_pfnPutTip;		//输出提示回调函数
	void						* m_pThis;			//回调this指针

	map<SOCKET, SOCKET>							m_mapBridge;		//桥接表
	map<SOCKET, ContextBuf::SSendContext *>		m_mapLimit;			//限制表

	map<ContextBuf::SSendContext *
		, BridgeCtrl::SRecvItem *>		m_mapRecvItem;		//接收条目表
};