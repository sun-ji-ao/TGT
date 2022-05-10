#ifndef GETFILEPATH_H_FILE
#define GETFILEPATH_H_FILE


#include<string>
#include<stdio.h>
#include<Windows.h>
#include<algorithm>
#include<unordered_map>
#include"LogWriter.h"
#pragma warning(disable : 4129)
#pragma warning(disable : 4996)


typedef struct _STRUCT_PDBINFO
{
	std::string strPdbName;	     //PDB文件名
	std::string  strPdbGuid;	 //PDB_guid
	std::string	strPdbURL;		 //PDB下载网址
	std::string	strPdbPath;		 //PDB生成路径

}STRUCT_PDBINFO, * PSTRUCT_PDBINFO;

extern int PeNumber;
extern std::string strPePath;
extern std::unordered_map<std::string, STRUCT_PDBINFO>pdb_map;

DWORD  search_pe_directory(const   std::string& caseFileName);

#endif