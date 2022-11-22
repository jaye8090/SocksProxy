#include "StdAfx.h"

#include "TipsBox.h"

CTipsBox::CTipsBox(void)
{
}

CTipsBox::~CTipsBox(void)
{
}

/******************************************************************************
初始化
******************************************************************************/
void CTipsBox::Init(CEdit * peditTips, CWnd * pParentWnd)
{
	//提示框指针
	if (peditTips == NULL)
	{
		string strErr = "";
		_OnErr(TipsBox::INIT_TIPS_BOX_NULL, strErr);
		return;
	}
	m_peditTips = peditTips;

	//父窗口指针
	if (pParentWnd == NULL)
	{
		string strErr = "";
		_OnErr(TipsBox::INIT_PARENT_WND_NULL, strErr);
		return;
	}
	m_pParentWnd = pParentWnd;

	//获取软件目录
	m_strSoftwarePath = _GetSoftwarePath();

	//设置最大长度
	m_peditTips->SetLimitText(0);
	//获取提示框区域
	m_peditTips->GetWindowRect(&m_rcTips);
	m_pParentWnd->ScreenToClient(&m_rcTips);

	//默认滚动
	m_bScroll = true;
}

/******************************************************************************
输出提示
******************************************************************************/
void CTipsBox::PutTip(CString strTip)
{
	//接口互斥加锁
	_LockInterface();

	//输出提示
	_PutTip(strTip);
	//是否需要保存
	if (_IsNeedSave())
	{
		//保存提示信息
		_SaveTips();
	}

	//接口互斥解锁
	_UnlockInterface();
}

/******************************************************************************
处理点击事件
******************************************************************************/
void CTipsBox::OnClick(const CPoint & pt)
{
	//变量互斥加锁
	_LockScroll();

	//是否点中提示框，没点中则滚动
	m_bScroll = !m_rcTips.PtInRect(pt);

	//变量互斥解锁
	_UnlockScroll();
}

/******************************************************************************
保存提示信息
******************************************************************************/
void CTipsBox::SaveTips(void)
{
	//接口互斥加锁
	_LockInterface();

	//保存提示信息
	_SaveTips();

	//接口互斥解锁
	_UnlockInterface();
}

/******************************************************************************
输出提示
******************************************************************************/
void CTipsBox::_PutTip(CString strTip)
{
	if (m_peditTips == NULL)
	{
		string strErr = "";
		_OnErr(TipsBox::PUT_TIP_TIPS_BOX_NULL, strErr);
		return;
	}

	//添加日期时间前缀
	CTime time = CTime::GetCurrentTime();
	strTip = time.Format("[%m-%d] [%H:%M:%S] ") + strTip;

	//获取滚动状态，防止下面两次获取的不一样
	_LockScroll();
	bool bScroll = m_bScroll;
	_UnlockScroll();

	//获取当前的选中
	int nStartChar, nEndChar;
	m_peditTips->GetSel(nStartChar, nEndChar);
	//如果有选中的内容，则不滚动
	if (nStartChar < nEndChar)
	{
		bScroll = false;
	}

	//获取文本长度
	int nTextLen = m_peditTips->GetWindowTextLength();
	//非第一行，才加换行符
	if (nTextLen > 0)
	{
		strTip = CString("\r\n") + strTip;
	}

	//如果不滚动
	if (!bScroll)
	{
		//关闭重绘
		m_peditTips->SetRedraw(FALSE);
	}

	//用尾部替换法，累加文本
	m_peditTips->SetSel(nTextLen, nTextLen);
	m_peditTips->ReplaceSel(strTip);

	//如果不滚动
	if (!bScroll)
	{
		//选回以前的位置
		m_peditTips->SetSel(nStartChar, nEndChar);
		//打开重绘
		m_peditTips->SetRedraw(TRUE);
	}
}

/******************************************************************************
是否需要保存
******************************************************************************/
bool CTipsBox::_IsNeedSave(void)
{
	if (m_peditTips == NULL)
	{
		string strErr = "";
		_OnErr(TipsBox::IS_NEED_SAVE_TIPS_BOX_NULL, strErr);
		return false;
	}

	//获取文本长度
	int nTextLen = m_peditTips->GetWindowTextLength();
	//没达到保存大小，则不保存
	if (nTextLen < TipsBox::SAVE_TIPS_SIZE)
	{
		return false;
	}

	return true;
}

