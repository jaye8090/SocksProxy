#pragma once

#include <string>
#include <map>
using namespace std;

#include "BridgeCtrlDef.h"

/******************************************************************************
�Žӿ�����
���ܣ�
	��ӡ�ɾ���Žӣ��ṩ�Žӿ��ƹ��ܡ�
���ߣ�
	��Ҳ 2623168833 jaye8090@qq.com
ʱ�䣺
	2016-8-2 �� 2016-8-8
******************************************************************************/
class CBridgeCtrl
{
public:
	CBridgeCtrl(BridgeCtrl::PFNPutTip pfnPutTip = NULL, void * pThis = NULL);

	~CBridgeCtrl(void);

	/**************************************************************************
	��ʼ��
	**************************************************************************/
	void Init(void);

//�����ӿ�
public:
	/**************************************************************************
	����Ž�
	������
		socket1		�׽���1
		socket2		�׽���2
	**************************************************************************/
	void AddBridge(SOCKET socket1, SOCKET socket2);

	/**************************************************************************
	ɾ���Ž�
	������
		socket		�׽���
	**************************************************************************/
	void DelBridge(SOCKET socket);

	/**************************************************************************
	���ƽ���
	������
		sSend				�����׽���
		pSendContext		����������
	**************************************************************************/
	void LimitRecv(SOCKET sSend, ContextBuf::SSendContext * pSendContext);

	/**************************************************************************
	��ͣ����
	������
		sRecv				�����׽���
		pRecvContext		����������
	���أ�
		��ͣ��Ч����true�����򷵻�false��
	**************************************************************************/
	bool PauseRecv(SOCKET sRecv, ContextBuf::SRecvContext * pRecvContext);

	/**************************************************************************
	�ָ�����
	������
		sRecv				���ֵ�������׽��֡�
		pRecvContext		���ֵ�����������ġ�
		sSend				�����׽���
		pSendContext		����������
	���أ�
		�ָ���Ч����true�����򷵻�false��
	**************************************************************************/
	bool ResumeRecv(SOCKET & sRecv, ContextBuf::SRecvContext * & pRecvContext
		, SOCKET sSend, ContextBuf::SSendContext * pSendContext);

//�ڲ�����
private:
	/**************************************************************************
	���½�����Ŀ��
	������
		pSendContextOld		�Ϸ���������
		pSendContextNew		�·���������
	**************************************************************************/
	void _UpdateRecvItemKey(ContextBuf::SSendContext * pSendContextOld
		, ContextBuf::SSendContext * pSendContextNew);

	/**************************************************************************
	�Ƿ��Ѿ��Ž�
	������
		socket		�׽���
	���أ�
		�Ѿ��Žӷ���true�����򷵻�false��
	**************************************************************************/
	bool _IsBridged(SOCKET socket);

	/**************************************************************************
	��ȡ����������
	������
		sRecv		�����׽���
	���أ�
		�Ѿ��ŽӲ����ƣ��򷵻ط��������ģ����򷵻�NULL��
	**************************************************************************/
	ContextBuf::SSendContext * _GetSendContext(SOCKET sRecv);

	/**************************************************************************
	ȡ������
	������
		sSend				�����׽���
		pSendContext		����������
	**************************************************************************/
	void _CancelLimit(SOCKET sSend, ContextBuf::SSendContext * pSendContext);

	/**************************************************************************
	�����ʾ
	������
		strTip		��ʾ
		iErr		������
		strErr		����
	**************************************************************************/
	void _PutTip(string strTip, int iErr = 0, string strErr = "");

//�ڲ�����
private:
	BridgeCtrl::PFNPutTip		m_pfnPutTip;		//�����ʾ�ص�����
	void						* m_pThis;			//�ص�thisָ��

	map<SOCKET, SOCKET>							m_mapBridge;		//�Žӱ�
	map<SOCKET, ContextBuf::SSendContext *>		m_mapLimit;			//���Ʊ�

	map<ContextBuf::SSendContext *
		, BridgeCtrl::SRecvItem *>		m_mapRecvItem;		//������Ŀ��
};