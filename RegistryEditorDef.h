#pragma once

namespace RegistryEditor
{

/******************************************************************************
错误码定义
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
常量定义
******************************************************************************/
typedef enum Const
{
	READ_STR_BUF_SIZE = 1024,		//读取字符串缓冲区大小
}EConst;

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