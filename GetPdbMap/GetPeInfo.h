#pragma once
#include"stdafx.h"
typedef struct _SYM_FECODE_INFO {
    uint64_t offset;//代码段偏移量
    uint32_t codeSize;//代码段大小
    uint32_t machine = 0;//运行平台
}SYM_FECODE_INFO, * PSYM_FECODE_INFO;
int GetPeCodeInfo(const char* path, std::list<SYM_FECODE_INFO>* codeList);
