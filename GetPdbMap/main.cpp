#include"stdafx.h"
#include"SetPe.h"
int main(int argc, char* argv[])
{

	std::string pzsPePath;
	std::string pszPdbPath;

	if (argc <2) {
		printf("Param error,please enter parameters\n");
		return 0;
	}
	else if (argc == 2)
	{
		pzsPePath = argv[1];
	}
	else if (argc == 3)
	{
		pzsPePath = argv[1];
		pszPdbPath = argv[2];
	}

	


	if (pszPdbPath.empty())
	{
		if (!PathFileExistsA(pzsPePath.c_str())) {
			LOGE("path is not exist,pepath:%s,pdb:%s\n", pzsPePath);
			return 0;
		}

		DWORD dwResult = GetPeSymbol(pzsPePath.c_str());
		if (dwResult != 0) {
			LOGE("GetPeSymbol fail,pepath:%s,pdb:%s\n", pzsPePath, pszPdbPath);
			return 0;
		}
	}
	else
	{
		if (!PathFileExistsA(pzsPePath.c_str()) || !PathFileExistsA(pszPdbPath.c_str())) {
			LOGE("path is not exist,pepath:%s,pdb:%s\n", pzsPePath.c_str(), pszPdbPath.c_str());
			return 0;
		}

		DWORD dwResult = GetPeSymbol(pzsPePath.c_str(), pszPdbPath.c_str());
		if (dwResult != 0) {
			LOGE("GetPeSymbol fail,pepath:%s,pdb:%s\n", pzsPePath, pszPdbPath);
			return 0;
		}
	}


	return 0;
}