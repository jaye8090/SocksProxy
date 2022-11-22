#include "StdAfx.h"

#include "TipsBox.h"

CTipsBox::CTipsBox(void)
{
}

CTipsBox::~CTipsBox(void)
{
}

/******************************************************************************
��ʼ��
******************************************************************************/
void CTipsBox::Init(CEdit * peditTips, CWnd * pParentWnd)
{
	//��ʾ��ָ��
	if (peditTips == NULL)
	{
		string strErr = "";
		_OnErr(TipsBox::INIT_TIPS_BOX_NULL, strErr);
		return;
	}
	m_peditTips = peditTips;

	//������ָ��
	if (pParentWnd == NULL)
	{
		string strErr = "";
		_OnErr(TipsBox::INIT_PARENT_WND_NULL, strErr);
		return;
	}
	m_pParentWnd = pParentWnd;

	//��ȡ���Ŀ¼
	m_strSoftwarePath = _GetSoftwarePath();

	//������󳤶�
	m_peditTips->SetLimitText(0);
	//��ȡ��ʾ������
	m_peditTips->GetWindowRect(&m_rcTips);
	m_pParentWnd->ScreenToClient(&m_rcTips);

	//Ĭ�Ϲ���
	m_bScroll = true;
}

/******************************************************************************
�����ʾ
******************************************************************************/
void CTipsBox::PutTip(CString strTip)
{
	//�ӿڻ������
	_LockInterface();

	//�����ʾ
	_PutTip(strTip);
	//�Ƿ���Ҫ����
	if (_IsNeedSave())
	{
		//������ʾ��Ϣ
		_SaveTips();
	}

	//�ӿڻ������
	_UnlockInterface();
}

/******************************************************************************
�������¼�
******************************************************************************/
void CTipsBox::OnClick(const CPoint & pt)
{
	//�����������
	_LockScroll();

	//�Ƿ������ʾ��û���������
	m_bScroll = !m_rcTips.PtInRect(pt);

	//�����������
	_UnlockScroll();
}

/******************************************************************************
������ʾ��Ϣ
******************************************************************************/
void CTipsBox::SaveTips(void)
{
	//�ӿڻ������
	_LockInterface();

	//������ʾ��Ϣ
	_SaveTips();

	//�ӿڻ������
	_UnlockInterface();
}

/******************************************************************************
�����ʾ
******************************************************************************/
void CTipsBox::_PutTip(CString strTip)
{
	if (m_peditTips == NULL)
	{
		string strErr = "";
		_OnErr(TipsBox::PUT_TIP_TIPS_BOX_NULL, strErr);
		return;
	}

	//�������ʱ��ǰ׺
	CTime time = CTime::GetCurrentTime();
	strTip = time.Format("[%m-%d] [%H:%M:%S] ") + strTip;

	//��ȡ����״̬����ֹ�������λ�ȡ�Ĳ�һ��
	_LockScroll();
	bool bScroll = m_bScroll;
	_UnlockScroll();

	//��ȡ��ǰ��ѡ��
	int nStartChar, nEndChar;
	m_peditTips->GetSel(nStartChar, nEndChar);
	//�����ѡ�е����ݣ��򲻹���
	if (nStartChar < nEndChar)
	{
		bScroll = false;
	}

	//��ȡ�ı�����
	int nTextLen = m_peditTips->GetWindowTextLength();
	//�ǵ�һ�У��żӻ��з�
	if (nTextLen > 0)
	{
		strTip = CString("\r\n") + strTip;
	}

	//���������
	if (!bScroll)
	{
		//�ر��ػ�
		m_peditTips->SetRedraw(FALSE);
	}

	//��β���滻�����ۼ��ı�
	m_peditTips->SetSel(nTextLen, nTextLen);
	m_peditTips->ReplaceSel(strTip);

	//���������
	if (!bScroll)
	{
		//ѡ����ǰ��λ��
		m_peditTips->SetSel(nStartChar, nEndChar);
		//���ػ�
		m_peditTips->SetRedraw(TRUE);
	}
}

/******************************************************************************
�Ƿ���Ҫ����
******************************************************************************/
bool CTipsBox::_IsNeedSave(void)
{
	if (m_peditTips == NULL)
	{
		string strErr = "";
		_OnErr(TipsBox::IS_NEED_SAVE_TIPS_BOX_NULL, strErr);
		return false;
	}

	//��ȡ�ı�����
	int nTextLen = m_peditTips->GetWindowTextLength();
	//û�ﵽ�����С���򲻱���
	if (nTextLen < TipsBox::SAVE_TIPS_SIZE)
	{
		return false;
	}

	return true;
}

/******************************************************************************
������ʾ��Ϣ
******************************************************************************/
void CTipsBox::_SaveTips(void)
{
	if (m_peditTips == NULL)
	{
		string strErr = "";
		_OnErr(TipsBox::SAVE_TIPS_TIPS_BOX_NULL, strErr);
		return;
	}

	//��ȡ��ʾ��Ϣ
	CString strTips;
	m_peditTips->GetWindowText(strTips);

	//������ʾ��־Ŀ¼
	_CreateTipsLogPath();

	//������ʱ�������ļ�
	CTime time = CTime::GetCurrentTime();
	CString strFileName = time.Format("\\TipsLog\\%Y%m%d_%H%M%S.log");

	//���浽�ļ�
	if (m_strSoftwarePath == _T(""))
	{
		string strErr = "";
		_OnErr(TipsBox::SAVE_TIPS_SOFTWARE_PATH_EMPTY, strErr);
		return;
	}
	_SaveToFile(m_strSoftwarePath + strFileName, strTips);

	//�����ʾ��
	strTips = _T("");
	m_peditTips->SetWindowText(strTips);

	//������ʾ
	strTips.Format(_T("������ʾ��Ϣ���Ѿ����浽�� %s �ļ��С�"), strFileName);
	_PutTip(strTips);
}

