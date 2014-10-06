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
	char	szFileFullPath[MAX_PATH];	// 相对路径+文件/目录名
	DWORD	dwFileAttributes;			// 属性
	DWORD	dwFileSize;					// 原始大小
}BINARYINSTALLRESOURCEHEAD;

typedef struct tagCompressedPart
{
	DWORD	dwPartSize;				// 分割段大小(<=1MB)
	DWORD	dwCompressedFileSize;	// 压缩大小
	unsigned char	szCompressProp[5];
}COMPRESSEDPART;
//////////////////////////////////////////////////////////////////////////
//	生成合并文件
//////////////////////////////////////////////////////////////////////////

//	先文件，后目录(目录递归调用)，保留空目录
extern size_t GetPartLength(size_t srcLen, int nCount);
extern int CreatePartCompressedBinary(const char* pszRoot, const char* pszSrcFileDirectory, const char* pszDesFileDirectory, char* szMsg, int nMsgLen);
//extern int OutputCompressedBinary(BINARYINSTALLRESOURCEHEAD* pHead,const char* pszSrcFile, const char* pszDesFileDirectory);
extern int OutputPartCompressedBinary(BINARYINSTALLRESOURCEHEAD* pHead,const char* pszSrcFile, const char* pszDesFileDirectory, DWORD dwPartSize=COMPRESS_PART_LEN);
extern DWORD CalculatePackageSize(const char* pszSrcFile);
//extern int TestFileExist(const char* pszFileName);

/*
extern void ShowProgress(const char* pszFile,DWORD dwTotalLen,DWORD dwDealLength);

//////////////////////////////////////////////////////////////////////////
//	解析文件
//////////////////////////////////////////////////////////////////////////

extern int UnPackageFile(const char* pszSrcFile, const char* pszDesPath, char* szMsg, int nMsgLen);
*/

// 0-目录,1-文件,-1错误格式
extern int CheckSourceFileName(CString strFileName);
extern int CheckDestinationFileName(CString strFileName);

//////////////////////////////////////////////////////////////////////////
//	自定义宏
//////////////////////////////////////////////////////////////////////////

//	自定义消息
#define THREAD_CREATE_BINARY_RESOURCE_OVER			WM_USER+0x0001

//	生成资源文件线程处理结果
#define CREATE_BINARY_RESOURCE_STATUS_SUCCESS		0
#define CREATE_BINARY_RESOURCE_STATUS_FAIL			1
#define CREATE_BINARY_RESOURCE_STATUS_CANCEL		2


#endif