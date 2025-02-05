#ifndef PATCHES_HPP
#define PATCHES_HPP

#include "debug.h"
#include "xvpatcher.h"
#include "Utils.h"



bool CMSPatch(HANDLE hProcess, uintptr_t moduleBaseAddress) {


    // The game has an additional check that XV2 used to have in the first versions
    // Basically it checks if a CMS ID is less than 100, if it is, it proceeds
    // Assigning its own id, otherwise, it flags it as CAC


   	const char* newBytes1 = "\x90\x90\xb8\x01\x90\x90\x90\x90\x90\x90\x90"; 
	LPVOID address1 = nullptr;
	LPVOID address2 = nullptr;
    SIZE_T numberOfBytesWritten;
    DWORD oldProtect;

    if (moduleBaseAddress != 0) {
        address1 = (LPVOID)(moduleBaseAddress + 0x15EE3A);
    }
	else{
		UPRINTF("ModuleBaseAddress = 0");
	}

    /////////////// PATCHES GO HERE ///////////////

    // CMS Patch 1
    if (address1 == nullptr) {
        UPRINTF("Failed to calculate the address.\n");
    }

    if (WriteProcessMemory(hProcess, address1, newBytes1, strlen(newBytes1), &numberOfBytesWritten)) {
        DPRINTF("Successfully applied CMS Patch n.1\n");
    }
    else {
        UPRINTF("Failed to replace the bytes.\n");
    }

	return 0;
}

wchar_t* RetrieveVersionString(HANDLE hProcess, uintptr_t moduleBaseAddress) {
    const wchar_t* versionString = L"ver.1.08.00";
    const size_t patchsize = (wcslen(versionString) + 1) * sizeof(wchar_t); // Include null terminator
    wchar_t buf[patchsize / sizeof(wchar_t)] = {}; // Buffer per contenere i dati letti

    LPVOID address1 = nullptr;
    SIZE_T numberOfBytesRead;
    
    if (moduleBaseAddress != 0) {
        address1 = (LPVOID)(moduleBaseAddress + 0x11ACFB4);
    }

    if (address1 == nullptr) {
        UPRINTF("Failed to calculate the address.\n");
        return nullptr;
    }

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
	const BYTE patchsize = wcslen(L"\x76\x65\x72\x2e\x31\x2e\x30\x38\x2e\x30\x30") * sizeof(wchar_t);
    const wchar_t* newBytes1 = L"\x58\x56\x50\x61\x74\x63\x68\x65\x72";

    LPVOID address1 = nullptr;
    SIZE_T numberOfBytesWritten;
    DWORD oldProtect;
    DWORD newProtect = PAGE_EXECUTE_READWRITE;

    if (moduleBaseAddress != 0) {
        address1 = (LPVOID)(moduleBaseAddress + 0x11ACFB4);
    }

    ////////////////// PATCHES GO HERE ///////////////

    if (address1 == nullptr) {
        UPRINTF("Failed to calculate the address.\n");
        return false;
    }

    if (!VirtualProtect(address1, patchsize, newProtect, &oldProtect)) {
        UPRINTF("Failed to change memory protection.\n");
        return false;
    }

    if (!WriteProcessMemory(hProcess, address1, newBytes1, patchsize, &numberOfBytesWritten)) {
        UPRINTF("Failed to replace the bytes.\n");
        return false;
    }

    if (!VirtualProtect(address1, patchsize, oldProtect, &newProtect)) {
        UPRINTF("Failed to restore memory protection.\n");
        return false;
    }

    DPRINTF("Successfully applied Version String Patch\n");
    return true;
}

bool BacBcmPatch(HANDLE hProcess, uintptr_t moduleBaseAddress) {
    const char* newBytes1 = "\x90\x90\x75\x21\xC7";

    LPVOID address1 = nullptr;
    SIZE_T numberOfBytesWritten;
    DWORD oldProtect;
    DWORD newProtect = PAGE_EXECUTE_READWRITE;

    if (moduleBaseAddress != 0) {
        address1 = (LPVOID)(moduleBaseAddress + 0x1A7F7D);
    }

    ////////////////// PATCHES GO HERE ///////////////

    if (address1 == nullptr) {
        UPRINTF("Failed to calculate the address.\n");
        return false;
    }

    if (!VirtualProtect(address1, strlen(newBytes1), newProtect, &oldProtect)) {
        UPRINTF("Failed to change memory protection.\n");
        return false;
    }

    if (!WriteProcessMemory(hProcess, address1, newBytes1, strlen(newBytes1), &numberOfBytesWritten)) {
        UPRINTF("Failed to replace the bytes.\n");
        return false;
    }

    if (!VirtualProtect(address1, strlen(newBytes1), oldProtect, &newProtect)) {
        UPRINTF("Failed to restore memory protection.\n");
        return false;
    }

    DPRINTF("Successfully applied BAC/BCM Patch\n");
    return true;
}
bool InfiniteTimerPatch(HANDLE hProcess, uintptr_t moduleBaseAddress) {
    const char* newBytes1 = "\xFF\xFF";

    LPVOID address1 = nullptr;
    SIZE_T numberOfBytesWritten;
    DWORD oldProtect;
    DWORD newProtect = PAGE_EXECUTE_READWRITE;

    if (moduleBaseAddress != 0) {
        address1 = (LPVOID)(moduleBaseAddress + 0x1193B52);
    }

    ////////////////// PATCHES GO HERE ///////////////

    if (address1 == nullptr) {
        UPRINTF("Failed to calculate the address.\n");
        return false;
    }

    if (!VirtualProtect(address1, strlen(newBytes1), newProtect, &oldProtect)) {
        UPRINTF("Failed to change memory protection.\n");
        return false;
    }

    if (!WriteProcessMemory(hProcess, address1, newBytes1, strlen(newBytes1), &numberOfBytesWritten)) {
        UPRINTF("Failed to replace the bytes.\n");
        return false;
    }

    if (!VirtualProtect(address1, strlen(newBytes1), oldProtect, &newProtect)) {
        UPRINTF("Failed to restore memory protection.\n");
        return false;
    }

    DPRINTF("Successfully applied Infinite Timer Patch\n");
    return true;
}
#endif