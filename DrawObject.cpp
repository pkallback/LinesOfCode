// DrawObject.cpp : implementation file
//

#include "stdafx.h"
#include "DrawObject.h"
#include <vector>
#include <memory>

// CDrawObject

// CIVDrawObject member functions

void CDrawObject::SetWindowRect(const int cx, const int cy)
{
	mCxGraphArea = cx;
	mCyGraphArea = cy;
}

bool CDrawObject::DrawObject(Graphics* pG)
{
	if (pG == nullptr) {
		return false;
	}

	// Set smoothing mode
	pG->SetSmoothingMode(SmoothingModeAntiAlias);

	// Paint window area with frame background color.	
	SolidBrush sbFramBkgnd(_COLOR_BKGROUND);
	pG->FillRectangle(&sbFramBkgnd, 0, 0, mCxGraphArea, mCyGraphArea);

	// Write the directory content
	WriteDirectoryContent(pG);

	return true;
}

void CDrawObject::SetFilenamePath(const CString filenamePath)
{
	mFilenamePath = filenamePath;
}

void CDrawObject::WriteDirectoryContent(Graphics *pG)
{
	// Select color for the text
	std::shared_ptr<SolidBrush> pColorFontSolidBrush = std::make_shared<SolidBrush>(_COLOR_TEXT);

	// Measure the height for the title and normal text
	std::shared_ptr<FontFamily> pFamilyFont = std::make_shared<FontFamily>(_T("Consolas"));
	std::shared_ptr<Gdiplus::Font> pTitleFont =
		std::make_shared<Gdiplus::Font>(pFamilyFont.get(), 14.f, FontStyleBold, UnitPoint);
	RectF titleFontRect;
	CString measureString = _T("HelÅÄÖjgqµ");
	pG->MeasureString(measureString, measureString.GetLength(), pTitleFont.get(), PointF(0.0,0.0), &titleFontRect);
	std::shared_ptr<Gdiplus::Font> pNormalFont =
		std::make_shared<Gdiplus::Font>(pFamilyFont.get(), 12.f, FontStyleRegular, UnitPoint);
	RectF normalFontRect;
	pG->MeasureString(measureString, measureString.GetLength(), pNormalFont.get(), PointF(0.0,0.0), &normalFontRect);

	// As title set the FNameExt
	// As next row, set the directory path
	CString title = _T("No file selected.");
	CString paragraph = _T("");
	std::vector<CString> directoryObjects;

	// If the filename is not empty, split the file path
	if (!mFilenamePath.IsEmpty()) {		
		CFile file;
		BOOL isFileOpen = file.Open(mFilenamePath, CFile::modeWrite);

		if (isFileOpen) {
			file.Close();

			TCHAR drive[_MAX_DRIVE];
			TCHAR dir[_MAX_DIR];
			TCHAR fname[_MAX_FNAME];
			TCHAR ext[_MAX_EXT];
			TCHAR path[_MAX_PATH];

			_tsplitpath_s(mFilenamePath, drive, dir, fname, ext);
			_tmakepath_s(path, NULL, NULL, fname, NULL);
			title = path;

			_tmakepath_s(path, drive, dir, NULL, NULL);
			paragraph = path;

			mPathName = path;

			directoryObjects.clear();
			GetDirectoryContent(directoryObjects, mPathName, 1);
		}
		else {
			title = _T("File could not be opened!");
		}
	}

	PointF point = PointF(32, 32);
	REAL yPosition = point.Y;
	pG->DrawString(title, title.GetLength(), pTitleFont.get(), point, pColorFontSolidBrush.get());

	yPosition += titleFontRect.Height;

	if (!mPathName.IsEmpty()) {
		yPosition += normalFontRect.Height * 0.15f;
		point.Y = yPosition;
		paragraph = mPathName;
		pG->DrawString(paragraph, paragraph.GetLength(), pNormalFont.get(), point, pColorFontSolidBrush.get());
	}

	for (auto directory : directoryObjects) {
		yPosition += normalFontRect.Height * 1.15f;
		point.Y = yPosition;		
		pG->DrawString(directory, directory.GetLength(), pNormalFont.get(), point, pColorFontSolidBrush.get());
	}
}

bool CDrawObject::GetDirectoryContent(std::vector<CString> &directoryObjects, const CString fullPath, const int level)
{
	if (fullPath.IsEmpty()) {
		return false;
	}

	//CString sTest;
	//int i, j;

	// Seek number of regions
	CString mainFolder;
	mainFolder = fullPath;
	mainFolder += _T('*'); // Add * to the folder name e.g. c:\ will be c:\*

	/////////////////////////////////////////////////////////////
	// Store the number of folders
	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(mainFolder, &ffd);

	if (hFind == INVALID_HANDLE_VALUE) { // Return if something is wrong
		return false; //_SPECTRA_FOLDERMISSING_;
	}

	do
	{
		bool isDirectory = false;
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) { // File directory
			CString folder = ffd.cFileName;
			CString nextFolder = _T("");
			if ((folder.Compare(_T("."))) != 0 && (folder.Compare(_T(".."))) != 0) {
				folder += "\\";
				nextFolder = fullPath;
				nextFolder += folder;
				isDirectory = true;
			}
			
			if (isDirectory) {
				CString object = _T("");
				for (int i = 0; i < level; i++) { 
					for (int j = 0; j < 4; j++) {
						object += _T(' ');
					}
				} // Add space
				object += nextFolder;
				directoryObjects.push_back(object);
				GetDirectoryContent(directoryObjects,nextFolder,level+1);
			}
			else {
				CString object = _T("");
				for (int i = 0; i < level; i++) { 
					for (int j = 0; j < 4; j++) {
						object += _T(' ');
					}
				} // Add space
				object += folder;
				directoryObjects.push_back(object);
			}
		}
		else { // Regular file
			CString object = _T("");
			for (int i = 0; i < level; i++) {
				for (int j = 0; j < 4; j++) {
					object += _T(' ');
				}
			} // Add space
			object += ffd.cFileName;
			directoryObjects.push_back(object);
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);

	return true;
}
