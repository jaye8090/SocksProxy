#include "StdAfx.h"

#include "RegistryEditor.h"

CRegistryEditor::CRegistryEditor(RegistryEditor::PFNPutTip pfnPutTip
	, void * pThis)
{
	m_pfnPutTip = pfnPutTip;
	m_pThis = pThis;
}

CRegistryEditor::~CRegistryEditor(void)
{
}

/******************************************************************************
���ü�
******************************************************************************/
void CRegistryEditor::SetKey(HKEY hKey, CString strSubKey)
{
	m_hKey = hKey;
	m_strSubKey = strSubKey;
}

/******************************************************************************
��ȡ
******************************************************************************/
void CRegistryEditor::Read(CString strKeyValue, CString & strData)
{
	//�򿪼�
	_OpenKey();

	//��ȡ��ֵ
	if (m_handle == NULL
		|| strKeyValue == _T(""))
	{
		_PutTip("", RegistryEditor::READ_STR_QUERY_PARAM);
		return;
	}
	TCHAR acBuf[RegistryEditor::READ_STR_BUF_SIZE] = {0};
	DWORD dwSize = sizeof(acBuf);
	LONG lQuery = RegQueryValueEx(m_handle
		, strKeyValue, NULL, NULL, (LPBYTE)acBuf, &dwSize);
	if (lQuery != ERROR_SUCCESS
		&& lQuery != ERROR_FILE_NOT_FOUND)
	{
		_PutTip("", RegistryEditor::READ_STR_QUERY_FAIL);
		return;
	}
	if (lQuery == ERROR_SUCCESS)
	{
		strData = acBuf;
	}

	//�ر��Ӽ�
	_CloseKey();
}

/******************************************************************************
��ȡ
******************************************************************************/
void CRegistryEditor::Read(CString strKeyValue, int & iData)
{
	//�򿪼�
	_OpenKey();

	//��ȡ��ֵ
	if (m_handle == NULL
		|| strKeyValue == _T(""))
	{
		_PutTip("", RegistryEditor::READ_INT_QUERY_PARAM);
		return;
	}
	DWORD dwSize = sizeof(iData);
	LONG lQuery = RegQueryValueEx(m_handle
		, strKeyValue, NULL, NULL, (LPBYTE)&iData, &dwSize);
	if (lQuery != ERROR_SUCCESS
		&& lQuery != ERROR_FILE_NOT_FOUND)
	{
		_PutTip("", RegistryEditor::READ_INT_QUERY_FAIL);
		return;
	}

	//�ر��Ӽ�
	_CloseKey();
}

/******************************************************************************
��ȡ
******************************************************************************/
void CRegistryEditor::Read(CString strKeyValue, UINT & uiData)
{
	int iData = (int)uiData;
	Read(strKeyValue, iData);
	uiData = (UINT)iData;
}

/******************************************************************************
д��
******************************************************************************/
void CRegistryEditor::Write(CString strKeyValue, CString strData)
{
	//�򿪼�
	_OpenKey();

	//д���ֵ
	if (m_handle == NULL
		|| strKeyValue == _T(""))
	{
		_PutTip("", RegistryEditor::WRITE_STR_SET_PARAM);
		return;
	}
	if (RegSetValueEx(m_handle, strKeyValue, 0, REG_SZ
		, (LPBYTE)strData.GetBuffer()
		, (strData.GetLength() + 1) * sizeof(TCHAR)) != ERROR_SUCCESS)
	{
		_PutTip("", RegistryEditor::WRITE_STR_SET_FAIL);
		return;
	}

	//�ر��Ӽ�
	_CloseKey();
}

/******************************************************************************
д��
******************************************************************************/
void CRegistryEditor::Write(CString strKeyValue, int iData)
{
	//�򿪼�
	_OpenKey();

	//д���ֵ
	if (m_handle == NULL
		|| strKeyValue == _T(""))
	{
		_PutTip("", RegistryEditor::WRITE_INT_SET_PARAM);
		return;
	}
	if (RegSetValueEx(m_handle, strKeyValue, 0, REG_DWORD, (LPBYTE)&iData
		, sizeof(iData)) != ERROR_SUCCESS)
	{
		_PutTip("", RegistryEditor::WRITE_INT_SET_FAIL);
		return;
	}

	//�ر��Ӽ�
	_CloseKey();
}

/******************************************************************************
ɾ��ֵ
******************************************************************************/
void CRegistryEditor::DelValue(CString strKeyValue)
{
	//�򿪼�
	_OpenKey();

	if (m_handle == NULL
		|| strKeyValue == _T(""))
	{
		_PutTip("", RegistryEditor::DEL_VALUE_DEL_PARAM);
		return;
	}
	LONG lDel = RegDeleteValue(m_handle, strKeyValue);
	if (lDel != ERROR_SUCCESS
		&& lDel != ERROR_FILE_NOT_FOUND)
	{
		_PutTip("", RegistryEditor::DEL_VALUE_DEL_FAIL);
		return;
	}

	//�ر��Ӽ�
	_CloseKey();
}

/******************************************************************************
�򿪼�
******************************************************************************/
void CRegistryEditor::_OpenKey(void)
{
	if (m_hKey == NULL
		|| m_strSubKey == _T(""))
	{
		_PutTip("", RegistryEditor::OPEN_KEY_CREATE_PARAM);
		return;
	}
	if (RegCreateKeyEx(m_hKey, m_strSubKey, 0, NULL, REG_OPTION_NON_VOLATILE
		, KEY_ALL_ACCESS, NULL, &m_handle, NULL) != ERROR_SUCCESS)
	{
		_PutTip("", RegistryEditor::OPEN_KEY_CREATE_FAIL);
		return;
	}
}

/******************************************************************************
�رռ�
******************************************************************************/
void CRegistryEditor::_CloseKey(void)
{
	//�ر��Ӽ�
	if (m_handle == NULL)
	{
		_PutTip("", RegistryEditor::CLOSE_KEY_CLOSE_PARAM);
		return;
	}
	if (RegCloseKey(m_handle) != ERROR_SUCCESS)
	{
		_PutTip("", RegistryEditor::CLOSE_KEY_CLOSE_FAIL);
		return;
	}
}

/******************************************************************************
�����ʾ��Ϣ
******************************************************************************/
void CRegistryEditor::_PutTip(string strTip, int iErr, string strErr)
{
	//�����ʾ�ص�
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "RegistryEditor", iErr, strErr, m_pThis);
}