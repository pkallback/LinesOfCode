// LOCView.h : interface of the CLOCView class
//

#pragma once

#include <memory>
#include "MFCListCtrlMod.h"
#include "SlnAlgorithm.h"

class CDBGraphics;
class CDrawObject;
class CSlnAlgorithm;
class CMFCListCtrlMod;

class CLOCView : public CView
{
protected: // create from serialization only
	CLOCView() noexcept;
	DECLARE_DYNCREATE(CLOCView)

// Attributes
public:
	CLOCDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CLOCView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	std::shared_ptr<CDBGraphics> mpMemGraphics{ nullptr };
	std::shared_ptr<CDrawObject> mpDrawObject{ nullptr };
	CSlnAlgorithm mSlnAlgorithm;
	int mCx{ 2 }; // Window area width
	int mCy{ 2 }; // Window area height
	BOOL mUpdateGraphics{ FALSE };

public:
	CMFCListCtrlMod mListCtrl;
	CFont mFont;
	CString mPathNameSource{ _T("") };

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnFileOpenSource();

protected:
	void UpdateListCtrl();

};

#ifndef _DEBUG  // debug version in LOCView.cpp
inline CLOCDoc* CLOCView::GetDocument() const
   { return reinterpret_cast<CLOCDoc*>(m_pDocument); }
#endif