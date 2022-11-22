
// SocksProxyDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SocksProxy.h"
#include "SocksProxyDlg.h"
#include "afxdialogex.h"
#include "VerCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSocksProxyDlg �Ի���




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


// CSocksProxyDlg ��Ϣ�������

BOOL CSocksProxyDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	//���ô��ڱ���
	SetWindowText(WND_TITLE);
	//������ʾ�ؼ�
	_SetTipCtrl();

	//��ʼ����ʾ��
	m_TipsBox.Init(&m_editTips, this);
	//��ʼ����������
	m_ConfigCenter.Init();
	//��ȡ����
	_ReadConfig();

	//��ʼ����Ϣ����
	m_MsgCenter.Init(this);
	//��ʼ�����
	m_MsgCenter.PostMsg(MsgCenter::MSG_INIT_SOFTWARE);
	//����������
	m_MsgCenter.PostMsg(MsgCenter::MSG_PROCESS_STARTUP_RUN);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CSocksProxyDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CSocksProxyDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSocksProxyDlg::OnBnClickedOk()
{
}

/******************************************************************************
�����ʾ�ص�����
******************************************************************************/
void CSocksProxyDlg::callbackPutTip(string strTip, string strClassName, int iErr
	, string strErr, void * pThis)
{
	if (pThis == NULL)
	{
		return;
	}
	CSocksProxyDlg * pThis2 = (CSocksProxyDlg *)pThis;

	//��ʾ
	CString strPutTip = _T("");
	strPutTip += strTip.c_str();

	//��Ӵ��󴮺�׺
	if (iErr != 0)
	{
		//Ĭ����ʾ
		if (strTip == "")
		{
			strPutTip += _T("�����ڲ�����");
		}
		strPutTip += _T("���󴮣�");

		//����
		strPutTip += strClassName.c_str();
		strPutTip += _T(".");

		//������
		wchar_t acBuf[12] = {0};
		_itow_s(iErr, acBuf, sizeof(acBuf) / sizeof(wchar_t), 10);
		strPutTip += acBuf;
		strPutTip += _T(".");

		//����
		strPutTip += strErr.c_str();
	}

	//�����ʾ
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
	//������ʾ�����¼�
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

	//����ESC��
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{
		OnClose();
		return TRUE;
	}

	//����ؼ���ʾ
	m_TipCtrl.RelayEvent(pMsg);

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CSocksProxyDlg::_SetTipCtrl(void)
{
	//����
	if (!m_TipCtrl.Create(this))
	{
		return;
	}

	//֧�ֶ���
	m_TipCtrl.SetMaxTipWidth(0);

	//��ʾ��
	if (!m_TipCtrl.AddTool(&m_editTips
		, _T("�������ͣ����\nѡ��һ�����ݿ�ֹͣ����\n")))
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