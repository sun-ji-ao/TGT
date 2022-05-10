// Dia2Dump.cpp : Defines the entry point for the console application.
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
#include "Dia2Dump.h"
#include "PrintSymbol.h"
#include "diacreate.h"


#include "Callback.h"

#pragma warning (disable : 4100)
#pragma warning (disable : 4267)
const wchar_t* g_szFilename;
IDiaDataSource* g_pDiaDataSource;
IDiaSession* g_pDiaSession;
IDiaSymbol* g_pGlobalSymbol;
DWORD g_dwMachineType = CV_CFL_80386;
std::unordered_map<uint64_t, uint64_t>publicSymbol_map;

////////////////////////////////////////////////////////////
// Create an IDiaData source and open a PDB file
bool LoadDataFromPdb(
    const wchar_t* szFilename,
    IDiaDataSource** ppSource,
    IDiaSession** ppSession,
    IDiaSymbol** ppGlobal)
{
    wchar_t wszExt[MAX_PATH];
    const wchar_t* wszSearchPath = L"SRV**\\\\symbols\\symbols"; // Alternate path to search for debug data
    DWORD dwMachType = 0;

    HRESULT hr = CoInitialize(NULL);

    std::string strPath;
    char szExePath[MAX_PATH] = { 0 };
    GetModuleFileNameA(NULL, szExePath, MAX_PATH);
    PathRemoveFileSpecA(szExePath);
    PathAddBackslashA(szExePath);

    strPath = (std::string)szExePath+ "bin\\amd64\\msdia140.dll";
    //printf("strPath:%s\n", strPath.c_str());

   
    int pSize = MultiByteToWideChar(CP_OEMCP, 0, strPath.c_str(), strlen(strPath.c_str()) + 1, NULL, 0);
    wchar_t* pWCStrKey = new wchar_t[pSize];
    //第二次调用将单字节字符串转换成双字节字符串
    MultiByteToWideChar(CP_OEMCP, 0, strPath.c_str(), strlen(strPath.c_str()) + 1, pWCStrKey, pSize);
 


    // Obtain access to the provider

   /* hr = CoCreateInstance(__uuidof(DiaSource),
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          __uuidof(IDiaDataSource),
                          (void **) ppSource);*/
    hr = NoRegCoCreate(
        pWCStrKey,
        __uuidof(DiaSource),
        __uuidof(IDiaDataSource),
        (void**)ppSource);

    if (FAILED(hr)) {
        wprintf(L"NoRegCoCreate failed - HRESULT = %08X\n", hr);

        return false;
    }

    _wsplitpath_s(szFilename, NULL, 0, NULL, 0, NULL, 0, wszExt, MAX_PATH);

    if (!_wcsicmp(wszExt, L".pdb")) {
        // Open and prepare a program database (.pdb) file as a debug data source

        hr = (*ppSource)->loadDataFromPdb(szFilename);

        if (FAILED(hr)) {
            wprintf(L"loadDataFromPdb failed - HRESULT = %08X\n", hr);

            return false;
        }
    }

    else {
        CCallback callback; // Receives callbacks from the DIA symbol locating procedure,
                            // thus enabling a user interface to report on the progress of
                            // the location attempt. The client application may optionally
                            // provide a reference to its own implementation of this
                            // virtual base class to the IDiaDataSource::loadDataForExe method.
        callback.AddRef();

        // Open and prepare the debug data associated with the executable

        hr = (*ppSource)->loadDataForExe(szFilename, wszSearchPath, &callback);

        if (FAILED(hr)) {
            wprintf(L"loadDataForExe failed - HRESULT = %08X\n", hr);

            return false;
        }
    }

    // Open a session for querying symbols

    hr = (*ppSource)->openSession(ppSession);

    if (FAILED(hr)) {
        wprintf(L"openSession failed - HRESULT = %08X\n", hr);

        return false;
    }

    // Retrieve a reference to the global scope

    hr = (*ppSession)->get_globalScope(ppGlobal);

    if (hr != S_OK) {
        wprintf(L"get_globalScope failed\n");

        return false;
    }

    // Set Machine type for getting correct register names

    if ((*ppGlobal)->get_machineType(&dwMachType) == S_OK) {
        switch (dwMachType) {
        case IMAGE_FILE_MACHINE_I386: g_dwMachineType = CV_CFL_80386; break;
        case IMAGE_FILE_MACHINE_IA64: g_dwMachineType = CV_CFL_IA64; break;
        case IMAGE_FILE_MACHINE_AMD64: g_dwMachineType = CV_CFL_AMD64; break;
        }
    }

    return true;
}





////////////////////////////////////////////////////////////
// Release DIA objects and CoUninitialize
void Cleanup()
{
    if (g_pGlobalSymbol) {
        g_pGlobalSymbol->Release();
        g_pGlobalSymbol = NULL;
    }

    if (g_pDiaSession) {
        g_pDiaSession->Release();
        g_pDiaSession = NULL;
    }

    CoUninitialize();
}


bool DumpAllFunctions(IDiaSymbol* pGlobal, PDBSymInfoVector& vecInfo, std::list<SYM_FECODE_INFO>* pecodeList)
{
    IDiaEnumSymbols* pEnumSymbols;
    IDiaSymbol* pSymbol;
    enum SymTagEnum dwSymTags[] = { SymTagFunction, SymTagBlock, SymTagPublicSymbol };
    ULONG celt = 0;


    for (size_t i = 0; i < _countof(dwSymTags); i++, pEnumSymbols = NULL) {
        if (SUCCEEDED(pGlobal->findChildren(dwSymTags[i], NULL, nsNone, &pEnumSymbols))) {
            while (SUCCEEDED(pEnumSymbols->Next(1, &pSymbol, &celt)) && (celt == 1)) {
                PrintFunctionSymbol(pSymbol, &vecInfo, pecodeList);

                pSymbol->Release();
            }

            pEnumSymbols->Release();
        }

        else {
            wprintf(L"ERROR - DumpAllFunctions() returned no symbols\n");

            return false;
        }
    }

    putwchar(L'\n');

    return true;
}

struct OMAP_DATA
{
    DWORD dwRVA;
    DWORD dwRVATo;
};