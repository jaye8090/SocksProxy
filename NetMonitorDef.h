#pragma once

namespace NetMonitor
{

/******************************************************************************
�����붨��
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