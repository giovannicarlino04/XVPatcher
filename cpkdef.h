#ifndef CPKDEF_H
#define CPKDEF_H

#include "xvpatcher.h"
#include "CpkFile.h"
#include "debug.h"
#include "patch.h"
#include "symbols.h"
#include <cstdint>
#include <cstring>
#include <cstdio>


uint8_t *read_file_from(const char *file, uint64_t offset, unsigned int *psize);
CpkFile *get_cpk_toc(const char *file, uint64_t *toc_offset, uint64_t *toc_size, uint8_t **hdr_buf, uint8_t **toc_buf);
bool get_cpk_tocs(CpkFile **data, CpkFile **data2, CpkFile **datap1, CpkFile **datap2, CpkFile **datap3);
void patch_toc(CpkFile *cpk);
bool IsThisFile(HANDLE hFile, const char *name);
uint64_t GetFilePointer(HANDLE hFile);

BOOL WINAPI ReadFile_patched(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);

uint8_t __thiscall cpk_file_exists_patched(void *object, char *file);

void patches();

#endif // CPKDEF_H
