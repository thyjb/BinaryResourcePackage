
// BResourcePackageDlg.h : 头文件
//
#include "Function.h"
#pragma once


// CBResourcePackageDlg 对话框
class CBResourcePackageDlg : public CDialogEx
{
// 构造
public:
	CBResourcePackageDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_BRESOURCEPACKAGE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnClickedButtonExpdes();
	afx_msg void OnClickedButtonExpsrc();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg LRESULT OnCreateResourceOver(WPARAM wParam,LPARAM lParam);

public:
	CString m_strSource;
	CString m_strDestination;
	CProgressCtrl m_pcProgress;
	CString m_strInformation;

private:
	void InitWtFace(int nStatus);

private:	// 生成资源文件
	void ShowProgress(const char* pszFile,DWORD dwTotalLen,DWORD dwDealLength);
	void UpdateProgressPos(size_t dwDone);
	int CreatePartCompressedBinary(const char* pszRoot, const char* pszSrcFileDirectory, const char* pszDesFileDirectory, char* szMsg, int nMsgLen);
	int OutputPartCompressedBinary(BINARYINSTALLRESOURCEHEAD* pHead,const char* pszSrcFile, const char* pszDesFileDirectory, DWORD dwPartSize=COMPRESS_PART_LEN);

private:	// 执行线程
	BOOL	InitThread(CString& strMsg);
	int		ThreadDoJob();
	void	ExitThread();

	static unsigned WINAPI ThreadProc(void* lpParam);
	inline void	CloseThreadHand();
	void	UpdateProgress();
private:
	CString	m_strSrcRoot;
 	HANDLE	m_hThread;
 	BOOL	m_bExitTask;

	int		m_nSrcType;

	// 进度显示
	PROGRESSINFORMATIONQUEUE	m_infoQueue;
	CCriticalSection			m_csInfoQueue;	// 当前处理文件
	DWORD						m_dwTotalSize;
	DWORD						m_dwDoneSize;
public:
	CStatic m_static_information;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
