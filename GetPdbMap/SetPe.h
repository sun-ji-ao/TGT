#pragma once
#include"stdafx.h"
#include"GetPeInfo.h"

typedef struct _STRUCT_PDBINFO
{
	std::string strPdbName;	     //PDB文件名
	std::string  strPdbAddress;	     //PDB地址

}STRUCT_PDBINFO, * PSTRUCT_PDBINFO;

typedef struct _STRUCT_PEINFO
{
	std::string strPeName;				//Pe文件名
	std::string  strSymbolPath;			//Symbol所在路径
	std::string  strPeGuid;				//Pe GUI
	std::list<SYM_FECODE_INFO> peCodeList;			//pe代码段

}STRUCT_PEINFO, * PSTRUCT_PEINFO;


extern std::unordered_map<std::string, STRUCT_PEINFO>pe_map;
extern std::unordered_map<std::string, STRUCT_PDBINFO>pdb_map;


DWORD  search_pe_directory(const   std::string& caseFileName);
void dfsFolder(std::string folderPath, std::string strPeGuid, std::string& strPdbNewPath);
DWORD GetPeSymbol(LPCSTR lpPePath);
DWORD GetPeSymbol(LPCSTR lpPePath, LPCSTR lpPdbPath);


