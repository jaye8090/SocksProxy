#pragma once

namespace MsgCenter
{

/******************************************************************************
错误码定义
******************************************************************************/
typedef enum ErrorCode
{
	RUN_BEGIN_THREAD_FAIL = 1,

	POST_MSG_SET_EVENT_FAIL,

	GET_MSG_LOCK_EVENT_FAIL,

	DISPATCH_MSG_UNKNOW_MSG,

	ON_INIT_SOFTWARE_MAIN_WND_NULL,

	ON_ADD_TRAY_ICON_MAIN_WND_NULL,

	ON_CLOSE_BOX_MAIN_WND_NULL,

	ON_CLOSE_MAIN_WND_NULL,

	ON_PROCESS_TRAY_ICON_MAIN_WND_NULL,

	ON_BUTTON_START_PROXY_MAIN_WND_NULL,

	ON_SAVE_CONFIG_MAIN_WND_NULL,

	ON_PROCESS_STARTUP_RUN_MAIN_WND_NULL,
}EErrorCode;

/******************************************************************************
消息结构体
******************************************************************************/
typedef struct Msg
{
	unsigned int uiMsg;		//消息
	void * pParam;			//参数
}SMsg;

/******************************************************************************
消息代码定义
******************************************************************************/
typedef enum MsgCode
{
	MSG_INIT_SOFTWARE = 1,
	MSG_ADD_TRAY_ICON,
	MSG_CLOSE_BOX,
	MSG_CLOSE,
	MSG_PROCESS_TRAY_ICON,
	MSG_BUTTON_START_PROXY,
	MSG_SAVE_CONFIG,
	MSG_PROCESS_STARTUP_RUN,
}EMsgCode;

/******************************************************************************
关闭完成消息定义
******************************************************************************/
#define WM_CLOSE_DONE WM_USER+2

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