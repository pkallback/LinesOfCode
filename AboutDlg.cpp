// AboutDlg.cpp : Implementation file.
//

#include "pch.h"
#include "LOC.h"
#include "AboutDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAboutDlg::CAboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX) {
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX) {
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CLOCApp::OnAppAbout() {
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}
