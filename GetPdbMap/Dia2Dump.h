#include "dia2.h"
#include "diacreate.h"
#include"GetPeInfo.h"
#include"stdafx.h"


extern const wchar_t* g_szFilename;
extern IDiaDataSource* g_pDiaDataSource;
extern IDiaSession* g_pDiaSession;
extern IDiaSymbol* g_pGlobalSymbol;
extern DWORD g_dwMachineType;

extern std::unordered_map<uint64_t, uint64_t>publicSymbol_map;



void Cleanup();
bool LoadDataFromPdb(const wchar_t*, IDiaDataSource**, IDiaSession**, IDiaSymbol**);
bool DumpAllFunctions(IDiaSymbol* pGlobal, PDBSymInfoVector& vecInfo, std::list<SYM_FECODE_INFO>* pecodeList);




///////////////////////////////////////////////////////////////////
// Functions defined in regs.cpp
const wchar_t* SzNameC7Reg(USHORT, DWORD);
const wchar_t* SzNameC7Reg(USHORT);

