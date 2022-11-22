#pragma once

#include <string>
using namespace std;

#include "AutoLockDef.h"

/******************************************************************************
自动锁类
功能：
	利用构造、析构函数，自动加锁、解锁。
作者：
	佳也 2623168833 jaye8090@qq.com
时间：
	2015-4-23 到 2015-4-23
******************************************************************************/
class CAutoLock
{
public:
	CAutoLock(CSyncObject * pLock
		, AutoLock::PFNPutTip pfnPutTip = NULL, void * pThis = NULL);

	~CAutoLock(void);

//内部操作
public:
	/**************************************************************************
	输出提示
	参数：
		strTip		提示
		iErr		错误码
		strErr		错误串
	**************************************************************************/
	void _PutTip(string strTip, int iErr = 0, string strErr = "");

//内部数据
private:
	AutoLock::PFNPutTip		m_pfnPutTip;		//输出提示回调函数
	void					* m_pThis;			//回调this指针

	CSyncObject		* m_pLock;		//锁
};