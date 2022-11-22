#pragma once

namespace DstConn
{

/******************************************************************************
�����붨��
******************************************************************************/
typedef enum ErrorCode
{
	INIT_BRIDGE_NULL = 1,
	INIT_CONN_PARAM,
	INIT_BRIDGE_PARAM,

	COMM_SEND_BRIDGE_NULL,

	CLOSE_BRIDGE_NULL,
	CLOSE_MAIN_WND_NULL,

	SEND_DATA_SEND_PARAM,

	PROCESS_CONN_BRIDGE_NULL,
	PROCESS_CONN_MAIN_WND_NULL,
	PROCESS_CONN_USER_CONN_NULL,

	PROCESS_DEL_CONN_DEL_PARAM,

	PROCESS_RECV_BRIDGE_NULL,
	PROCESS_RECV_COMM_RECV_PARAM,
	PROCESS_RECV_MAIN_WND_NULL,
}EErrorCode;

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