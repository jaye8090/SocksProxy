#pragma once

namespace UserPtl
{

/******************************************************************************
�����붨��
******************************************************************************/
typedef enum ErrorCode
{
	GET_SOCKS_VER_DATA_INVALID = 1,
	GET_SOCKS_VER_VER_INVALID,

	PARSE_AUTH_REQUEST_DATA_INVALID,
	PARSE_AUTH_REQUEST_PARSE_FAIL,
	PARSE_AUTH_REQUEST_METHOD_LIST_NULL,
	PARSE_AUTH_REQUEST_AUTH_METHOD_INVALID,

	PARSE_SOCKS5_CONN_REQUEST_DATA_INVALID,
	PARSE_SOCKS5_CONN_REQUEST_PARSE_FAIL_1,
	PARSE_SOCKS5_CONN_REQUEST_CMD_INVALID,
	PARSE_SOCKS5_CONN_REQUEST_PARSE_FAIL_2,
	PARSE_SOCKS5_CONN_REQUEST_ADDR_INVALID,
	PARSE_SOCKS5_CONN_REQUEST_PARSE_FAIL_3,

	PARSE_SOCKS4_CONN_REQUEST_DATA_INVALID,
	PARSE_SOCKS4_CONN_REQUEST_PARSE_FAIL_1,
	PARSE_SOCKS4_CONN_REQUEST_CMD_INVALID,
	PARSE_SOCKS4_CONN_REQUEST_PARSE_FAIL_2,
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