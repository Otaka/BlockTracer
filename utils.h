#pragma once

#include "stdafx.h"
std::wstring GetFileNameFromHandle(HANDLE hFile);

ULONG64 GetStartAddress(HANDLE hProcess);

const wchar_t *GetWC(const char *c);

std::string getWindowsLastErrorAsString();

std::wstring timestampToString(long timestamp);


std::wstring& trim(std::wstring& s);

bool startWith(std::wstring&s, std::wstring&toFind);