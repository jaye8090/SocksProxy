#pragma once

#include <string>
using namespace std;

#include "DstConnDef.h"
#include "ChannelDef.h"

class CSocksProxyDlg;
class CChannel;

/******************************************************************************
目标连接类
功能：
	处理和目标的数据收发
作者：
	佳也 2623168833 jaye8090@qq.com
时间：
	2016-2-3 到 2016-2-4
******************************************************************************/
class CDstConn
{
public:
	CDstConn(DstConn::PFNPutTip pfnPutTip = NULL, void * pThis = NULL);

	~CDstConn(void);

	/**************************************************************************
	初始化
	说明：
		此接口是线程安全的
	参数：
		pMainWnd		主窗口
		pChannel		通道
		pBridge			桥接
		sUserConn		用户连接套接字
		ulDstIP			目标IP
		usDstPort		目标端口
	**************************************************************************/
	void Init(CSocksProxyDlg * pMainWnd, CChannel * pChannel
		, Channel::SBridge * pBridge, SOCKET sUserConn
		, unsigned long ulDstIP, unsigned short usDstPort);

//基本接口
public:
	/**************************************************************************
	通信发送
	说明：
		向目标发送通信数据
		此接口是线程安全的
	参数：
		pData			数据
		uiDataSize		数据大小
	**************************************************************************/
	void CommSend(void * pData, unsigned int uiDataSize);

	/**************************************************************************
	关闭
	说明：
		关闭套接字，断开连接。
		此接口是线程安全的
	**************************************************************************/
	void Close(void);

//内部操作
private:
	/**************************************************************************
	连接回调函数
	**************************************************************************/
	static void _callbackConn(bool bSucc, string strErr, void * pThis);

	/**************************************************************************
	接收回调函数
	**************************************************************************/
	static void _callbackRecv(void * pData, unsigned int uiDataSize
		, void * pThis);

	/**************************************************************************
	发送数据
	参数：
		pData			数据
		uiDataSize		数据大小
	**************************************************************************/
	void _SendData(void * pData, unsigned int uiDataSize);

	/**************************************************************************
	处理连接
	参数：
		bDelConn		输出值。是否删除连接。
		bSucc			连接是否成功
		strErr			错误串
	**************************************************************************/
	void _ProcessConn(bool & bDelConn, bool bSucc, string strErr);

	/**************************************************************************
	处理删除连接
	参数：
		bDelConn		是否删除连接
	**************************************************************************/
	void _ProcessDelConn(bool bDelConn);

	/**************************************************************************
	处理接收
	参数：
		bDelConn		输出值。是否删除连接。
		pData			数据
		uiDataSize		数据大小
	**************************************************************************/
	void _ProcessRecv(bool & bDelConn, void * pData, unsigned int uiDataSize);

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
	DstConn::PFNPutTip		m_pfnPutTip;		//输出提示回调函数
	void					* m_pThis;			//回调this指针

	CSocksProxyDlg		* m_pMainWnd;		//主窗口

	CChannel				* m_pChannel;		//通道
	Channel::SBridge		* m_pBridge;		//桥接

	SOCKET		m_socket;		//套接字
};