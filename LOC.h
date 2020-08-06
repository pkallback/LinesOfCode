// LOC.h : main header file for the LOC application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CLOCApp:
// See LOC.cpp for the implementation of this class
//

class CLOCApp : public CWinAppEx
{
public:
	CLOCApp() noexcept;

private:
	// GDI+
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	bool mIsSlnFileOpen{ false };

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

public:
	void SetSlnFileOpen() {
		mIsSlnFileOpen = true;
	}
	void ConfirmSlnFileOpen() {
		mIsSlnFileOpen = false;
	}
	bool IsSlnFileOpen() {
		return mIsSlnFileOpen;
	}
};

extern CLOCApp theApp;
