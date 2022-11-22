#pragma once

#include <mswsock.h>
#include <string>
#include <map>
using namespace std;

#include "TCPDef.h"
#include "SocketBuf.h"
#include "ContextBuf.h"
#include "BridgeCtrl.h"

/******************************************************************************
TCP类
功能：
	用IOCP技术实现的异步TCP类，支持客户端和服务器的常用功能。
作者：
	佳也 2623168833 jaye8090@qq.com
时间：
	2015-10-29 到 2016-10-21
******************************************************************************/
class CTCP
{
public:
	CTCP(TCP::PFNPutTip pfnPutTip = NULL, void * pThis = NULL);

	~CTCP(void);

	/**************************************************************************
	初始化
	**************************************************************************/
	void Init(void);

//基本接口
public:
	/**************************************************************************
	绑定
	说明：
		绑定端口
		此接口是线程安全的
	参数：
		socket			输出值。套接字。
		usLocalPort		本地端口
	返回：
		绑定端口成功返回true，否则返回false。
	**************************************************************************/
	bool Bind(SOCKET & socket, unsigned short usLocalPort);

	/**************************************************************************
	绑定
	说明：
		绑定端口
		此接口是线程安全的
	参数：
		socket			输出值。套接字。
		ulLocalIP		本地IP，INADDR_ANY表示不指定本地IP。
		usLocalPort		本地端口
	返回：
		绑定端口成功返回true，否则返回false。
	**************************************************************************/
	bool Bind(SOCKET & socket, unsigned long ulLocalIP
		, unsigned short usLocalPort);

	/**************************************************************************
	监听
	说明：
		监听套接字，并指定接受连接的回调函数。
		此接口是线程安全的
	参数：
		socket			套接字
		pfnAccept		接受回调函数
		pThis			回调this指针
	**************************************************************************/
	void Listen(SOCKET socket, ContextBuf::PFNAccept pfnAccept, void * pThis);

	/**************************************************************************
	接收
	说明：
		接收数据，并指定接收数据的回调函数。
		接受到连接、连接成功后，必须调用此接口才会开始接收数据。
		此接口是线程安全的
	参数：
		socket		套接字
		pfnRecv		接收回调函数
		pThis		回调this指针
	**************************************************************************/
	void Recv(SOCKET socket, ContextBuf::PFNRecv pfnRecv, void * pThis);

	/**************************************************************************
	连接
	说明：
		创建连接，并指定连接通知的回调函数。
		此接口是线程安全的
	参数：
		socket				输出值。套接字。
		ulRemoteIP			远程IP
		usRemotePort		远程端口
		pfnConn				连接回调函数
		pThis				回调this指针
	**************************************************************************/
	void Conn(SOCKET & socket
		, unsigned long ulRemoteIP, unsigned short usRemotePort
		, ContextBuf::PFNConn pfnConn, void * pThis);

	/**************************************************************************
	连接
	说明：
		创建连接，并指定连接通知的回调函数。
		此接口是线程安全的
	参数：
		socket				输出值。套接字。
		ulLocalIP			本地IP，INADDR_ANY表示不指定本地IP。
		ulRemoteIP			远程IP
		usRemotePort		远程端口
		pfnConn				连接回调函数
		pThis				回调this指针
	**************************************************************************/
	void Conn(SOCKET & socket, unsigned long ulLocalIP
		, unsigned long ulRemoteIP, unsigned short usRemotePort
		, ContextBuf::PFNConn pfnConn, void * pThis);

	/**************************************************************************
	发送
	说明：
		发送数据
		此接口是线程安全的
	参数：
		socket			套接字
		pData			数据
		uiDataSize		数据大小
	**************************************************************************/
	void Send(SOCKET socket, void * pData, unsigned int uiDataSize);

	/**************************************************************************
	桥接
	说明：
		桥接两个套接字，让这两个连接的网速同步。
		此接口是线程安全的
	参数：
		socket1		套接字1
		socket2		套接字2
	**************************************************************************/
	void Bridge(SOCKET socket1, SOCKET socket2);

	/**************************************************************************
	关闭
	说明：
		关闭套接字，可停止监听端口、断开连接。
		连接失败、连接断开后，必须调用此接口关闭套接字。
		此接口是线程安全的
	参数：
		socket		套接字
	**************************************************************************/
	void Close(SOCKET socket);

