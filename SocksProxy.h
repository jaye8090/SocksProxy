
// SocksProxy.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSocksProxyApp:
// �йش����ʵ�֣������ SocksProxy.cpp
//

class CSocksProxyApp : public CWinApp
{
public:
	CSocksProxyApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CSocksProxyApp theApp;