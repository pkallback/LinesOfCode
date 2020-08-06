// MFCListCtrlMod.h : interface of the CMFCListCtrlMod class
//

#pragma once
#include <string>

class CMFCListCtrlMod : public CMFCListCtrl
{
public:
	virtual HFONT OnGetCellFont(int nRow, int nColum, DWORD dwData = 0);
	void SetFont(CFont *pFont);

protected:
	CFont *mpFont{ nullptr };

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

protected:
	void CopySelectionToClipboard(POSITION first, POSITION last);
	bool CopyToClipboard(const std::wstring &w);
public:
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnCopy();
	afx_msg void OnUpdateCopy(CCmdUI *pCmdUI);
};