	/**************************************************************************
	停止
	说明：
		停止所有端口的监听、断开所有连接、清空所有资源。
	**************************************************************************/
	void Stop(void);

//内部操作
private:
	/**************************************************************************
	启动并创建IOCP
	**************************************************************************/
	void _StartupCreateIOCP(void);

	/**************************************************************************
	启动IOCP线程
	**************************************************************************/
	void _StartupIOCPThread(void);

	/**************************************************************************
	加载扩展函数
	**************************************************************************/
	void _LoadExFunc(void);

	/**************************************************************************
	设置套接字有效
	参数：
		socket			套接字
		pContext		上下文
	**************************************************************************/
	void _SetSocketValid(SOCKET socket, ContextBuf::SContext * pContext);

	/**************************************************************************
	投递接受请求
	参数：
		socket				套接字
		pAcceptContext		接受上下文
	**************************************************************************/
	void _PostAccept(SOCKET socket
		, ContextBuf::SAcceptContext * pAcceptContext);

	/**************************************************************************
	投递接收请求
	参数：
		socket				套接字
		pRecvContext		接收上下文
	**************************************************************************/
	void _PostRecv(SOCKET socket, ContextBuf::SRecvContext * pRecvContext);

	/**************************************************************************
	投递连接请求
	参数：
		socket				套接字
		addrRemote			远程地址信息
		pConnContext		连接上下文
	**************************************************************************/
	void _PostConn(SOCKET socket, sockaddr_in & addrRemote
		, ContextBuf::SConnContext * pConnContext);

	/**************************************************************************
	投递发送请求
	参数：
		socket				套接字
		pSendContext		发送上下文
	返回：
		成功返回true，失败返回false。
	**************************************************************************/
	bool _PostSend(SOCKET socket, ContextBuf::SSendContext * pSendContext);

	/**************************************************************************
	退出IOCP线程
	**************************************************************************/
	void _ExitIOCPThread(void);

	/**************************************************************************
	清理网络
	**************************************************************************/
	void _CleanupNet(void);

	/**************************************************************************
	IOCP工作线程
	**************************************************************************/
	static UINT _threadIOCPWork(LPVOID pParam);

	/**************************************************************************
	IOCP工作
	**************************************************************************/
	void _IOCPWork(void);

	/**************************************************************************
	退出IOCP工作
	**************************************************************************/
	void _ExitIOCPWork(void);

	/**************************************************************************
	获取IO通知
	参数：
		dwErr			输出值。错误码。
		bIOSucc			输出值。IO是否成功。
		dwBytes			输出值。字节数。
		socket			输出值。套接字。
		pContext		输出值。上下文。
	**************************************************************************/
	void _GetIONotice(DWORD & dwErr, BOOL & bIOSucc, DWORD & dwBytes
		, SOCKET & socket, ContextBuf::SContext * & pContext);

	/**************************************************************************
	是否退出
	参数：
		bIOSucc			IO是否成功
		dwBytes			字节数
		socket			套接字
		pContext		上下文
	返回：
		退出返回true，否则返回false。
	**************************************************************************/
	bool _IsExit(BOOL bIOSucc, DWORD dwBytes
		, SOCKET socket, ContextBuf::SContext * pContext);

	/**************************************************************************
	是否接受取消
	参数：
		dwErr			错误码
		bIOSucc			IO是否成功
		dwBytes			字节数
		socket			套接字
		pContext		上下文
	返回：
		取消返回true，否则返回false。
	**************************************************************************/
	bool _IsAcceptCancel(DWORD dwErr, BOOL bIOSucc, DWORD dwBytes
		, SOCKET socket, ContextBuf::SContext * pContext);

	/**************************************************************************
	处理接受取消
	参数：
		bIOSucc				IO是否成功
		pAcceptContext		接受上下文
	**************************************************************************/
	void _ProcessAcceptCancel(BOOL bIOSucc
		, ContextBuf::SAcceptContext * pAcceptContext);

	/**************************************************************************
	是否断开
	参数：
		dwErr			错误码
		bIOSucc			IO是否成功
		dwBytes			字节数
		socket			套接字
		pContext		上下文
	返回：
		断开返回true，否则返回false。
	**************************************************************************/
	bool _IsDiscon(DWORD dwErr, BOOL bIOSucc, DWORD dwBytes
		, SOCKET socket, ContextBuf::SContext * pContext);

	/**************************************************************************
	处理断开
	参数：
		pRecvContext		接收上下文
	**************************************************************************/
	void _ProcessDiscon(ContextBuf::SRecvContext * pRecvContext);

