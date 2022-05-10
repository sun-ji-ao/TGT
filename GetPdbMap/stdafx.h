// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include<list>
#include<string>
#include<iostream>
#include<unordered_map>




#include<io.h>
#include<stdio.h>
#include<tchar.h>
#include<atlstr.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<shlwapi.h>
#include<Windows.h>






// TODO: reference additional headers your program requires here
#include "dia2.h"
#include <string>
#include <vector>
#include"LogWriter.h"
#pragma warning(disable:26495)


typedef struct _FUNCINFO
{
    std::wstring name;	     //函数名
    ULONGLONG  rva;			 //函数RVA
    ULONGLONG  size;         //函数大小
    ULONG  type;	     //函数类型

}FUNCINFO, PFUNCINFO;

typedef std::vector<FUNCINFO> PDBSymInfoVector;