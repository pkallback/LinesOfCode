// SlnAlgorithm.cpp : implementation of the CSlnAlgorithm class
//

#include "pch.h"
#include "resource.h"
#include <string>
#include "SlnAlgorithm.h"

CSlnAlgorithm::CSlnAlgorithm() {
}

CSlnAlgorithm::~CSlnAlgorithm() {
}

// Public CSlnAlgorithm implementations
bool CSlnAlgorithm::ExtractSlnInformation(CString &slnFileFullPath) {
	if (mSlnFileFullPath.Compare(slnFileFullPath) == 0) {
		return true;
	} 
	else {
		if (mIsExtractSlnInfoDone) {
			// Clear all data!
			mSlnFileFullPath = _T("");
			mSlnAbsolutePath = _T("");
			mSlnGuid = _T("");
			mIsExtractSlnInfoDone = 0;
			mIsExtractProjectFilterFileDone = 0;
			mSlnProjectArray.clear();
			mFilterFileArray.clear();
		}
	}
	
	// Validate sln file
	if (!IsFileValid(slnFileFullPath)) {
		return false;
	}

	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];
	TCHAR path[_MAX_PATH];
	
	_tsplitpath_s(slnFileFullPath, drive, dir, fname, ext);
	_tmakepath_s(path, drive, dir, nullptr, nullptr);

	// Is sln extension?
	CString extTest = ext;
	extTest.MakeLower();
	if (extTest.Compare(_T(".sln")) != 0) {
		return false;
	}
	
	mSlnFileFullPath = slnFileFullPath;
	mSlnAbsolutePath = path;

	// Extract sln info
	CFile file;
	if (!file.Open(mSlnFileFullPath, CFile::modeRead)) {
		return false;
	}

	std::string slnContentTmp;
	slnContentTmp.resize(static_cast<size_t>(file.GetLength()));
	file.Read(&slnContentTmp.at(0), static_cast<UINT>(file.GetLength()));
	file.Close();

	CString slnContent;
	Utf8ToUnicode(slnContentTmp, slnContent);

	mSlnProjectArray.clear();

	int posBegin = 0;

	while (true) {
		SlnProject slnProject;
		
		posBegin = slnContent.Find(_T("Project("), posBegin);
		if (posBegin < 0) {
			break;
		}
		int posEnd = posBegin;
		posEnd = slnContent.Find(_T("\r\n"), posEnd);

		if (posEnd > posBegin) {
			CString projectLine = slnContent.Mid(posBegin, posEnd - posBegin);
			
			// Project name
			int start = projectLine.Find(_T(") = \""), 0);
			int stop = projectLine.Find(_T("\", \""), 0);
			slnProject.mProjectName = projectLine.Mid(start + 5,
				stop - start - 5);

			// Project relative path
			start = stop;
			stop += 2;
			stop = projectLine.Find(_T("\", \""), stop);
			slnProject.mVcxprojRelativePath = projectLine.Mid(start + 4,
				stop - start - 4);

			// Project guid
			start = stop;
			stop += 2;
			stop = projectLine.Find(_T("}\""), stop);
			slnProject.mVcxprojGuid = projectLine.Mid(start + 4,
				stop - start - 3);

			// Project full path
			slnProject.mVcxprojFilesFullPath = mSlnAbsolutePath + 
				slnProject.mVcxprojRelativePath;

			// Project absolute path
			_tsplitpath_s(slnProject.mVcxprojFilesFullPath, drive, dir, fname,
				ext);
			_tmakepath_s(path, drive, dir, nullptr, nullptr);
			slnProject.mVcxprojAbsolutePath = path;

			// Project filters full path
			slnProject.mVcxprojFiltersFilesFullPath = 
				slnProject.mVcxprojFilesFullPath + _T(".filters");

			mSlnProjectArray.push_back(slnProject);
		}
		else {
			break;
		}
		posBegin = posEnd;
	}

	if (mSlnProjectArray.size() < 1) {
		return false;
	}

	mIsExtractSlnInfoDone = true;
	return true;
}

