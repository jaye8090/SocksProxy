#pragma once

namespace RegistryEditor
{

/******************************************************************************
�����붨��
******************************************************************************/
typedef enum ErrorCode
{
	READ_STR_QUERY_PARAM = 1,
	READ_STR_QUERY_FAIL,

	READ_INT_QUERY_PARAM,
	READ_INT_QUERY_FAIL,

	WRITE_STR_SET_PARAM,
	WRITE_STR_SET_FAIL,

	WRITE_INT_SET_PARAM,
	WRITE_INT_SET_FAIL,

	DEL_VALUE_DEL_PARAM,
	DEL_VALUE_DEL_FAIL,

	OPEN_KEY_CREATE_PARAM,
	OPEN_KEY_CREATE_FAIL,

	CLOSE_KEY_CLOSE_PARAM,
	CLOSE_KEY_CLOSE_FAIL,
}EErrorCode;

/******************************************************************************
��������
******************************************************************************/
typedef enum Const
{
	READ_STR_BUF_SIZE = 1024,		//��ȡ�ַ�����������С
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