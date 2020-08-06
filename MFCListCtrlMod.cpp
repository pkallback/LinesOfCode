// MFCListCtrlMod.cpp : implementation of the CMFCListCtrlMod class
//

#include "pch.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview,
// thumbnail and search filter handlers and allows sharing of document
// code with that project.
#ifndef SHARED_HANDLERS
#include "LOC.h"
#endif
#include <vector>
#include "MFCListCtrlMod.h"

HFONT CMFCListCtrlMod::OnGetCellFont(int nRow, int nColum, 
	DWORD /*dwData* = 0*/) {
	return *mpFont;
}

void CMFCListCtrlMod::SetFont(CFont *pFont) {
	mpFont = pFont;
}

BEGIN_MESSAGE_MAP(CMFCListCtrlMod, CMFCListCtrl)
ON_WM_KEYUP()
ON_WM_CONTEXTMENU()
ON_COMMAND(ID_COPY, &CMFCListCtrlMod::OnCopy)
ON_UPDATE_COMMAND_UI(ID_COPY, &CMFCListCtrlMod::OnUpdateCopy)
END_MESSAGE_MAP()

void CMFCListCtrlMod::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
	// TODO: Add your message handler code here and/or call default
	if (nChar = 67 && ::GetKeyState(VK_CONTROL) < 0) {
		OnCopy();
	}

	CMFCListCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CMFCListCtrlMod::CopySelectionToClipboard(POSITION firstPos,
	POSITION lastPos) {
	// Copy column header title text
	int columns = 0;
	LVCOLUMNW columnData;
	columnData.mask = LVCF_TEXT;
	columnData.cchTextMax = 256;
	columnData.pszText = new WCHAR [256];
	ZeroMemory(columnData.pszText, sizeof(WCHAR) * 256);
	std::vector<CString> columnTextArr;
	while (this->GetColumn(columns, &columnData)) {
		columnTextArr.push_back(columnData.pszText);
		columns++;
	}
	delete[] columnData.pszText;
	columnData.pszText = nullptr;

	// Initiate items data
	std::vector<std::vector<CString>> itemsData;
	int first = static_cast<int>(reinterpret_cast<std::uintptr_t>(firstPos));
	int last = static_cast<int>(reinterpret_cast<std::uintptr_t>(lastPos));
	int rows = last - first + 1;
	itemsData.resize(static_cast<size_t>(last - first + 1));
	for (auto &items : itemsData) {
		items.resize(static_cast<size_t>(columns));
	}

	// Copy item text
	for (int i = first; i <= last; i++) {
		for (int j = 0; j < columns; j++) {
			itemsData[i - first][j] = this->GetItemText(i - 1, j);
		}
	}

	// Copy to wstring
	std::wstring content;
	content.clear();
	for (int j = 0; j < columns; j++) {
		content += columnTextArr[j];
		if (j < columns - 1) {
			content += _T('\t');
		}
		else {
			content += _T('\n');
		}
	}
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			content += itemsData[i][j];
			if (j < columns - 1) {
				content += _T('\t');
			}
			else if (j == columns - 1 && i < rows - 1) {
				content += _T('\n');
			}
			else if (j == columns - 1 && i == rows - 1) {
				content += static_cast<TCHAR>(0);
			}
		}
	}

	// Copy to clipboard
	if (CopyToClipboard(content)) {
		AfxMessageBox(_T("Selected Items Copied to Clipboard!"),
			MB_ICONINFORMATION);
	}
}

bool CMFCListCtrlMod::CopyToClipboard(const std::wstring &w) {
	if (OpenClipboard()) {
		HGLOBAL hgClipBuffer = nullptr;
		std::size_t sizeInWords = w.size() + 1;
		std::size_t sizeInBytes = sizeInWords * sizeof(wchar_t);
		hgClipBuffer = GlobalAlloc(GHND | GMEM_SHARE, sizeInBytes);
		if (!hgClipBuffer) {
			CloseClipboard();
			return false;
		}
		wchar_t *wgClipBoardBuffer = static_cast<wchar_t*>(
			GlobalLock(hgClipBuffer));
		wcscpy_s(wgClipBoardBuffer, sizeInWords, w.c_str());
		GlobalUnlock(hgClipBuffer);
		EmptyClipboard();
		SetClipboardData(CF_UNICODETEXT, hgClipBuffer);
		CloseClipboard();
		return true;
	}
	else {
		return false;
	}
}

void CMFCListCtrlMod::OnContextMenu(CWnd* /*pWnd*/, CPoint point) {
	// TODO: Add your message handler code here
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_COPY, point.x, point.y,
		this, TRUE);
}


void CMFCListCtrlMod::OnCopy() {
	POSITION firstPosition = this->GetFirstSelectedItemPosition();
	POSITION nextPosition = 0;
	POSITION lastPosition = firstPosition;
	while (nextPosition != firstPosition) {
		this->GetNextSelectedItem(nextPosition);
	}
	while (nextPosition > 0) {
		this->GetNextSelectedItem(nextPosition);
		if (nextPosition > lastPosition) {
			lastPosition = nextPosition;
		}
	}

	if (firstPosition == nullptr && lastPosition == nullptr) {
		return;
	}

	CopySelectionToClipboard(firstPosition, lastPosition);
}


void CMFCListCtrlMod::OnUpdateCopy(CCmdUI *pCmdUI) {
	POSITION firstPosition = this->GetFirstSelectedItemPosition();
	bool isItemsSelected = firstPosition > 0 ? true : false;

	pCmdUI->Enable(isItemsSelected);
}
