#include "stdafx.h"
#include "Function.h"

#include "LzmaLib.h"
#include <imagehlp.h>
#pragma comment(lib, "imagehlp.lib")

//////////////////////////////////////////////////////////////////////////
//	生成合并文件
//////////////////////////////////////////////////////////////////////////

/*
void ShowProgress(const char* pszFile,DWORD dwTotalLen,DWORD dwDealLength)
{
 	m_strProgress.Format("File=%s,dwTotalLength=%d,dwDealLength=%ld",pszFile,dwTotalLen,dwDealLength);
 	UpdateData(FALSE);
 	Invalidate();
}*/

//	先文件，后目录(目录递归调用)，保留空目录
int CreatePartCompressedBinary(const char* pszRoot, const char* pszSrcFileDirectory, const char* pszDesFileDirectory, char* szMsg, int nMsgLen)
{
	VERIFY(NULL!=szMsg);
	STRINGVECTOR	UnusedDirectoryVector;
	DIRECTORYSTACK	ScanDirectoryStack;
	BINARYINSTALLRESOURCEHEAD	FileHead;

	// 当前目录
	memset(&FileHead,0x00,sizeof(FileHead));
	sprintf(FileHead.szFileFullPath,"%s",pszRoot);

//	ShowProgress(FileHead.szFileFullPath,0,0);

	FileHead.dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;
	int nRet=OutputPartCompressedBinary(&FileHead,pszSrcFileDirectory,pszDesFileDirectory);
	if (0 != nRet)
	{
		sprintf(szMsg,"写目录[%s]失败",pszSrcFileDirectory);
		return -1;
	}

	WIN32_FIND_DATA	FindFileData;
	memset(&FindFileData,0x00,sizeof(FindFileData));
	// 处理文件
	char szScanDir[MAX_PATH]={0};
	sprintf(szScanDir,"%s*",pszSrcFileDirectory);
	HANDLE hFile=FindFirstFile(szScanDir,&FindFileData);
	if (hFile==INVALID_HANDLE_VALUE)
	{
		sprintf(szMsg,"%s","查找目录失败");
		TRACE(szMsg);
		TRACE("\r\n");
		return -1;
	}

	while (0 != FindNextFile(hFile,&FindFileData))
	{
		TRACE(FindFileData.cFileName);
		TRACE("\r\n");
		if (FILE_ATTRIBUTE_DIRECTORY != FindFileData.dwFileAttributes)
		{
			memset(&FileHead,0x00,sizeof(FileHead));
			sprintf(FileHead.szFileFullPath,"%s%s",pszRoot,FindFileData.cFileName);
			CString strSrcFile;
			strSrcFile.Format("%s%s",pszSrcFileDirectory,FindFileData.cFileName);
//			ShowProgress(strSrcFile,0,0);
			FileHead.dwFileAttributes=FindFileData.dwFileAttributes;
			FileHead.dwFileSize=(FindFileData.nFileSizeHigh * MAXDWORD) + FindFileData.nFileSizeLow;

			nRet=OutputPartCompressedBinary(&FileHead,strSrcFile,pszDesFileDirectory);
			if (0 != nRet)
			{
				sprintf(szMsg,"写文件[%s]失败",FindFileData.cFileName);
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

	// 处理目录
	for (STRINGVECTOR::iterator iPos=UnusedDirectoryVector.begin();iPos!=UnusedDirectoryVector.end();iPos++)
	{
		CString strRoot=(*iPos).c_str();
		int nPos=strRoot.Find(pszRoot);
		strRoot=strRoot.Right(strRoot.GetLength()-nPos+1);
		nRet=CreatePartCompressedBinary(strRoot, (*iPos).c_str(),pszDesFileDirectory,szMsg,nMsgLen);
		if (0 != nRet)
			return nRet;
	}
	return 0;
}

// 0第1段
size_t GetPartLength(size_t srcLen, int nCount)
{
	// 最后一段可以为1.1MB(1KB以内的数据不成一段)
	size_t nMiddleLen=(nCount+1)*COMPRESS_PART_LEN;
	
	if (nMiddleLen <= srcLen)
	{
		nMiddleLen=srcLen-nMiddleLen;
		if (nMiddleLen <= 1024)
			return COMPRESS_PART_LEN+nMiddleLen;

		return COMPRESS_PART_LEN;
	}

	return srcLen-nCount*COMPRESS_PART_LEN;
}

int OutputPartCompressedBinary(BINARYINSTALLRESOURCEHEAD* pHead,const char* pszSrcFile, const char* pszDesFileDirectory, DWORD dwPartSize)
{
	// 头部信息+分割段信息+数据+分割段信息+数据+..
	// 输出头部
	FILE* pFileHead = _tfopen(pszDesFileDirectory,"ab+");
	if (pFileHead == NULL)
	{
		_ftprintf(stderr, _T("创建文件出错!"));
		return -1;
	}

	fwrite(pHead,sizeof(char),sizeof(*pHead),pFileHead);
	if (FILE_ATTRIBUTE_DIRECTORY == pHead->dwFileAttributes)
	{
		fclose(pFileHead);
		return 0;
	}

	// 输出文件内容
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
	unsigned char* psrcRead = NULL; //原始文件数据
	unsigned char* pLzma = NULL;	//存放压缩数据
	size_t nDoneLen=0;
	size_t nPartLen=0;
	int	nPartCount=0;

	do
	{
		nPartLen = GetPartLength(srcLen,nPartCount);
		nPartCount++;
		nDoneLen+=nPartLen;
		destLen=nPartLen*2;

		psrcRead = new unsigned char[nPartLen]; //原始文件数据
		pLzma = new unsigned char[destLen]; //存放压缩数据
		memset(psrcRead,0x00,nPartLen);
		memset(pLzma,0x00,destLen);

		unsigned char prop[5] = {0};
		size_t sizeProp = 5;
		fread(psrcRead, sizeof(char), nPartLen, pFile);
		if (SZ_OK != LzmaCompress(pLzma, &destLen, psrcRead, nPartLen, prop,
			&sizeProp, 9, (1 << 24), 3, 0, 2, 32, 2))
		{
			//出错了
			_ftprintf(stderr, _T("压缩时出错！"));
			delete [] psrcRead;
			delete [] pLzma;
			fclose(pFile);
			fclose(pFileHead);
			return -1;
		}

		// 输出分割段头，分割断头不压缩
		COMPRESSEDPART compressedPart;
		memset(&compressedPart,0x00,sizeof(compressedPart));
		compressedPart.dwCompressedFileSize=destLen;
		compressedPart.dwPartSize=nPartLen;
		memcpy(compressedPart.szCompressProp,prop,sizeof(compressedPart.szCompressProp));
		fwrite(&compressedPart,sizeof(char),sizeof(compressedPart),pFileHead);
		//写入压缩后的数据
		fwrite(pLzma, sizeof(char), destLen, pFileHead);

		delete [] psrcRead;
		delete [] pLzma;
		psrcRead=NULL;
		pLzma=NULL;

		//ShowProgress(pszSrcFile,srcLen,nDoneLen);
	}
	while(nDoneLen < srcLen);

	fclose(pFile);
	fclose(pFileHead);

	delete [] psrcRead;
	delete [] pLzma;

	return 0;
}

DWORD CalculatePackageSize(const char* pszSrcFile)
{
	DWORD dwTotal=0;

	STRINGVECTOR	UnusedDirectoryVector;
	DIRECTORYSTACK	ScanDirectoryStack;

	// 当前目录
	dwTotal+=1;

	WIN32_FIND_DATA	FindFileData;
	memset(&FindFileData,0x00,sizeof(FindFileData));
	// 处理文件
	char szScanDir[MAX_PATH]={0};
	sprintf(szScanDir,"%s*",pszSrcFile);
	HANDLE hFile=FindFirstFile(szScanDir,&FindFileData);
	if (hFile==INVALID_HANDLE_VALUE)
	{
		TRACE("查找目录失败");
		TRACE("\r\n");
		return 0;
	}

	while (0 != FindNextFile(hFile,&FindFileData))
	{
		TRACE(FindFileData.cFileName);
		TRACE("\r\n");
		if (FILE_ATTRIBUTE_DIRECTORY != FindFileData.dwFileAttributes)
		{
			dwTotal += (FindFileData.nFileSizeHigh * MAXDWORD) + FindFileData.nFileSizeLow;
		}
		else
		{
			if (0==strcmp(".",FindFileData.cFileName) || 0==strcmp("..",FindFileData.cFileName))
				continue;
			string stringDirectory=pszSrcFile;
			stringDirectory+=FindFileData.cFileName;
			stringDirectory+="\\";
			UnusedDirectoryVector.push_back(stringDirectory);
		}
	}

	// 处理目录
	for (STRINGVECTOR::iterator iPos=UnusedDirectoryVector.begin();iPos!=UnusedDirectoryVector.end();iPos++)
	{
		CString strRoot=(*iPos).c_str();
		DWORD dwCurrent=CalculatePackageSize(strRoot);
		if (0==dwCurrent)
		{
			return 0;
		}
		dwTotal+=dwCurrent;
	}

	return dwTotal;
}
/*
int TestFileExist(const char* pszFileName)
{
	CFile file;
	if (file.Open(pszFileName,CFile::modeRead))
	{
		file.Close();
		return 1;
	}
	
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//	解析文件
//////////////////////////////////////////////////////////////////////////

int UnPackageFile(const char* pszSrcFile, const char* pszDesPath, char* szMsg, int nMsgLen)
{
	// 输出文件内容

	FILE* pFile = _tfopen(pszSrcFile, _T("rb"));
	
	if (pFile == NULL)
	{
		sprintf(szMsg,"打开源文件[%s]失败",
			pszSrcFile);
		return -1;
	}
	
	size_t srcLen = 0;
	size_t destLen = 0;
	unsigned char* psrcRead = NULL; //原始文件数据
	unsigned char* pDecomress = NULL; //存放解压缩数据
	
	unsigned char prop[5] = {0};
	size_t sizeProp = 5;

	size_t sizeRead = 0;
	CString strFileDir;
	do
	{
		// 文件头信息处理
		BINARYINSTALLRESOURCEHEAD	FileHead;
		memset(&FileHead,0x00,sizeof(FileHead));
		sizeRead=fread(&FileHead,sizeof(char),sizeof(FileHead),pFile);
		
		if (0==sizeRead)
		{
			fclose(pFile);
			break;
		}

		if (sizeRead != sizeof(FileHead))
		{
			sprintf(szMsg,"读取文件失败");
			fclose(pFile);
			return -1;
		}

		if (FILE_ATTRIBUTE_DIRECTORY == FileHead.dwFileAttributes)
		{
			// 创建目录
			strFileDir.Empty();
			strFileDir.Format("%s%s",pszDesPath,FileHead.szFileFullPath);
			CFileStatus FileStatus;
			if (!CFile::GetStatus(strFileDir, FileStatus))
			{
				if (0 == MakeSureDirectoryPathExists(strFileDir))
				{
					sprintf(szMsg,"创建目录[%s]失败",FileHead.szFileFullPath);
					return -1;
				}
			}

			continue;
		}

		// 写目标文件
		CString strFile;
		strFile.Format("%s%s",pszDesPath,FileHead.szFileFullPath);

		// 创建目录
		int nPos=strFile.ReverseFind('\\');
		if (-1 == nPos)
		{

		}

		strFileDir=strFile.Left(nPos);
		CFileStatus FileStatus;
		if (!CFile::GetStatus(strFileDir, FileStatus))
		{
			if (0 == MakeSureDirectoryPathExists(strFileDir))
			{
				sprintf(szMsg,"创建目录[%s]失败",FileHead.szFileFullPath);
				return -1;
			}
		}

		FILE* pDecompressFile = _tfopen(strFile, _T("ab")); 
		//写入解压缩数据
		if (pDecompressFile == NULL)
		{
			delete [] psrcRead;
			delete [] pDecomress;
			sprintf(szMsg,"写入文件[%s]失败",FileHead.szFileFullPath);
			return -1;
		}

		int nWriteDoneLength=0;
		do
		{
			// 读分割段信息
			COMPRESSEDPART	compressedPart;
			memset(&compressedPart,0x00,sizeof(compressedPart));
			sizeRead=fread(&compressedPart,sizeof(char),sizeof(compressedPart),pFile);
			if (0==sizeRead)
			{
				fclose(pFile);
				break;
			}

			if (sizeRead != sizeof(compressedPart))
			{
				sprintf(szMsg,"读取文件失败");
				fclose(pFile);
				fclose(pDecompressFile);
				return -1;
			}
			
			srcLen=compressedPart.dwCompressedFileSize;
			destLen=compressedPart.dwPartSize;

			psrcRead = new unsigned char[srcLen]; //原始文件数据
			pDecomress = new unsigned char[destLen]; //存放解压缩数据
			memset(psrcRead,0x00,srcLen);
			memset(pDecomress,0x00,destLen);

			sizeRead=fread(psrcRead,sizeof(char),srcLen,pFile);
			if (0==sizeRead)
			{
				fclose(pFile);
				delete [] psrcRead;
				delete [] pDecomress;
				break;
			}
			if (sizeRead != srcLen)
			{
				sprintf(szMsg,"读取文件失败");
				fclose(pFile);
				fclose(pDecompressFile);
				delete [] psrcRead;
				delete [] pDecomress;
				return -1;
			}

			//注意：解压缩时props参数要使用压缩时生成的outProps，这样才能正常解压缩
			if (SZ_OK != LzmaUncompress(pDecomress, &destLen, psrcRead, &srcLen, compressedPart.szCompressProp, 5))
			{
				delete [] psrcRead;
				delete [] pDecomress;
				fclose(pFile);
				fclose(pDecompressFile);
				sprintf(szMsg,"文件[%s]解压失败",FileHead.szFileFullPath);
				return -1;
			}

			if (compressedPart.dwPartSize != destLen)
			{
				delete [] psrcRead;
				delete [] pDecomress;
				fclose(pFile);
				fclose(pDecompressFile);
				sprintf(szMsg,"文件[%s]损坏",FileHead.szFileFullPath);
				return -1;
			}

			fwrite(pDecomress, sizeof(char), destLen, pDecompressFile);
			nWriteDoneLength+=destLen;
		}
		while (nWriteDoneLength < FileHead.dwFileSize);

		fclose(pDecompressFile);

	}
	while(1);

	fclose(pFile);

	return 0;
}
*/
int CheckSourceFileName(CString strFileName)
{
	if (""==strFileName)
		return -1;

	if (PathIsDirectory(strFileName))
	{
		return 0;
	}
	else
	{
		// 检查文件是否存在
		CFileStatus FileStatus;
		if (!CFile::GetStatus(strFileName,FileStatus))
		{
			return -1;
		}

		return 0;
	}
	return -1;
}

int CheckDestinationFileName(CString strFileName)
{
	if (""==strFileName)
		return -1;

	if (PathIsDirectory(strFileName))
	{
		return -1;
	}
	else
	{
		// 检查目录是否正确
		int nPosition=strFileName.Find('/');
		if (-1==nPosition)
		{
			nPosition=strFileName.Find('\\');
		}
		if (-1==nPosition)
		{
			return -1;
		}
		CString strPath=strFileName.Left(nPosition);
		if (PathIsDirectory(strPath))
		{
			return 0;
		}

		return -1;
	}
	return -1;
}