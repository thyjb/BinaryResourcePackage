#ifndef _FUNCTION_H
#define _FUNCTION_H

#include "vector"
#include "string"
#include "stack"
#include "queue"

using namespace std;

#define COMPRESS_PART_LEN	2097152

typedef vector<string>	STRINGVECTOR;
typedef stack<string>	DIRECTORYSTACK;
typedef queue<string>	PROGRESSINFORMATIONQUEUE;

typedef struct tagBinaryInstallResourceHead
{
	char	szFileFullPath[MAX_PATH];	// ���·��+�ļ�/Ŀ¼��
	DWORD	dwFileAttributes;			// ����
	DWORD	dwFileSize;					// ԭʼ��С
}BINARYINSTALLRESOURCEHEAD;

typedef struct tagCompressedPart
{
	DWORD	dwPartSize;				// �ָ�δ�С(<=1MB)
	DWORD	dwCompressedFileSize;	// ѹ����С
	unsigned char	szCompressProp[5];
}COMPRESSEDPART;
//////////////////////////////////////////////////////////////////////////
//	���ɺϲ��ļ�
//////////////////////////////////////////////////////////////////////////

//	���ļ�����Ŀ¼(Ŀ¼�ݹ����)��������Ŀ¼
extern size_t GetPartLength(size_t srcLen, int nCount);
extern int CreatePartCompressedBinary(const char* pszRoot, const char* pszSrcFileDirectory, const char* pszDesFileDirectory, char* szMsg, int nMsgLen);
//extern int OutputCompressedBinary(BINARYINSTALLRESOURCEHEAD* pHead,const char* pszSrcFile, const char* pszDesFileDirectory);
extern int OutputPartCompressedBinary(BINARYINSTALLRESOURCEHEAD* pHead,const char* pszSrcFile, const char* pszDesFileDirectory, DWORD dwPartSize=COMPRESS_PART_LEN);
extern DWORD CalculatePackageSize(const char* pszSrcFile);
//extern int TestFileExist(const char* pszFileName);

/*
extern void ShowProgress(const char* pszFile,DWORD dwTotalLen,DWORD dwDealLength);

//////////////////////////////////////////////////////////////////////////
//	�����ļ�
//////////////////////////////////////////////////////////////////////////

extern int UnPackageFile(const char* pszSrcFile, const char* pszDesPath, char* szMsg, int nMsgLen);
*/

// 0-Ŀ¼,1-�ļ�,-1�����ʽ
extern int CheckSourceFileName(CString strFileName);
extern int CheckDestinationFileName(CString strFileName);

//////////////////////////////////////////////////////////////////////////
//	�Զ����
//////////////////////////////////////////////////////////////////////////

//	�Զ�����Ϣ
#define THREAD_CREATE_BINARY_RESOURCE_OVER			WM_USER+0x0001

//	������Դ�ļ��̴߳�����
#define CREATE_BINARY_RESOURCE_STATUS_SUCCESS		0
#define CREATE_BINARY_RESOURCE_STATUS_FAIL			1
#define CREATE_BINARY_RESOURCE_STATUS_CANCEL		2


#endif