#ifndef PATCHES_H
#define PATCHES_H

#include <windows.h>

bool ApplyMemoryPatch(HANDLE hProcess, uintptr_t address, const void* patchBytes, SIZE_T patchSize);

bool CMSPatches(HANDLE hProcess, uintptr_t moduleBaseAddress);
bool VersionStringPatch(HANDLE hProcess, uintptr_t moduleBaseAddress);
bool BacBcmPatch(HANDLE hProcess, uintptr_t moduleBaseAddress);
bool InfiniteTimerPatch(HANDLE hProcess, uintptr_t moduleBaseAddress);

#endif // PATCHES_H
