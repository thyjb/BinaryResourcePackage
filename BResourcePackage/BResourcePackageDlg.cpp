
// BResourcePackageDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "BResourcePackage.h"
#include "BResourcePackageDlg.h"
#include "afxdialogex.h"

#include "Function.h"

#include "LzmaLib.h"
#include <imagehlp.h>
#pragma comment(lib, "imagehlp.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBResourcePackageDlg �Ի���




CBResourcePackageDlg::CBResourcePackageDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CBResourcePackageDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_strSource = _T("");
	m_strInformation = _T("");
	m_strSrcRoot = _T("");
	m_hThread=NULL;
	m_bExitTask=FALSE;
 	m_nSrcType=-1;
}

void CBResourcePackageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_SOURCE, m_strSource);
	DDX_Text(pDX, IDC_EDIT_DESTINATION, m_strDestination);
	DDX_Control(pDX, IDC_PROGRESS, m_pcProgress);
	DDX_Text(pDX, IDC_STATIC_INFORMATION, m_strInformation);
	DDX_Control(pDX, IDC_STATIC_INFORMATION, m_static_information);
}

BEGIN_MESSAGE_MAP(CBResourcePackageDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_EXPDES, &CBResourcePackageDlg::OnClickedButtonExpdes)
	ON_BN_CLICKED(IDC_BUTTON_EXPSRC, &CBResourcePackageDlg::OnClickedButtonExpsrc)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CBResourcePackageDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CBResourcePackageDlg::OnBnClickedOk)
	ON_MESSAGE(THREAD_CREATE_BINARY_RESOURCE_OVER,&CBResourcePackageDlg::OnCreateResourceOver)
ON_WM_TIMER()
END_MESSAGE_MAP()


// CBResourcePackageDlg ��Ϣ�������

BOOL CBResourcePackageDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CBResourcePackageDlg::OnPaint()
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
HCURSOR CBResourcePackageDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CBResourcePackageDlg::OnClickedButtonExpdes()
{
	// TODO: Add your control notification handler code here
	TCHAR szFilter[] = _T("Binary Files (*.exe)||");
	CFileDialog FileDlg(FALSE,
		_T("exe"),
		NULL,
		OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		szFilter,
		NULL,
		0,
		TRUE);
	if (IDOK==FileDlg.DoModal())
	{
		m_strDestination=FileDlg.GetPathName();
		UpdateData(FALSE);
	}
}	


void CBResourcePackageDlg::OnClickedButtonExpsrc()
{
	// TODO: Add your control notification handler code here
	BROWSEINFO binfo;
	memset(&binfo,0x00,sizeof(binfo));
	binfo.hwndOwner=GetSafeHwnd();
	TCHAR szSrcPath[MAX_PATH]={0};
	binfo.lpszTitle=_T("��ѡ��Դ�ļ�/Ŀ¼");
	binfo.ulFlags=BIF_BROWSEINCLUDEFILES;//BIF_RETURNONLYFSDIRS;
	LPITEMIDLIST lpDlist;
	lpDlist=SHBrowseForFolder(&binfo);
	if (NULL!=lpDlist)
	{
		SHGetPathFromIDList(lpDlist,szSrcPath);
		m_strSource=szSrcPath;
		UpdateData(FALSE);
	}

}


void CBResourcePackageDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	ExitThread();
	InitWtFace(0);
}


void CBResourcePackageDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	InitWtFace(1);

	UpdateData(TRUE);
	CString strMsg;
	int nSrcType=CheckSourceFileName(m_strSource);
	if (-1 == nSrcType)
	{
		strMsg=_T("��ѡ����Դ�ļ�/Ŀ¼");
		MessageBox(strMsg);
		InitWtFace(0);
		return;
	}
	m_nSrcType=nSrcType;
	if (0 == m_nSrcType)
	{
		if ('\\' != m_strSource.GetAt(m_strSource.GetLength()-1)
			&& '/' != m_strSource.GetAt(m_strSource.GetLength()-1) )
		{
			m_strSource+="\\";
		}
	}

	int nDesType=CheckDestinationFileName(m_strDestination);
	if (-1 == nDesType)
	{
		strMsg=_T("��ѡ��Ŀ���ļ�");
		MessageBox(strMsg);
		InitWtFace(0);
		return;
	}

	// ����ִ���߳�
	if (!InitThread(strMsg))
	{
		MessageBox(strMsg);
		InitWtFace(0);
		return;
	}

}

