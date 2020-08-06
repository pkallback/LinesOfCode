// SlnAlgorithm.h : interface of the CSlnAlgorithm class
//

#pragma once

#include <map>
#include <vector>

class SlnProject {
public:
	CString mProjectName;
	CString mVcxprojRelativePath;
	CString mVcxprojGuid;
	CString mVcxprojFilesFullPath;
	CString mVcxprojAbsolutePath;
	CString mVcxprojFiltersFilesFullPath;
	std::map<CString, CString> mFileCategory;
};

class FilterFile {
public:
	CString mProjectName; //*
	CString mFnameExt; //*
	CString mExt; //*
	CString mFileType; //*
	CString mFileNameRelativePath; //*
	CString mFileNameFullPath; //*
	int mLines{ 0 };
	int mBlank{ 0 };
	int mComment{ 0 };
	int mCode{ 0 };
};

class FileType {
public:
	uint8_t byteID[8]{ 0, 0, 0, 0, 0, 0, 0, 0 };
	int numberOfbytesInID{ 0 };
	int fileType{ 0 };
};

// Base class
class CSlnAlgorithm
{
// Constructor
public:
	CSlnAlgorithm();
	~CSlnAlgorithm();

// Attributes
public:
	std::vector<SlnProject> mSlnProjectArray;
	std::vector<FilterFile> mFilterFileArray;
protected:
	CString mSlnFileFullPath{ _T("") };
	CString mSlnAbsolutePath{ _T("") };
	CString mSlnGuid;
	bool mIsExtractSlnInfoDone{ false };
	bool mIsExtractProjectFilterFileDone{ false };

// Implementation
public:
	// Extract sln information and get vcxproj paths stored in SlnProject Class
	bool ExtractSlnInformation(CString &slnFileFullPath);
	bool ExtractProjectFilterFile();
	bool CountStatistics();
	bool CountStatisticsOfFile(CString &pathName,
		int &lines, int &blank, int &comment, int &code);

protected:
	bool IsFileValid(const CString &fullPathFile);
	bool AnsiToUnicode(const std::string &ansiString, CString &unicodeString);
	bool Utf7ToUnicode(const std::string &utf8String, CString &unicodeString);
	bool Utf8ToUnicode(const std::string &utf8String, CString &unicodeString);
	bool GetExtAndFNameExt(
		const CString &fileName, CString &fnameExt, CString &extension);
	bool CureFullPathFile(CString &fullPathFile);
	int GetFileFormat(const uint8_t *pData, FileType &fileType);
	bool ByteOrderTest(
		const uint8_t *pDataA, const uint8_t *pDataB, int numOfBytes);
	bool AsciiTest(const uint8_t *pData, int numOfBytes);
	bool isXml(const CString &content);
	bool CountLines(const CString &content,
		int &lines, int &blank, int &comment, int &code);

private:
	bool MultibyteToUnicode(const unsigned codePage, const std::string &multiByteString,
		CString &unicodeString);
};

