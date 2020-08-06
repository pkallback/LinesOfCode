// LOCView.cpp : implementation of the CLOCView class
//

#include "pch.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "LOC.h"
#endif

#include "../../Comn/DBGraphics/DBGraphics.h"
#include "DrawObject.h"
#include "LOCDoc.h"
#include "LOCView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CLOCView

IMPLEMENT_DYNCREATE(CLOCView, CView)

BEGIN_MESSAGE_MAP(CLOCView, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_COMMAND(ID_FILE_OPENSOURCE, &CLOCView::OnFileOpenSource)
END_MESSAGE_MAP()

// CLOCView construction/destruction

CLOCView::CLOCView() noexcept {
}

CLOCView::~CLOCView() {
}

BOOL CLOCView::PreCreateWindow(CREATESTRUCT& cs) {
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	mpMemGraphics = std::make_shared<CDBGraphics>();
	mpDrawObject = std::make_shared<CDrawObject>();

	return CView::PreCreateWindow(cs);
}

// CLOCView drawing

void CLOCView::OnDraw(CDC* pDC) {
	// Include the framework for GDI+ and the nice double buffer routines!
	CRect rect;
	GetClientRect(&rect);

	Graphics g(pDC->m_hDC);

	CLOCDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) {
		return;
	}

	CString pathName = pDoc->GetPathName();

	if (!pathName.IsEmpty() && theApp.IsSlnFileOpen()) {
		theApp.ConfirmSlnFileOpen();
		bool ret = mSlnAlgorithm.ExtractSlnInformation(pathName);
		if (!ret) {
			AfxMessageBox(_T("Couldn't Extract Sln Information!"), 
				MB_ICONEXCLAMATION);
		}
		else {
			ret = mSlnAlgorithm.ExtractProjectFilterFile();
			if (!ret) {
				AfxMessageBox(_T("Couldn't Extract Project Filter File!"), 
					MB_ICONEXCLAMATION);
			}
			else {
				ret = mSlnAlgorithm.CountStatistics();
				if (!ret) {
					AfxMessageBox(_T("Couldn't Count Statistics!"), 
						MB_ICONEXCLAMATION);
				}
			}
		}

		if (ret) {
			UpdateListCtrl();
		}

		mpDrawObject->SetFilenamePath(pathName);
		mUpdateGraphics = TRUE;
	}

	// Check if GDI+ double buffer is working
	if (mpMemGraphics->CanDoubleBuffer()) {
	
		if (mUpdateGraphics) {
			mUpdateGraphics = FALSE;
			mpDrawObject->DrawObject(mpMemGraphics->GetHandle());
	
			mpMemGraphics->Render(&g);
	
			// Copy window content to memory DC
			mpMemGraphics->CopyDC(pDC, rect.Width(), rect.Height());
		}
		else {
			// This part in the code is  running when the
			// window is not in focus.
			mpMemGraphics->Render(&g);
		}
	}

	// When a document is opened OnDraw is updated.
	// Write the directory path and the name of the files included 
}

void CLOCView::OnRButtonUp(UINT /* nFlags */, CPoint point) {
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CLOCView::OnContextMenu(CWnd* /* pWnd */, CPoint point) {
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, 
		point.x, point.y, this, TRUE);
#endif
}

// CLOCView diagnostics

#ifdef _DEBUG
void CLOCView::AssertValid() const {
	CView::AssertValid();
}

void CLOCView::Dump(CDumpContext& dc) const {
	CView::Dump(dc);
}

CLOCDoc* CLOCView::GetDocument() const // non-debug version is inline {
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CLOCDoc)));
	return (CLOCDoc*)m_pDocument;
}
#endif //_DEBUG

// CLOCView message handlers

BOOL CLOCView::OnEraseBkgnd(CDC* pDC) {
	return TRUE;
}

void CLOCView::OnSize(UINT nType, int cx, int cy) {
	CView::OnSize(nType, cx, cy);

	BOOL bRet;

	cx = cx < 2 ? 2 : cx;
	cy = cy < 2 ? 2 : cy;

	mCx = cx;
	mCy = cy;

	// Create double buffer for GDI+
	if (mpMemGraphics == NULL) {
		return;
	}

	bRet = mpMemGraphics->CreateDoubleBuffer(cx, cy);
	ASSERT(bRet);

	// Set perimeter for window in draw object
	mpDrawObject->SetWindowRect(cx, cy);

	if (!bRet) {
		return;
	}

	// Resize list control
	CRect clientRect;
	GetClientRect(clientRect);
	WINDOWPLACEMENT placement;
	mListCtrl.GetWindowPlacement(&placement);
	placement.rcNormalPosition.top = clientRect.top;
	placement.rcNormalPosition.left = clientRect.left;
	placement.rcNormalPosition.right = clientRect.right;
	placement.rcNormalPosition.bottom = clientRect.bottom;
	mListCtrl.SetWindowPlacement(&placement);

	mUpdateGraphics = TRUE;
	Invalidate(FALSE);
}

