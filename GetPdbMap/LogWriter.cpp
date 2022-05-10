#include"stdafx.h"
#include "LogWriter.h"
#include<stdarg.h>
#include <ShlObj.h>

CLogWriter g_LogWriter;


CLogWriter::CLogWriter()
{
	InitializeCriticalSection(&m_cs);
	m_hLog = INVALID_HANDLE_VALUE;
	m_dwMaxFilesize = 1024 * 1024 * 256;
}

CLogWriter::~CLogWriter()
{
	if (INVALID_HANDLE_VALUE != m_hLog)
	{
		//FlushFileBuffers(m_hLog);
		CloseHandle(m_hLog);
		m_hLog = INVALID_HANDLE_VALUE;
	}
	DeleteCriticalSection(&m_cs);
}

BOOL CLogWriter::WriteRaw(LPCTSTR str, DWORD str_len)
{
	EnterCriticalSection(&m_cs);

	if (m_hLog == INVALID_HANDLE_VALUE)
	{

		TCHAR szPath[MAX_PATH + 1] = _T("");
		GetModuleFileName((HMODULE)&__ImageBase, szPath, MAX_PATH);


		PathAppendW(szPath, L"..\\logs");

		if (!::PathFileExists(szPath))
			CreateDirectory(szPath, NULL);


		WCHAR szFile[MAX_PATH] = L"";
		GetModuleFileName((HMODULE)&__ImageBase, szFile, MAX_PATH);

		CString strFile;
		strFile.Format(L"\\%s.log", PathFindFileNameW(szFile));

		PathAppendW(szPath, strFile);
		m_strFile = szPath;


		m_hLog = CreateFile(szPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_hLog != INVALID_HANDLE_VALUE)
		{
#ifdef _UNICODE
			{
				// unicode ·Ú§Õ
				LARGE_INTEGER file_size;

				file_size.QuadPart = 0;
				file_size.LowPart = ::GetFileSize(m_hLog, (LPDWORD)&file_size.HighPart);

				if (file_size.QuadPart == 0)
				{
					DWORD tag = 0xFEFF;
					DWORD nHave = 0;
					WriteFile(m_hLog, &tag, 2, &nHave, NULL);
				}
			}
#endif

			SetFilePointer(m_hLog, 0, 0, FILE_END);
		}


	}

	if (m_hLog == INVALID_HANDLE_VALUE)
		return FALSE;


	if (m_dwMaxFilesize)
	{
		LARGE_INTEGER lFileSize;
		lFileSize.QuadPart = 0;

		GetFileSizeEx(m_hLog, &lFileSize);
		if (lFileSize.QuadPart > m_dwMaxFilesize)
		{
			lFileSize.QuadPart = 0;
#ifdef _UNICODE
			lFileSize.QuadPart = 2;
#endif
			::SetFilePointer(m_hLog, lFileSize.LowPart, &lFileSize.HighPart, FILE_BEGIN);
			::SetEndOfFile(m_hLog);
		}
	}

	DWORD nHave = 0;
	WriteFile(m_hLog, str, str_len * sizeof(TCHAR), &nHave, NULL);

	LeaveCriticalSection(&m_cs);

	return TRUE;
}

