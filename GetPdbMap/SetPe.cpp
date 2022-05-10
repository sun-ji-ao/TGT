#include"stdafx.h"
#include"SetPe.h"
#include"SetPdb.h"
#include"GetPeGuid.h"
#include"GetSymbolByIda.h"
#pragma warning(disable:4996)
#pragma warning(disable:4244)


std::unordered_map<std::string, STRUCT_PEINFO>pe_map;
std::unordered_map<std::string, STRUCT_PDBINFO>pdb_map;


void dfsFolder(std::string folderPath, std::string strPeGuid, std::string& strPdbNewPath)
{
    size_t pos;
    std::string newPath;
    __finddata64_t   FileInfo;
    std::string strPePathGuid;
    std::string strfind = folderPath + "\\*";
    intptr_t   Handle = _findfirst64(strfind.c_str(), &FileInfo);

    if (Handle == -1L)
    {
        printf("can not match the folder path");
    }
    do {
        if (FileInfo.attrib & _A_SUBDIR)
        {
            if ((strcmp(FileInfo.name, ".") != 0) && (strcmp(FileInfo.name, "..") != 0))
            {
                newPath = folderPath + "\\" + FileInfo.name;
                dfsFolder(newPath, strPeGuid, strPdbNewPath);
            }
        }
        else
        {
            pos = folderPath.find_last_of("\\");
            strPePathGuid = folderPath.substr(pos + 1);
            if (strcmp(strPePathGuid.c_str(), strPeGuid.c_str()) == 0)
            {
                strPdbNewPath = folderPath + "\\" + FileInfo.name;
            }

        }
    } while (_findnext64(Handle, &FileInfo) == 0);

    _findclose(Handle);
}


DWORD GetPeSymbol(LPCSTR lpPePath)
{

    DWORD dwResult = 0;
    std::string strPdbPath;
    std::string strPdbFile;


    dwResult = search_pe_directory(lpPePath);
    if (dwResult != 0) {
        return dwResult;
    }



    for (std::unordered_map<std::string, STRUCT_PEINFO>::iterator iter = pe_map.begin(); iter != pe_map.end(); iter++)
    {
      
            size_t     pos;
            std::string strPEPath;
            std::string strPdbdefect;
            std::string strPdbNewPath;
            std::string frontPath = lpPePath;
            std::string strVersion= iter->first;
            

            for (int i = 0; i < 2; i++)
            {
                pos = strVersion.find_last_of('\\');
                strVersion = strVersion.substr(0, pos);
            }
            pos = strVersion.find_last_of('\\');
            strVersion = strVersion.substr(pos+1);
           

            strPdbFile = frontPath + "\\pdb\\" + strVersion + "\\" + iter->second.strPeName + ".pdb\\" + iter->second.strPeGuid +
                "\\" + iter->second.strPeName + ".pdb";
  


            if (PathFileExistsA(strPdbFile.c_str())) 
            {
                dwResult = SaveSymbol(strPdbFile.c_str(), &iter->second);

                if (dwResult != 0) {
                    strPEPath = iter->second.strSymbolPath.substr(0, iter->second.strSymbolPath.find_last_of('.'));
                    LOGE("SaveSymbol fail,PDBPath:%s,PEPath:%s\n", strPdbFile.c_str(), strPEPath.c_str());
                   /* 
                    if (GetPdbMap(&iter->second) != 0)
                    {
                        strPEPath = iter->second.strSymbolPath.substr(0, iter->second.strSymbolPath.find_last_of('.'));
                        LOGE("Ida parsing failed, path:%s\n\n", strPEPath.c_str());
                    }*/
                }
            }
            else
            {
                strPdbdefect = frontPath + "\\pdb\\" + strVersion;
                if (PathFileExistsA(strPdbdefect.c_str()))
                {
                    dfsFolder(strPdbdefect, iter->second.strPeGuid, strPdbNewPath);
                    if (strPdbNewPath != "")
                    {
                        dwResult = SaveSymbol(strPdbNewPath.c_str(), &iter->second);
                        if (dwResult != 0) {
                            strPEPath = iter->second.strSymbolPath.substr(0, iter->second.strSymbolPath.find_last_of('.'));
                            LOGE("SaveSymbol2 fail,PDBPath:%s,PEPath:%s", strPdbNewPath.c_str(), strPEPath.c_str());
                        }
                        strPdbNewPath = "";
                    }
                    else
                    {
                        
                        if (GetPdbMap(&iter->second) != 0)
                        {
                            strPEPath = iter->second.strSymbolPath.substr(0, iter->second.strSymbolPath.find_last_of('.'));
                            LOGE("Ida parsing failed, path:%s\n\n", strPEPath.c_str());
                        }
                    }
                }
                else
                {
                    if (GetPdbMap(&iter->second) != 0)
                    {
                        strPEPath = iter->second.strSymbolPath.substr(0, iter->second.strSymbolPath.find_last_of('.'));
                        LOGE("Ida parsing failed, path:%s\n\n", strPEPath.c_str());
                    }
                }
            }


    }

    return 0;

}





