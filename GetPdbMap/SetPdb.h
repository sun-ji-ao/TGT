#pragma once
#include"stdafx.h"
#include"SetPe.h"
#include"Dia2Dump.h"
#include"GetPeInfo.h"

#include<cstdio>
#include<cstdlib>
#include<dia2.h>
#include<TCHAR.h>
#include <dbghelp.h>


#define _NO_CVCONST_H 

#define MAX_LINE 1024
#define MAX_PATH          260

typedef struct _SYM_FUNCTION_INFO {
    std::string funcname;//函数名
    uint64_t offset;//函数偏移量
    uint32_t funcsize;//函数大小
    uint32_t functype = 0;//函数类型
}SYM_FUNCTION_INFO, * PSYM_FUNCTION_INFO;

typedef struct _SYM_PARAM {
    std::list<SYM_FUNCTION_INFO>* functionSymList;
    std::unordered_map<uint64_t, uint64_t>pubSym_map;
}SYM_PARAM, * PSYM_PARAM;


DWORD SaveSymbol(LPCSTR lpPdbPath, STRUCT_PEINFO* stPeInfo);
int ReadPDB(LPCTSTR pPdbPath, PDBSymInfoVector& vecInfo, std::list<SYM_FECODE_INFO>* peCodeList);
void ExpSymTab(LPCSTR lpSymbolPath, std::vector<FUNCINFO>* PDBSymInfoVector);
