#pragma once

#include <string>
using namespace std;

#include "TrayIconDef.h"

/******************************************************************************
托盘图标类
功能：
	管理托盘图标
作者：
	佳也 2623168833 jaye8090@qq.com
时间：
	2014-7-2 到 2015-5-9
******************************************************************************/
class CTrayIcon
{
public:
	CTrayIcon(TrayIcon::PFNPutTip pfnPutTip = NULL, void * pThis = NULL);

	~CTrayIcon(void);

	/**************************************************************************
	初始化
	参数：
		pMainWnd		主窗口
		strTip			提示
		uiIconID		图标ID
	**************************************************************************/
	void Init(CWnd * pMainWnd, CString strTip, UINT uiIconID);

//基本接口
public:
	/**************************************************************************
	添加
	**************************************************************************/
	void Add(void);

	/**************************************************************************
	删除
	**************************************************************************/
	void Del(void);

//外部属性
public:
	//任务栏创建消息（防止托盘图标在任务栏重建后消失）
	static UINT		m_wmTaskbarCreated;

//内部操作
private:
	/**************************************************************************
	输出提示信息
	参数：
		strTip		提示信息
		iErr		错误码
		strErr		错误串
	**************************************************************************/
	void _PutTip(string strTip, int iErr = 0, string strErr = "");

//内部数据
private:
	TrayIcon::PFNPutTip		m_pfnPutTip;		//输出提示回调函数
	void					* m_pThis;			//回调this指针

	CWnd		* m_pMainWnd;		//主窗口指针

	CString		m_strTip;		//提示

	UINT		m_uiIconID;		//图标ID

	NOTIFYICONDATA		m_IconData;		//图标数据
};