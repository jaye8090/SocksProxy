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
TCP��
���ܣ�
	��IOCP����ʵ�ֵ��첽TCP�֧࣬�ֿͻ��˺ͷ������ĳ��ù��ܡ�
���ߣ�
	��Ҳ 2623168833 jaye8090@qq.com
ʱ�䣺
	2015-10-29 �� 2016-10-21
******************************************************************************/
class CTCP
{
public:
	CTCP(TCP::PFNPutTip pfnPutTip = NULL, void * pThis = NULL);

	~CTCP(void);

	/**************************************************************************
	��ʼ��
	**************************************************************************/
	void Init(void);

//�����ӿ�
public:
	/**************************************************************************
	��
	˵����
		�󶨶˿�
		�˽ӿ����̰߳�ȫ��
	������
		socket			���ֵ���׽��֡�
		usLocalPort		���ض˿�
	���أ�
		�󶨶˿ڳɹ�����true�����򷵻�false��
	**************************************************************************/
	bool Bind(SOCKET & socket, unsigned short usLocalPort);

	/**************************************************************************
	��
	˵����
		�󶨶˿�
		�˽ӿ����̰߳�ȫ��
	������
		socket			���ֵ���׽��֡�
		ulLocalIP		����IP��INADDR_ANY��ʾ��ָ������IP��
		usLocalPort		���ض˿�
	���أ�
		�󶨶˿ڳɹ�����true�����򷵻�false��
	**************************************************************************/
	bool Bind(SOCKET & socket, unsigned long ulLocalIP
		, unsigned short usLocalPort);

	/**************************************************************************
	����
	˵����
		�����׽��֣���ָ���������ӵĻص�������
		�˽ӿ����̰߳�ȫ��
	������
		socket			�׽���
		pfnAccept		���ܻص�����
		pThis			�ص�thisָ��
	**************************************************************************/
	void Listen(SOCKET socket, ContextBuf::PFNAccept pfnAccept, void * pThis);

	/**************************************************************************
	����
	˵����
		�������ݣ���ָ���������ݵĻص�������
		���ܵ����ӡ����ӳɹ��󣬱�����ô˽ӿڲŻῪʼ�������ݡ�
		�˽ӿ����̰߳�ȫ��
	������
		socket		�׽���
		pfnRecv		���ջص�����
		pThis		�ص�thisָ��
	**************************************************************************/
	void Recv(SOCKET socket, ContextBuf::PFNRecv pfnRecv, void * pThis);

	/**************************************************************************
	����
	˵����
		�������ӣ���ָ������֪ͨ�Ļص�������
		�˽ӿ����̰߳�ȫ��
	������
		socket				���ֵ���׽��֡�
		ulRemoteIP			Զ��IP
		usRemotePort		Զ�̶˿�
		pfnConn				���ӻص�����
		pThis				�ص�thisָ��
	**************************************************************************/
	void Conn(SOCKET & socket
		, unsigned long ulRemoteIP, unsigned short usRemotePort
		, ContextBuf::PFNConn pfnConn, void * pThis);

	/**************************************************************************
	����
	˵����
		�������ӣ���ָ������֪ͨ�Ļص�������
		�˽ӿ����̰߳�ȫ��
	������
		socket				���ֵ���׽��֡�
		ulLocalIP			����IP��INADDR_ANY��ʾ��ָ������IP��
		ulRemoteIP			Զ��IP
		usRemotePort		Զ�̶˿�
		pfnConn				���ӻص�����
		pThis				�ص�thisָ��
	**************************************************************************/
	void Conn(SOCKET & socket, unsigned long ulLocalIP
		, unsigned long ulRemoteIP, unsigned short usRemotePort
		, ContextBuf::PFNConn pfnConn, void * pThis);

	/**************************************************************************
	����
	˵����
		��������
		�˽ӿ����̰߳�ȫ��
	������
		socket			�׽���
		pData			����
		uiDataSize		���ݴ�С
	**************************************************************************/
	void Send(SOCKET socket, void * pData, unsigned int uiDataSize);

	/**************************************************************************
	�Ž�
	˵����
		�Ž������׽��֣������������ӵ�����ͬ����
		�˽ӿ����̰߳�ȫ��
	������
		socket1		�׽���1
		socket2		�׽���2
	**************************************************************************/
	void Bridge(SOCKET socket1, SOCKET socket2);

	/**************************************************************************
	�ر�
	˵����
		�ر��׽��֣���ֹͣ�����˿ڡ��Ͽ����ӡ�
		����ʧ�ܡ����ӶϿ��󣬱�����ô˽ӿڹر��׽��֡�
		�˽ӿ����̰߳�ȫ��
	������
		socket		�׽���
	**************************************************************************/
	void Close(SOCKET socket);