bool CSlnAlgorithm::ExtractProjectFilterFile() {
	if (!mIsExtractSlnInfoDone) {
		return false;
	}

	for (auto &proj : mSlnProjectArray) {
		proj.mFileCategory.clear();

		// Validate filter file
		if (!IsFileValid(proj.mVcxprojFiltersFilesFullPath)) {
			continue;
		}

		// Extract sln info
		CFile file;
		if (!file.Open(proj.mVcxprojFiltersFilesFullPath, CFile::modeRead)) {
			continue;
			//return false;
		}
		std::string filterContentTmp;
		filterContentTmp.resize(static_cast<size_t>(file.GetLength()));
		file.Read(&filterContentTmp.at(0), 
			static_cast<UINT>(file.GetLength()));
		file.Close();

		CString filterContent;
		Utf8ToUnicode(filterContentTmp, filterContent);

		int posBegin = 0;
		int lastPos = 0;

		// Extract File Category
		while (true) {
			posBegin = filterContent.Find(_T("<Filter Include="),posBegin);
			if (posBegin < 0) {
				break;
			}
			int posEnd = posBegin;
			posEnd = filterContent.Find(_T("</Filter>"), posEnd);
			int posExt = posBegin;
			posExt = filterContent.Find(_T("<Extensions>"), posExt);
			if (posExt > posBegin && posExt < posEnd) {
				// File type
				int posIncBegin = filterContent.Find(_T("\""), posBegin);
				int posIncEnd = filterContent.Find(_T("\""), posIncBegin + 1);
				CString fileType = filterContent.Mid(posIncBegin + 1,
					posIncEnd - posIncBegin - 1);
				// Ext array
				int posExtBegin = posExt + 12;
				int posExtEnd = filterContent.Find(_T("</Extensions>"),
					posExtBegin);
				CString extLine = filterContent.Mid(posExtBegin,
					posExtEnd - posExtBegin);
				posExtBegin = 0;
				posExtEnd = 0;
				std::vector<CString> extArr;
				while (true) {
					if ((posExtEnd = extLine.Find(_T(";"), posExtBegin)) < 0) {
						break;
					}
					CString ext = extLine.Mid(posExtBegin,
						posExtEnd - posExtBegin);
					extArr.push_back(ext);
					posExtBegin = posExtEnd + 1;
					posExtEnd = posExtBegin;
				}
				CString ext = extLine.Mid(posExtBegin,
					extLine.GetLength() - posExtBegin);
				extArr.push_back(ext);
				// Add to SlnProject map file category
				for (auto extension : extArr) {
					proj.mFileCategory.insert
						(std::pair<CString, CString>(extension, fileType));
				}
			}
			lastPos = posBegin = posEnd;
		}

		posBegin = lastPos;

		// Extract files in the project
		while (true) {
			FilterFile filterFile;
			// mProjectName
			filterFile.mProjectName = proj.mProjectName;
			CString query = _T("Include=\"");
			posBegin = filterContent.Find(query, posBegin);
			if (posBegin < 0) {
				break;
			}
			posBegin += query.GetLength();
			query = _T("\"");
			int posEnd = filterContent.Find(query, posBegin);
			// mFileNameRelativePath
			filterFile.mFileNameRelativePath = 
				filterContent.Mid(posBegin, posEnd - posBegin);
			posBegin = posEnd;

			// mFnameExt
			// mExt
			GetExtAndFNameExt(
				filterFile.mFileNameRelativePath,
				filterFile.mFnameExt,
				filterFile.mExt);

			// mFileType, if unknown file type, ignore adding file
			auto it = proj.mFileCategory.find(filterFile.mExt);
			if (it != proj.mFileCategory.end()) {
				filterFile.mFileType = it->second;

				CString fileFullPath =
					proj.mVcxprojAbsolutePath;
				fileFullPath +=
					filterFile.mFileNameRelativePath;

				// The lines below cures the directory
				// name if ..\..\ is in the path.
				CureFullPathFile(fileFullPath);
				filterFile.mFileNameFullPath = fileFullPath;

				mFilterFileArray.push_back(filterFile);
			}
		}
	}

	mIsExtractProjectFilterFileDone = true;

	return true;
}

