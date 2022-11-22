#pragma once

namespace TipsBox
{

/******************************************************************************
错误码定义
******************************************************************************/
typedef enum ErrorCode
{
	INIT_TIPS_BOX_NULL = 1,		//提示框指针为空
	INIT_PARENT_WND_NULL,		//主框架指针为空

	PUT_TIP_TIPS_BOX_NULL,		//提示框指针为空

	LOCK_INTERFACE_LOCK_FAIL,			//加锁失败
	LOCK_INTERFACE_LOCK_EXCEPTION,		//加锁异常

	UNLOCK_INTERFACE_UNLOCK_FAIL,		//解锁失败

	LOCK_SCROLL_LOCK_FAIL,			//加锁失败
	LOCK_SCROLL_LOCK_EXCEPTION,		//加锁异常

	UNLOCK_SCROLL_UNLOCK_FAIL,		//解锁失败

	IS_NEED_SAVE_TIPS_BOX_NULL,		//提示框指针为空

	SAVE_TIPS_TIPS_BOX_NULL,			//提示框指针为空
	SAVE_TIPS_SOFTWARE_PATH_EMPTY,		//主框架指针为空

	SAVE_TO_FILE_OPEN_FAIL,						//文件打开失败
	SAVE_TO_FILE_SET_LOCALE_FAIL,				//设置语言失败
	SAVE_TO_FILE_WRITE_STRING_EXCEPTION,		//写入文件异常

	CREATE_TIPS_LOG_PATH_SOFTWARE_PATH_EMPTY,		//主框架指针为空
	CREATE_TIPS_LOG_PATH_CREATE_PATH_FAIL,			//创建目录失败
}EErrorCode;

/******************************************************************************
常量定义
******************************************************************************/
typedef enum Const
{
	SAVE_TIPS_SIZE = 10 * 1024 * 1024,		//保存提示信息的大小（10M）
}EConst;

}