	/**************************************************************************
	处理IO通知
	参数：
		dwErr			错误码
		bIOSucc			IO是否成功
		dwBytes			字节数
		socket			套接字
		pContext		上下文
	**************************************************************************/
	void _ProcessIONotice(DWORD dwErr, BOOL bIOSucc, DWORD dwBytes
		, SOCKET socket, ContextBuf::SContext * pContext);

	/**************************************************************************
	处理接受通知
	参数：
		dwErr				错误码
		bIOSucc				IO是否成功
		socket				套接字
		pAcceptContext		接受上下文
	**************************************************************************/
	void _ProcessAcceptNotice(DWORD dwErr, BOOL bIOSucc, SOCKET socket
		, ContextBuf::SAcceptContext * pAcceptContext);

	/**************************************************************************
	处理连接通知
	参数：
		dwErr				错误码
		bIOSucc				IO是否成功
		pConnContext		连接上下文
	**************************************************************************/
	void _ProcessConnNotice(DWORD dwErr, BOOL bIOSucc
		, ContextBuf::SConnContext * pConnContext);

	/**************************************************************************
	处理发送通知
	参数：
		dwErr				错误码
		bIOSucc				IO是否成功
		dwBytes				字节数
		socket				套接字
		pSendContext		发送上下文
	**************************************************************************/
	void _ProcessSendNotice(DWORD dwErr, BOOL bIOSucc, DWORD dwBytes
		, SOCKET socket, ContextBuf::SSendContext * pSendContext);

	/**************************************************************************
	处理接收通知
	参数：
		dwErr				错误码
		bIOSucc				IO是否成功
		dwBytes				字节数
		socket				套接字
		pRecvContext		接收上下文
	**************************************************************************/
	void _ProcessRecvNotice(DWORD dwErr, BOOL bIOSucc, DWORD dwBytes
		, SOCKET socket, ContextBuf::SRecvContext * pRecvContext);

	/**************************************************************************
	继续投递接受请求
	参数：
		socket				套接字
		pAcceptContext		接受上下文
	**************************************************************************/
	void _PostAcceptContinue(SOCKET socket
		, ContextBuf::SAcceptContext * pAcceptContext);

	/**************************************************************************
	恢复接收
	参数：
		sSend				发送套接字
		pSendContext		发送上下文
	**************************************************************************/
	void _ResumeRecv(SOCKET sSend, ContextBuf::SSendContext * pSendContext);

	/**************************************************************************
	继续投递接收请求
	参数：
		socket				套接字
		pRecvContext		接收上下文
	**************************************************************************/
	void _PostRecvContinue(SOCKET socket
		, ContextBuf::SRecvContext * pRecvContext);

	/**************************************************************************
	套接字是否有效
	参数：
		socket			套接字
		pContext		上下文
	返回：
		有效返回true，否则返回false。
	**************************************************************************/
	bool _IsSocketValid(SOCKET socket, ContextBuf::SContext * pContext);

	/**************************************************************************
	投递接受取消通知
	参数：
		pAcceptContext		接受上下文
	**************************************************************************/
	void _PostAcceptCancel(ContextBuf::SAcceptContext * pAcceptContext);

	/**************************************************************************
	投递断开通知
	参数：
		pRecvContext		接收上下文
	**************************************************************************/
	void _PostDiscon(ContextBuf::SRecvContext * pRecvContext);

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
	TCP::PFNPutTip		m_pfnPutTip;		//输出提示回调函数
	void				* m_pThis;			//回调this指针

	CCriticalSection		m_csLock;		//互斥锁

	CSocketBuf		m_SocketBuf;		//套接字缓冲区
	CContextBuf		m_ContextBuf;		//上下文缓冲区

	CBridgeCtrl		m_BridgeCtrl;		//桥接控制

	HANDLE		m_hIOCP;		//IOCP

	unsigned int		m_uiThreadCnt;		//线程数
	CSemaphore			* m_pThreadSem;		//线程信号量

	LPFN_ACCEPTEX		m_pfnAcceptEx;		//接受函数
	LPFN_CONNECTEX		m_pfnConnectEx;		//连接函数
	//获取地址函数
	LPFN_GETACCEPTEXSOCKADDRS		m_pfnGetAcceptExSockaddrs;

	//套接字有效表
	map<SOCKET, ContextBuf::SContext *>		m_mapSocketValid;
};