#pragma once

namespace DataPack
{

/******************************************************************************
错误码定义
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
常量定义
******************************************************************************/
typedef enum Const
{
	ALLOC_BUF_SIZE = 1024,		//每次申请的大小
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