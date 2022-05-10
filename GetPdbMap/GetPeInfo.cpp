#include"stdafx.h"
#include"GetPeInfo.h"



#pragma warning(disable:4996)
int __is_code_sections(IMAGE_SECTION_HEADER* pmySectionHeader)
{
	uint64_t secCharacteristics = pmySectionHeader->Characteristics;
	const char* secName = (char*)pmySectionHeader->Name;
	if ((secCharacteristics & (IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ)) !=
		(IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ))
	{
		return 0;
	}
	return 1;
}

int GetPeCodeInfo(const char* path, std::list<SYM_FECODE_INFO>* codeList)
{
	errno_t err;
	FILE* pfile = NULL;
	uint32_t machine = 0;
	int32_t e_lfanew = 0;
	uint32_t numOfSec = 0;
	IMAGE_DOS_HEADER myDosHeader = { 0 };
	IMAGE_NT_HEADERS64 myNTHeader = { 0 };
	IMAGE_SECTION_HEADER* pmySectionHeader = NULL, * pmySectionHeader_s = NULL;

	codeList->clear();

	if ((err = fopen_s(&pfile, path, "rb")) != 0)
	{
		printf("open file fial,%s\n", path);
		return 0;
	}

	fseek(pfile, 0, SEEK_SET);

	//DOS头部分
	if (0 == fread(&myDosHeader, 1, sizeof(IMAGE_DOS_HEADER), pfile))
	{
		printf("fread fail,%d,%s\n", GetLastError(), path);
		return 0;
	}
	e_lfanew = myDosHeader.e_lfanew;

	//NT头部分
	fseek(pfile, e_lfanew, SEEK_SET);

	if (0 == fread(&myNTHeader, 1, sizeof(IMAGE_NT_HEADERS64), pfile))
	{
		printf("fread nt header fail,%d\n", GetLastError());
		return 0;
	}
	
	machine = myNTHeader.OptionalHeader.Magic;
	/*printf("machine:%x\n", machine);*/
	////根据NT头中的FileHeader.Machine判断是哪种 32位or64位PE文件
	//const char* PeType;
	//switch (machine)
	//{
	//case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
	//{
	//	PeType = "Bit64";
	//	break;
	//}
	//case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
	//{
	//	PeType = "Bit32";
	//	break;
	//}

	//default: PeType = "Unknow";
	//}


		//OPTIONAL头部分
		numOfSec = myNTHeader.FileHeader.NumberOfSections;


		//节表目录
		pmySectionHeader = (IMAGE_SECTION_HEADER*)calloc(numOfSec, sizeof(IMAGE_SECTION_HEADER));
		if (pmySectionHeader == NULL) {
			printf("calloc fail\n");
			return -1;
		}

		pmySectionHeader_s = pmySectionHeader;

		if (machine == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
		{
			fseek(pfile, (e_lfanew + sizeof(IMAGE_NT_HEADERS32)), SEEK_SET);
		}
		if (machine == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
		{
			fseek(pfile, (e_lfanew + sizeof(IMAGE_NT_HEADERS64)), SEEK_SET);
		}
		


		fread(pmySectionHeader, sizeof(IMAGE_SECTION_HEADER), numOfSec, pfile);
		if (pmySectionHeader == NULL)
		{
			printf("Error reading section header");
			return -1;
		}

		for (int i = 0; i < (int)numOfSec; i++, pmySectionHeader++)
		{
			if (__is_code_sections(pmySectionHeader)) {
				SYM_FECODE_INFO peCode;
				peCode.machine = machine;
				peCode.offset = pmySectionHeader->VirtualAddress;
				peCode.codeSize = pmySectionHeader->SizeOfRawData;
				codeList->push_back (peCode);
			}
		}


	if (pmySectionHeader_s) {
		free(pmySectionHeader_s);
	}

	if (pfile) {
		fclose(pfile);
	}

	if (codeList->size() == 0)
	{

		return -1;
	}

	return 0;
}