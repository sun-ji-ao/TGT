#include<stdio.h>
#include<string>
#include"GetPdb.h"
#include"LogWriter.h"
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib,"Wininet.lib")
std::string strPePath;
int PeNumber = 0;
int main(int argc, char* argv[])
{

	DWORD dwResult;

	if (argc != 2) {
		printf("Parameter error!");
		return 0;
	}

	strPePath = argv[1];

	dwResult = GetPdb(strPePath);
	if (dwResult != 0) {
		LOGE("GetPdb fail,PdbPath:%s\n", strPePath.c_str());
		return 0;
	}
	printf("\nPeNumber=%d", PeNumber);


	return 0;
}
