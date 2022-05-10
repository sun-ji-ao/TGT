#include <Windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <memory>
#include <WinInet.h>

#pragma warning(disable : 4018)

int	get_pdb_url(std::string& path, std::string& url);
int	download_pdb(std::string url, std::string path);