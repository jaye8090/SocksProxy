#pragma once

namespace ContextBuf
{

/******************************************************************************
错误码定义
******************************************************************************/
typedef enum ErrorCode
{
	ALLOC_ACCEPT_ALLOC_CONTEXT_FAIL = 1,
	ALLOC_ACCEPT_ALLOC_BUF_FAIL,
	ALLOC_ACCEPT_INSERT_SET_FAIL,

	ALLOC_CONN_ALLOC_CONTEXT_FAIL,
	ALLOC_CONN_INSERT_SET_FAIL,

	ALLOC_RECV_ALLOC_CONTEXT_FAIL,
	ALLOC_RECV_ALLOC_BUF_FAIL,
	ALLOC_RECV_INSERT_SET_FAIL,

	ALLOC_SEND_ALLOC_CONTEXT_FAIL,
	ALLOC_SEND_DATA_SIZE_INVALID,
	ALLOC_SEND_ALLOC_BUF_FAIL,
	ALLOC_SEND_DATA_NULL,
	ALLOC_SEND_INSERT_SET_FAIL,

	FREE_CONTEXT_NULL,
	FREE_CONTEXT_NOT_EXIST,
	FREE_ERASE_EXCEPTION,

	CLEAR_CONTEXT_NULL,
	CLEAR_ERASE_EXCEPTION,

	FREE_BUF_CONTEXT_NULL,
}EErrorCode;

/******************************************************************************
常量定义
******************************************************************************/
typedef enum Const
{
	RECV_BUF_SIZE = 1024,		//接收缓冲区大小
}EConst;

/******************************************************************************
操作类型定义
******************************************************************************/
typedef enum OperateType
{
	OPERATE_ACCEPT,		//接受
	OPERATE_CONN,		//连接
	OPERATE_SEND,		//发送
	OPERATE_RECV,		//接收
}EOperateType;

/******************************************************************************
接受回调函数定义
参数：
	socket			套接字，监听结束则为INVALID_SOCKET。
	ulRemoteIP		远程IP
	pThis			回调this指针
******************************************************************************/
typedef void (* PFNAccept)(SOCKET socket
	, unsigned long ulRemoteIP, void * pThis);

/******************************************************************************
连接回调函数定义
参数：
	bSucc		连接是否成功
	strErr		错误串
	pThis		回调this指针
******************************************************************************/
typedef void (* PFNConn)(bool bSucc, string strErr, void * pThis);

/******************************************************************************
接收回调函数定义
参数：
	pData			数据，连接断开则为NULL。
	uiDataSize		数据大小
	pThis			回调this指针
******************************************************************************/
typedef void (* PFNRecv)(void * pData, unsigned int uiDataSize, void * pThis);

/******************************************************************************
上下文
******************************************************************************/
typedef struct Context
{
	OVERLAPPED			overlap;			//重叠结构
	EOperateType		eOperateType;		//操作类型
}SContext;

/******************************************************************************
接受上下文
******************************************************************************/
typedef struct AcceptContext
	: SContext
{
	SOCKET			socket;			//套接字
	void			* pBuf;			//缓冲区
	PFNAccept		pfnAccept;		//接受回调函数
	void			* pThis;		//回调this指针
}SAcceptContext;

/******************************************************************************
连接上下文
******************************************************************************/
typedef struct ConnContext
	: SContext
{
	PFNConn		pfnConn;		//连接回调函数
	void		* pThis;		//回调this指针
}SConnContext;

/******************************************************************************
接收上下文
******************************************************************************/
typedef struct RecvContext
	: SContext
{
	WSABUF		buf;			//缓冲区
	PFNRecv		pfnRecv;		//接收回调函数
	void		* pThis;		//回调this指针
}SRecvContext;

/******************************************************************************
发送上下文
******************************************************************************/
typedef struct SendContext
	: SContext
{
	WSABUF		buf;		//缓冲区
}SSendContext;

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