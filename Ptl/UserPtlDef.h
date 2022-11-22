#pragma once

namespace UserPtl
{

/******************************************************************************
错误码定义
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