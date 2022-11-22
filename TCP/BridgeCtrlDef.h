#pragma once

#include "ContextBufDef.h"

namespace BridgeCtrl
{

/******************************************************************************
�����붨��
******************************************************************************/
typedef enum ErrorCode
{
	ADD_BRIDGE_SOCKET_INVALID = 1,
	ADD_BRIDGE_INSERT_BRIDGE_FAIL1,
	ADD_BRIDGE_INSERT_BRIDGE_FAIL2,

	DEL_BRIDGE_SOCKET_INVALID1,
	DEL_BRIDGE_SOCKET_INVALID2,
	DEL_BRIDGE_NOT_EXIST,
	DEL_BRIDGE_ERASE_EXCEPTION,

	LIMIT_RECV_INSERT_LIMIT_PARAM,

	PAUSE_RECV_ALLOC_RECV_ITEM_FAIL,
	PAUSE_RECV_INSERT_RECV_ITEM_FAIL,

	RESUME_RECV_SEND_CONTEXT_NULL,
	RESUME_RECV_RECV_ITEM_NULL,
	RESUME_RECV_ERASE_EXCEPTION,

	UPDATE_RECV_ITEM_KEY_FIND_MAP_PARAM,
	UPDATE_RECV_ITEM_KEY_ERASE_EXCEPTION,
	UPDATE_RECV_ITEM_KEY_INSERT_MAP_PARAM,
	UPDATE_RECV_ITEM_KEY_INSERT_MAP_FAIL,

	IS_BRIDGED_SOCKET_INVALID,

	GET_SEND_CONTEXT_RECV_SOCKET_INVALID,
	GET_SEND_CONTEXT_SEND_SOCKET_INVALID,
	GET_SEND_CONTEXT_SEND_CONTEXT_NULL,

	CALCEL_LIMIT_SOCKET_INVALID,
	CALCEL_LIMIT_ERASE_EXCEPTION,
}EErrorCode;

/******************************************************************************
������Ŀ
******************************************************************************/
typedef struct RecvItem
{
	SOCKET							sRecv;				//�����׽���
	ContextBuf::SRecvContext		* pRecvContext;		//����������
}SRecvItem;

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