bool CSlnAlgorithm::CountStatistics() {
	if (!mIsExtractProjectFilterFileDone) {
		return false;
	}

	for (auto &filterFile : mFilterFileArray) {
		CountStatisticsOfFile(filterFile.mFileNameFullPath,
			filterFile.mLines,
			filterFile.mBlank,
			filterFile.mComment,
			filterFile.mCode);
	}
	return true;
}

bool CSlnAlgorithm::CountStatisticsOfFile(CString &pathName,
	int &lines, int &blank, int &comment, int &code) {
	lines = 0;
	blank = 0;
	comment = 0;
	code = 0;

	CFile file;
	if (!file.Open(pathName, CFile::modeRead)) {
		return false;
	}

	std::vector<uint8_t> contentTmp;
	CString content;
	contentTmp.resize(static_cast<size_t>(file.GetLength()));
	file.Read(&contentTmp.at(0), static_cast<UINT>(file.GetLength()));
	file.Close();

	FileType fileType;
	int fileID = GetFileFormat(contentTmp.data(), fileType);

	std::string contentAnsi;
	std::string contentUtf7;
	std::string contentUtf8;
	std::wstring contentUtf16;
	bool isLineCheck = false;

	switch (fileID)
	{
	case 0:
	case ID_BIN_BMP:
	case ID_BIN_CUR:
	case ID_BIN_GIF:
	case ID_BIN_ICO:
	case ID_BIN_JPG:
	case ID_BIN_PNG:
	case ID_BIN_TIF:
	case ID_BIN_WAV:
		break;
	case ID_CHAR_ANSI:
		contentAnsi.resize(contentTmp.size());
		memcpy_s(&contentAnsi.at(0), contentTmp.size(),
			contentTmp.data(), contentTmp.size());
		AnsiToUnicode(contentAnsi, content);
		isLineCheck = true;
		break;
	case ID_CHAR_UTF7:
		contentUtf7.resize(contentTmp.size());
		memcpy_s(&contentUtf7.at(0),
			contentTmp.size() - fileType.numberOfbytesInID,
			contentTmp.data() + fileType.numberOfbytesInID,
			contentTmp.size() - fileType.numberOfbytesInID);
		Utf7ToUnicode(contentUtf7, content);
		isLineCheck = true;
		break;
	case ID_CHAR_UTF8:
		contentUtf8.resize(contentTmp.size());
		memcpy_s(&contentUtf8.at(0),
			contentTmp.size() - fileType.numberOfbytesInID,
			contentTmp.data() + fileType.numberOfbytesInID,
			contentTmp.size() - fileType.numberOfbytesInID);
		Utf8ToUnicode(contentUtf8, content);
		isLineCheck = true;
		break;
	case ID_CHAR_UTF16:
		contentUtf16.resize(contentTmp.size() / 2);
		memcpy_s(&contentUtf16.at(0),
			contentTmp.size() - fileType.numberOfbytesInID,
			contentTmp.data() + fileType.numberOfbytesInID,
			contentTmp.size() - fileType.numberOfbytesInID);
		content = contentUtf16.c_str();
		isLineCheck = true;
		break;
	default:
		break;
	}

	if (isLineCheck && !isXml(content)) {
		// Count lines!!
		CountLines(content,
			lines,
			blank,
			comment,
			code);
	}

	return true;
}

// Protected CSlnAlgorithm implementations
bool CSlnAlgorithm::IsFileValid(const CString &fullPathFile) {
	bool doesFileExcist = true;
	if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributes(fullPathFile) &&
		GetLastError() == ERROR_FILE_NOT_FOUND) {
		doesFileExcist = false;
	}
	return doesFileExcist;
}

bool CSlnAlgorithm::AnsiToUnicode(
	const std::string &ansiString, 
	CString &unicodeString) {
	return MultibyteToUnicode(CP_ACP, ansiString, unicodeString);
}

bool CSlnAlgorithm::Utf7ToUnicode(
	const std::string &utf7String, 
	CString &unicodeString) {
	return MultibyteToUnicode(CP_UTF7, utf7String, unicodeString);
}

bool CSlnAlgorithm::Utf8ToUnicode(
	const std::string &utf8String, 
	CString &unicodeString) {
	return MultibyteToUnicode(CP_UTF8, utf8String, unicodeString);
}

