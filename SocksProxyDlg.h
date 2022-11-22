
// SocksProxyDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "Resource.h"

#include "TipsBox.h"
#include "ConfigCenter.h"
#include "MsgCenter.h"
#include "TrayIcon.h"
#include "TCP/TCP.h"
#include "Channel/Channel.h"
#include "NetMonitor.h"

// CSocksProxyDlg 对话框
class CSocksProxyDlg : public CDialogEx
{
// 构造
public:
	CSocksProxyDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SOCKSPROXY_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

public:
	CTipsBox			m_TipsBox;			//提示框
	CConfigCenter		m_ConfigCenter;		//设置中心
	CMsgCenter			m_MsgCenter;		//消息中心
	CTrayIcon			m_TrayIcon;			//托盘图标
	CTCP				m_TCP;				//TCP
	CChannel			m_Channel;			//通道
	CNetMonitor			m_NetMonitor;		//网络监视

public:
	/**************************************************************************
	输出提示回调函数
	说明：
		此接口是线程安全的
	参数：
		strTip			提示
		strClass		类名
		iErr			错误码
		strErr			错误串
		pThis			回调this指针
	**************************************************************************/
	static void callbackPutTip(string strTip, string strClass, int iErr
		, string strErr, void * pThis);

	/**************************************************************************
	写入设置信息
	**************************************************************************/
	void WriteConfig(void);

private:
	void _SetTipCtrl(void);
	void _ReadConfig();

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedOk();
	CEdit m_editTips;
	CToolTipCtrl m_TipCtrl;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
	afx_msg LRESULT OnCloseDone(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTrayIcon(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTaskbarCreated(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnBnClickedButtonStartProxy();
	CEdit m_editConnCnt;
	CEdit m_editNetSpeed;
	CEdit m_editTraffic;
	BOOL m_bStartupRun;
	afx_msg void OnBnClickedCheckStartupRun();
	CString m_strPort;
	afx_msg void OnEnKillfocusEditPort();
};
