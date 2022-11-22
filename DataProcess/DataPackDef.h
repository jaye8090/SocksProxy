#pragma once

namespace DataPack
{

/******************************************************************************
�����붨��
******************************************************************************/
typedef enum ErrorCode
{
	ALLOC_DATA_BUF_ALLOC_FAIL = 1,

	COPY_DATA_DATA_BUF_NULL,
	COPY_DATA_DATA_SIZE_INVALID,
	COPY_DATA_PUSH_DATA_INVALID,
	COPY_DATA_BUF_SIZE_SMALL,
}EErrorCode;

/******************************************************************************
��������
******************************************************************************/
typedef enum Const
{
	ALLOC_BUF_SIZE = 1024,		//ÿ������Ĵ�С
}EConst;

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