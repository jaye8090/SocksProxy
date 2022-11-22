#pragma once

namespace TrayIcon
{

/******************************************************************************
�����붨��
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
����ͼ����Ϣ����
******************************************************************************/
#define WM_TRAY_ICON WM_USER+1

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