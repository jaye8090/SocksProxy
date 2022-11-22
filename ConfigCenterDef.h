#pragma once

namespace ConfigCenter
{

/******************************************************************************
错误码定义
******************************************************************************/
typedef enum ErrorCode
{
	GET_EXE_FILE_NAME_GET_FAIL = 1,

	LOCK_LOCK_FAIL,
	LOCK_LOCK_EXCEPTION,

	UNLOCK_UNLOCK_FAIL,

	SAVE_CONFIG_EXE_FILE_NAME_EMPTY,
}EErrorCode;

/******************************************************************************
字符串常量定义
******************************************************************************/
const CString		DEFAULT_PORT = _T("1080");		//默认端口号

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