// 0-��ִ��״̬��1-ȡ��ִ��״̬
void CBResourcePackageDlg::InitWtFace(int nStatus)
{
	UINT uID[]={IDC_EDIT_SOURCE,IDC_BUTTON_EXPSRC,IDC_EDIT_DESTINATION,IDC_BUTTON_EXPDES,IDOK};
	switch(nStatus)
	{
	case 0:
		{
			for (int i=0;i<sizeof(uID)/sizeof(uID[0]);i++)
			{
				GetDlgItem(uID[i])->EnableWindow(TRUE);
			}

			GetDlgItem(IDC_BUTTON_CANCEL)->EnableWindow(FALSE);
		}
		break;

	case 1:
		{
			for (int i=0;i<sizeof(uID)/sizeof(uID[0]);i++)
			{
				GetDlgItem(uID[i])->EnableWindow(FALSE);
			}

			GetDlgItem(IDC_BUTTON_CANCEL)->EnableWindow(TRUE);
		}
		break;

	default:
		break;
	}

}


LRESULT CBResourcePackageDlg::OnCreateResourceOver(WPARAM wParam,LPARAM lParam)
{
	int nResult=(int)wParam;
	char* pMsg=(char*)lParam;

	if (CREATE_BINARY_RESOURCE_STATUS_FAIL==nResult)
	{
		MessageBox(pMsg);
	}
	WaitForSingleObject(m_hThread,INFINITE);
	KillTimer(0x01);
	CloseThreadHand();
	m_bExitTask=FALSE;
	while(0 != m_infoQueue.size())
	{
		m_infoQueue.pop();
	}
	m_strInformation=_T("");
	UpdateData(FALSE);
	m_static_information.Invalidate();
	m_pcProgress.SetPos(0);
	InitWtFace(0);
	return 0;
}

//////////////////////////////////////////////////////////////////////////
//	ִ���߳�
//////////////////////////////////////////////////////////////////////////
void CBResourcePackageDlg::CloseThreadHand()
{
	if (NULL!=m_hThread)
	{
		CloseHandle(m_hThread);
		m_hThread=NULL;
	}

	// 	if (NULL != m_hExitEvent)
	// 	{
	// 		CloseHandle(m_hExitEvent);
	// 		m_hExitEvent=NULL;
	// 	}
}

BOOL CBResourcePackageDlg::InitThread(CString& strMsg)
{
	CloseThreadHand();
	m_bExitTask=FALSE;

	m_dwTotalSize=CalculatePackageSize(m_strSource);
	if (0==m_dwTotalSize)
	{
		strMsg=_T("����Դ�ļ�����Сʧ��");
		return FALSE;
	}
	m_dwDoneSize=0;
	m_pcProgress.SetRange(0,m_dwTotalSize/1024);
	m_pcProgress.SetPos(0);
	m_csInfoQueue.Lock();
	while(0 != m_infoQueue.size())
	{
		m_infoQueue.pop();
	}
	m_csInfoQueue.Unlock();

	SetTimer(0x01,100,NULL);

	m_hThread=(HANDLE)_beginthreadex(NULL,0,ThreadProc,(void* )this,1,NULL);
	if (NULL==m_hThread)
	{
		KillTimer(0x01);
		strMsg=_T("���������߳�ʧ��");
		CloseThreadHand();
		return FALSE;
	}
	return TRUE;
}

