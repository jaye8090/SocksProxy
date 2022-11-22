#pragma once

namespace DstConn
{

/******************************************************************************
错误码定义
******************************************************************************/
typedef enum ErrorCode
{
	INIT_BRIDGE_NULL = 1,
	INIT_CONN_PARAM,
	INIT_BRIDGE_PARAM,

	COMM_SEND_BRIDGE_NULL,

	CLOSE_BRIDGE_NULL,
	CLOSE_MAIN_WND_NULL,

	SEND_DATA_SEND_PARAM,

	PROCESS_CONN_BRIDGE_NULL,
	PROCESS_CONN_MAIN_WND_NULL,
	PROCESS_CONN_USER_CONN_NULL,

	PROCESS_DEL_CONN_DEL_PARAM,

	PROCESS_RECV_BRIDGE_NULL,
	PROCESS_RECV_COMM_RECV_PARAM,
	PROCESS_RECV_MAIN_WND_NULL,
}EErrorCode;

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