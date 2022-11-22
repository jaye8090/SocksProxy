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
初始化
******************************************************************************/
void CConfigCenter::Init(void)
{
	//获取EXE文件名
	strExeFileName = _GetExeFileName();
	//填写默认设置信息
	_FillDefaultConfig();
	//加载设置
	LoadConfig();
}

/******************************************************************************
获取EXE文件名
******************************************************************************/
CString CConfigCenter::_GetExeFileName(void)
{
	//获取软件EXE文件全路径
	TCHAR acBuf[MAX_PATH + 1] = {0};
	if (GetModuleFileName(NULL, acBuf, MAX_PATH) == 0)
	{
		_PutTip("", ConfigCenter::GET_EXE_FILE_NAME_GET_FAIL);
		return _T("");
	}

	return acBuf;
}

/******************************************************************************
填写默认设置信息
******************************************************************************/
void CConfigCenter::_FillDefaultConfig(void)
{
	//端口号
	strPort = ConfigCenter::DEFAULT_PORT;
	//开机运行
	bStartupRun = FALSE;
}

/******************************************************************************
加锁
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
解锁
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
加载设置
******************************************************************************/
void CConfigCenter::LoadConfig(void)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	m_RegistryEditor.SetKey(HKEY_CURRENT_USER, _T("Software\\SocksProxy"));

	//端口号
	m_RegistryEditor.Read(_T("Port"), strPort);
	//开机运行
	m_RegistryEditor.Read(_T("StartupRun"), bStartupRun);
}

/******************************************************************************
保存设置
******************************************************************************/
void CConfigCenter::SaveConfig(void)
{
	//互斥锁
	CAutoLock lock(&m_csLock, m_pfnPutTip, m_pThis);

	m_RegistryEditor.SetKey(HKEY_CURRENT_USER, _T("Software\\SocksProxy"));

	//端口号
	m_RegistryEditor.Write(_T("Port"), strPort);
	//开机运行
	m_RegistryEditor.Write(_T("StartupRun"), bStartupRun);

	//处理开机运行
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
输出提示
******************************************************************************/
void CConfigCenter::_PutTip(string strTip, int iErr, string strErr)
{
	//输出提示回调
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "ConfigCenter", iErr, strErr, m_pThis);
}