#pragma once

namespace UserConn
{

/******************************************************************************
错误码定义
******************************************************************************/
typedef enum ErrorCode
{
	INIT_BRIDGE_NULL = 1,
	INIT_RECV_PARAM,

	CONN_RESPONSE_BRIDGE_NULL,

	COMM_RECV_BRIDGE_NULL,

	CHECK_ALIVE_BRIDGE_NULL,

	CLOSE_BRIDGE_NULL,
	CLOSE_MAIN_WND_NULL,

	SEND_DATA_SEND_PARAM,

	PROCESS_RECV_BRIDGE_NULL,

	PROCESS_DEL_CONN_DEL_PARAM,

	GET_SOCKS_VER_DATA_INVALID,

	PROCESS_AUTH_REQUEST_DATA_INVALID,

	PROCESS_SOCKS5_CONN_REQUEST_DATA_INVALID,

	PROCESS_SOCKS4_CONN_REQUEST_DATA_INVALID,

	PROCESS_COMM_SEND_COMM_SEND_PARAM,

	NEW_INIT_DST_CONN_DST_INVALID,
	NEW_INIT_DST_CONN_NEW_PARAM,
	NEW_INIT_DST_CONN_NEW_FAIL,
	NEW_INIT_DST_CONN_INIT_PARAM,
}EErrorCode;

/******************************************************************************
常量定义
******************************************************************************/
typedef enum Const
{
	OFFLINE_TIME = 10 * 60 * 1000,		//掉线时间
}EConst;

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