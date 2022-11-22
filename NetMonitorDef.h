#pragma once

namespace NetMonitor
{

/******************************************************************************
错误码定义
******************************************************************************/
typedef enum ErrorCode
{
	INIT_RESET_EVENT_FAIL = 1,
	INIT_BEGIN_THREAD_FAIL,

	ADD_DATA_DATA_SIZE_INVALID,

	STOP_SET_EVENT_FAIL,
	STOP_LOCK_EVENT_FAIL,

	THREAD_MONITOR_SET_EVENT_FAIL,

	MONITOR_MAIN_WND_NULL,
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