#pragma once

namespace AutoLock
{

/******************************************************************************
错误码定义
******************************************************************************/
typedef enum ErrorCode
{
	CONSTRUCT_LOCK_NULL = 1,
	CONSTRUCT_LOCK_FAIL,
	CONSTRUCT_LOCK_EXCEPTION,

	DESTRUCT_LOCK_NULL,
	DESTRUCT_UNLOCK_FAIL,
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