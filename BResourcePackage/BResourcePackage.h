
// BResourcePackage.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CBResourcePackageApp:
// �йش����ʵ�֣������ BResourcePackage.cpp
//

class CBResourcePackageApp : public CWinApp
{
public:
	CBResourcePackageApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CBResourcePackageApp theApp;