int CBResourcePackageDlg::ThreadDoJob()
{
	static char szMsg[1024]={0};

	while(TRUE)
	{
		memset(szMsg,0x00,sizeof(szMsg));

		// ���ȡ��
		if (m_bExitTask)
		{
			::PostMessage(GetSafeHwnd(),THREAD_CREATE_BINARY_RESOURCE_OVER,(WPARAM)CREATE_BINARY_RESOURCE_STATUS_CANCEL,NULL);
			return 0;
		}

		// ����ָ������
		int nReturn=0;
		switch (m_nSrcType)
		{
		case 0:	// Ŀ¼
			{
				memset(szMsg,0x00,sizeof(szMsg));
				nReturn=CreatePartCompressedBinary(m_strSrcRoot,m_strSource,m_strDestination,szMsg,sizeof(szMsg));
				if (0 != nReturn)
				{
					::PostMessage(GetSafeHwnd(),THREAD_CREATE_BINARY_RESOURCE_OVER,(WPARAM)CREATE_BINARY_RESOURCE_STATUS_FAIL,(LPARAM)szMsg);
				}
				else
				{
					::PostMessage(GetSafeHwnd(),THREAD_CREATE_BINARY_RESOURCE_OVER,(WPARAM)CREATE_BINARY_RESOURCE_STATUS_SUCCESS,(LPARAM)szMsg);
				}
				return nReturn;
			}
			break;
		case 1:	// �ļ�
			{

			}
			break;
		default:
			break;
		}
	}
	return 0;
}

void CBResourcePackageDlg::ExitThread()
{
	
	m_bExitTask=TRUE;
	WaitForSingleObject(m_hThread,INFINITE);
	KillTimer(0x01);
	m_pcProgress.SetPos(0);
	CloseThreadHand();
	m_bExitTask=FALSE;
}