BOOL CLogWriter::WriteRaw(LPCSTR str, DWORD str_len)
{
	EnterCriticalSection(&m_cs);

	if (m_hLog == INVALID_HANDLE_VALUE)
	{

		CHAR szPath[MAX_PATH + 1] = "";
		GetModuleFileNameA((HMODULE)&__ImageBase, szPath, MAX_PATH);


		PathAppendA(szPath, "..\\logs");

		if (!::PathFileExistsA(szPath))
			CreateDirectoryA(szPath, NULL);


		CHAR szFile[MAX_PATH] = "";
		GetModuleFileNameA((HMODULE)&__ImageBase, szFile, MAX_PATH);

		CStringA strFile;
		strFile.Format("\\%s.log", PathFindFileNameA(szFile));

		PathAppendA(szPath, strFile);
		m_strFile = szPath;

		m_hLog = CreateFileA(szPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_hLog != INVALID_HANDLE_VALUE)
		{
			SetFilePointer(m_hLog, 0, 0, FILE_END);
		}
	}

	if (m_hLog == INVALID_HANDLE_VALUE)
		return FALSE;


	if (m_dwMaxFilesize)
	{
		LARGE_INTEGER lFileSize;
		lFileSize.QuadPart = 0;

		GetFileSizeEx(m_hLog, &lFileSize);
		if (lFileSize.QuadPart > m_dwMaxFilesize)
		{
			lFileSize.QuadPart = 0;

			::SetFilePointer(m_hLog, lFileSize.LowPart, &lFileSize.HighPart, FILE_BEGIN);
			::SetEndOfFile(m_hLog);
		}
	}

	DWORD nHave = 0;
	WriteFile(m_hLog, str, str_len * sizeof(CHAR), &nHave, NULL);

	LeaveCriticalSection(&m_cs);

	return TRUE;
}
void CLogWriter::Trunc()
{
	if (m_hLog == INVALID_HANDLE_VALUE)
		return;

	LARGE_INTEGER lFileSize;
	lFileSize.QuadPart = 0;

#ifdef _UNICODE
	lFileSize.QuadPart = 2;
#endif
	::SetFilePointer(m_hLog, lFileSize.LowPart, &lFileSize.HighPart, FILE_BEGIN);
	::SetEndOfFile(m_hLog);
}

LPCTSTR CLogWriter::GetLogFilePath()
{
	return m_strFile.GetString();
}

BOOL CLogWriter::WriteLog(LPCWSTR fmt, ...)
{
	if (!fmt)
		return FALSE;


	CString var_str;
	CStringA var_stra;

	SYSTEMTIME timeNow;
	GetLocalTime(&timeNow);

	var_str.Format(_T("[%04d-%02d-%02d %02d:%02d:%02d] "),
		timeNow.wYear, timeNow.wMonth, timeNow.wDay,
		timeNow.wHour, timeNow.wMinute, timeNow.wSecond);

	va_list	ap;
	va_start(ap, fmt);
	var_str.AppendFormatV(fmt, ap);
	va_end(ap);

	var_str.Append(_T("\r\n"));

	var_stra = var_str;

	return WriteRaw(var_stra.GetString(), var_stra.GetLength());
}

BOOL WLOG(LPCWSTR fmt, ...) {

	if (!fmt)
		return FALSE;

	CString var_str;
	CStringA var_stra;

	SYSTEMTIME timeNow;
	GetLocalTime(&timeNow);

	var_str.Format(_T("[%04d-%02d-%02d %02d:%02d:%02d] "),
		timeNow.wYear, timeNow.wMonth, timeNow.wDay,
		timeNow.wHour, timeNow.wMinute, timeNow.wSecond);

	va_list	ap;
	va_start(ap, fmt);
	var_str.AppendFormatV(fmt, ap);
	va_end(ap);

	var_str.Append(_T("\r\n"));

	var_stra = var_str;

	return g_LogWriter.WriteRaw(var_stra.GetString(), var_stra.GetLength());
}
BOOL WLOG(LPCSTR fmt, ...) {

	if (!fmt)
		return FALSE;

	CStringA var_str;

	SYSTEMTIME timeNow;
	GetLocalTime(&timeNow);

	var_str.Format("[%04d-%02d-%02d %02d:%02d:%02d] ",
		timeNow.wYear, timeNow.wMonth, timeNow.wDay,
		timeNow.wHour, timeNow.wMinute, timeNow.wSecond);

	va_list	ap;
	va_start(ap, fmt);
	var_str.AppendFormatV(fmt, ap);
	va_end(ap);

	var_str.Append("\r\n");

	return g_LogWriter.WriteRaw(var_str.GetString(), var_str.GetLength());
}

BOOL CLogWriter::WriteLog(LPSTR fmt, ...)
{
	if (!fmt)
		return FALSE;


	CStringA var_str;

	SYSTEMTIME timeNow;
	GetLocalTime(&timeNow);

	var_str.Format("[%04d-%02d-%02d %02d:%02d:%02d] ",
		timeNow.wYear, timeNow.wMonth, timeNow.wDay,
		timeNow.wHour, timeNow.wMinute, timeNow.wSecond);

	va_list	ap;
	va_start(ap, fmt);
	var_str.AppendFormatV(fmt, ap);
	va_end(ap);

	var_str.Append("\r\n");


	CString strTmp = var_str.GetString();
	return WriteRaw(strTmp, strTmp.GetLength());
}

