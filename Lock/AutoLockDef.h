#pragma once

namespace AutoLock
{

/******************************************************************************
�����붨��
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