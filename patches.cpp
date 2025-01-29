#include "patches.h"
#include "debug.h"

bool ApplyMemoryPatch(HANDLE hProcess, uintptr_t address, const void* patchBytes, SIZE_T patchSize) {
    if (address == 0) {
        UPRINTF("Failed to calculate the address.\n");
        return false;
    }

    SIZE_T numberOfBytesWritten;
    DWORD oldProtect;
    DWORD newProtect = PAGE_EXECUTE_READWRITE;

    if (!VirtualProtect((LPVOID)address, patchSize, newProtect, &oldProtect)) {
        UPRINTF("Failed to change memory protection.\n");
        return false;
    }

    if (!WriteProcessMemory(hProcess, (LPVOID)address, patchBytes, patchSize, &numberOfBytesWritten)) {
        UPRINTF("Failed to replace the bytes.\n");
        return false;
    }

    if (!VirtualProtect((LPVOID)address, patchSize, oldProtect, &newProtect)) {
        UPRINTF("Failed to restore memory protection.\n");
        return false;
    }

    return true;
}

bool CMSPatches(HANDLE hProcess, uintptr_t moduleBaseAddress) {
    const char newBytes1[] = "\x7F\x7C\x09\xB8\x00";
    const char newBytes2[] = "\x70\x7D\x6E\xC7\x45";

    if (moduleBaseAddress == 0) {
        UPRINTF("ModuleBaseAddress = 0");
        return false;
    }

    uintptr_t address1 = moduleBaseAddress + 0x15EE39;
    uintptr_t address2 = moduleBaseAddress + 0x19363A;

    bool patch1 = ApplyMemoryPatch(hProcess, address1, newBytes1, sizeof(newBytes1) - 1);
    bool patch2 = ApplyMemoryPatch(hProcess, address2, newBytes2, sizeof(newBytes2) - 1);

    return patch1 && patch2;
}

bool VersionStringPatch(HANDLE hProcess, uintptr_t moduleBaseAddress) {
    const wchar_t newBytes1[] = L"\x58\x56\x50\x61\x74\x63\x68\x65\x72";
    SIZE_T patchSize = sizeof(newBytes1) - sizeof(wchar_t);

    if (moduleBaseAddress == 0) {
        return false;
    }

    uintptr_t address1 = moduleBaseAddress + 0x11ACFB4;
    return ApplyMemoryPatch(hProcess, address1, newBytes1, patchSize);
}

bool BacBcmPatch(HANDLE hProcess, uintptr_t moduleBaseAddress) {
    const char newBytes1[] = "\x90\x90\x75\x21\xC7";

    if (moduleBaseAddress == 0) {
        return false;
    }

    uintptr_t address1 = moduleBaseAddress + 0x1A7F7D;
    return ApplyMemoryPatch(hProcess, address1, newBytes1, sizeof(newBytes1) - 1);
}

bool InfiniteTimerPatch(HANDLE hProcess, uintptr_t moduleBaseAddress) {
    const char newBytes1[] = "\xFF\xFF";

    if (moduleBaseAddress == 0) {
        return false;
    }

    uintptr_t address1 = moduleBaseAddress + 0x1193B52;
    return ApplyMemoryPatch(hProcess, address1, newBytes1, sizeof(newBytes1) - 1);
}