/******************************************************************************
保存提示信息
******************************************************************************/
void CTipsBox::_SaveTips(void)
{
	if (m_peditTips == NULL)
	{
		string strErr = "";
		_OnErr(TipsBox::SAVE_TIPS_TIPS_BOX_NULL, strErr);
		return;
	}

	//提取提示信息
	CString strTips;
	m_peditTips->GetWindowText(strTips);

	//创建提示日志目录
	_CreateTipsLogPath();

	//以日期时间命名文件
	CTime time = CTime::GetCurrentTime();
	CString strFileName = time.Format("\\TipsLog\\%Y%m%d_%H%M%S.log");

	//保存到文件
	if (m_strSoftwarePath == _T(""))
	{
		string strErr = "";
		_OnErr(TipsBox::SAVE_TIPS_SOFTWARE_PATH_EMPTY, strErr);
		return;
	}
	_SaveToFile(m_strSoftwarePath + strFileName, strTips);

	//清空提示框
	strTips = _T("");
	m_peditTips->SetWindowText(strTips);

	//给出提示
	strTips.Format(_T("以上提示信息，已经保存到了 %s 文件中。"), strFileName);
	_PutTip(strTips);
}

/******************************************************************************
创建提示日志目录
******************************************************************************/
void CTipsBox::_CreateTipsLogPath(void)
{
	if (m_strSoftwarePath == _T(""))
	{
		string strErr = "";
		_OnErr(TipsBox::CREATE_TIPS_LOG_PATH_SOFTWARE_PATH_EMPTY, strErr);
		return;
	}

	//如果目录不存在
	CString strPath = m_strSoftwarePath + _T("\\TipsLog");
	if (!PathFileExists(strPath))
	{
		//创建目录
		if (!CreateDirectory(strPath, NULL))
		{
			DWORD dwErr = GetLastError();

			string strErr = "";
			//把错误码接入错误串
			char achBuf[12] = {0};
			_itoa_s(dwErr, achBuf, sizeof(achBuf), 10);
			strErr += achBuf;
			strErr += ".";
			//错误处理
			_OnErr(TipsBox::CREATE_TIPS_LOG_PATH_CREATE_PATH_FAIL, strErr);
			return;
		}
	}
}

/******************************************************************************
保存到文件
******************************************************************************/
void CTipsBox::_SaveToFile(CString strFileName, const CString & strText)
{
	if (strFileName == ""
		|| strText == "")
	{
		return;
	}

	//保存为文件
	CStdioFile file;
	if (!file.Open(strFileName, CFile::modeCreate | CFile::modeWrite))
	{
		string strErr = "";
		_OnErr(TipsBox::SAVE_TO_FILE_OPEN_FAIL, strErr);
		return;
	}

	//设置语言
	char * pcSetLocale = setlocale(LC_CTYPE, "chs");
	if (pcSetLocale == NULL)
	{
		string strErr = "";
		_OnErr(TipsBox::SAVE_TO_FILE_SET_LOCALE_FAIL, strErr);
		return;
	}

	//写入提示信息
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

	//关闭文件
	file.Close();
}

/******************************************************************************
获取软件目录
******************************************************************************/
CString CTipsBox::_GetSoftwarePath(void)
{
	//获取软件EXE文件全路径
	TCHAR acBuf[MAX_PATH + 1] = {0};
	if (GetModuleFileName(NULL, acBuf, MAX_PATH) == 0)
	{
		return _T("");
	}
	CString strSoftwarePath = acBuf;

	//把全路径的文件名截取掉
	int nPos = strSoftwarePath.ReverseFind('\\');
	if (nPos == -1)
	{
		return _T("");
	}

	return strSoftwarePath.Left(nPos);
}

/******************************************************************************
错误处理
******************************************************************************/
void CTipsBox::_OnErr(int iErrCode, string strErr)
{
	//提示
	CString strTip = _T("提示框，发生严重错误！");

	//错误码
	char acBuf[12] = {0};
	_itoa_s(iErrCode, acBuf, sizeof(acBuf), 10);
	string strErrCode = acBuf;
	strErrCode += ".";

	//把错误码接入错误串
	strErr = strErrCode + strErr;

	//把错误串接入提示
	strTip += _T("错误串：");
	strTip += strErr.c_str();
	strTip += _T("。");

	//弹出对话框
	MessageBox(NULL, strTip, _T("严重错误"), MB_ICONERROR);
}

/******************************************************************************
接口互斥加锁
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
接口互斥解锁
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
变量互斥加锁
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
变量互斥解锁
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