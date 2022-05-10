#include"stdafx.h"
#include"SetPdb.h"
#include"GetSymbolByIda.h"
#pragma warning(disable:4996)
#pragma warning(disable:6387)
void ExpSymTab(LPCSTR lpSymbolPath, std::vector<FUNCINFO>* PDBSymInfoVector)
{
    std::string symFile;
    std::string strFile;
    std::string  strFileName;
    std::vector<FUNCINFO>::iterator itor;

    symFile = lpSymbolPath;

    FILE* symfp = fopen(symFile.c_str(), "wb+");
    if (symfp == NULL)
    {
        LOGE("Fail to open or create symbolFile!%s\n",symFile.c_str());
        return;
    }

    for (itor = PDBSymInfoVector->begin(); itor != PDBSymInfoVector->end(); itor++)
    {
        fprintf(symfp, "%ls ", itor->name.c_str());
        fprintf(symfp, "%lld ", itor->size);
        fprintf(symfp, "%d ", itor->type);
        fprintf(symfp, "0x%llX\n", itor->rva);
    }
    printf("%s export succeeded!\n", symFile.c_str());
    if (fclose(symfp) != 0)
    {
        LOGE("Error in closing file %s\n", symFile.c_str());
    }
}

int ReadPDB(LPCTSTR pPdbPath, PDBSymInfoVector& vecInfo, std::list<SYM_FECODE_INFO>* peCodeList)
{
    if (!pPdbPath)
    {
        return ERROR_INTERNAL_ERROR;
    }
    FILE* pFile;
    if (_wfopen_s(&pFile, pPdbPath, L"r") || !pFile)
    {
       
        LOGE("invalid file name or file does not exist(ReadPDB):%s\n", pPdbPath);
        return ERROR_INTERNAL_ERROR;
    }
    fclose(pFile);

    IDiaDataSource* pDiaDataSource;
    IDiaSession* pDiaSession;
    IDiaSymbol* pGlobalSymbol;
    DWORD dwMachineType = CV_CFL_80386;
    if (!LoadDataFromPdb(pPdbPath, &pDiaDataSource, &pDiaSession, &pGlobalSymbol))
    {
        LOGE("LoadDataFromPdb fail:%ls", pPdbPath);
        return ERROR_INTERNAL_ERROR;
    }
    if (!DumpAllFunctions(pGlobalSymbol, vecInfo, peCodeList))
    {
        LOGE("DumpAllFunctions fail!%s\n", pPdbPath);
        Cleanup();
        return ERROR_INTERNAL_ERROR;
    }


    Cleanup();
    return 0;
}


DWORD SaveSymbol(LPCSTR lpPdbPath, STRUCT_PEINFO* stPeInfo) {
  
    DWORD dwResult = 0;
    std::list<SYM_FUNCTION_INFO> functionSymList;
    SYM_PARAM stParam = { 0 };


    PDBSymInfoVector vecInfo;


    if (0 != ReadPDB(CString(lpPdbPath).GetString(), vecInfo, &(stPeInfo->peCodeList)))
    {
       
        LOGE("ReadPDB fail:%s",lpPdbPath);
        return ERROR_INTERNAL_ERROR;
    }


    if (vecInfo.size()==0) {
        LOGE("PDBSymInfoVector is null,%s", lpPdbPath);
        return ERROR_INTERNAL_ERROR;
    }
  
    ExpSymTab(stPeInfo->strSymbolPath.c_str(), &vecInfo);

    return 0;
}


