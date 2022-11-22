
// SocksProxyDlg.h : ͷ�ļ�
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

// CSocksProxyDlg �Ի���
class CSocksProxyDlg : public CDialogEx
{
// ����
public:
	CSocksProxyDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SOCKSPROXY_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

public:
	CTipsBox			m_TipsBox;			//��ʾ��
	CConfigCenter		m_ConfigCenter;		//��������
	CMsgCenter			m_MsgCenter;		//��Ϣ����
	CTrayIcon			m_TrayIcon;			//����ͼ��
	CTCP				m_TCP;				//TCP
	CChannel			m_Channel;			//ͨ��
	CNetMonitor			m_NetMonitor;		//�������

public:
	/**************************************************************************
	�����ʾ�ص�����
	˵����
		�˽ӿ����̰߳�ȫ��
	������
		strTip			��ʾ
		strClass		����
		iErr			������
		strErr			����
		pThis			�ص�thisָ��
	**************************************************************************/
	static void callbackPutTip(string strTip, string strClass, int iErr
		, string strErr, void * pThis);

	/**************************************************************************
	д��������Ϣ
	**************************************************************************/
	void WriteConfig(void);

private:
	void _SetTipCtrl(void);
	void _ReadConfig();

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
