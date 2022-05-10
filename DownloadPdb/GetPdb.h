#include<stdio.h>
#include <direct.h>
#include"GetPdbUrl.h"
#include"LogWriter.h"
#include"GetFilePath.h"

BOOL IsDirExist(const CString& csDir);
DWORD GetPdb(std::string& path);
