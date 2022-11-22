#pragma once

class CUserConn;
class CDstConn;

namespace Channel
{

/******************************************************************************
错误码定义
******************************************************************************/
typedef enum ErrorCode
{
	INIT_BIND_PARAM = 1,
	INIT_SOCKET_INVALID,
	INIT_RESET_EVENT_FAIL,
	INIT_BEGIN_THREAD_FAIL,

	CLOSE_CLOSE_PARAM,
	CLOSE_SET_EVENT_FAIL,

	THREAD_CHECK_ALIVE_SET_EVENT_FAIL,

	CHECK_ALIVE_BRIDGE_NULL,

	CLOSE_ALL_BRIDGE_BRIDGE_NULL,

	DEL_USER_CONN_BRIDGE_PARAM,

	DEL_DST_CONN_BRIDGE_PARAM,

	DEL_BRIDGE_BRIDGE_NULL,
	DEL_BRIDGE_NOT_EXIST,
	DEL_BRIDGE_ERASE_EXCEPTION,

	NEW_BRIDGE_NEW_FAIL,
	NEW_BRIDGE_NEW_USER_CONN_FAIL,
	NEW_BRIDGE_INIT_USER_CONN_PARAM,
	NEW_BRIDGE_INSERT_SET_FAIL,
}EErrorCode;

/******************************************************************************
常量定义
******************************************************************************/
typedef enum Const
{
	CHECK_ALIVE_SPACE_TIME = 60 * 1000,		//检查活跃间隔时间
}EConst;

/******************************************************************************
桥接结构体
******************************************************************************/
typedef struct Bridge
{
	CCriticalSection		csLock;				//互斥锁
	CUserConn				* pUserConn;		//用户连接
	CDstConn				* pDstConn;			//目标连接
}SBridge;

/******************************************************************************
输出提示回调函数定义
参数：
	strTip			提示
	strClass		类名
	iErr			错误码
	strErr			错误串
	pThis			回调this指针
******************************************************************************/
typedef void (* PFNPutTip)(string strTip, string strClass, int iErr
	, string strErr, void * pThis);

}