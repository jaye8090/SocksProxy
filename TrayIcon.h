#pragma once

#include <string>
using namespace std;

#include "TrayIconDef.h"

/******************************************************************************
����ͼ����
���ܣ�
	��������ͼ��
���ߣ�
	��Ҳ 2623168833 jaye8090@qq.com
ʱ�䣺
	2014-7-2 �� 2015-5-9
******************************************************************************/
class CTrayIcon
{
public:
	CTrayIcon(TrayIcon::PFNPutTip pfnPutTip = NULL, void * pThis = NULL);

	~CTrayIcon(void);

	/**************************************************************************
	��ʼ��
	������
		pMainWnd		������
		strTip			��ʾ
		uiIconID		ͼ��ID
	**************************************************************************/
	void Init(CWnd * pMainWnd, CString strTip, UINT uiIconID);

//�����ӿ�
public:
	/**************************************************************************
	����
	**************************************************************************/
	void Add(void);

	/**************************************************************************
	ɾ��
	**************************************************************************/
	void Del(void);

//�ⲿ����
public:
	//������������Ϣ����ֹ����ͼ�����������ؽ�����ʧ��
	static UINT		m_wmTaskbarCreated;

//�ڲ�����
private:
	/**************************************************************************
	�����ʾ��Ϣ
	������
		strTip		��ʾ��Ϣ
		iErr		������
		strErr		����
	**************************************************************************/
	void _PutTip(string strTip, int iErr = 0, string strErr = "");

//�ڲ�����
private:
	TrayIcon::PFNPutTip		m_pfnPutTip;		//�����ʾ�ص�����
	void					* m_pThis;			//�ص�thisָ��

	CWnd		* m_pMainWnd;		//������ָ��

	CString		m_strTip;		//��ʾ

	UINT		m_uiIconID;		//ͼ��ID

	NOTIFYICONDATA		m_IconData;		//ͼ������
};