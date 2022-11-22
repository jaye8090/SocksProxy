#include "StdAfx.h"

#include "ConfigCenter.h"
#include "Lock/AutoLock.h"

CConfigCenter::CConfigCenter(ConfigCenter::PFNPutTip pfnPutTip, void * pThis)
	: m_RegistryEditor(pfnPutTip, pThis)
{
	m_pfnPutTip = pfnPutTip;
	m_pThis = pThis;
}

CConfigCenter::~CConfigCenter(void)
{
}

/******************************************************************************
��ʼ��
******************************************************************************/
void CConfigCenter::Init(void)
{
	//��ȡEXE�ļ���
	strExeFileName = _GetExeFileName();
	//��дĬ��������Ϣ
	_FillDefaultConfig();
	//��������
	LoadConfig();
}

/******************************************************************************
��ȡEXE�ļ���
******************************************************************************/
CString CConfigCenter::_GetExeFileName(void)
{
	//��ȡ���EXE�ļ�ȫ·��
	TCHAR acBuf[MAX_PATH + 1] = {0};
	if (GetModuleFileName(NULL, acBuf, MAX_PATH) == 0)
	{
		_PutTip("", ConfigCenter::GET_EXE_FILE_NAME_GET_FAIL);
		return _T("");
	}

	return acBuf;
}

/******************************************************************************
��дĬ��������Ϣ
******************************************************************************/
void CConfigCenter::_FillDefaultConfig(void)
{
	//�˿ں�
	strPort = ConfigCenter::DEFAULT_PORT;
	//��������
	bStartupRun = FALSE;
}

/******************************************************************************
����
******************************************************************************/
void CConfigCenter::Lock(void)
{
	try
	{
		if (!m_csLock.Lock())
		{
			_PutTip("", ConfigCenter::LOCK_LOCK_FAIL);
			return;
		}
	}
	catch (...)
	{
		_PutTip("", ConfigCenter::LOCK_LOCK_EXCEPTION);
		return;
	}
}

/******************************************************************************
����
******************************************************************************/
void CConfigCenter::Unlock(void)
{
	if (!m_csLock.Unlock())
	{
		_PutTip("", ConfigCenter::UNLOCK_UNLOCK_FAIL);
		return;
	}
}

/******************************************************************************
��������
******************************************************************************/
void CConfigCenter::LoadConfig(void)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	m_RegistryEditor.SetKey(HKEY_CURRENT_USER, _T("Software\\SocksProxy"));

	//�˿ں�
	m_RegistryEditor.Read(_T("Port"), strPort);
	//��������
	m_RegistryEditor.Read(_T("StartupRun"), bStartupRun);
}

/******************************************************************************
��������
******************************************************************************/
void CConfigCenter::SaveConfig(void)
{
	//������
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	m_RegistryEditor.SetKey(HKEY_CURRENT_USER, _T("Software\\SocksProxy"));

	//�˿ں�
	m_RegistryEditor.Write(_T("Port"), strPort);
	//��������
	m_RegistryEditor.Write(_T("StartupRun"), bStartupRun);

	//����������
	m_RegistryEditor.SetKey(HKEY_CURRENT_USER
		, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"));
	if (bStartupRun)
	{
		if (strExeFileName == _T(""))
		{
			_PutTip("", ConfigCenter::SAVE_CONFIG_EXE_FILE_NAME_EMPTY);
			return;
		}
		m_RegistryEditor.Write(_T("SocksProxy"), strExeFileName);
	}
	else
	{
		m_RegistryEditor.DelValue(_T("SocksProxy"));
	}
}

/******************************************************************************
�����ʾ
******************************************************************************/
void CConfigCenter::_PutTip(string strTip, int iErr, string strErr)
{
	//�����ʾ�ص�
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "ConfigCenter", iErr, strErr, m_pThis);
}