/******************************************************************************
������ʾ��־Ŀ¼
******************************************************************************/
void CTipsBox::_CreateTipsLogPath(void)
{
	if (m_strSoftwarePath == _T(""))
	{
		string strErr = "";
		_OnErr(TipsBox::CREATE_TIPS_LOG_PATH_SOFTWARE_PATH_EMPTY, strErr);
		return;
	}

	//���Ŀ¼������
	CString strPath = m_strSoftwarePath + _T("\\TipsLog");
	if (!PathFileExists(strPath))
	{
		//����Ŀ¼
		if (!CreateDirectory(strPath, NULL))
		{
			DWORD dwErr = GetLastError();

			string strErr = "";
			//�Ѵ�����������
			char achBuf[12] = {0};
			_itoa_s(dwErr, achBuf, sizeof(achBuf), 10);
			strErr += achBuf;
			strErr += ".";
			//������
			_OnErr(TipsBox::CREATE_TIPS_LOG_PATH_CREATE_PATH_FAIL, strErr);
			return;
		}
	}
}

/******************************************************************************
���浽�ļ�
******************************************************************************/
void CTipsBox::_SaveToFile(CString strFileName, const CString & strText)
{
	if (strFileName == ""
		|| strText == "")
	{
		return;
	}

	//����Ϊ�ļ�
	CStdioFile file;
	if (!file.Open(strFileName, CFile::modeCreate | CFile::modeWrite))
	{
		string strErr = "";
		_OnErr(TipsBox::SAVE_TO_FILE_OPEN_FAIL, strErr);
		return;
	}

	//��������
	char * pcSetLocale = setlocale(LC_CTYPE, "chs");
	if (pcSetLocale == NULL)
	{
		string strErr = "";
		_OnErr(TipsBox::SAVE_TO_FILE_SET_LOCALE_FAIL, strErr);
		return;
	}

	//д����ʾ��Ϣ
	try
	{
		file.WriteString(strText);
	}
	catch (...)
	{
		string strErr = "";
		_OnErr(TipsBox::SAVE_TO_FILE_WRITE_STRING_EXCEPTION, strErr);
		return;
	}

	//�ر��ļ�
	file.Close();
}

/******************************************************************************
��ȡ���Ŀ¼
******************************************************************************/
CString CTipsBox::_GetSoftwarePath(void)
{
	//��ȡ���EXE�ļ�ȫ·��
	TCHAR acBuf[MAX_PATH + 1] = {0};
	if (GetModuleFileName(NULL, acBuf, MAX_PATH) == 0)
	{
		return _T("");
	}
	CString strSoftwarePath = acBuf;

	//��ȫ·�����ļ�����ȡ��
	int nPos = strSoftwarePath.ReverseFind('\\');
	if (nPos == -1)
	{
		return _T("");
	}

	return strSoftwarePath.Left(nPos);
}

/******************************************************************************
������
******************************************************************************/
void CTipsBox::_OnErr(int iErrCode, string strErr)
{
	//��ʾ
	CString strTip = _T("��ʾ�򣬷������ش���");

	//������
	char acBuf[12] = {0};
	_itoa_s(iErrCode, acBuf, sizeof(acBuf), 10);
	string strErrCode = acBuf;
	strErrCode += ".";

	//�Ѵ�����������
	strErr = strErrCode + strErr;

	//�Ѵ��󴮽�����ʾ
	strTip += _T("���󴮣�");
	strTip += strErr.c_str();
	strTip += _T("��");

	//�����Ի���
	MessageBox(NULL, strTip, _T("���ش���"), MB_ICONERROR);
}

/******************************************************************************
�ӿڻ������
******************************************************************************/
void CTipsBox::_LockInterface(void)
{
	try
	{
		if (!m_csInterface.Lock())
		{
			string strErr = "";
			_OnErr(TipsBox::LOCK_INTERFACE_LOCK_FAIL, strErr);
			return;
		}
	}
	catch (...)
	{
		string strErr = "";
		_OnErr(TipsBox::LOCK_INTERFACE_LOCK_EXCEPTION, strErr);
		return;
	}
}

/******************************************************************************
�ӿڻ������
******************************************************************************/
void CTipsBox::_UnlockInterface(void)
{
	if (!m_csInterface.Unlock())
	{
		string strErr = "";
		_OnErr(TipsBox::UNLOCK_INTERFACE_UNLOCK_FAIL, strErr);
		return;
	}
}

/******************************************************************************
�����������
******************************************************************************/
void CTipsBox::_LockScroll(void)
{
	try
	{
		if (!m_csScroll.Lock())
		{
			string strErr = "";
			_OnErr(TipsBox::LOCK_SCROLL_LOCK_FAIL, strErr);
			return;
		}
	}
	catch (...)
	{
		string strErr = "";
		_OnErr(TipsBox::LOCK_SCROLL_LOCK_EXCEPTION, strErr);
		return;
	}
}

/******************************************************************************
�����������
******************************************************************************/
void CTipsBox::_UnlockScroll(void)
{
	if (!m_csScroll.Unlock())
	{
		string strErr = "";
		_OnErr(TipsBox::UNLOCK_SCROLL_UNLOCK_FAIL, strErr);
		return;
	}
}