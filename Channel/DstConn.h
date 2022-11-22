#pragma once

#include <string>
using namespace std;

#include "DstConnDef.h"
#include "ChannelDef.h"

class CSocksProxyDlg;
class CChannel;

/******************************************************************************
Ŀ��������
���ܣ�
	�����Ŀ��������շ�
���ߣ�
	��Ҳ 2623168833 jaye8090@qq.com
ʱ�䣺
	2016-2-3 �� 2016-2-4
******************************************************************************/
class CDstConn
{
public:
	CDstConn(DstConn::PFNPutTip pfnPutTip = NULL, void * pThis = NULL);

	~CDstConn(void);

	/**************************************************************************
	��ʼ��
	˵����
		�˽ӿ����̰߳�ȫ��
	������
		pMainWnd		������
		pChannel		ͨ��
		pBridge			�Ž�
		sUserConn		�û������׽���
		ulDstIP			Ŀ��IP
		usDstPort		Ŀ��˿�
	**************************************************************************/
	void Init(CSocksProxyDlg * pMainWnd, CChannel * pChannel
		, Channel::SBridge * pBridge, SOCKET sUserConn
		, unsigned long ulDstIP, unsigned short usDstPort);

//�����ӿ�
public:
	/**************************************************************************
	ͨ�ŷ���
	˵����
		��Ŀ�귢��ͨ������
		�˽ӿ����̰߳�ȫ��
	������
		pData			����
		uiDataSize		���ݴ�С
	**************************************************************************/
	void CommSend(void * pData, unsigned int uiDataSize);

	/**************************************************************************
	�ر�
	˵����
		�ر��׽��֣��Ͽ����ӡ�
		�˽ӿ����̰߳�ȫ��
	**************************************************************************/
	void Close(void);

//�ڲ�����
private:
	/**************************************************************************
	���ӻص�����
	**************************************************************************/
	static void _callbackConn(bool bSucc, string strErr, void * pThis);

	/**************************************************************************
	���ջص�����
	**************************************************************************/
	static void _callbackRecv(void * pData, unsigned int uiDataSize
		, void * pThis);

	/**************************************************************************
	��������
	������
		pData			����
		uiDataSize		���ݴ�С
	**************************************************************************/
	void _SendData(void * pData, unsigned int uiDataSize);

	/**************************************************************************
	��������
	������
		bDelConn		���ֵ���Ƿ�ɾ�����ӡ�
		bSucc			�����Ƿ�ɹ�
		strErr			����
	**************************************************************************/
	void _ProcessConn(bool & bDelConn, bool bSucc, string strErr);

	/**************************************************************************
	����ɾ������
	������
		bDelConn		�Ƿ�ɾ������
	**************************************************************************/
	void _ProcessDelConn(bool bDelConn);

	/**************************************************************************
	�������
	������
		bDelConn		���ֵ���Ƿ�ɾ�����ӡ�
		pData			����
		uiDataSize		���ݴ�С
	**************************************************************************/
	void _ProcessRecv(bool & bDelConn, void * pData, unsigned int uiDataSize);

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
	DstConn::PFNPutTip		m_pfnPutTip;		//�����ʾ�ص�����
	void					* m_pThis;			//�ص�thisָ��

	CSocksProxyDlg		* m_pMainWnd;		//������

	CChannel				* m_pChannel;		//ͨ��
	Channel::SBridge		* m_pBridge;		//�Ž�

	SOCKET		m_socket;		//�׽���
};