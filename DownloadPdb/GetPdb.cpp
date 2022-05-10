#include"GetPdb.h"
#include"GetPdbBySymChk.h"

BOOL IsDirExist(const CString& csDir)
{
	DWORD dwAttrib = GetFileAttributes(csDir);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 != (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}
DWORD GetPdb(std::string& path)
{
	size_t pos;
	DWORD dwResult;
	std::string strPeTempPath;
	std::string strPeFailPath;

	pos = strPePath.find_last_of('\\');
	strPeTempPath = strPePath.substr(0, pos);
	strPeFailPath = strPeTempPath + "\\pdbfail\\";

	dwResult = search_pe_directory(strPePath);
	if (dwResult != 0) {
		LOGE("search_pe_directory failed!file path:%s", strPePath);
		return dwResult;
	}


	for (std::unordered_map<std::string, STRUCT_PDBINFO>::iterator iter = pdb_map.begin(); iter != pdb_map.end(); iter++)
	{
		size_t pos;
		std::string strCmd;
		std::string strPdbURL;
		std::string strPdbVerDir;
		std::string strPdbDirPath;
		std::string strPdbFilePath;
		std::string strPdbFailedFilePath;

		strPdbURL.clear();
		strPdbDirPath.clear();
		strPdbURL = iter->second.strPdbURL; //PDB下载网址
		strPdbVerDir= iter->second.strPdbPath;
		strPdbDirPath = iter->second.strPdbPath; //PDB生成路径

		pos = strPdbVerDir.rfind('\\');
		strPdbVerDir = strPdbVerDir.substr(0, pos);
		pos = strPdbVerDir.rfind('\\');
		strPdbVerDir = strPdbVerDir.substr(0, pos);
		//printf("strPdbVerDir:%s\n", strPdbVerDir.c_str());

		if (!IsDirExist(strPdbVerDir.c_str())) {
			strCmd = "mkdir " + strPdbVerDir;
			if (system(strCmd.c_str())) {
				LOGE("Create dir failed!dir path:%s,err:%d\n", strPdbVerDir.c_str(), GetLastError());
				return GetLastError();
			}
		}

		dwResult = GetPdbBySymChk(iter->first, strPdbVerDir);
		if (dwResult != 0) {
			dwResult = GetPdbBySymChk(iter->first, strPdbVerDir);
			dwResult = GetPdbBySymChk(iter->first, strPdbVerDir);
			if (dwResult != 0){
				if (!IsDirExist(strPdbDirPath.c_str())) {
					strCmd = "mkdir " + strPdbDirPath;
					if (system(strCmd.c_str())) {
						LOGE("Create dir failed!dir path:%s,err:%d\n", strPdbDirPath.c_str(), GetLastError());
						return GetLastError();
					}
				}
				strPdbFilePath = strPdbDirPath + "\\" + iter->second.strPdbName;
				dwResult = download_pdb(strPdbURL, strPdbFilePath);
				if (dwResult != 0) {
					dwResult = download_pdb(strPdbURL, strPdbFilePath);
					if (dwResult != 0) {
						LOGE("Pdb download failed:%s\n", iter->first.c_str());

						strPeTempPath.clear();
						strPdbFailedFilePath.clear();

						pos = strPdbDirPath.rfind('\\');
						strPdbDirPath = strPdbDirPath.substr(0, pos);
						strCmd = "rmdir /s/q " + strPdbDirPath;
						system(strCmd.c_str());



						strPeTempPath = iter->first.substr(strPePath.size() + 1);
						strPeTempPath = strPeFailPath + strPeTempPath;
						strPdbFailedFilePath = strPeTempPath;
						PathRemoveFileSpecA(&strPeTempPath[0]);

						if (!IsDirExist(strPeTempPath.c_str()))
						{
							strCmd = "mkdir " + strPeTempPath;
							if (system(strCmd.c_str())) {
								LOGE("Create dir failed!dir path:%s,err:%d\n", strPeTempPath.c_str(), GetLastError());
								return GetLastError();
							}
						}



						if (!CopyFileA(iter->first.c_str(), strPdbFailedFilePath.c_str(), TRUE)) {
							LOGE("Copy file failed!path:%s\n", iter->first.c_str());
							return GetLastError();
						}


					}
				}
			}

		}


	}



	return 0;
}