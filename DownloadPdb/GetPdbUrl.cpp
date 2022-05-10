#include"GetPdbUrl.h"
int	get_pdb_url(std::string& path, std::string& url)
{
	// 打开文件
	HANDLE hFile = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE) {
		return -1;
	}

	// 获取文件大小并申请空间
	DWORD size = GetFileSize(hFile, 0);//获取指定文件的大小
	std::shared_ptr<unsigned char> buffer(new unsigned char[size]);
	if (buffer == nullptr)
	{
		CloseHandle(hFile);
		return -1;
	}

	// 将文件数据读出
	DWORD bytes = 0;
	DWORD ret = ReadFile(hFile, buffer.get(), size, &bytes, 0);
	if (ret == FALSE || bytes != size)
	{
		CloseHandle(hFile);
		return -1;
	}

	// 关闭句柄
	CloseHandle(hFile);

	// 验证dos标识
	PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)buffer.get();
	if (dos->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return -1;
	}
	// 验证nt标识
	PIMAGE_NT_HEADERS32 nt32 = (PIMAGE_NT_HEADERS32)(dos->e_lfanew + buffer.get());
	if (nt32->Signature != IMAGE_NT_SIGNATURE)
	{
		return -1;
	}

	// 读取基本信息
	PIMAGE_SECTION_HEADER sec = IMAGE_FIRST_SECTION(nt32);
	DWORD sec_count = nt32->FileHeader.NumberOfSections;
	DWORD debug_rva = nt32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
	DWORD debug_size = nt32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;
	// 64和32位不同
	PIMAGE_NT_HEADERS64 nt64 = (PIMAGE_NT_HEADERS64)(dos->e_lfanew + buffer.get());
	if (nt64->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64)
	{
		sec = IMAGE_FIRST_SECTION(nt64);
		sec_count = nt64->FileHeader.NumberOfSections;
		debug_rva = nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
		debug_size = nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;
	}

	// 程序没有调试数据信息
	if (debug_rva == 0 || debug_size == 0)
	{
		return -1;
	}
	// 相对虚拟地址转化文件偏移
	auto rva_to_file = [](PIMAGE_SECTION_HEADER sec, int count, int rva) -> int
	{
		for (int i = 0; i < count; i++)
			if (sec[i].VirtualAddress < rva && sec[i].VirtualAddress + sec[i].Misc.VirtualSize > rva)
				return rva - sec[i].VirtualAddress + sec[i].PointerToRawData;
		return 0;
	};

	// 取得调试数据的文件偏移
	int debug_file_offset = rva_to_file(sec, sec_count, debug_rva);
	if (debug_file_offset == 0)
	{
		return -1;
	}

	// 调试信息结构体
	typedef struct _debug_information_
	{
		unsigned long signature;
		GUID guid;
		unsigned long age;
		char pdb[1];
	}debug_information, * pdebug_information;

	// 取得第一个调试结构指针
	PIMAGE_DEBUG_DIRECTORY debug_dir = (PIMAGE_DEBUG_DIRECTORY)(debug_file_offset + buffer.get());


	for (int i = 0; i * sizeof(IMAGE_DEBUG_DIRECTORY) < debug_size; i++)
	{
		// 类型要求
		if (debug_dir[i].Type != IMAGE_DEBUG_TYPE_CODEVIEW)
		{
			continue;
		}
		pdebug_information info = (pdebug_information)(buffer.get() + debug_dir[i].PointerToRawData);

		// 格式化网络url路径
		std::stringstream sym;
		sym << info->pdb << "/";
		sym << std::setfill('0') << std::setw(8) << std::hex << info->guid.Data1 << std::setw(4) << std::hex << info->guid.Data2 << std::setw(4) << std::hex << info->guid.Data3;
		for (const auto i : info->guid.Data4) sym << std::setw(2) << std::hex << +i;
		sym << "1/" << info->pdb;
		url = "http://msdl.microsoft.com/download/symbols/" + sym.str();
		// 返回成功
		return 0;
	}

	// 返回失败
	return -1;
}

int	download_pdb(std::string url, std::string path)
{
	// 初始化
	HINTERNET hOpen = InternetOpenA("", INTERNET_OPEN_TYPE_PRECONFIG, 0, 0, 0);
	if (hOpen == NULL) return -1;

	// 打开url
	HINTERNET hUrl = InternetOpenUrlA(hOpen, url.c_str(), 0, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_UI | INTERNET_FLAG_PRAGMA_NOCACHE, 0);
	if (hUrl == NULL)
	{
		InternetCloseHandle(hOpen);
		return -1;
	}

	// 查询程序大小
	DWORD file_size = 0, buf_size = sizeof(DWORD);
	BOOL is_valid_url = HttpQueryInfo(hUrl, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_CONTENT_LENGTH, &file_size, &buf_size, 0);
	if (is_valid_url == FALSE || file_size == 0)
	{
		InternetCloseHandle(hUrl);
		InternetCloseHandle(hOpen);
		return -1;
	}

	// 创建本地文件
	HANDLE h = CreateFileA(path.c_str(), GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
	if (h == INVALID_HANDLE_VALUE)
	{
		InternetCloseHandle(hUrl);
		InternetCloseHandle(hOpen);
		return -1;
	}

	// 开始循环读取网络程序数据
	const unsigned int max_tcp = 4000;
	unsigned int total = 0;
	unsigned int last_value = 0;
	unsigned char buffer[max_tcp]{ 0 };
	memset(buffer, 0, max_tcp);
	printf("\n%s:\n", path.c_str());
	while (InternetReadFile(hUrl, buffer, max_tcp, &buf_size))
	{
		if (buf_size)
		{
			WriteFile(h, buffer, max_tcp, &buf_size, 0);
			total += buf_size;

			
			unsigned int value = (unsigned int)((double)total / (double)file_size * (double)100.0f);
			if (value != last_value)
			{
				last_value = value;
				if (!(value % 10))
				{
					printf("%d%%  ", value);
				}
			}
		}
		else break;
	}

	InternetCloseHandle(hUrl);
	InternetCloseHandle(hOpen);
	return 0;
}