int CLOCView::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CView::OnCreate(lpCreateStruct) == -1) {
		return -1;
	}

	// TODO:  Add your specialized creation code here
	mFont.CreatePointFont(100, _T("Consolas"));

	DWORD dwStyle = WS_CHILD|WS_VISIBLE|LVS_REPORT|LVS_EDITLABELS;
	CRect rect(10, 10, 200, 100);
	mListCtrl.Create(dwStyle, rect, this, IDC_LISTCTRL);

	mListCtrl.SetFont(&mFont);

	mListCtrl.InsertColumn(0, _T("#"), LVCFMT_LEFT, 30);
	mListCtrl.InsertColumn(1, _T("Project"), LVCFMT_LEFT, 150);
	mListCtrl.InsertColumn(2, _T("Filename"), LVCFMT_LEFT, 200);
	mListCtrl.InsertColumn(3, _T("Extension"), LVCFMT_LEFT, 75);
	mListCtrl.InsertColumn(4, _T("Filetype"), LVCFMT_LEFT, 120);
	mListCtrl.InsertColumn(5, _T("Lines"), LVCFMT_RIGHT, 75);
	mListCtrl.InsertColumn(6, _T("Blank"), LVCFMT_RIGHT, 75);
	mListCtrl.InsertColumn(7, _T("Comment"), LVCFMT_RIGHT, 75);
	mListCtrl.InsertColumn(8, _T("Code"), LVCFMT_RIGHT, 75);

	return 0;
}

void CLOCView::UpdateListCtrl() {
	mListCtrl.DeleteAllItems();

	int itemCount = 0;
	for (auto filterFile : mSlnAlgorithm.mFilterFileArray) {
		CString val;
		val.Format(_T("%d"), itemCount + 1);
		mListCtrl.InsertItem(itemCount, val);
		mListCtrl.SetItemData(itemCount, 0);
		mListCtrl.SetItemText(itemCount, 1, filterFile.mProjectName);
		mListCtrl.SetItemText(itemCount, 2, filterFile.mFnameExt);
		mListCtrl.SetItemText(itemCount, 3, filterFile.mExt);
		mListCtrl.SetItemText(itemCount, 4, filterFile.mFileType);
		val.Format(_T("%d"), filterFile.mLines);
		mListCtrl.SetItemText(itemCount, 5, val);
		val.Format(_T("%d"), filterFile.mBlank);
		mListCtrl.SetItemText(itemCount, 6, val);
		val.Format(_T("%d"), filterFile.mComment);
		mListCtrl.SetItemText(itemCount, 7, val);
		val.Format(_T("%d"), filterFile.mCode);
		mListCtrl.SetItemText(itemCount, 8, val);
		itemCount++;
	}

	mListCtrl.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, 
		LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	mListCtrl.EnableMarkSortedColumn();
}

void CLOCView::OnFileOpenSource() {
	CFileDialog dlg(
		TRUE, // true for File Open dialog box
		NULL, // The default file name extension
		NULL, // The default file name
		OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR, // bunch of flags
		NULL);
	auto result = dlg.DoModal();
	if (result != IDOK) {
		mPathNameSource = _T("");
		return ; // failed
	}
	mPathNameSource = dlg.GetPathName();

	int lines, blank, comment, code;
	mSlnAlgorithm.CountStatisticsOfFile(mPathNameSource, 
		lines, blank, comment, code);

	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];
	TCHAR path[_MAX_PATH];
	
	_tsplitpath_s(mPathNameSource, drive, dir, fname, ext);
	_tmakepath_s(path, nullptr, nullptr, fname, ext);

	CString fnameExt = path;
	CString message;

	message.Format(_T("The file %ls contains:\
		\n%d lines in total\n%d blank lines\n%d comment lines\n%d code lines\n"), 
		fnameExt, lines, blank, comment, code);

	AfxMessageBox(message, MB_ICONINFORMATION);
}
