#pragma once

#include <string>
using namespace std;

#include "UserConnDef.h"
#include "ChannelDef.h"
#include "../Ptl/UserPtl.h"
#include "../TimeCnt.h"

class CSocksProxyDlg;
class CChannel;

/******************************************************************************
用户连接类
功能：
	处理和SOCKS用户的数据收发
作者：
	佳也 2623168833 jaye8090@qq.com
时间：
	2016-1-9 到 2016-1-9
******************************************************************************/
class CUserConn
{
public:
	CUserConn(UserConn::PFNPutTip pfnPutTip = NULL, void * pThis = NULL);

	~CUserConn(void);

	/**************************************************************************
	初始化
	说明：
		此接口是线程安全的
	参数：
		pMainWnd		主窗口
		pChannel		通道
		pBridge			桥接
		socket			套接字
	**************************************************************************/
	void Init(CSocksProxyDlg * pMainWnd, CChannel * pChannel
		, Channel::SBridge * pBridge, SOCKET socket);

//基本接口
public:
	/**************************************************************************
	连接响应
	说明：
		给用户的目标连接响应
		此接口是线程安全的
	**************************************************************************/
	void ConnResponse(void);

	/**************************************************************************
	通信接收
	说明：
		给用户的目标通信数据
		此接口是线程安全的
	参数：
		data		数据
	**************************************************************************/
	void CommRecv(CDataPack & data);

	/**************************************************************************
	检查活跃
	说明：
		检查连接是否活跃
		此接口是线程安全的
	**************************************************************************/
	void CheckAlive(void);

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
	接收回调函数
	**************************************************************************/
	static void _callbackRecv(void * pData, unsigned int uiDataSize
		, void * pThis);

	/**************************************************************************
	发送数据
	参数：
		data		数据
	**************************************************************************/
	void _SendData(CDataPack & data);

	/**************************************************************************
	处理接收
	参数：
		bDelConn		输出值。是否删除连接。
		pData			数据
		uiDataSize		数据大小
	**************************************************************************/
	void _ProcessRecv(bool & bDelConn, void * pData, unsigned int uiDataSize);

	/**************************************************************************
	处理删除连接
	参数：
		bDelConn		是否删除连接
	**************************************************************************/
	void _ProcessDelConn(bool bDelConn);

	/**************************************************************************
	处理SOCKS请求
	参数：
		pData			数据
		uiDataSize		数据大小
	**************************************************************************/
	void _ProcessSocksRequest(void * pData, unsigned int uiDataSize);

	/**************************************************************************
	获取SOCKS版本
	参数：
		pData			数据
		uiDataSize		数据大小
	返回：
		成功返回true，失败返回false。
	**************************************************************************/
	bool _GetSocksVer(void * pData, unsigned int uiDataSize);

	/**************************************************************************
	处理认证请求
	参数：
		pData			数据
		uiDataSize		数据大小
	**************************************************************************/
	void _ProcessAuthRequest(void * pData, unsigned int uiDataSize);

	/**************************************************************************
	处理SOCKS5连接请求
	参数：
		pData			数据
		uiDataSize		数据大小
	**************************************************************************/
	void _ProcessSocks5ConnRequest(void * pData, unsigned int uiDataSize);

	/**************************************************************************
	处理SOCKS4连接请求
	参数：
		pData			数据
		uiDataSize		数据大小
	**************************************************************************/
	void _ProcessSocks4ConnRequest(void * pData, unsigned int uiDataSize);

	/**************************************************************************
	处理通信发送
	参数：
		pData			数据
		uiDataSize		数据大小
	**************************************************************************/
	void _ProcessCommSend(void * pData, unsigned int uiDataSize);

	/**************************************************************************
	新建初始化目标连接
	参数：
		ulDstIP			目标IP
		usDstPort		目标端口
	**************************************************************************/
	void _NewInitDstConn(unsigned long ulDstIP, unsigned short usDstPort);

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
	UserConn::PFNPutTip		m_pfnPutTip;		//输出提示回调函数
	void					* m_pThis;			//回调this指针

	CSocksProxyDlg		* m_pMainWnd;		//主窗口

	CChannel				* m_pChannel;		//通道
	Channel::SBridge		* m_pBridge;		//桥接

	SOCKET		m_socket;		//套接字

	bool		m_bAuthRequestDone;		//认证请求是否完成
	bool		m_bConnRequestDone;		//连接请求是否完成

	CUserPtl		m_UserPtl;		//用户协议

	unsigned char		m_ucSocksVer;		//SOCKS版本

	CTimeCnt		m_AliveTimeCnt;		//活跃计时
};