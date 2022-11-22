#pragma once

namespace SocketBuf
{

/******************************************************************************
错误码定义
******************************************************************************/
typedef enum ErrorCode
{
	CREATE_CREATE_FAIL = 1,
	CREATE_INSERT_SET_FAIL,

	CREATE_ADDR_INVALID,
	CREATE_SOCKET_INVALID,
	CREATE_BIND_FAIL,

	CLOSE_SOCKET_INVALID,
	CLOSE_SOCKET_NOT_EXIST,
	CLOSE_ERASE_EXCEPTION,
	CLOSE_CLOSE_SOCKET_FAIL,

	CLEAR_SOCKET_INVALID,
	CLEAR_ERASE_EXCEPTION,
	CLEAR_CLOSE_SOCKET_FAIL,
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