bool CSlnAlgorithm::GetExtAndFNameExt(
	const CString &fileName, 
	CString &fnameExt, 
	CString &extension) {
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];
	TCHAR path[_MAX_PATH];

	errno_t err = _tsplitpath_s(fileName, drive, dir, fname, ext);
	_tmakepath_s(path, nullptr, nullptr, fname, ext);
	fnameExt = path;
	extension = ext;
	extension.MakeLower();
	extension = extension.Right(extension.GetLength() - 1);

	return err == 0 ? true: false;
}

bool CSlnAlgorithm::CureFullPathFile(CString &fullPathFile) {
	bool isFileCured = true;
	if (IsFileValid(fullPathFile)) {
		CFile fileTmp;
		fileTmp.Open(fullPathFile, CFile::modeRead);
		if (fullPathFile.Compare(fileTmp.GetFilePath()) != 0) {
			fullPathFile = fileTmp.GetFilePath();
		}
		else {
			isFileCured = false; // No cure needed.
		}
		fileTmp.Close();
	}
	else {
		isFileCured = false; // Invalid File
	}
	return isFileCured;
}

int CSlnAlgorithm::GetFileFormat(const uint8_t *pData, FileType &fileType) {
	// Check BOM (byte order mark)
	// https://en.wikipedia.org/wiki/Byte_order_mark
	FileType fileTypes[] = {
		{ { 0x42, 0x4d }, 2, ID_BIN_BMP },
		{ { 0xff, 0xfe }, 2, ID_CHAR_UTF16 }, // Might be ID_CHAR_UTF32!
		{ { 0xef, 0xbb, 0xbf }, 3, ID_CHAR_UTF8 },
		{ { 0x4d, 0x4d, 0x00, 0x2a }, 4, ID_BIN_TIF },
		{ { 0x49, 0x49, 0x2a, 0x00 }, 4, ID_BIN_TIF },
		{ { 0x52, 0x49, 0x46, 0x46 }, 4, ID_BIN_WAV },
		{ { 0xff, 0xfe, 0x00, 0x00 }, 4, ID_CHAR_UTF32 },
		{ { 0x00, 0x00, 0x01, 0x00 }, 4, ID_BIN_ICO },
		{ { 0x00, 0x00, 0x02, 0x00 }, 4, ID_BIN_CUR },
		{ { 0x2b, 0x2f, 0x76, 0x38 }, 4, ID_CHAR_UTF7 },
		{ { 0x2b, 0x2f, 0x76, 0x39 }, 4, ID_CHAR_UTF7 },
		{ { 0x2b, 0x2f, 0x76, 0x2b }, 4, ID_CHAR_UTF7 },
		{ { 0x2b, 0x2f, 0x76, 0x2f }, 4, ID_CHAR_UTF7 },
		{ { 0x2b, 0x2f, 0x76, 0x38, 0x2d }, 5, ID_CHAR_UTF7 },
		{ { 0x47, 0x49, 0x46, 0x38, 0x37, 0x61 }, 6, ID_BIN_GIF },
		{ { 0x47, 0x49, 0x46, 0x38, 0x39, 0x61 }, 6, ID_BIN_GIF },
		{ { 0xff, 0xd8, 0xff, 0xe0, 0x00, 0x10, 0x4a, 0x46 }, 8, ID_BIN_JPG },
		{ { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a }, 8, ID_BIN_PNG },
	};

	// Check the 8 first characters
	int fileID = 0;
	for (auto fileTypeTest : fileTypes) {
		if (ByteOrderTest(fileTypeTest.byteID, pData, 
			fileTypeTest.numberOfbytesInID)) {
			fileID = fileTypeTest.fileType;
			fileType = fileTypeTest;
		}
	}
	if (fileID == 0 && AsciiTest(pData, 8)) {
		fileID = ID_CHAR_ANSI;
	}

	return fileID;
}

bool CSlnAlgorithm::ByteOrderTest(const uint8_t *pDataA, const uint8_t *pDataB,
	int numOfBytes) {
	bool isSame = true;
	for (int i = 0; i < numOfBytes; i++) {
		if (pDataA[i] != pDataB[i]) {
			isSame = false;
			break;
		}
	}
	return isSame;
}

