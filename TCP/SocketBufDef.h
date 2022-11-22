#pragma once

namespace SocketBuf
{

/******************************************************************************
�����붨��
******************************************************************************/
typedef enum ErrorCode
{
	CREATE_CREATE_FAIL = 1,
	CREATE_INSERT_SET_FAIL,

	CREATE_ADDR_INVALID,
	CREATE_SOCKET_INVALID,
	CREATE_BIND_FAIL,

	CLOSE_SOCKET_INVALID,
	CLOSE_SOCKET_NOT_EXIST,
	CLOSE_ERASE_EXCEPTION,
	CLOSE_CLOSE_SOCKET_FAIL,

	CLEAR_SOCKET_INVALID,
	CLEAR_ERASE_EXCEPTION,
	CLEAR_CLOSE_SOCKET_FAIL,
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