//
//  base64 encoding and decoding with C++.
//  Version: 1.01.00
//

#pragma once
#include <stddef.h>

void base64_encode(void* pInputData, unsigned int len, char *pOut, size_t pOutSize);
void base64_decode(const char *pInputStr, void *pOutput, size_t pOutSize);
