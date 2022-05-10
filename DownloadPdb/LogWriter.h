#ifndef LOGWRITER_H_FILE
#define LOGWRITER_H_FILE
#include <atlstr.h>


class CLogWriter
{
private:
	CRITICAL_SECTION m_cs;
	HANDLE m_hLog;
	DWORD m_dwMaxFilesize;
	CString m_strFile;

public:
	CLogWriter();
	~CLogWriter();
	void Trunc();
	LPCTSTR GetLogFilePath();
	BOOL WriteLog(LPCWSTR fmt, ...);
	BOOL WriteLog(LPSTR fmt, ...);
	BOOL WriteRaw(LPCTSTR str, DWORD str_len);
	BOOL WriteRaw(LPCSTR str, DWORD str_len);
};
#endif//endif

BOOL WLOG(LPCSTR fmt, ...);
BOOL WLOG(LPCWSTR fmt, ...);
extern CLogWriter g_LogWriter;

#define LOGE(format, ...)  WLOG("[Error] " __FUNCTION__ " " format, ##__VA_ARGS__)
#define LOGI(format, ...)  WLOG("[Infor] " __FUNCTION__ " " format, ##__VA_ARGS__)