unsigned WINAPI CBResourcePackageDlg::ThreadProc(void* lpParam)
{
	CBResourcePackageDlg* pThread=(CBResourcePackageDlg* )lpParam;
	VERIFY(NULL != lpParam);
	return pThread->ThreadDoJob();
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//	������Դ�ļ�
//////////////////////////////////////////////////////////////////////////
void CBResourcePackageDlg::ShowProgress(const char* pszFile,DWORD dwTotalLen,DWORD dwDealLength)
{
// 	m_strInformation.Format("File=%s,t=%d,d=%d",pszFile,dwTotalLen,dwDealLength);
// 	UpdateData(FALSE);
// 	m_static_information.Invalidate();
// 	Sleep(100);
	string strInformation=pszFile;
	m_csInfoQueue.Lock();
	m_infoQueue.push(strInformation);
	m_dwDoneSize+=dwDealLength;
	m_pcProgress.SetPos(m_dwDoneSize/1024);
	m_csInfoQueue.Unlock();
}

//	���ļ�����Ŀ¼(Ŀ¼�ݹ����)��������Ŀ¼
int CBResourcePackageDlg::CreatePartCompressedBinary(const char* pszRoot, const char* pszSrcFileDirectory, const char* pszDesFileDirectory, char* szMsg, int nMsgLen)
{
	VERIFY(NULL!=szMsg);
	STRINGVECTOR	UnusedDirectoryVector;
	DIRECTORYSTACK	ScanDirectoryStack;
	BINARYINSTALLRESOURCEHEAD	FileHead;

	// ��ǰĿ¼
	memset(&FileHead,0x00,sizeof(FileHead));
	sprintf(FileHead.szFileFullPath,"%s",pszRoot);

	ShowProgress(FileHead.szFileFullPath,0,0);

	FileHead.dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;
	int nRet=OutputPartCompressedBinary(&FileHead,pszSrcFileDirectory,pszDesFileDirectory);
	if (0 != nRet)
	{
		sprintf(szMsg,"дĿ¼[%s]ʧ��",pszSrcFileDirectory);
		return -1;
	}

	WIN32_FIND_DATA	FindFileData;
	memset(&FindFileData,0x00,sizeof(FindFileData));
	// �����ļ�
	char szScanDir[MAX_PATH]={0};
	sprintf(szScanDir,"%s*",pszSrcFileDirectory);
	HANDLE hFile=FindFirstFile(szScanDir,&FindFileData);
	if (hFile==INVALID_HANDLE_VALUE)
	{
		sprintf(szMsg,"%s","����Ŀ¼ʧ��");
		TRACE(szMsg);
		TRACE("\r\n");
		return -1;
	}

	while (0 != FindNextFile(hFile,&FindFileData))
	{
		// ���ȡ��
		if (m_bExitTask)
		{
			::PostMessage(GetSafeHwnd(),THREAD_CREATE_BINARY_RESOURCE_OVER,(WPARAM)CREATE_BINARY_RESOURCE_STATUS_CANCEL,NULL);
			return 0;
		}

		TRACE(FindFileData.cFileName);
		TRACE("\r\n");
		if (FILE_ATTRIBUTE_DIRECTORY != FindFileData.dwFileAttributes)
		{
			memset(&FileHead,0x00,sizeof(FileHead));
			sprintf(FileHead.szFileFullPath,"%s%s",pszRoot,FindFileData.cFileName);
			CString strSrcFile;
			strSrcFile.Format("%s%s",pszSrcFileDirectory,FindFileData.cFileName);
			ShowProgress(strSrcFile,0,0);
			FileHead.dwFileAttributes=FindFileData.dwFileAttributes;
			FileHead.dwFileSize=(FindFileData.nFileSizeHigh * MAXDWORD) + FindFileData.nFileSizeLow;

			nRet=OutputPartCompressedBinary(&FileHead,strSrcFile,pszDesFileDirectory);
			if (0 != nRet)
			{
				sprintf(szMsg,"д�ļ�[%s]ʧ��",FindFileData.cFileName);
				return -1;
			}
		}
		else
		{
			if (0==strcmp(".",FindFileData.cFileName) || 0==strcmp("..",FindFileData.cFileName))
				continue;
			string stringDirectory=pszSrcFileDirectory;
			stringDirectory+=FindFileData.cFileName;
			stringDirectory+="\\";
			UnusedDirectoryVector.push_back(stringDirectory);
		}
	}

	// ����Ŀ¼
	for (STRINGVECTOR::iterator iPos=UnusedDirectoryVector.begin();iPos!=UnusedDirectoryVector.end();iPos++)
	{
		// ���ȡ��
		if (m_bExitTask)
		{
			::PostMessage(GetSafeHwnd(),THREAD_CREATE_BINARY_RESOURCE_OVER,(WPARAM)CREATE_BINARY_RESOURCE_STATUS_CANCEL,NULL);
			return 0;
		}

		CString strRoot=(*iPos).c_str();
		int nPos=strRoot.Find(pszRoot);
		strRoot=strRoot.Right(strRoot.GetLength()-nPos+1);
		nRet=CreatePartCompressedBinary(strRoot, (*iPos).c_str(),pszDesFileDirectory,szMsg,nMsgLen);
		if (0 != nRet)
			return nRet;
	}
	return 0;
}

int CBResourcePackageDlg::OutputPartCompressedBinary(BINARYINSTALLRESOURCEHEAD* pHead,const char* pszSrcFile, const char* pszDesFileDirectory, DWORD dwPartSize)
{
	// ͷ����Ϣ+�ָ����Ϣ+����+�ָ����Ϣ+����+..
	// ���ͷ��
	FILE* pFileHead = _tfopen(pszDesFileDirectory,"ab+");
	if (pFileHead == NULL)
	{
		_ftprintf(stderr, _T("�����ļ�����!"));
		return -1;
	}

//	ShowProgress(pszSrcFile,sizeof(BINARYINSTALLRESOURCEHEAD),0);
	fwrite(pHead,sizeof(char),sizeof(*pHead),pFileHead);
	if (FILE_ATTRIBUTE_DIRECTORY == pHead->dwFileAttributes)
	{
		fclose(pFileHead);
		return 0;
	}
//	ShowProgress(pszSrcFile,sizeof(*pHead),sizeof(*pHead));
	UpdateProgressPos(sizeof(*pHead));

	// ����ļ�����
	FILE* pFile = _tfopen(pszSrcFile, _T("rb"));

	if (pFile == NULL)
	{
		_ftprintf(stderr, _T("Error to Open the file!"));
		fclose(pFileHead);
		return -1;
	}

	size_t srcLen = 0;
	fseek(pFile, 0, SEEK_END);
	srcLen = ftell(pFile);
	rewind(pFile);
	VERIFY(srcLen==pHead->dwFileSize);
	srcLen=pHead->dwFileSize;

	size_t destLen = 0;
	unsigned char* psrcRead = NULL; //ԭʼ�ļ�����
	unsigned char* pLzma = NULL;	//���ѹ������
	size_t nDoneLen=0;
	size_t nPartLen=0;
	int	nPartCount=0;

//	ShowProgress(pszSrcFile,srcLen,0);
	do
	{
		// ���ȡ��
		if (m_bExitTask)
		{
			::PostMessage(GetSafeHwnd(),THREAD_CREATE_BINARY_RESOURCE_OVER,(WPARAM)CREATE_BINARY_RESOURCE_STATUS_CANCEL,NULL);
			return 0;
		}

		nPartLen = GetPartLength(srcLen,nPartCount);
		nPartCount++;
		nDoneLen+=nPartLen;
		destLen=nPartLen*2;

		psrcRead = new unsigned char[nPartLen+1]; //ԭʼ�ļ�����
		pLzma = new unsigned char[destLen]; //���ѹ������
		memset(psrcRead,0x00,nPartLen+1);
		memset(pLzma,0x00,destLen);

		unsigned char prop[5] = {0};
		size_t sizeProp = 5;
		fread(psrcRead, sizeof(char), nPartLen, pFile);
		if (SZ_OK != LzmaCompress(pLzma, &destLen, psrcRead, nPartLen, prop,
			&sizeProp, 9, (1 << 24), 3, 0, 2, 32, 2))
		{
			//������
			_ftprintf(stderr, _T("ѹ��ʱ����"));
			delete[] psrcRead;
			delete[] pLzma;
			fclose(pFile);
			fclose(pFileHead);
			return -1;
		}

		// ����ָ��ͷ���ָ��ͷ��ѹ��
		COMPRESSEDPART compressedPart;
		memset(&compressedPart,0x00,sizeof(compressedPart));
		compressedPart.dwCompressedFileSize=destLen;
		compressedPart.dwPartSize=nPartLen;
		memcpy(compressedPart.szCompressProp,prop,sizeof(compressedPart.szCompressProp));
		fwrite(&compressedPart,sizeof(char),sizeof(compressedPart),pFileHead);
		//д��ѹ���������
		fwrite(pLzma, sizeof(char), destLen, pFileHead);

		delete[] psrcRead;
		delete[] pLzma;
		psrcRead=NULL;
		pLzma=NULL;

//		ShowProgress(pszSrcFile,srcLen,nDoneLen);
		UpdateProgressPos(nPartLen);
	}
	while(nDoneLen < srcLen);

	fclose(pFile);
	fclose(pFileHead);

	delete[] psrcRead;
	delete[] pLzma;
	psrcRead=NULL;
	pLzma=NULL;

	return 0;
}

void CBResourcePackageDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (0x01 == nIDEvent)
	{
		KillTimer(0x01);
		UpdateProgress();
		SetTimer(0x01,200,NULL);
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CBResourcePackageDlg::UpdateProgress()
{
	m_csInfoQueue.Lock();
	if (!m_infoQueue.empty())
	{
		string strInfo=m_infoQueue.front();
		m_infoQueue.pop();

		m_strInformation=strInfo.c_str();
	}
	else
	{
		UpdateData(TRUE);
		if (!m_strInformation.IsEmpty())
		{
			if ("..." == m_strInformation.Right(3))
			{
				m_strInformation=m_strInformation.Left(m_strInformation.GetLength()-3);
				m_strInformation+=".";
			}
			else if (".."==m_strInformation.Right(2))
			{
				m_strInformation=m_strInformation.Left(m_strInformation.GetLength()-2);
				m_strInformation+="...";
			}
			else if ("."==m_strInformation.Right(1))
			{
				m_strInformation=m_strInformation.Left(m_strInformation.GetLength()-1);
				m_strInformation+="..";
			}
			else
			{
				m_strInformation+=".";
			}
		}
	}
	m_csInfoQueue.Unlock();
	UpdateData(FALSE);
	m_static_information.Invalidate();
	TRACE(m_strInformation);
	TRACE("\r\n");
}

void CBResourcePackageDlg::UpdateProgressPos(size_t dwDone)
{
	m_dwDoneSize+=dwDone;
	m_pcProgress.SetPos(m_dwDoneSize/1024);
}