bool CSlnAlgorithm::AsciiTest(const uint8_t *pData, int numOfBytes) {
	// If an ascii text file...
	// Allowed characters \t(#9), \n(#10), \r(#13),
	// and ASCII #32 - #127

	bool isAscii = true;
	for (int i = 0; i < numOfBytes; i++) {
		if (pData[i] > 127) {
			isAscii = false;
			break;
		}
		if (pData[i] < 32) {
			if (pData[i] != 10 && // Line feed
				pData[i] != 13 && // Carriage return
				pData[i] != 9) { // Horizontal tab
				isAscii = false;
				break;
			}
		}
	}
	return isAscii;
}

bool CSlnAlgorithm::isXml(const CString &content) {
	CString xmlTag = _T("<?xml");
	CString isXml = content.Left(5);
	isXml.MakeLower();
	return xmlTag.Compare(isXml) == 0 ? true : false;
}

bool CSlnAlgorithm::CountLines(const CString &content,
	int &lines, int &blank, int &comment, int &code) 
{
	// Source code can be with both /r/n and /n.
	// Count /n only
	lines = 0;
	blank = 0;
	comment = 0;
	code = 0;

	int beginLine = 0;
	int endLine = 0;
	bool isCstyleComment = false; /* This is C-style comment */
	bool isCstyleCommentEnd = false;
	bool isCstyleCommentFinish = false;
	for (int i = 0; i < content.GetLength(); i++) {
		if (content.GetAt(i) == _T('\n') ||
			i == (content.GetLength() - 1)) {
			lines++;
			endLine = i;

			if (lines == 27) {
				int dummy2 = 13;
			}

			// Analyze Line
			bool isBlank = false;
			bool isText = false;
			bool isBackslash = false;
			bool isCppCommentCandidate1 = false;
			bool isCppCommentCandidate2 = false;
			bool isOtherChar = false;

			for (int j = beginLine; j <= endLine; j++) {
				TCHAR ch = content.GetAt(j);
				if (ch >= _T(' ')) {
					if (ch == _T(' ') &&
						!isCppCommentCandidate1 &&
						!isCppCommentCandidate2 &&
						!isOtherChar && 
						!isCstyleComment &&
						!isCstyleCommentEnd) {
						isBlank = true;
					}
					else if (ch == _T('\"') &&
						!isCppCommentCandidate1 &&
						!isCstyleComment &&
						!isCstyleCommentEnd &&
						!isText) {
						isBlank = false;
						isText = true;
					}
					else if (ch == _T('\\') &&
						!isCppCommentCandidate1 &&
						!isCstyleComment &&
						!isCstyleCommentEnd &&
						isText) {
						isBlank = false;
						isBackslash = true;
					}
					else if (ch == _T('\"') &&
						!isCppCommentCandidate1 &&
						!isCstyleComment &&
						!isCstyleCommentEnd &&
						isText) {
						isBlank = false;
						if (!isBackslash) {
							isText = false;
						}
						else {
							isBackslash = false;
						}
					}
					else if (ch == _T('/') &&
						!isText &&
						!isCppCommentCandidate1 &&
						!isCstyleComment &&
						!isCstyleCommentEnd) {
						isBlank = false;
						isBackslash = false;
						isCppCommentCandidate1 = true;
					}
					else if (ch == _T('/') &&
						!isText &&
						isCppCommentCandidate1 &&
						!isCppCommentCandidate2 &&
						!isCstyleComment &&
						!isCstyleCommentEnd) {
						isBlank = false;
						isBackslash = false;
						isCppCommentCandidate2 = true;
						if (!isOtherChar) {
							break; // It's ok to break!
						}
					}
					else if (ch == _T('/') &&
						!isText &&
						!isCppCommentCandidate1 &&
						!isCppCommentCandidate2 &&
						isCstyleComment &&
						isCstyleCommentEnd) {
						isBlank = false;
						isBackslash = false;
						isCstyleComment = false;
						isCstyleCommentEnd = false;
						isCstyleCommentFinish = true;
					}
					else if (ch == _T('*') &&
						!isText &&
						isCppCommentCandidate1 &&
						!isCppCommentCandidate2 &&
						!isCstyleComment &&
						!isCstyleCommentEnd) {
						isBlank = false;
						isBackslash = false;
						isCppCommentCandidate1 = false;
						isCstyleComment = true;
					}
					else if (ch == _T('*') &&
						!isText &&
						!isCppCommentCandidate1 &&
						!isCppCommentCandidate2 &&
						isCstyleComment &&
						!isCstyleCommentEnd) {
						isBlank = false;
						isBackslash = false;
						isCstyleCommentEnd = true;
					}
					else if (ch >= _T(' ') &&
						!isCppCommentCandidate1 &&
						!isCppCommentCandidate2 &&
						isCstyleComment &&
						isCstyleCommentEnd &&
						!isCstyleCommentFinish) {
						isBlank = false;
						isBackslash = false;
						isCstyleCommentEnd = false;
						isOtherChar = true;
					}
					else if (ch > _T(' ') &&
						!isCppCommentCandidate1 &&
						!isCppCommentCandidate2 &&
						!isOtherChar &&
						!isCstyleComment &&
						!isCstyleCommentEnd &&
						!isCstyleCommentFinish) {
						isBlank = false;
						isBackslash = false;
						isOtherChar = true;
					}
					else if (ch > _T(' ') &&
						!isCppCommentCandidate1 &&
						!isCppCommentCandidate2 &&
						!isOtherChar &&
						!isCstyleComment &&
						!isCstyleCommentEnd &&
						isCstyleCommentFinish) {
						isBlank = false;
						isBackslash = false;
						isCstyleCommentFinish = false;
						isOtherChar = true;
					}
					else if (ch <= _T(' ') &&
						!isCppCommentCandidate1 &&
						!isCppCommentCandidate2 &&
						!isOtherChar &&
						!isCstyleComment &&
						!isCstyleCommentEnd &&
						isCstyleCommentFinish) {
						isBlank = false;
						isBackslash = false;
					}
				}
				else {
					if (ch <= _T(' ') &&
						!isCppCommentCandidate1 &&
						!isCppCommentCandidate2 &&
						!isOtherChar &&
						!isCstyleComment &&
						!isCstyleCommentEnd &&
						!isCstyleCommentFinish) {
						isBlank = true;
					}
				}
			}

			beginLine = endLine + 1;

			if (isBlank &&
				!isCppCommentCandidate1 &&
				!isCppCommentCandidate2 &&
				!isOtherChar &&
				!isCstyleComment &&
				!isCstyleCommentEnd) {
				blank++;
			} 
			else if (
				isOtherChar) {
				code++;
			}
			else if (!isBlank &&
				isCppCommentCandidate1 &&
				isCppCommentCandidate2 &&
				!isCstyleComment &&
				!isCstyleCommentEnd) {
				comment++;
			}
			else if (
				isCstyleComment &&
				!isCstyleCommentEnd &&
				!isCstyleCommentFinish) {
				comment++;
			}
			else if (
				isCstyleCommentFinish) {
				comment++;
			}

			int dummy1 = 13;
		}
	}

	return true;
}

// Private CSlnAlgorithm implementations
bool CSlnAlgorithm::MultibyteToUnicode(
	const unsigned codePage,
	const std::string &multiByteString,
	CString &unicodeString) {

	// Get Unicode length
	int unicodeLength = ::MultiByteToWideChar(CP_UTF8, 0,
		reinterpret_cast<LPCCH>(&multiByteString.at(0)),
		static_cast<int>(multiByteString.length()),
		nullptr, 0);

	std::wstring unicodeStr;
	unicodeStr.resize(unicodeLength);

	// Convert from UTF-8 to Unicode
	::MultiByteToWideChar(CP_UTF8, 0,
		reinterpret_cast<LPCCH>(&multiByteString.at(0)),
		static_cast<int>(multiByteString.length()),
		reinterpret_cast<LPWSTR>(&unicodeStr.at(0)),
		static_cast<int>(unicodeLength));

	unicodeString = unicodeStr.c_str();

	return true;
}
