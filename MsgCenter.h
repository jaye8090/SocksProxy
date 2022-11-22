#pragma once

#include <string>
#include <queue>
using namespace std;

#include "MsgCenterDef.h"

class CSocksProxyDlg;

/******************************************************************************
消息中心类
功能：
	处理用户界面的消息
作者：
	佳也 2623168833 jaye8090@qq.com
时间：
	2013-9-30 到 2013-12-14
******************************************************************************/
class CMsgCenter
{
public:
	CMsgCenter(MsgCenter::PFNPutTip pfnPutTip = NULL, void * pThis = NULL);

	~CMsgCenter(void);

	/**************************************************************************
	初始化
	参数：
		pMainWnd		主窗口
	**************************************************************************/
	void Init(CSocksProxyDlg * pMainWnd);

//基本接口
public:
	/**************************************************************************
	发送消息
	说明：
		此接口是线程安全的
	参数：
		uiMsg		消息
	**************************************************************************/
	void PostMsg(unsigned int uiMsg);

	/**************************************************************************
	发送消息
	说明：
		此接口是线程安全的
	参数：
		uiMsg		消息
		pParam		消息参数
	返回：
		成功返回true，失败返回false。
	**************************************************************************/
	bool PostMsg(unsigned int uiMsg, void * pParam);

//内部操作
private:
	/**************************************************************************
	运行消息中心
	**************************************************************************/
	void _Run(void);

	/**************************************************************************
	消息处理线程
	**************************************************************************/
	static UINT _threadProcessMsg(LPVOID pParam);

	/**************************************************************************
	获取消息
	说明：
		如果消息队列为空，则阻塞到有消息到达，才返回。
	参数：
		sMsg		返回值。消息结构体。
	返回：
		获取到了返回true，否则返回false。
	**************************************************************************/
	bool _GetMsg(MsgCenter::SMsg & sMsg);

	/**************************************************************************
	分发消息
	参数：
		sMsg		消息结构体
	返回：
		停止处理消息返回true，否则返回false。
	**************************************************************************/
	bool _DispatchMsg(const MsgCenter::SMsg & sMsg);

	/**************************************************************************
	输出提示
	参数：
		strTip		提示
		iErr		错误码
		strErr		错误串
	**************************************************************************/
	void _PutTip(string strTip, int iErr = 0, string strErr = "");

//消息响应
private:
	void _OnInitSoftware();
	void _OnAddTrayIcon();
	void _OnCloseBox();
	void _OnClose();
	void _OnProcessTrayIcon(unsigned int uiMsg);
	void _OnButtonStartProxy();
	void _OnSaveConfig();
	void _OnProcessStartupRun();

//内部数据
private:
	MsgCenter::PFNPutTip		m_pfnPutTip;		//输出提示回调函数
	void						* m_pThis;			//回调this指针

	CSocksProxyDlg		* m_pMainWnd;		//主窗口

	queue<MsgCenter::SMsg>		m_queueMsg;		//消息队列
	CCriticalSection			m_csMsg;		//消息队列互斥锁
	CEvent						m_eventMsg;		//消息队列事件

	bool		m_bClose;		//是否已经关闭
};