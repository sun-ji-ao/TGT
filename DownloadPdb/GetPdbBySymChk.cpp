#include"GetPdbBySymChk.h"
int	GetPdbBySymChk(std::string strPePath, std::string strPdbPath) {

	std::string strCmd;
	std::string strPathCmd;
	std::string strSymChkPath;
	char szExePath[MAX_PATH] = { 0 };


	printf("GetPdb:%s\n", strPePath.c_str());
	//printf("strPdbPath:%s\n", strPdbPath.c_str());

	GetModuleFileNameA(NULL, szExePath, MAX_PATH);
	PathRemoveFileSpecA(szExePath);
	PathAddBackslashA(szExePath);
	strSymChkPath =(std::string)szExePath;
	strPathCmd = "cd /d " + strSymChkPath;
	//printf("strPathCmd:%s\n", strPathCmd.c_str());


	strCmd = strPathCmd+"&& symchk /r "+ strPePath +" /s SRV*"+ strPdbPath+"\\*http://msdl.microsoft.com/download/symbols";
	if (system(strCmd.c_str())) {
		return -1;
	}

	return 0;
}