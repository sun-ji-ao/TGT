#pragma once
#include"stdafx.h"
typedef struct _SYM_FECODE_INFO {
    uint64_t offset;//�����ƫ����
    uint32_t codeSize;//����δ�С
    uint32_t machine = 0;//����ƽ̨
}SYM_FECODE_INFO, * PSYM_FECODE_INFO;
int GetPeCodeInfo(const char* path, std::list<SYM_FECODE_INFO>* codeList);
