
// SocksProxyDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SocksProxy.h"
#include "SocksProxyDlg.h"
#include "afxdialogex.h"
#include "VerCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSocksProxyDlg 对话框




CSocksProxyDlg::CSocksProxyDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSocksProxyDlg::IDD, pParent)
	, m_ConfigCenter(callbackPutTip, this)
	, m_MsgCenter(callbackPutTip, this)
	, m_TrayIcon(callbackPutTip, this)
	, m_TCP(callbackPutTip, this)
	, m_Channel(callbackPutTip, this)
	, m_NetMonitor(callbackPutTip, this)
	, m_bStartupRun(FALSE)
	, m_strPort(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSocksProxyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_TIPS, m_editTips);
	DDX_Control(pDX, IDC_EDIT_CONN_CNT, m_editConnCnt);
	DDX_Control(pDX, IDC_EDIT_NET_SPEED, m_editNetSpeed);
	DDX_Control(pDX, IDC_EDIT_TRAFFIC, m_editTraffic);
	DDX_Check(pDX, IDC_CHECK_STARTUP_RUN, m_bStartupRun);
	DDX_Text(pDX, IDC_EDIT_PORT, m_strPort);
}

BEGIN_MESSAGE_MAP(CSocksProxyDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CSocksProxyDlg::OnBnClickedOk)
	ON_WM_CLOSE()
	ON_MESSAGE(WM_CLOSE_DONE, &CSocksProxyDlg::OnCloseDone)
	ON_MESSAGE(WM_TRAY_ICON, &CSocksProxyDlg::OnTrayIcon)
	ON_REGISTERED_MESSAGE(CTrayIcon::m_wmTaskbarCreated, OnTaskbarCreated)
	ON_BN_CLICKED(IDC_BUTTON_START_PROXY, &CSocksProxyDlg::OnBnClickedButtonStartProxy)
	ON_BN_CLICKED(IDC_CHECK_STARTUP_RUN, &CSocksProxyDlg::OnBnClickedCheckStartupRun)
	ON_EN_KILLFOCUS(IDC_EDIT_PORT, &CSocksProxyDlg::OnEnKillfocusEditPort)
END_MESSAGE_MAP()


// CSocksProxyDlg 消息处理程序

BOOL CSocksProxyDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	//设置窗口标题
	SetWindowText(WND_TITLE);
	//设置提示控件
	_SetTipCtrl();

	//初始化提示框
	m_TipsBox.Init(&m_editTips, this);
	//初始化设置中心
	m_ConfigCenter.Init();
	//读取设置
	_ReadConfig();

	//初始化消息中心
	m_MsgCenter.Init(this);
	//初始化软件
	m_MsgCenter.PostMsg(MsgCenter::MSG_INIT_SOFTWARE);
	//处理开机运行
	m_MsgCenter.PostMsg(MsgCenter::MSG_PROCESS_STARTUP_RUN);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CSocksProxyDlg::_ReadConfig()
{
	m_ConfigCenter.Lock();

	m_strPort = m_ConfigCenter.strPort;
	m_bStartupRun = m_ConfigCenter.bStartupRun;

	m_ConfigCenter.Unlock();

	UpdateData(FALSE);
}

void CSocksProxyDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (nID == SC_MINIMIZE)
	{
		ShowWindow(SW_HIDE);
		return;
	}
	CDialogEx::OnSysCommand(nID, lParam);
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSocksProxyDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CSocksProxyDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSocksProxyDlg::OnBnClickedOk()
{
}

/******************************************************************************
输出提示回调函数
******************************************************************************/
void CSocksProxyDlg::callbackPutTip(string strTip, string strClassName, int iErr
	, string strErr, void * pThis)
{
	if (pThis == NULL)
	{
		return;
	}
	CSocksProxyDlg * pThis2 = (CSocksProxyDlg *)pThis;

	//提示
	CString strPutTip = _T("");
	strPutTip += strTip.c_str();

	//添加错误串后缀
	if (iErr != 0)
	{
		//默认提示
		if (strTip == "")
		{
			strPutTip += _T("发生内部错误！");
		}
		strPutTip += _T("错误串：");

		//类名
		strPutTip += strClassName.c_str();
		strPutTip += _T(".");

		//错误码
		wchar_t acBuf[12] = {0};
		_itow_s(iErr, acBuf, sizeof(acBuf) / sizeof(wchar_t), 10);
		strPutTip += acBuf;
		strPutTip += _T(".");

		//错误串
		strPutTip += strErr.c_str();
	}

	//输出提示
	pThis2->m_TipsBox.PutTip(strPutTip);
}

void CSocksProxyDlg::WriteConfig(void)
{
	m_ConfigCenter.Lock();

	m_ConfigCenter.strPort = m_strPort;
	m_ConfigCenter.bStartupRun = m_bStartupRun;

	m_ConfigCenter.Unlock();
}

BOOL CSocksProxyDlg::PreTranslateMessage(MSG* pMsg)
{
	//处理提示框点击事件
	if (pMsg == NULL)
	{
		return FALSE;
	}
	if (pMsg->message == WM_LBUTTONDOWN 
		|| pMsg->message == WM_RBUTTONDOWN)
	{
		static CPoint pt;
		pt = pMsg->pt;
		ScreenToClient(&pt);
		m_TipsBox.OnClick(pt);
	}

	//屏蔽ESC键
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{
		OnClose();
		return TRUE;
	}

	//处理控件提示
	m_TipCtrl.RelayEvent(pMsg);

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CSocksProxyDlg::_SetTipCtrl(void)
{
	//创建
	if (!m_TipCtrl.Create(this))
	{
		return;
	}

	//支持多行
	m_TipCtrl.SetMaxTipWidth(0);

	//提示框
	if (!m_TipCtrl.AddTool(&m_editTips
		, _T("点击可暂停滚动\n选中一段内容可停止滚动\n")))
	{
		return;
	}
}

void CSocksProxyDlg::OnClose()
{
	m_MsgCenter.PostMsg(MsgCenter::MSG_CLOSE_BOX);
}

LRESULT CSocksProxyDlg::OnCloseDone(WPARAM wParam, LPARAM lParam)
{
	OnOK();
	return 0;
}

LRESULT CSocksProxyDlg::OnTrayIcon(WPARAM wParam, LPARAM lParam)
{
	m_MsgCenter.PostMsg(MsgCenter::MSG_PROCESS_TRAY_ICON, (void *)lParam);
	return 0;
}

LRESULT CSocksProxyDlg::OnTaskbarCreated(WPARAM wParam, LPARAM lParam)
{
	m_MsgCenter.PostMsg(MsgCenter::MSG_ADD_TRAY_ICON);
	return 0;
}

void CSocksProxyDlg::OnBnClickedButtonStartProxy()
{
	UpdateData(TRUE);
	m_MsgCenter.PostMsg(MsgCenter::MSG_BUTTON_START_PROXY);
}

void CSocksProxyDlg::OnBnClickedCheckStartupRun()
{
	UpdateData(TRUE);
	m_MsgCenter.PostMsg(MsgCenter::MSG_SAVE_CONFIG);
}

void CSocksProxyDlg::OnEnKillfocusEditPort()
{
	UpdateData(TRUE);
	m_MsgCenter.PostMsg(MsgCenter::MSG_SAVE_CONFIG);
}