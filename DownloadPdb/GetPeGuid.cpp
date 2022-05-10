#include "GetPeGuid.h"

#include <atlstr.h>
#include <stdint.h>

#define CV_SIGNATURE_NB10   '01BN'
#define CV_SIGNATURE_RSDS   'SDSR'


// CodeView header 
typedef struct _CV_HEADER
{
	DWORD CvSignature; // NBxx
	LONG  Offset;      // Always 0 for NB10
}CV_HEADER;

// CodeView NB10 debug information 
// (used when debug information is stored in a PDB 2.00 file) 
typedef struct _CV_INFO_PDB20
{
	CV_HEADER  Header;
	DWORD      Signature;       // seconds since 01.01.1970
	DWORD      Age;             // an always-incrementing value 
	BYTE       PdbFileName[1];  // zero terminated string with the name of the PDB file 
}CV_INFO_PDB20;

// CodeView RSDS debug information 
// (used when debug information is stored in a PDB 7.00 file) 
typedef struct _CV_INFO_PDB70
{
	DWORD      CvSignature;
	GUID       Signature;       // unique identifier 
	DWORD      Age;             // an always-incrementing value 
	BYTE       PdbFileName[1];  // zero terminated string with the name of the PDB file 
}CV_INFO_PDB70;


void Guid2String(GUID& Guid, DWORD dwAge, OUT LPSTR lpGuid, DWORD dwSize)
{
	sprintf_s(lpGuid, dwSize, "%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x%01x",
		Guid.Data1, Guid.Data2, Guid.Data3, Guid.Data4[0], Guid.Data4[1], Guid.Data4[2],
		Guid.Data4[3], Guid.Data4[4], Guid.Data4[5], Guid.Data4[6], Guid.Data4[7], dwAge);

}

DWORD GetGuidByDebugInfo(LPSTR pDebugInfo, DWORD DebugInfoSize, OUT LPSTR lpGuid, DWORD dwSize)
{

	if ((pDebugInfo == 0) || (DebugInfoSize == 0) || (lpGuid == NULL) || (dwSize == 0))
		return ERROR_INVALID_PARAMETER;


	DWORD CvSignature = *(DWORD*)pDebugInfo;

	// 只支持RSDSS
	if (CvSignature == CV_SIGNATURE_RSDS)
	{
		// RSDS -> PDB 7.00 
		CV_INFO_PDB70* pCvInfo = (CV_INFO_PDB70*)pDebugInfo;

		Guid2String(pCvInfo->Signature, pCvInfo->Age, lpGuid, dwSize);
	}
	else
	{
		return ERROR_NOT_SUPPORTED;

	}

	return 0;

}

DWORD GetPdbGuid(LPCSTR lpPePath, LPSTR lpGuid, DWORD dwSize)
{

	DWORD dwResult = 0;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	char* pBuf = NULL;
	char* pImageBuf = NULL;
	BOOL bWin64 = FALSE;
	DWORD nBufSize = 0;
	DWORD dwHeadSize = 0;
	DWORD dwDebugRVA = 0;
	char pszModule[MAX_PATH] = { 0 };
	BOOL bUnicode = FALSE;

	if (lpPePath == NULL || lpGuid == NULL || dwSize < sizeof(DWORD)) {
		printf("param error\n");
		return ERROR_INVALID_PARAMETER;
	}

	hFile = CreateFileA(lpPePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		dwResult = GetLastError();
		printf("CreateFileA error,%d\n", dwResult);
		return dwResult;
	}

	do {
		LARGE_INTEGER pliFileSiz = { 0 };
		if (!GetFileSizeEx(hFile, &pliFileSiz)) {
			dwResult = GetLastError();
			printf("GetFileSizeEx fail, error: %d.", dwResult);
			break;
		}

		uint64_t iFileSize = pliFileSiz.QuadPart;

		pBuf = (char*)malloc((size_t)iFileSize);
		if (!pBuf)
		{
			dwResult = ERROR_NOT_ENOUGH_MEMORY;
			printf("malloc fail, error: %d.", dwResult);
			break;
		}
		memset(pBuf, 0, (size_t)iFileSize);

		DWORD nNunberOfRead = 0;
		if (!ReadFile(hFile, pBuf, (DWORD)iFileSize, &nNunberOfRead, NULL)) {
			dwResult = GetLastError();
			printf("ReadFilefail, error: %d.", dwResult);
			break;
		}

		//检查文件格式
		PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pBuf;
		if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
			break;

		PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)((PBYTE)pDosHeader + pDosHeader->e_lfanew);
		if (pNtHeader->Signature != IMAGE_NT_SIGNATURE)
			break;

		if (pNtHeader->FileHeader.SizeOfOptionalHeader == 0)
			break;

		USHORT Magic = pNtHeader->OptionalHeader.Magic;

		if (Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC && Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
			break;

		PIMAGE_FILE_HEADER pFileHead = &pNtHeader->FileHeader;
		PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNtHeader);

		int iSection = pFileHead->NumberOfSections;


		if (Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
			PIMAGE_OPTIONAL_HEADER32 optionalHeader;
			optionalHeader = &((PIMAGE_NT_HEADERS32)pNtHeader)->OptionalHeader;
			dwDebugRVA = optionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
			nBufSize = optionalHeader->SizeOfImage;
			dwHeadSize = optionalHeader->SizeOfHeaders;

			bWin64 = FALSE;
		}
		else {
			PIMAGE_OPTIONAL_HEADER64 optionalHeader;
			optionalHeader = &((PIMAGE_NT_HEADERS64)pNtHeader)->OptionalHeader;
			dwDebugRVA = optionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
			nBufSize = optionalHeader->SizeOfImage;
			dwHeadSize = optionalHeader->SizeOfHeaders;

			bWin64 = TRUE;
		}

		pImageBuf = (char*)malloc(nBufSize);
		if (!pImageBuf)
		{
			dwResult = ERROR_NOT_ENOUGH_MEMORY;
			printf("malloc fail, error: %d.", dwResult);
			break;
		}

		memset(pImageBuf, 0, nBufSize);

		memcpy(pImageBuf, pBuf, dwHeadSize);
		//
		// 复制各种头数据
		//
		for (int num = 0; num < iSection; num++)
		{
			if (pSection->SizeOfRawData == 0)
			{
				pSection++;
				continue;
			}

			memcpy(pImageBuf + pSection->VirtualAddress,
				pBuf + pSection->PointerToRawData,
				pSection->SizeOfRawData
			);
			pSection++;
		}

		PIMAGE_DEBUG_DIRECTORY stIdd = (PIMAGE_DEBUG_DIRECTORY)(pImageBuf + dwDebugRVA);
		DWORD dwPdbRva = stIdd->PointerToRawData;
		DWORD dwPdbSize = stIdd->SizeOfData;

		char* pDebugInfo = pBuf + dwPdbRva;

		dwResult = GetGuidByDebugInfo(pDebugInfo, dwPdbSize, lpGuid, dwSize);

	} while (false);

	if (hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	if (pBuf) {
		free(pBuf);
	}
	if (pImageBuf) {
		free(pImageBuf);
	}

	return dwResult;
}
