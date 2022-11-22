#pragma once

namespace MsgCenter
{

/******************************************************************************
�����붨��
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
��Ϣ�ṹ��
******************************************************************************/
typedef struct Msg
{
	unsigned int uiMsg;		//��Ϣ
	void * pParam;			//����
}SMsg;

/******************************************************************************
��Ϣ���붨��
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
�ر������Ϣ����
******************************************************************************/
#define WM_CLOSE_DONE WM_USER+2

/******************************************************************************
�����ʾ�ص���������
������
	strTip			��ʾ
	strClass		����
	iErr			������
	strErr			����
	pThis			�ص�thisָ��
******************************************************************************/
typedef void (* PFNPutTip)(string strTip, string strClass, int iErr
	, string strErr, void * pThis);

}