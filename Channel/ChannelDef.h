#pragma once

class CUserConn;
class CDstConn;

namespace Channel
{

/******************************************************************************
�����붨��
******************************************************************************/
typedef enum ErrorCode
{
	INIT_BIND_PARAM = 1,
	INIT_SOCKET_INVALID,
	INIT_RESET_EVENT_FAIL,
	INIT_BEGIN_THREAD_FAIL,

	CLOSE_CLOSE_PARAM,
	CLOSE_SET_EVENT_FAIL,

	THREAD_CHECK_ALIVE_SET_EVENT_FAIL,

	CHECK_ALIVE_BRIDGE_NULL,

	CLOSE_ALL_BRIDGE_BRIDGE_NULL,

	DEL_USER_CONN_BRIDGE_PARAM,

	DEL_DST_CONN_BRIDGE_PARAM,

	DEL_BRIDGE_BRIDGE_NULL,
	DEL_BRIDGE_NOT_EXIST,
	DEL_BRIDGE_ERASE_EXCEPTION,

	NEW_BRIDGE_NEW_FAIL,
	NEW_BRIDGE_NEW_USER_CONN_FAIL,
	NEW_BRIDGE_INIT_USER_CONN_PARAM,
	NEW_BRIDGE_INSERT_SET_FAIL,
}EErrorCode;

/******************************************************************************
��������
******************************************************************************/
typedef enum Const
{
	CHECK_ALIVE_SPACE_TIME = 60 * 1000,		//����Ծ���ʱ��
}EConst;

/******************************************************************************
�Žӽṹ��
******************************************************************************/
typedef struct Bridge
{
	CCriticalSection		csLock;				//������
	CUserConn				* pUserConn;		//�û�����
	CDstConn				* pDstConn;			//Ŀ������
}SBridge;

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