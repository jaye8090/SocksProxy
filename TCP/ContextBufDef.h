#pragma once

namespace ContextBuf
{

/******************************************************************************
�����붨��
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
��������
******************************************************************************/
typedef enum Const
{
	RECV_BUF_SIZE = 1024,		//���ջ�������С
}EConst;

/******************************************************************************
�������Ͷ���
******************************************************************************/
typedef enum OperateType
{
	OPERATE_ACCEPT,		//����
	OPERATE_CONN,		//����
	OPERATE_SEND,		//����
	OPERATE_RECV,		//����
}EOperateType;

/******************************************************************************
���ܻص���������
������
	socket			�׽��֣�����������ΪINVALID_SOCKET��
	ulRemoteIP		Զ��IP
	pThis			�ص�thisָ��
******************************************************************************/
typedef void (* PFNAccept)(SOCKET socket
	, unsigned long ulRemoteIP, void * pThis);

/******************************************************************************
���ӻص���������
������
	bSucc		�����Ƿ�ɹ�
	strErr		����
	pThis		�ص�thisָ��
******************************************************************************/
typedef void (* PFNConn)(bool bSucc, string strErr, void * pThis);

/******************************************************************************
���ջص���������
������
	pData			���ݣ����ӶϿ���ΪNULL��
	uiDataSize		���ݴ�С
	pThis			�ص�thisָ��
******************************************************************************/
typedef void (* PFNRecv)(void * pData, unsigned int uiDataSize, void * pThis);

/******************************************************************************
������
******************************************************************************/
typedef struct Context
{
	OVERLAPPED			overlap;			//�ص��ṹ
	EOperateType		eOperateType;		//��������
}SContext;

/******************************************************************************
����������
******************************************************************************/
typedef struct AcceptContext
	: SContext
{
	SOCKET			socket;			//�׽���
	void			* pBuf;			//������
	PFNAccept		pfnAccept;		//���ܻص�����
	void			* pThis;		//�ص�thisָ��
}SAcceptContext;

/******************************************************************************
����������
******************************************************************************/
typedef struct ConnContext
	: SContext
{
	PFNConn		pfnConn;		//���ӻص�����
	void		* pThis;		//�ص�thisָ��
}SConnContext;

/******************************************************************************
����������
******************************************************************************/
typedef struct RecvContext
	: SContext
{
	WSABUF		buf;			//������
	PFNRecv		pfnRecv;		//���ջص�����
	void		* pThis;		//�ص�thisָ��
}SRecvContext;

/******************************************************************************
����������
******************************************************************************/
typedef struct SendContext
	: SContext
{
	WSABUF		buf;		//������
}SSendContext;

/******************************************************************************
�����ʾ�ص���������
������
	strTip			��ʾ
	strClass		����
	iErr			������
	strErr			����
	pThis			�ص�thisָ��
******************************************************************************/
typedef void (* PFNPutTip)(string strTip, string strClass, int iErr
	, string strErr, void * pThis);

}