	/**************************************************************************
	ֹͣ
	˵����
		ֹͣ���ж˿ڵļ������Ͽ��������ӡ����������Դ��
	**************************************************************************/
	void Stop(void);

//�ڲ�����
private:
	/**************************************************************************
	����������IOCP
	**************************************************************************/
	void _StartupCreateIOCP(void);

	/**************************************************************************
	����IOCP�߳�
	**************************************************************************/
	void _StartupIOCPThread(void);

	/**************************************************************************
	������չ����
	**************************************************************************/
	void _LoadExFunc(void);

	/**************************************************************************
	�����׽�����Ч
	������
		socket			�׽���
		pContext		������
	**************************************************************************/
	void _SetSocketValid(SOCKET socket, ContextBuf::SContext * pContext);

	/**************************************************************************
	Ͷ�ݽ�������
	������
		socket				�׽���
		pAcceptContext		����������
	**************************************************************************/
	void _PostAccept(SOCKET socket
		, ContextBuf::SAcceptContext * pAcceptContext);

	/**************************************************************************
	Ͷ�ݽ�������
	������
		socket				�׽���
		pRecvContext		����������
	**************************************************************************/
	void _PostRecv(SOCKET socket, ContextBuf::SRecvContext * pRecvContext);

	/**************************************************************************
	Ͷ����������
	������
		socket				�׽���
		addrRemote			Զ�̵�ַ��Ϣ
		pConnContext		����������
	**************************************************************************/
	void _PostConn(SOCKET socket, sockaddr_in & addrRemote
		, ContextBuf::SConnContext * pConnContext);

	/**************************************************************************
	Ͷ�ݷ�������
	������
		socket				�׽���
		pSendContext		����������
	���أ�
		�ɹ�����true��ʧ�ܷ���false��
	**************************************************************************/
	bool _PostSend(SOCKET socket, ContextBuf::SSendContext * pSendContext);

	/**************************************************************************
	�˳�IOCP�߳�
	**************************************************************************/
	void _ExitIOCPThread(void);

	/**************************************************************************
	��������
	**************************************************************************/
	void _CleanupNet(void);

	/**************************************************************************
	IOCP�����߳�
	**************************************************************************/
	static UINT _threadIOCPWork(LPVOID pParam);

	/**************************************************************************
	IOCP����
	**************************************************************************/
	void _IOCPWork(void);

	/**************************************************************************
	�˳�IOCP����
	**************************************************************************/
	void _ExitIOCPWork(void);

	/**************************************************************************
	��ȡIO֪ͨ
	������
		dwErr			���ֵ�������롣
		bIOSucc			���ֵ��IO�Ƿ�ɹ���
		dwBytes			���ֵ���ֽ�����
		socket			���ֵ���׽��֡�
		pContext		���ֵ�������ġ�
	**************************************************************************/
	void _GetIONotice(DWORD & dwErr, BOOL & bIOSucc, DWORD & dwBytes
		, SOCKET & socket, ContextBuf::SContext * & pContext);

	/**************************************************************************
	�Ƿ��˳�
	������
		bIOSucc			IO�Ƿ�ɹ�
		dwBytes			�ֽ���
		socket			�׽���
		pContext		������
	���أ�
		�˳�����true�����򷵻�false��
	**************************************************************************/
	bool _IsExit(BOOL bIOSucc, DWORD dwBytes
		, SOCKET socket, ContextBuf::SContext * pContext);

	/**************************************************************************
	�Ƿ����ȡ��
	������
		dwErr			������
		bIOSucc			IO�Ƿ�ɹ�
		dwBytes			�ֽ���
		socket			�׽���
		pContext		������
	���أ�
		ȡ������true�����򷵻�false��
	**************************************************************************/
	bool _IsAcceptCancel(DWORD dwErr, BOOL bIOSucc, DWORD dwBytes
		, SOCKET socket, ContextBuf::SContext * pContext);

	/**************************************************************************
	�������ȡ��
	������
		bIOSucc				IO�Ƿ�ɹ�
		pAcceptContext		����������
	**************************************************************************/
	void _ProcessAcceptCancel(BOOL bIOSucc
		, ContextBuf::SAcceptContext * pAcceptContext);

	/**************************************************************************
	�Ƿ�Ͽ�
	������
		dwErr			������
		bIOSucc			IO�Ƿ�ɹ�
		dwBytes			�ֽ���
		socket			�׽���
		pContext		������
	���أ�
		�Ͽ�����true�����򷵻�false��
	**************************************************************************/
	bool _IsDiscon(DWORD dwErr, BOOL bIOSucc, DWORD dwBytes
		, SOCKET socket, ContextBuf::SContext * pContext);

