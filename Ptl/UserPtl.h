#pragma once

#include <string>
using namespace std;

#include "UserPtlDef.h"

class CDataPack;

/******************************************************************************
�û�Э����
���ܣ�
	ʵ��SOCKS�û����ݰ��Ĵ���ͽ���
���ߣ�
	��Ҳ 2623168833 jaye8090@qq.com
ʱ�䣺
	2016-1-8 �� 2016-1-8
******************************************************************************/
class CUserPtl
{
public:
	CUserPtl(UserPtl::PFNPutTip pfnPutTip = NULL, void * pThis = NULL);

	~CUserPtl(void);

//�����ӿ�
public:
	/**************************************************************************
	��ȡSOCKS�汾
	������
		pData			����
		uiDataSize		���ݴ�С
	���أ�
		�ɹ�����SOCKS�汾��ʧ�ܷ���0��
	**************************************************************************/
	unsigned char GetSocksVer(void * pData, unsigned int uiDataSize);

	/**************************************************************************
	������֤�����
	������
		pData			����
		uiDataSize		���ݴ�С
	���أ�
		�ɹ�����true��ʧ�ܷ���false��
	**************************************************************************/
	bool ParseAuthRequest(void * pData, unsigned int uiDataSize);

	/**************************************************************************
	�����֤��Ӧ��
	������
		pack		���ֵ�������
	**************************************************************************/
	void PackAuthResponse(CDataPack & pack);

	/**************************************************************************
	����SOCKS5���������
	������
		ulDstIP			���ֵ��Ŀ��IP��
		ulDstPort		���ֵ��Ŀ��˿ڡ�
		pData			����
		uiDataSize		���ݴ�С
	���أ�
		�ɹ�����true��ʧ�ܷ���false��
	**************************************************************************/
	bool ParseSocks5ConnRequest(unsigned long & ulDstIP
		, unsigned short & usDstPort, void * pData, unsigned int uiDataSize);

	/**************************************************************************
	����SOCKS4���������
	������
		ulDstIP			���ֵ��Ŀ��IP��
		ulDstPort		���ֵ��Ŀ��˿ڡ�
		pData			����
		uiDataSize		���ݴ�С
	���أ�
		�ɹ�����true��ʧ�ܷ���false��
	**************************************************************************/
	bool ParseSocks4ConnRequest(unsigned long & ulDstIP
		, unsigned short & usDstPort, void * pData, unsigned int uiDataSize);

	/**************************************************************************
	���SOCKS5������Ӧ��
	������
		pack		���ֵ�������
	**************************************************************************/
	void PackSocks5ConnResponse(CDataPack & pack);

	/**************************************************************************
	���SOCKS4������Ӧ��
	������
		pack		���ֵ�������
	**************************************************************************/
	void PackSocks4ConnResponse(CDataPack & pack);

//�ڲ�����
private:
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
	UserPtl::PFNPutTip		m_pfnPutTip;		//�����ʾ�ص�����
	void					* m_pThis;			//�ص�thisָ��
};