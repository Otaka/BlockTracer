#include "stdafx.h"

std::wstring GetFileNameFromHandle(HANDLE hFile)
{
	BOOL bSuccess = FALSE;
	TCHAR pszFilename[MAX_PATH + 1];
	HANDLE hFileMap;

	// Get the file size.
	DWORD dwFileSizeHi = 0;
	DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi);

	if (dwFileSizeLo == 0 && dwFileSizeHi == 0)
	{
		_tprintf(TEXT("Cannot map a file with a length of zero.\n"));
		return FALSE;
	}

	// Create a file mapping object.
	hFileMap = CreateFileMapping(hFile,
		NULL,
		PAGE_READONLY,
		0,
		1,
		NULL);

	if (hFileMap)
	{
		// Create a file mapping to get the file name.
		void* pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);

		if (pMem)
		{
			if (GetMappedFileName(GetCurrentProcess(),
				pMem,
				pszFilename,
				MAX_PATH))
			{

				// Translate path with device name to drive letters.
				TCHAR szTemp[1024];
				szTemp[0] = '\0';

				if (GetLogicalDriveStrings(1024 - 1, szTemp))
				{
					TCHAR szName[MAX_PATH];
					TCHAR szDrive[3] = TEXT(" :");
					BOOL bFound = FALSE;
					TCHAR* p = szTemp;

					do
					{
						// Copy the drive letter to the template string
						*szDrive = *p;

						// Look up each device name
						if (QueryDosDevice(szDrive, szName, MAX_PATH))
						{
							size_t uNameLen = _tcslen(szName);

							if (uNameLen < MAX_PATH)
							{
								bFound = _tcsnicmp(pszFilename, szName, uNameLen) == 0
									&& *(pszFilename + uNameLen) == _T('\\');

								if (bFound)
								{
									// Reconstruct pszFilename using szTempFile
									// Replace device path with DOS path
									TCHAR szTempFile[MAX_PATH];
									StringCchPrintf(szTempFile,
										MAX_PATH,
										TEXT("%s%s"),
										szDrive,
										pszFilename + uNameLen);
									StringCchCopyN(pszFilename, MAX_PATH + 1, szTempFile, _tcslen(szTempFile));
								}
							}
						}

						// Go to the next NULL character.
						while (*p++);
					} while (!bFound && *p); // end of string
				}
			}
			bSuccess = TRUE;
			UnmapViewOfFile(pMem);
		}

		CloseHandle(hFileMap);
	}

	return pszFilename;
}

ULONG64 GetStartAddress(HANDLE hProcess) {
	SYMBOL_INFO *pSymbol;
	pSymbol = (SYMBOL_INFO *)new BYTE[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	pSymbol->MaxNameLen = MAX_SYM_NAME;
	SymFromName(hProcess, "wWinMainCRTStartup", pSymbol);
	// Store address, before deleting pointer  
	ULONG64 dwAddress = pSymbol->Address;
	delete[](BYTE*)pSymbol; // Valid syntax!
	return dwAddress;
}

const wchar_t *GetWC(const char *c)
{
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	size_t converted = 0;
	mbstowcs_s(&converted, wc, cSize, c, cSize);

	return wc;
}

std::string getWindowsLastErrorAsString(){
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::string(); //No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);
	//Free the buffer.
	LocalFree(messageBuffer);
	return message;
}

std::wstring timestampToString(long timestamp) {
	time_t t = (time_t)timestamp;
	struct tm res;
	localtime_s(&res,&t);
	char date[20];
	strftime(date, sizeof(date), "%H-%M-%S", &res);
	const wchar_t * wdateCharArray=GetWC(date);
	std::wstring wdateString = wdateCharArray;
	delete [] wdateCharArray;
	return wdateString;
}
// trim from end of string (right)
std::wstring& rtrim(std::wstring& s, const wchar_t* t){
	s.erase(s.find_last_not_of(t) + 1);
	return s;
}

// trim from beginning of string (left)
std::wstring& ltrim(std::wstring& s, const wchar_t* t){
	s.erase(0, s.find_first_not_of(t));
	return s;
}

// trim from both ends of string (left & right)
std::wstring& trim(std::wstring& s){
	const wchar_t*whitespace = L" \t\n\r\f\v";
	return ltrim(rtrim(s, whitespace), whitespace);
}

bool startWith(std::wstring&s,std::wstring&toFind) {
	return s.rfind(toFind, 0) == 0;
}