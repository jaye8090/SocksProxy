#pragma once

namespace ConfigCenter
{

/******************************************************************************
�����붨��
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
�ַ�����������
******************************************************************************/
const CString		DEFAULT_PORT = _T("1080");		//Ĭ�϶˿ں�

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