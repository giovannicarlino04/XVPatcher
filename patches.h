#ifndef PATCHES_HPP
#define PATCHES_HPP

#include "debug.h"
#include "xvpatcher.h"
#include "Utils.h"

// Helper function to apply byte patches
bool ApplyPatch(HANDLE hProcess, uintptr_t moduleBaseAddress, uintptr_t offset, const void* newBytes, SIZE_T byteLength);

// Individual patch functions
bool CMSPatch(HANDLE hProcess, uintptr_t moduleBaseAddress);
wchar_t* RetrieveVersionString(HANDLE hProcess, uintptr_t moduleBaseAddress);
bool VersionStringPatch(HANDLE hProcess, uintptr_t moduleBaseAddress);
bool BacBcmPatch(HANDLE hProcess, uintptr_t moduleBaseAddress);
bool InfiniteTimerPatch(HANDLE hProcess, uintptr_t moduleBaseAddress);

#endif
