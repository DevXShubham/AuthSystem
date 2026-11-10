#pragma once 

#include <windows.h>

bool writeAll(HANDLE pipe, const void* data, DWORD size);
bool readExact(HANDLE pipe, void* data, DWORD size);