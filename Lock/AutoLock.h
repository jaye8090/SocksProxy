#pragma once

#include <string>
using namespace std;

#include "AutoLockDef.h"

/******************************************************************************
�Զ�����
���ܣ�
	���ù��졢�����������Զ�������������
���ߣ�
	��Ҳ 2623168833 jaye8090@qq.com
ʱ�䣺
	2015-4-23 �� 2015-4-23
******************************************************************************/
class CAutoLock
{
public:
	CAutoLock(CSyncObject * pLock
		, AutoLock::PFNPutTip pfnPutTip = NULL, void * pThis = NULL);

	~CAutoLock(void);

//�ڲ�����
public:
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
	AutoLock::PFNPutTip		m_pfnPutTip;		//�����ʾ�ص�����
	void					* m_pThis;			//�ص�thisָ��

	CSyncObject		* m_pLock;		//��
};