DWORD GetPeSymbol(LPCSTR lpPePath, LPCSTR lpPdbPath)
{

    DWORD dwResult = 0;
    std::string strPEPath;
    std::string strPdbFile;
    std::string strPdbNewPath;
    std::string strPdbDetailPath;
    std::string strPdbPath = lpPdbPath;
    std::string strPdbdefect = lpPdbPath;
    
  
    

    dwResult = search_pe_directory(lpPePath);
    if (dwResult != 0) {
        return dwResult;
    }

  
        if (strPdbPath.back() != '\\') {
            strPdbPath += "\\";
        }
  

    for (std::unordered_map<std::string, STRUCT_PEINFO>::iterator iter = pe_map.begin(); iter != pe_map.end(); iter++)
    {

        strPdbDetailPath = strPdbPath+ iter->second.strPeName + ".pdb\\" + iter->second.strPeGuid +
            "\\" + iter->second.strPeName + ".pdb";
        //printf("strPdbDetailPath:%s\n", strPdbDetailPath.c_str());


        if (PathFileExistsA(strPdbDetailPath.c_str()))
        {
            dwResult = SaveSymbol(strPdbDetailPath.c_str(), &iter->second);
            if (dwResult != 0)
            {
                strPEPath = iter->second.strSymbolPath.substr(0, iter->second.strSymbolPath.find_last_of('.'));
                LOGE("SaveSymbol fail,PDBPath:%s,PEPath:%s", strPdbDetailPath.c_str(), strPEPath.c_str());
                /*
                if (GetPdbMap(&iter->second) != 0)
                {
                    strPEPath = iter->second.strSymbolPath.substr(0, iter->second.strSymbolPath.find_last_of('.'));
                    LOGE("Ida parsing failed, path:%s\n\n", strPEPath.c_str());
                }*/
            }
        }
        else
        {
             
            if (PathFileExistsA(strPdbdefect.c_str()))
            {
                dfsFolder(strPdbdefect, iter->second.strPeGuid, strPdbNewPath);
                if (!strPdbNewPath.empty())
                {
                    dwResult = SaveSymbol(strPdbNewPath.c_str(), &iter->second);
                    if (dwResult != 0) {
                        strPEPath = iter->second.strSymbolPath.substr(0, iter->second.strSymbolPath.find_last_of('.'));
                        LOGE("SaveSymbol2 fail,PDBPath:%s,PEPath:%s", strPdbNewPath.c_str(), strPEPath.c_str());
                    }
                    strPdbNewPath = "";
                }
                else
                {
                    if (GetPdbMap(&iter->second) != 0)
                    {
                        strPEPath = iter->second.strSymbolPath.substr(0, iter->second.strSymbolPath.find_last_of('.'));
                        LOGE("Ida parsing failed, path:%s\n\n", strPEPath.c_str());
                    }
                }
            }
            else
            {
                if (GetPdbMap(&iter->second) != 0)
                {
                    strPEPath = iter->second.strSymbolPath.substr(0, iter->second.strSymbolPath.find_last_of('.'));
                    LOGE("Ida parsing failed, path:%s\n\n", strPEPath.c_str());
                }
            }

        }
    }

    return 0;

}

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
        std::string  strFile;
        std::string  strPePath;
        std::string  strPeName;
        std::string  strFileSuffix;


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
                if (loc > 0&&loc<MAX_PATH)
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

                        strPeName = strFile.substr(0,pos);
                        pos = strPeName.find_last_of('\\');
                        strPeName = strPeName.substr(pos+1);
                        //printf("strPeName:%s\n", strPeName.c_str());


                        strPePath = caseFileName.c_str() + std::string("\\") + std::string(fData.cFileName);
                        //printf("strPePath:%s\n", strPePath.c_str());
                        
                        auto it = pe_map.find(strPePath);
                        if (it == pe_map.end())
                        {
                            pe_map[strPePath].strPeName = strPeName;
                            //printf("strPeName:%s\n", strPeName.c_str());


                            pe_map[strPePath].strSymbolPath = caseFileName + std::string("\\") + fData.cFileName + ".pdbmap";
                            

                            std::list<SYM_FECODE_INFO> codeList;                            
                            int nVaOffset = GetPeCodeInfo(strPePath.c_str(), &codeList);                         
                            if (nVaOffset != 0) {
                                dwResult = GetLastError();
                            
                                if (dwResult == 0) {
                                    dwResult = ERROR_INTERNAL_ERROR;
                                }
                                LOGE("GetPeCodeInfo fail,%s\n", strPePath.c_str());
                                return dwResult;
                            }
                            else
                            {
                                pe_map[strPePath].peCodeList = codeList;
                            }


                            char pszGuid[MAX_PATH] = { 0 };
                            DWORD dwResult = GetPdbGuid(strPePath.c_str(), pszGuid, MAX_PATH);
                            if (dwResult != 0) {
                                LOGE("GetPdbGuid fail:%d,PePath:%s\n", dwResult, strPePath.c_str());
                                return dwResult;
                            }
                            pe_map[strPePath].strPeGuid = strupr(pszGuid);

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










