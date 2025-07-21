#include "patches.h"

// Helper function definition
bool ApplyPatch(HANDLE hProcess, uintptr_t moduleBaseAddress, uintptr_t offset, const void* newBytes, SIZE_T byteLength) {
    LPVOID address = (LPVOID)(moduleBaseAddress + offset);
    SIZE_T numberOfBytesWritten;
    DWORD oldProtect;
    DWORD newProtect = PAGE_EXECUTE_READWRITE;

    if (address == nullptr) {
        UPRINTF("Failed to calculate the address.\n");
        return false;
    }

    if (!VirtualProtect(address, byteLength, newProtect, &oldProtect)) {
        UPRINTF("Failed to change memory protection.\n");
        return false;
    }

    if (!WriteProcessMemory(hProcess, address, newBytes, byteLength, &numberOfBytesWritten)) {
        UPRINTF("Failed to replace the bytes.\n");
        return false;
    }

    if (!VirtualProtect(address, byteLength, oldProtect, &newProtect)) {
        UPRINTF("Failed to restore memory protection.\n");
        return false;
    }

    return true;
}

// Patch functions using the helper

bool CMSPatch(HANDLE hProcess, uintptr_t moduleBaseAddress) {
    const char* newBytes1 = "\x90\x90\xb8\x01\x90\x90\x90\x90\x90\x90\x90";
    return ApplyPatch(hProcess, moduleBaseAddress, 0x15EE3A, newBytes1, strlen(newBytes1));
}

wchar_t* RetrieveVersionString(HANDLE hProcess, uintptr_t moduleBaseAddress) {
    const wchar_t* versionString = L"ver.1.08.00";
    const size_t patchsize = (wcslen(versionString) + 1) * sizeof(wchar_t); // Include null terminator
    wchar_t buf[patchsize / sizeof(wchar_t)] = {}; // Buffer per contenere i dati letti

    LPVOID address1 = (LPVOID)(moduleBaseAddress + 0x11ACFB4);
    SIZE_T numberOfBytesRead;

    if (!ReadProcessMemory(hProcess, address1, buf, patchsize, &numberOfBytesRead)) {
        UPRINTF("Failed to read the version bytes.\n");
        return nullptr;
    }

    DPRINTF("Successfully checked Version String: %ls\n", buf);
    
    wchar_t* result = (wchar_t*)malloc(patchsize);
    if (result) {
        wcscpy(result, buf);
    }

    return result; // L'utente deve deallocare con `free()`
}

bool VersionStringPatch(HANDLE hProcess, uintptr_t moduleBaseAddress) {
    const wchar_t* newBytes1 = L"\x58\x56\x50\x61\x74\x63\x68\x65\x72\x90\x90";
    const BYTE patchsize = wcslen(newBytes1) * sizeof(wchar_t);
    return ApplyPatch(hProcess, moduleBaseAddress, 0x11ACFB4, newBytes1, patchsize);
}

bool BacBcmPatch(HANDLE hProcess, uintptr_t moduleBaseAddress) {
    const char* newBytes1 = "\x90\x90\x75\x21\xC7";
    return ApplyPatch(hProcess, moduleBaseAddress, 0x1A7F7D, newBytes1, strlen(newBytes1));
}

bool InfiniteTimerPatch(HANDLE hProcess, uintptr_t moduleBaseAddress) {
    const char* newBytes1 = "\xFF\xFF";
    return ApplyPatch(hProcess, moduleBaseAddress, 0x1193B52, newBytes1, strlen(newBytes1));
}
