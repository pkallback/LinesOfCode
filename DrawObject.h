#pragma once

#include <vector>

#define _COLOR_BKGROUND	Color(34,35,38)
#define _COLOR_TEXT		Color(223,225,229)

class CDrawObject : public CObject
{
public:
	CDrawObject() {};
	virtual ~CDrawObject() {};

	void SetWindowRect(const int cx, const int cy);
	bool DrawObject(Graphics* pG);
	void SetFilenamePath(const CString filenamePath);

protected:
	void WriteDirectoryContent(Graphics *pG);
	bool GetDirectoryContent(std::vector<CString> &directoryObjects, const CString fullPath, const int level);

private:
	int mCxGraphArea{ 100 };
	int mCyGraphArea{ 100 };
	CString mFilenamePath{ _T("") };
	CString mPathName{ _T("") };
};


