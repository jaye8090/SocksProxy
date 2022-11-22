#pragma once

namespace TrayIcon
{

/******************************************************************************
错误码定义
******************************************************************************/
typedef enum ErrorCode
{
	ADD_MAIN_WND_NULL = 1,
	ADD_LOAD_ICON_PARAM,
	ADD_LOAD_ICON_FAIL,
	ADD_TIP_EMPTY,
	ADD_ADD_FAIL,

	DEL_DEL_FAIL,
}EErrorCode;

/******************************************************************************
托盘图标消息定义
******************************************************************************/
#define WM_TRAY_ICON WM_USER+1

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