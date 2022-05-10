#include"GetPdbUrl.h"
#include"GetPeGuid.h"
#include"GetFilePath.h"


std::unordered_map<std::string, STRUCT_PDBINFO>pdb_map;

//搜索目录下的所有pe文件
DWORD  search_pe_directory(const std::string& caseFileName)
{
    DWORD dwResult = 0;

    if (GetFileAttributesA(caseFileName.c_str()) & FILE_ATTRIBUTE_DIRECTORY) {
        char path[MAX_PATH];
        WIN32_FIND_DATAA fData;
        HANDLE handle;

        size_t     pos;
        size_t     loc;
        std::string strUrl;
        std::string  strGuid;
        std::string  strFile;
        std::string  strTempURL;
        std::string  strPeFilePath;
        std::string  strPeTempPath;
        std::string  strFileSuffix;
        std::string  strPdbTempPath;


        sprintf(path, "%s\\*", caseFileName.c_str());
        handle = FindFirstFileA(path, &fData);


        if (INVALID_HANDLE_VALUE == handle) {
            dwResult = GetLastError();
            LOGE("FindFirstFile fail,%d\n", dwResult);
            return dwResult;
        }

        do {
            if (fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (!strcmp(fData.cFileName, ".") || !strcmp(fData.cFileName, "..")) {
                    continue;
                }

                search_pe_directory(caseFileName + std::string("\\") + std::string(fData.cFileName));
            }
            else {
                loc = ((std::string)fData.cFileName).find_last_of('.');
                if (loc > 0 && loc < MAX_PATH)
                {
                    continue;
                }
                else
                {
                    strFile = caseFileName;
                    pos = strFile.find_last_of('.');
                    if (pos > 0)
                        strFileSuffix = strFile.substr(pos + 1);
                    if (0 == strcmp("sys", strlwr((char*)strFileSuffix.c_str())) ||
                        0 == strcmp("dll", strlwr((char*)strFileSuffix.c_str())) ||
                        0 == strcmp("exe", strlwr((char*)strFileSuffix.c_str()))) {

                        PeNumber++;
                        strUrl.clear();
                        strPeFilePath = caseFileName + std::string("\\") + std::string(fData.cFileName);
                        //printf("strPeFilePath%s\n", strPeFilePath.c_str());
                        
                        dwResult = get_pdb_url(strPeFilePath, strUrl);
                        //printf("strUrl:%s\n", strUrl.c_str());
                        if (dwResult != 0)
                        {
                            std::string strPdbName;
                            char pszGuid[MAX_PATH] = { 0 };
                            dwResult = GetPdbGuid(strPeFilePath.c_str(), pszGuid, MAX_PATH);
                            if (dwResult != 0)
                            {
                                LOGE("Get URL failed!file path:%s", path);
                            }
                            pos = caseFileName.rfind('\\');
                            strPdbName = caseFileName.substr(pos + 1);
                            pos = strPdbName.rfind('.');
                            strPdbName = strPdbName.substr(0, pos+1)+"pdb";
                            strUrl = "http://msdl.microsoft.com/download/symbols/"+ strPdbName+"/"+ pszGuid + "/"+ strPdbName;
                            
                        }
                        //printf("strPeFilePath%s\n", strPeFilePath.c_str());

                        auto it = pdb_map.find(strPeFilePath);
                        if (it == pdb_map.end())
                        {
                            pdb_map[strPeFilePath].strPdbURL = strUrl;

                            pos = strUrl.find_last_of('\/');
                            strTempURL = strUrl.substr(0, pos);
                            pdb_map[strPeFilePath].strPdbName = strUrl.substr(pos + 1);
                            //printf("strPdbName:%s\n", strUrl.substr(pos + 1).c_str());
                            
                            pos = strTempURL.find_last_of('\/');
                            strGuid = strTempURL.substr(pos + 1);
                            transform(strGuid.begin(), strGuid.end(), strGuid.begin(),::toupper);
                            pdb_map[strPeFilePath].strPdbGuid= strGuid;
                            //printf("strPdbGuid:%s\n", pdb_map[strPeFilePath].strPdbGuid.c_str());

                            
                            pos = strPePath.find_last_of('\\');
                            strPeTempPath = strPePath.substr(0,pos);
                            strPdbTempPath = strPeTempPath + "\\pdb\\";

                           
                            std::string strSubpath = strPeFilePath.substr(strPePath.size() + 1);
                            pos = strSubpath.find_last_of('\\');
                            strPeTempPath = strSubpath.substr(0,pos);
                            pos = strPeTempPath.rfind('.');
                            strPeTempPath = strPeTempPath.substr(0, pos + 1) + "pdb\\";
                            pdb_map[strPeFilePath].strPdbPath= strPdbTempPath+ strPeTempPath+ strGuid;
                            //printf("strPdbPath:%s\n", pdb_map[strPeFilePath].strPdbPath.c_str());

                        }
                    }
                }
            }
        } while (FindNextFileA(handle, &fData));

        FindClose(handle);
    }
    else
    {

        LOGE("The [%s] is not a path\n", caseFileName.c_str());

    }

    return dwResult;
}