	/**************************************************************************
	����Ͽ�
	������
		pRecvContext		����������
	**************************************************************************/
	void _ProcessDiscon(ContextBuf::SRecvContext * pRecvContext);

	/**************************************************************************
	����IO֪ͨ
	������
		dwErr			������
		bIOSucc			IO�Ƿ�ɹ�
		dwBytes			�ֽ���
		socket			�׽���
		pContext		������
	**************************************************************************/
	void _ProcessIONotice(DWORD dwErr, BOOL bIOSucc, DWORD dwBytes
		, SOCKET socket, ContextBuf::SContext * pContext);

	/**************************************************************************
	�������֪ͨ
	������
		dwErr				������
		bIOSucc				IO�Ƿ�ɹ�
		socket				�׽���
		pAcceptContext		����������
	**************************************************************************/
	void _ProcessAcceptNotice(DWORD dwErr, BOOL bIOSucc, SOCKET socket
		, ContextBuf::SAcceptContext * pAcceptContext);

	/**************************************************************************
	��������֪ͨ
	������
		dwErr				������
		bIOSucc				IO�Ƿ�ɹ�
		pConnContext		����������
	**************************************************************************/
	void _ProcessConnNotice(DWORD dwErr, BOOL bIOSucc
		, ContextBuf::SConnContext * pConnContext);

	/**************************************************************************
	������֪ͨ
	������
		dwErr				������
		bIOSucc				IO�Ƿ�ɹ�
		dwBytes				�ֽ���
		socket				�׽���
		pSendContext		����������
	**************************************************************************/
	void _ProcessSendNotice(DWORD dwErr, BOOL bIOSucc, DWORD dwBytes
		, SOCKET socket, ContextBuf::SSendContext * pSendContext);

	/**************************************************************************
	�������֪ͨ
	������
		dwErr				������
		bIOSucc				IO�Ƿ�ɹ�
		dwBytes				�ֽ���
		socket				�׽���
		pRecvContext		����������
	**************************************************************************/
	void _ProcessRecvNotice(DWORD dwErr, BOOL bIOSucc, DWORD dwBytes
		, SOCKET socket, ContextBuf::SRecvContext * pRecvContext);

	/**************************************************************************
	����Ͷ�ݽ�������
	������
		socket				�׽���
		pAcceptContext		����������
	**************************************************************************/
	void _PostAcceptContinue(SOCKET socket
		, ContextBuf::SAcceptContext * pAcceptContext);

	/**************************************************************************
	�ָ�����
	������
		sSend				�����׽���
		pSendContext		����������
	**************************************************************************/
	void _ResumeRecv(SOCKET sSend, ContextBuf::SSendContext * pSendContext);

	/**************************************************************************
	����Ͷ�ݽ�������
	������
		socket				�׽���
		pRecvContext		����������
	**************************************************************************/
	void _PostRecvContinue(SOCKET socket
		, ContextBuf::SRecvContext * pRecvContext);

	/**************************************************************************
	�׽����Ƿ���Ч
	������
		socket			�׽���
		pContext		������
	���أ�
		��Ч����true�����򷵻�false��
	**************************************************************************/
	bool _IsSocketValid(SOCKET socket, ContextBuf::SContext * pContext);

	/**************************************************************************
	Ͷ�ݽ���ȡ��֪ͨ
	������
		pAcceptContext		����������
	**************************************************************************/
	void _PostAcceptCancel(ContextBuf::SAcceptContext * pAcceptContext);

	/**************************************************************************
	Ͷ�ݶϿ�֪ͨ
	������
		pRecvContext		����������
	**************************************************************************/
	void _PostDiscon(ContextBuf::SRecvContext * pRecvContext);

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
	TCP::PFNPutTip		m_pfnPutTip;		//�����ʾ�ص�����
	void				* m_pThis;			//�ص�thisָ��

	CCriticalSection		m_csLock;		//������

	CSocketBuf		m_SocketBuf;		//�׽��ֻ�����
	CContextBuf		m_ContextBuf;		//�����Ļ�����

	CBridgeCtrl		m_BridgeCtrl;		//�Žӿ���

	HANDLE		m_hIOCP;		//IOCP

	unsigned int		m_uiThreadCnt;		//�߳���
	CSemaphore			* m_pThreadSem;		//�߳��ź���

	LPFN_ACCEPTEX		m_pfnAcceptEx;		//���ܺ���
	LPFN_CONNECTEX		m_pfnConnectEx;		//���Ӻ���
	//��ȡ��ַ����
	LPFN_GETACCEPTEXSOCKADDRS		m_pfnGetAcceptExSockaddrs;

	//�׽�����Ч��
	map<SOCKET, ContextBuf::SContext *>		m_mapSocketValid;
};