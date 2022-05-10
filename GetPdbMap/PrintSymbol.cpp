// PrintSymbol.cpp : Defines the printing procedures for the symbols
//
// This is a part of the Debug Interface Access SDK 
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// This source code is only intended as a supplement to the
// Debug Interface Access SDK and related electronic documentation 
// provided with the library.
// See these sources for detailed information regarding the
// Debug Interface Access SDK API.
//

#include "stdafx.h"

#include <malloc.h>

#include "dia2.h"
#include "regs.h"
#include "PrintSymbol.h"
#include"GetPeInfo.h"

#include <locale>
#include <codecvt>

#pragma warning(disable:4996)


// convert string to wstring
inline std::wstring to_wide_string(const std::string& input)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(input);
}

void PrintFunctionSymbol(IDiaSymbol* pSymbol, PDBSymInfoVector* pvecInfo , std::list<SYM_FECODE_INFO>* pecodeList)
{
    DWORD dwSymTag;
    DWORD dwRVA;
    DWORD dwSeg;
    DWORD dwOff;
    uint64_t uiTempRVA;
    FUNCINFO fcnInfo;
    char TempName[100];
    std::string strTempName;




    if (pSymbol->get_symTag(&dwSymTag) != S_OK) {
        return;
    }
    if (dwSymTag == SymTagFunction || dwSymTag == SymTagPublicSymbol)//|| dwSymTag == SymTagBlock
    {

        if (pSymbol->get_length(&fcnInfo.size) == S_OK) {
            //wprintf(L" size = %08X, ", (ULONG)fcnInfo.size);
        }

        //if (dwSymTag == SymTagFunction) {
        //    DWORD dwCall;

        //    /* if (pSymbol->get_callingConvention(&dwCall) == S_OK) {
        //         fcnInfo.convention = SafeDRef(rgCallingConvention, dwCall);
        //         wprintf(L", %s", fcnInfo.convention.c_str());
        //     }*/
        //}
    }

    if (pSymbol->get_relativeVirtualAddress(&dwRVA) != S_OK) {
        dwRVA = 0xFFFFFFFF;
    }

    pSymbol->get_addressSection(&dwSeg);
    //pSymbol->get_addressOffset(&dwOff);
    // wprintf(L"%s: [%08X][%04X:%08X] ", rgTags[dwSymTag], dwRVA, dwSeg, dwOff);
   
    uiTempRVA = dwRVA;
    fcnInfo.rva = dwRVA;
    // fcnInfo.offset = dwOff;
    fcnInfo.type = dwSymTag;

    if (dwSymTag == SymTagThunk) {
        BSTR bstrName;

        if (pSymbol->get_name(&bstrName) == S_OK) {
            // wprintf(L"%s\n", bstrName);
            fcnInfo.name = bstrName;
            SysFreeString(bstrName);
        }

        else {
            if (pSymbol->get_targetRelativeVirtualAddress(&dwRVA) != S_OK) {
                dwRVA = 0xFFFFFFFF;
            }

            pSymbol->get_targetSection(&dwSeg);
            pSymbol->get_targetOffset(&dwOff);
            // wprintf(L"target -> [%08X][%04X:%08X]\n", dwRVA, dwSeg, dwOff);
        }
    }

    else {
        BSTR bstrName;
        BSTR bstrUndname;

        if (pSymbol->get_name(&bstrName) == S_OK) {
            if (pSymbol->get_undecoratedName(&bstrUndname) == S_OK) {
                //wprintf(L"%s(%s)\n", bstrName, bstrUndname);

                SysFreeString(bstrUndname);
            }

            /* else {
                 wprintf(L"%s\n", bstrName);
             }*/
            fcnInfo.name = bstrName;
            SysFreeString(bstrName);
        }
    }

   
    if (pvecInfo && uiTempRVA != 0xFFFFFFFF)
    {
        if (fcnInfo.name.substr(0, 1) == L"?"&&fcnInfo.name.substr(0, 2) != L"??")
        {
            int i = fcnInfo.name.find_first_of('@');
            if (i != -1)
            {
                fcnInfo.name = fcnInfo.name.substr(1, i - 1);
            }
        }
        if (fcnInfo.name.substr(0, 2) == L"??"||fcnInfo.name.size()>MAX_PATH)
        {
            _itoa(uiTempRVA, TempName, 16);
            strTempName = TempName;
            fcnInfo.name = to_wide_string("sub_0x" + strTempName);
        }

        for (auto it = pecodeList->begin(); it != pecodeList->end(); it++)
        {
            if (uiTempRVA >= it->offset && uiTempRVA < (it->offset + it->codeSize))
            {
                pvecInfo->push_back(fcnInfo);
            }
            else
            {
                continue;
            }
        }

    }
}

