#include"stdafx.h"
#include"GetSymbolByIda.h"


DWORD GetPdbMap(STRUCT_PEINFO* stPeInfo)
{
	uint32_t machine = 0;
	std::string cmd;
	std::string peName;
	std::string IDAPePath32 = "";
	std::string IDAPePath64 = "";

	char szExePath[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, szExePath, MAX_PATH);
	PathRemoveFileSpecA(szExePath);
	PathAddBackslashA(szExePath);	
	IDAPePath32 = (std::string)szExePath +"IDA7.0\\idat.exe";
	IDAPePath64 = (std::string)szExePath +"IDA7.0\\idat64.exe";
	//printf("IDAPePath64:%s", IDAPePath64.c_str());


	if (stPeInfo->peCodeList.size()==0)
	{
		return NULLREGION;
	}
	machine = stPeInfo->peCodeList.front().machine;
	peName = stPeInfo->strSymbolPath.substr(0, stPeInfo->strSymbolPath.find_last_of('.'));
	

	if (machine == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
	{
		cmd = IDAPePath32 + " -A -S" + "\"" + (std::string)szExePath +"get_func.py " + peName + "\"" + " \"" + peName + "\"";
	}
	if (machine == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
	{
		cmd = IDAPePath64 + " -A -S" + "\"" + (std::string)szExePath +"get_func.py " + peName + "\"" + " \"" + peName + "\"";
	}

	
	printf("%s\n", cmd.c_str());
	
	return system(cmd.c_str());

}